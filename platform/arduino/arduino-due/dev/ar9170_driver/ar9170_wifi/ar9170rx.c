/*
 * Atheros CARL9170 driver
 *
 * Copyright 2008, Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2009, 2010, Christian Lamparter <chunkeey@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, see
 * http://www.gnu.org/licenses/.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *    Copyright (c) 2007-2008 Atheros Communications, Inc.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "ar9170.h"
#include "fwcmd.h"
#include "usb_cmd_wrapper.h"
#include <sys\errno.h>
#include "ar9170_debug.h"
#include "mac80211.h"
#include <stdio.h>
#include "usb_lock.h"
#include "compiler.h"
#include "nl80211.h"
#include "delay.h"
#include "ieee80211_ibss.h"
#include "ar9170_mac.h"
#include "ar9170_wlan.h"
#include "skbuff.h"
#include "ar9170_state.h"
#include <time.h>
#include "if_ether.h"
#include "ieee80211.h"
#include "cfg80211.h"
#include "bitops.h"
#include "interrupt\interrupt_sam_nvic.h"
#include "etherdevice.h"
#include "kern.h"
#include "ar9170_psm.h"
#include "platform-conf.h"
#include "ieee80211_psm.h"
#include "ieee80211_rx.h"
#include "contiki-main.h"
#include "usb_wrapper.h"
#include "linked_list.h"
#include "ar9170.h"
#include "ar9170_psm.h"
#include "smalloc.h"
#include <stdint-gcc.h>
#include "wire_digital.h"
#include "pio.h"



#define AR9170_MAGIC_RSP_HDR_LENGTH		15
#define AR9170_MAGIC_RSP_0XFF_HDR_LEN	12
#define AR9170_MAGIC_RSP_NON_FF_LEN		((AR9170_MAGIC_RSP_HDR_LENGTH) - (AR9170_MAGIC_RSP_0XFF_HDR_LEN))
#define AR9170_MIN_HDR_LEN				4

const uint8_t magic_command_header[AR9170_MAGIC_RSP_HDR_LENGTH] = {0x00, 0x00, 0x4e, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	

void ar9170_dbg_message(struct ar9170 *ar, const char *buf, U32 len)
{
	bool restart = false;
	enum ar9170_restart_reasons reason = CARL9170_RR_NO_REASON;
	int i;
	UNUSED(reason);
		
	if (len > 3) {
		if (memcmp(buf, AR9170_ERR_MAGIC, 3) == 0) {
			ar->fw.err_counter++;
			if (ar->fw.err_counter > 3) {
				restart = true;
				reason = CARL9170_RR_TOO_MANY_FIRMWARE_ERRORS;
			}
		}

		if (memcmp(buf, AR9170_BUG_MAGIC, 3) == 0) {
			ar->fw.bug_counter++;
			restart = true;
			reason = CARL9170_RR_FATAL_FIRMWARE_ERROR;
		}
	}
	
	printf("DEBUG: Received debug message: ");
	for (i=0; i<len; i++)
		printf("%02x ",buf[i]);
	printf(" \n");
	//wiphy_info(ar->hw->wiphy, "FW: %.*s\n", len, buf);

	if (restart) {
		printf("INFO: Device shall restart because of either BUG or ERR notification from the firmware.\n");
		// TODO carl9170_restart(ar, reason);
	}

}

/*
 * This function is called within interrupt context.
 */
void ar9170_rx_untie_cmds( struct ar9170* ar, const uint8_t* buffer, const uint32_t len ) 
{
	#if AR9170_RX_DEBUG_DEEP
	printf("DEBUG: untie commands. len: %u.\n", (unsigned int)len);
	#endif
	
	struct ar9170_rsp *cmd;
	int j, i = 0;
	
	while (i < len ) {
		/* This is a workaround for checking the format of the 
		 * device command responses. We signal an "error", in
		 * case the response contains more than one commands,
		 * which is actually not an error, strictly speaking. 
		 */
		if (unlikely(i>0)) {
			#if AR9170_RX_DEBUG
			printf("WARNING: Got response with more than one commands!\n");
			#endif
		}
		/* Assign next command to the command response pointer */
		cmd = (void*) &buffer[i];
		/* Update next pointer position based on the length of the current command */
		i += cmd->hdr.len + AR9170_CMD_HDR_LEN;
		if (unlikely(i > len)) {
			printf("ERROR: Got response with problems.\n");
			for (j=0; j<len; j++)
				printf("%02x ", buffer[j]);
			printf(" \n");
			break;
		}
		ar9170_handle_command_response(ar, cmd, cmd->hdr.len + AR9170_CMD_HDR_LEN);		 
	}
	
	if(unlikely(i != len)) {
		printf("ERROR: Got response with problems. Not equal in the end.\n");
		
	}
	
}



/*
 * This function is called within interrupt context.
 */
void __ar9170_rx( struct ar9170* ar, uint8_t* buffer, uint32_t len )
{		
	/*
	 * We would have liked this function to be as simple as the one
	 * implemented on the Linux driver. However, for some reasons the
	 * device is grouping the responses and we must separate them, by
	 * writing a response wrapper here.

	 * Luckily the device is not mixing the responses; it just merges them
	 * one after the other, which makes our life easier. Or, at least, this
	 * is what we think.

	 * This parser only unties the command response from the bulky data, and
	 * leaves the untie of the bulky data to possibly more than one MPDUs for
	 * a later stage.

	 * So, finally, the way we solve our problem is to go through the response
	 * looking for the pattern: LL 00 00 4e ffff ffff ffff ffff ffff ffff, as
	 * this indicates the beginning of the command response. The response 
	 * follows later.
	 */
		
	/* Initially the remaining length is the received length minus the very first byte,
	 * which is the pipe received length and we do not need it.
	 */
	int32_t _remaining_len = len - 4;
	/* Command length is initially zero */
	
	/* Skip the first byte that contains the [virtual] response length */
	uint8_t* _command_buffer = buffer + 4;	
	
	/* Initialize the match flag to -1. */
	unsigned int match = -1;	
	
	/* The command has a minimum size of 15 [prefix] and 4 [CMD header - no payload] */
	while(_remaining_len >= AR9170_MIN_HDR_LEN) {		
	
		/* Check for the 0xff pattern. */
		if (*(_command_buffer) != 0xff) {
			
			/* If it was not found, do not bother to check until after 12 bytes. */
			_remaining_len -= AR9170_MAGIC_RSP_0XFF_HDR_LEN;
			_command_buffer += AR9170_MAGIC_RSP_0XFF_HDR_LEN;
			continue;			
		
		} else {
			
			/* We have found a 0xff which is a candidate to be inside the pattern. So,
			 * we must check in the range of this buffer position for pattern matching
			 */
			uint8_t index_counter = 0;
			bool pattern_found = false;
			
			while (index_counter < AR9170_MAGIC_RSP_0XFF_HDR_LEN) {
				
				/* Compare with the sequence before and after. */
				match = memcmp(&magic_command_header, _command_buffer-AR9170_MAGIC_RSP_NON_FF_LEN-index_counter, AR9170_MAGIC_RSP_HDR_LENGTH);
				
				if (match != 0) {
					/* The pattern has not been found. Increment reverse index counter. */
					index_counter++;
					
				} else {
					/* The pattern was found! */
					pattern_found = true;
					break;
				}				
			}
			if (pattern_found == false) {
				
				/* If it was not found, do not bother to check until after 12 bytes. */
				_remaining_len -= AR9170_MAGIC_RSP_0XFF_HDR_LEN;
				_command_buffer += AR9170_MAGIC_RSP_0XFF_HDR_LEN;
				continue;				
			
			} else {

				/* It seems that we have found the command starting pattern, so
				 * whatever is BEFORE this point, is a data packet and shall be 
				 * scheduled for MPDU processing, AFTER the command is handled.
				 */
				_command_buffer = _command_buffer-AR9170_MAGIC_RSP_NON_FF_LEN-index_counter;
				_remaining_len = _remaining_len + AR9170_MAGIC_RSP_NON_FF_LEN + index_counter;
				
				/*
				 * Whatever FOLLOWS the pattern is the actual response that starts
				 * with the response length _excluding_ the header! However, there
				 * may be more data after the command(s) response which shall not
				 * be send to the command response routine. For this reason, we
				 * in fact, do some hacking here: We check already the length of 
				 * the command, considering that the device does NOT buffer 
				 * command responses. [WE HAVE DONE SOME CHECKING ABOUT THIS :)]
				 */			
				struct ar9170_rsp *cmd = (struct ar9170_rsp*)(_command_buffer+AR9170_MAGIC_RSP_HDR_LENGTH); 
				uint32_t cmd_len = cmd->hdr.len + AR9170_CMD_HDR_LEN;
			
				if (cmd_len + AR9170_MAGIC_RSP_HDR_LENGTH > _remaining_len) {
					printf("ERROR: Command declares a size that brings us out of bounds.\n");
				}
			
				/* Handle the command inside the interrupt context. This needs to be fast. */
				ar9170_rx_untie_cmds(ar, _command_buffer+AR9170_MAGIC_RSP_HDR_LENGTH, cmd_len);
			
						
				if (_command_buffer != (buffer + 1)) {
					/* There is a bulk response before the command response, so we
					 * schedule the packet processing, so it is executed after we 
					 * exit the interrupt context.
					 */
					if (len - _remaining_len - 1 <= 4) {
						printf("ERROR: Received insufficient bulky data before command response: %u .\n",len-_remaining_len-1);
				
					} else {
						#if AR9170_RX_DEBUG_DEEP
						printf("DEBUG: Response is mixed.\n");
						#endif
						ar9170_handle_mpdu(ar, buffer+4, len-_remaining_len - 4 - 1);
					} 
				}
			
				if (not_expected(cmd_len != (_remaining_len-AR9170_MAGIC_RSP_HDR_LENGTH))) {
					/* The remaining length of the buffer exceeds the length of the 
					 * command, considering, of course, that there is only one. So
					 * either the device has actually grouped multiple commands in
					 * a single USB response, or there is a packet coming after the
					 * command. The second is not a problem, while the first is, as
					 * we do not handle it. We will consider here that grouping the
					 * commands never occurs, and handle the remaining bytes as an 
					 * MPDU.
					 */
					if (cmd_len < (_remaining_len-AR9170_MAGIC_RSP_HDR_LENGTH)) {
						#if AR9170_RX_DEBUG_DEEP
						printf("WARNING: There is something after the command response!"
							" Trash, packet or grouped responses? %d, %d.\n",
							cmd_len, _remaining_len-AR9170_MAGIC_RSP_HDR_LENGTH);
							int k;
							uint8_t* _extra_bytes = _command_buffer+AR9170_MAGIC_RSP_HDR_LENGTH+cmd_len;
							for (k=0; k<(_remaining_len-AR9170_MAGIC_RSP_HDR_LENGTH-cmd_len); k++)
								printf("%02x ",_extra_bytes[k]);
							printf(" \n");
						#endif
						/* What if this is an entirely new command response? We need 
						 * to check against the pattern; if not, this is handled as a
						 * packet, otherwise we execute this command within interrupt
						 * context. We do not expect to have a third command response
						 * in the same USB response, so we just do a simple check. We
						 * should normally handle the issue in an exhaustive while loop
						 * until we run out of remaining bytes. TODO
						 */
						uint8_t* new_command = _command_buffer + AR9170_MAGIC_RSP_HDR_LENGTH + cmd_len + 1;
						if (memcmp(&magic_command_header, new_command,
							AR9170_MAGIC_RSP_HDR_LENGTH) == 0) {
								/* This is a second command response attached to the same USB response. */
								struct ar9170_rsp* new_cmd = (struct ar9170_rsp*)(new_command+AR9170_MAGIC_RSP_HDR_LENGTH);
							
								uint32_t new_cmd_len = new_cmd->hdr.len + AR9170_CMD_HDR_LEN;

								/* Check if we go out of bounds. */
								if ((_remaining_len - new_cmd_len - cmd_len - 1 - AR9170_MAGIC_RSP_HDR_LENGTH) <= 4 ) {
									printf("ERROR: Attached command has brought us out of bounds.\n");
								}

								#if AR9170_RX_DEBUG_DEEP
								printf("ATTACHED CMD\n");
								#endif

								/* Handle the command inside the interrupt context. This needs to be fast. */
								ar9170_rx_untie_cmds(ar,(uint8_t*)new_cmd, new_cmd_len);
					
						} else {
							/* This is a packet. */
							#if AR9170_RX_DEBUG_DEEP
							printf("ATTACHED PKT\n");
							#endif
							ar9170_handle_mpdu(ar, _command_buffer+AR9170_MAGIC_RSP_HDR_LENGTH+cmd_len,
								(_remaining_len-AR9170_MAGIC_RSP_HDR_LENGTH-cmd_len));
						}
					
				
					} else {
						/* The command length is larger than the actual received buffer length. 
						 * This is a serious error and will be detected later by the handling 
						 * of the command response.
						 */
					}				
				}
				/* After the command pattern is found, we do not need to search for more. */			
				break;
			}
			
		}
	
	}
	/* We have now exited the while loop. If the pattern was not found, the received 
	 * USB response is data and we handle the whole response as an MPDU. In the very
	 * unlikely scenario that a single USB response corresponds to multiple packets,
	 * we have a problem, because such case is not handled.
	 */
	if (match != 0) {
		#if AR9170_RX_DEBUG_DEEP
		printf("DEBUG: Response is handled as a single packet.\n");
		#endif
		/* Skip the first four bytes [USB delimiter and extra bytes]. */
		ar9170_handle_mpdu(ar, buffer+4, len-4);
	}
		
	#if AR9170_RX_DEBUG_DEEP
	printf("DEBUG: End of __rx().\n");
	#endif
}	



/*
 * This function is called within interrupt context.
 */
int ar9170_check_sequence( struct ar9170 * ar, uint32_t seq ) 
{
	if (ar->cmd_seq < -1)
		return false;
	
	/*
	 * Initialize Counter
	 */
	if (ar->cmd_seq < 0)
		ar->cmd_seq = seq;
	
	/*
	 * The sequence is strictly monotonic increasing and it never skips!
	 *
	 * Therefore we can safely assume that whenever we received an
	 * unexpected sequence we have lost some valuable data.
	 */
	if (seq != ar->cmd_seq) {
		printf("ERROR: Lost command responses/traps! "
			"Received: %d while local sequence counter is: %d.\n", (int)seq, ar->cmd_seq);
		
		//carl9170_restart(ar, CARL9170_RR_LOST_RSP); TODO
		return -EIO;
	}
	/* Everything is fine. Update sequence counter. */
	ar->cmd_seq = (ar->cmd_seq + 1) % ar->fw.cmd_bufs;
		
	return false;
}



//************************************
// Method:		ar9170_handle_command_response
// FullName:	ar9170_handle_command_response
// Access:		public 
// Returns:		void
// Qualifier:	The method handles the command responses from the ar9170 device. 
//				Notice that the process runs in the context of a device interrupt
//				and as such, this method should not trigger synchronous commands,
//				as the interrupts in this project are not designed as nested, i.e.
//				interrupt code can not be interrupted by another interrupt of the
//				same priority level. In addition, special care is put on reducing
//				the CPU overhead of the interrupt code in the function. 
// Parameter:	struct ar9170 * ar	The ar9170 structure object
// Parameter:	void * buf	The buffer starting point of the response
// Parameter:	uint32_t len	The length of the received response in bytes
//************************************
void ar9170_handle_command_response(struct ar9170 *ar, void* buf, uint32_t len) {
	
	/* Record the current time, as it may be needed later, e.g. 
	 * for ATIM window interrupt registration.
	 */	
	rtimer_clock_t current_time = RTIMER_NOW();
			
	struct ieee80211_vif *vif;
	struct ar9170_rsp *cmd = buf;
	
	if (ar9170_check_sequence(ar, cmd->hdr.seq)) {
		printf("ERROR: Command Sequence numbers broken!\n");
		/* Restart? */
		return;
	}
		
	if ((cmd->hdr.cmd & CARL9170_RSP_FLAG) != CARL9170_RSP_FLAG) {
		if (!(cmd->hdr.cmd & CARL9170_CMD_ASYNC_FLAG)) {
			#if AR9170_RX_DEBUG_DEEP
			printf("DEBUG: Handling synchronous response.\n");
			#endif
			ar9170_cmd_callback(ar, len, buf);		
		
		} else {
			/* Responses to PSM commands are not carrying the 
			 * synchronous flag and are not handled as events
			 * so we need to handle them here, unlike in the 
			 * original CARL9170 driver.
			 */
			if (cmd->hdr.cmd == CARL9170_CMD_PSM_ASYNC) {
				/* We check the status of the RF here 
				 * and update accordingly the device 
				 * state.
				 */				
				ar9170_handle_ps(ar, cmd);				
			}
			
		}	
		#if AR9170_RX_DEBUG_DEEP
		if (cmd->hdr.cmd == (CARL9170_CMD_ASYNC_FLAG | CARL9170_CMD_BCN_CTRL)) {
			printf("DEBUG: Handling BCN CTRL response.\n");
			int i;
			uint8_t* test = buf;
			for(i=0;i<len; i++) {
				printf("%02x ", test[i]);
			}
			printf(" \n");
		
		} else if (cmd->hdr.cmd == CARL9170_CMD_WREG_ASYNC) {
			printf("DEBUG: Handling asynchronous write response.\n");
		}		
		#endif		
		return;
	}
			
	if (unlikely(cmd->hdr.len != len - AR9170_CMD_HDR_LEN)) {		
		printf("ERROR: Received over / under - sized command response.\n");
		return;
	}
	
	/* hardware event handlers */
	switch (cmd->hdr.cmd) {
	case AR9170_RSP_PRETBTT:
		/* This is a pre-TBTT interrupt. */
		#if AR9170_RX_DEBUG_DEEP
		printf("[%llu] TBTT [%d]\n",current_time, cmd->hdr.seq);
		#endif
		#ifdef WITH_LED_DEBUGGING
		digital_write(TX_ACTIVE_PIN, LOW);
		if(digital_read(PRE_TBTT_ACTIVE_PIN,PIO_TYPE_PIO_OUTPUT_0) == HIGH) {
			digital_write(PRE_TBTT_ACTIVE_PIN, LOW);
		} else {
			digital_write(PRE_TBTT_ACTIVE_PIN, HIGH);
		}
		#endif
		/* Increment the local TBTT counter. */		
		ibss_info->tbtt_counter += 1;
		
		/* Extract the state of the AR9170 RF, encapsulated in the TBTT response. */
		uint32_t received_psm_state = *((uint32_t*)(cmd->data));
		
		/* Always wake up device at TBTT [if not already] */
		if (likely(received_psm_state == 0)) {
			/* Everything is fine. the AR9170 PSM is OFF [false]. */
			ar->ps.state = false;
			#ifdef WITH_LED_DEBUGGING
			digital_write(DOZE_ACTIVE_PIN, LOW);
			#endif
		
		} else {
			/* The AR9170 woke up at TBTT with a powered-down RF. */
			if (likely(ar->ps.state == true)) {
				/* The RF is considered as powered-down. Wake-up the
				 * device, but issue a warning, as this should never
				 * happen.
				 */
				printf("WARNING: Device still in power-save after TBTT event!\n");
				//ar9170_psm_schedule_powersave(ar, false);
			
			} else {
				/* This is a serious error. The device is supposed to be 
				 * powered-up, but it returns a TBTT response with the 
				 * RF being powered-down. Who powered-it down?
				 */
				printf("ERROR: AR9170 PSM state is OFF, while TBTT returned a device in PSM state.\n");
			}
		}
				
		/* Assign pointer of the main virtual interface. */			
		vif = ar9170_get_main_vif(ar);

		if (not_expected(!vif)) {
			/* We can not continue after this, as we do not know the operation mode of the STA. */
			printf("ERROR: Virtual interface is NULL!.\n");
			break;
		}
				
		/* We currently handle the TBTT interrupt only for the ad-hoc mode. */
		switch (vif->type) {
		case NL80211_IFTYPE_STATION:
			ar9170_handle_ps(ar, cmd);
			break;
			
		case NL80211_IFTYPE_AP:
			ar9170_update_beacon(ar, true);
			break;
			
		case NL80211_IFTYPE_ADHOC:	
			/*
			 * If PSM is enabled, the device should be in the transmission window,
			 * at the beginning of the TBTT; if PSM is disabled, the device is 
			 * always in the transmission window. So, we signal an error, in case 
			 * the device is not in the transmission window. [unlikely] 
			 */
			if (not_expected((ar->ps_mgr.psm_state != AR9170_TX_WINDOW) &&
							 (ar->ps_mgr.psm_state != AR9170_SOFT_BCN_WINDOW))) {
			
				printf("ERROR: When pre-TBTT event arrives the device must be "
					"in the TX/SOFT-BCN period, not in [%d] state.\n", ar->ps_mgr.psm_state);			
			}
			
			if (not_expected(ibss_info->tbtt_counter == 1)) {
				
				/* (Re)-set beaconing. */
				ar9170_schedule_bcn_ctrl(ar);
			}
			/* If in PSM and not IBSS-DTIM period, schedule ATIM Window 
			 * interruption, erase the list of awake neighbors, and for
			 * each pending packet, create ATIM frame, to be transmitted
			 * within the ATIM Window. These operations, however, may 
			 * require a lot of time, and for that they should not be 
			 * handled within this interrupt context. Alternatively, we
			 * set a flag, to signal the AR9170 scheduler to perform the
			 * actions once the program exits from the interrupt handler.
			 */
			if (hw->conf.flags & IEEE80211_CONF_PS) {
												
				/* OFF_override_ps should start as a free flag. */
				ar->ps.off_override = 0; 
				
				/* At the beginning of TBTT the device RF is ON and we 
				 * schedule an ATIM interrupt to mark the beginning of
				 * the ATIM Window. We use the current time, so we are 
				 * not expecting to have time drifts in this function.
				 */										
				ar9170_psm_schedule_atim_window_start_interrupt(current_time);
				
				/* Update device PSM State to pre-TBTT Window. This 
				 * is fast, and, also, time-critical, so we change 
				 * the state within this interrupt context. 
				 */					
				ar->ps_mgr.psm_state = AR9170_PRE_TBTT;
														
				/* SIGNAL an event to the scheduler, so it knows that 
				 * it is time it created the required ATIM frames. The
				 * operation is moved out of the interrupt context, as 
				 * it might be time-consuming. 
				 */
				ar->ps_mgr.create_atims_flag = true;
								
				/* Erase the vector of awake neighbors. This will be 
				 * populated again during the ATIM Window. This 
				 * operation is perhaps time-critical, but we keep it
				 * outside the interrupt context, although we can also
				 * support moving it inside the critical section.
				 */
			
				if (ar->erase_awake_nodes_flag == true) {
					printf("ERROR: the flag to erase the awake nodes is set.\n");
				} else {
					ar->erase_awake_nodes_flag = true;
				}
			//	ieee80211_psm_erase_awake_neighbors(ar);

			
			} else {
				/* The device is not in PS Mode, so the starting of the TBTT 
				 * period does not change anything in the device operation.
				 */
			} 	
			
		/*	
			// Update beacon but do not submit it for transmission 
			if ((ibss_info->beacon_counter % 7) == 0 ) {
				ar9170_update_beacon(ar,false);
			}			
		*/			
			/* This is a workaround for status response handling. 
			 * It is possible that the TBTT interrupt arrives at
			 * a point, right before the status response for some
			 * pending packet, and overrides it. Due to this we
			 * might never get a response back, so we might never
			 * be able to send new packets. So here we clear the
			 * flags and hope this will not bring instability to
			 * the system.*/
			if (ar->tx_atim_wait == true) {
				
				/* We always clear this flag. The TBTT comes much later
				 * than the end of ATIM Window, so we can assume safely
				 * that the response was lost. If not, we will just get
				 * a warning by the status response trying to clear the
				 * flag.
				 */
				printf("CA\n");
				ar->atim_wait_cleared_count++;
				__complete(&ar->tx_atim_wait);
			}
			if (ar->tx_data_wait == true) {
				/* A status response has been delayed for more than
				 * a beacon cycle so we clear the flag immediately.
				 */
				if (ar->clear_tx_data_wait_at_next_tbtt == true) {
					printf("CD\n");
					__complete(&ar->tx_data_wait);
					__complete(&ar->clear_tx_data_wait_at_next_tbtt);
				
				} else {
					/* Instead, if the waiting flag for data packet is set 
					 * we schedule clearing it in the following pre-TBTT as
					 * it is possible that the packet was sent late, so the
					 * status response was suppressed by this interrupt. So
					 * in the worst case, we will not be able to send more
					 * packets for one beacon interval. This is kind of OK.
					 */
					#if AR9170_RX_DEBUG_DEEP
					printf("SD\n");
					#endif
					__start(&ar->clear_tx_data_wait_at_next_tbtt);
				}
			}	
			/* A similar thing for the Command Out callback. These 
			 * commands are asynchronous and we do not care so much
			 *  if they are delayed, so we clear the flag after one
			 * beacon interval. If we are here this is a warning, as
			 * we do not really expect command transmissions before
			 * pre-TBTT.
			 */
			if(ar->cmd_async_lock == true) {
				if (ar->clear_cmd_async_lock_at_next_tbtt == true) {
					printf("CC\n");
					__complete(&ar->cmd_async_lock);
					__complete(&ar->clear_cmd_async_lock_at_next_tbtt);
					
				} else {
					printf("SC\n");
					__start(&ar->clear_cmd_async_lock_at_next_tbtt);
				}
			}
			/* This is the USB Bulk OUT callback -related flag, and 
			 * we schedule to clear it in the following BCN interval
			 * similarly to the tx_wait flag. We do not really expect
			 * to lose many of these responses.
			 */
			if(ar->tx_async_lock == true) {
				if (ar->clear_tx_async_lock_at_next_tbtt == true) {
					printf("CT\n");
					__complete(&ar->tx_async_lock);
					__complete(&ar->clear_tx_async_lock_at_next_tbtt);
					
				} else {
					printf("ST\n");
					__start(&ar->clear_tx_async_lock_at_next_tbtt);
				}
			}
			#if AR9170_RX_DEBUG_DEEP
			printf("%u %u\n",linked_list_get_len(ar->tx_pending_atims), linked_list_get_len(ar->tx_pending_pkts));
			#endif
			break;			
		default:
			break;
		}		
		break;

	case AR9170_RSP_TXCOMP:
		/* TX status notification */
		#if AR9170_RX_DEBUG_DEEP
		printf("STATUS [%d]\n",cmd->hdr.seq);
		#endif
		ar9170_tx_process_status(ar, cmd);
		/* Important. We might have won the race 
		 * against the USB Bulk out callback, or
		 * the Bulk out Done interrupt was unfor-
		 * tunately suppressed. However, since 
		 * we always wait for status responses, 
		 * before sending a new packet, this 
		 * response safely indicates that the
		 * corresponding USB Bulk Out transfer
		 * was completed, so we clear the flag
		 * here.
		 */
		if(ar->tx_async_lock == true) {
			__complete(&ar->tx_async_lock);
			/* There exists a very rare scenario, 
			 * where the asynchronous lock is set
			 * due to a command transfer, not due
			 * to a packet, which we do NOT handle
			 * here properly.
			 */
			if (ar->clear_tx_async_lock_at_next_tbtt == true) {
				__complete(&ar->clear_tx_async_lock_at_next_tbtt);
			}
			if (ar->cmd_async_lock == true) {
				__complete(&ar->cmd_async_lock);
				__complete(&ar->clear_cmd_async_lock_at_next_tbtt);
				printf("CB\n");
			}
		}
		break;

	case AR9170_RSP_BEACON_CONFIG:
		/*
		 * (IBSS) beacon send notification
		 * bytes: 04 c2 XX YY B4 B3 B2 B1
		 *
		 * XX always 80
		 * YY always 00
		 * B1-B4 "should" be the number of send out beacons.
		 */
		#if AR9170_RX_DEBUG_DEEP
		printf("[%llu] BCN Send [SN:%02x %02x %02x].\n",current_time, cmd->hdr.seq, 
			((U8*)buf)[4], ((U8*)buf)[5]);
		#endif
		/* We skip power-saving in current beacon interval, 
		 * as a result of beacon reception. Also, we transit
		 * to the ATIM Window, if not already there.
		 */
		if (hw->conf.flags & IEEE80211_CONF_PS) {
			/* Device will stay awake in this BCN interval. */
			ar->ps.off_override |= PS_OFF_BCN;
		}			
			/*
			 * Workaround. We could use the BCN sent notification, to 
			 * transit to the ATIM Window. However, this would result
			 * in randomizing this transition, so early stations might
			 * sent ATIM frames too early for late nodes.
			 */ 
		/*	
			switch(ar->ps_mgr.psm_state) {
				
			case AR9170_ATIM_WINDOW:
				break;
				
			case AR9170_PRE_TBTT:
				ar->ps_mgr.psm_state = AR9170_ATIM_WINDOW;
				break;
				
			case AR9170_TX_WINDOW:
			case AR9170_SOFT_BCN_WINDOW:
				
				#if AR9170_RX_DEBUG_DEEP
				printf("LATE BCN.\n");
				#endif
									
				break;
			default:
				printf("ERROR: Unrecognized device state.\n");
		}					
		*/
		
		/* There is a strange scenario where the BCN sent
		 * response arrives after we have put the device 
		 * to sleep; in such a case we wake up the device
		 * immediately.
		 */
		if (not_expected(ar->ps.state == true)) {
			
			#if AR9170_RX_DEBUG_DEEP
			printf("WARNING: BCN Config response arrived while on power-save.\n");
			#endif
			/* Send a command to wake-up. TODO */
			//ar9170_psm_schedule_powersave(ar, false);
		}
		
		/* Increment the number of cycles that a beacon was transmitted. */
		//((stats_get_stat_eval())->local_psm_statistics.number_of_beacon_transmissions)++;		
		
		if (not_expected( (((U8*)buf)[4] == 1) && (((U8*)buf)[5] == 0))) {
		//if (not_expected((stats_get_stat_eval())->local_psm_statistics.number_of_beacon_transmissions == 1)) {
				
			/* We have just transmitted the first beacon, so the [default] IBSS has been 
			 *  successfully joined or created. Setting the following flag will signal an 
			 * event to the scheduler, which can now safely disable the packet filtering.
			 */	
			if (ar->clear_filtering == false) {
				ar->clear_filtering = true;
				/* Notification that the IBSS is now fully created / joined, 
				 * since beaconing has started.
				 */
				 ieee80211_set_ibss_started_beaconing();
			}				
			else {
				printf("ERROR: The clear_filtering flag is set! Overflow?\n");
			}
			
		}	
		break;
		
	case AR9170_RSP_ATIM:
		/* End of Atim Window */
		#if AR9170_RX_DEBUG
		printf("INFO: ATIM WINDOW END!\n");
		#endif
		break;
		
	case CARL9170_RSP_WATCHDOG:
		/* Watchdog Interrupt */
		printf("ERROR: Watch-dog Interrupt received. Must restart!\n");
		// FIXME carl9170_restart(ar, CARL9170_RR_WATCHDOG);
		break;

	case CARL9170_RSP_TEXT:
		/* firmware debug */
		ar9170_dbg_message(ar, (char *)buf + 4, len - 4);
		break;

	case CARL9170_RSP_HEXDUMP:
	/*	FIXME - I have never seem this response, and it is perhaps non important
		wiphy_dbg(ar->hw->wiphy, "FW: HD %d\n", len - 4);
		print_hex_dump_bytes("FW:", DUMP_PREFIX_NONE,
				     (char *)buf + 4, len - 4);
	*/	
		break;

	case CARL9170_RSP_RADAR:
	/*	FIXME - I have never seem this response, and it is perhaps non important
		if (!net_ratelimit())
			break;

		wiphy_info(ar->hw->wiphy, "FW: RADAR! Please report this "
		       "incident to linux-wireless@vger.kernel.org !\n");
	*/	
		break;

	case CARL9170_RSP_GPIO:
#ifdef CONFIG_CARL9170_WPC
		if (ar->wps.pbc) {
			bool state = !!(cmd->gpio.gpio & cpu_to_le32(
				AR9170_GPIO_PORT_WPS_BUTTON_PRESSED));

			if (state != ar->wps.pbc_state) {
				ar->wps.pbc_state = state;
				input_report_key(ar->wps.pbc, KEY_WPS_BUTTON,
						 state);
				input_sync(ar->wps.pbc);
			}
		}
#endif /* CONFIG_CARL9170_WPC */
		break;

	case CARL9170_RSP_BOOT:
		printf("INFO: AR9170 Firmware booted!\n");
		__complete(&ar->fw_boot_wait);
		break;

	default:	
		printf("WARNING: Cannot handle response.\n");
		break;
	}			
}


void ar9170_handle_ps(struct ar9170 *ar, struct ar9170_rsp *rsp)
{	
	#if AR9170_RX_DEBUG_DEEP
	printf("DEBUG: PS handling.\n");
	#endif
	
	/* The response of this kind ALWAYS contains a zero, i.e.
	 * no matter if the device has been put to power-save or
	 * not. So we can only use this response to confirm that
	 * the actual transition has been completed, but we can 
	 * not use it to confirm the state.
	 */
	//uint8_t returned_state = le32_to_cpu(rsp->psm.state);
	
	if (not_expected((ar->ps_mgr.psm_transit_to_wake & ar->ps_mgr.psm_transit_to_sleep) == true)) {
		printf("ERROR: Both PSM transit flags are set!");
		return;
	}
	
	if (ar->ps_mgr.psm_transit_to_sleep == true) {
		/* Everything is fine. Clear the transition flag. */
		ar->ps_mgr.psm_transit_to_sleep = false;
		/* The ps state is already set. Sanity check. */
		if (ar->ps.state == false) {
			printf("WARNING: Device RF was not set to OFF.\n");
			ar->ps.state = true;
		}
		
	} else if (ar->ps_mgr.psm_transit_to_wake == true) {
		
		/* Everything is fine. Clear transition flag. */
		ar->ps_mgr.psm_transit_to_wake = false;
		/* Set the device RF to awake. */
		if (ar->ps.state == true) {
			
			ar->ps.state = false;
		
		} else {
			printf("ERROR: Device RF is already ON.\n");
		}
	
	} else {
				
		/* We were not waiting for psm transition. This is an error. */
		printf("ERROR: We were not waiting for PSM transition.\n");
	}	
			
	#if AR9170_RX_DEBUG_DEEP
	printf("Power-saving state updated to %d.\n",ar->ps.state);
	#endif
/*	
	U32 ps;
	bool new_ps;

	ps = le32_to_cpu(rsp->psm.state);

	new_ps = (ps & AR9170_PSM_COUNTER) != AR9170_PSM_WAKE;
	if (ar->ps.state != new_ps) {
		if (!new_ps) {
			//ar->ps.sleep_ms = jiffies_to_msecs(jiffies -
			//ar->ps.last_action);
		}

		//ar->ps.last_action = jiffies;

		ar->ps.state = new_ps;
	}
*/
}

/* 
 * This function is called inside interrupt context.
 */
void ar9170_schedule_handle_mpdu( struct ar9170* ar, U8 *buf, int len )
{
	#if AR9170_RX_DEBUG_DEEP
	printf("DEBUG: Schedule MPDU Handling. Store Packet.\n");
	#endif
	
	if (len <= 0) {
		printf("WARNING: Wrong packet size.\n");
		return;
	}

	
	/* Check for overhearing. If the packet is not for us, we should not 
	 * spend more time with it. 
	 */
	
	
	#if AR9170_RX_DEBUG_DEEP
	printf("DEBUG: Schedule MPDU Handling. Store Packet.\n");
	printf("REC\n");
	#endif

		

	/* 
	 * We need to store the packet in the list of pending packets.
	 * if this list is full, we need to drop the packet, so it is
	 * good if we do the check right in the beginning, so we don't
	 * lose unnecessary time.
	 */ 
	if (linked_list_get_len(ar->rx_pending_pkts) >= AR9170_MAX_PENDING_RX_PKT_QUEUE_LEN) {
		#if AR9170_RX_DEBUG_DEEP
		printf("WARNING: The packet list is full. Received packet is dropped.\n");
		#endif
		return;
	}
	
	/* Schedule packet processing to be executed at the earliest
	 * possible occasion after the interrupt routine is completed;
	 * this is done by simply adding the pending packet in the rx
	 * pending packets' list. Before that we need to allocate all 
	 * the necessary resources for this packet.
	 */
	struct sk_buff* skb = (struct sk_buff*)malloc(sizeof(struct sk_buff));
	
	if (skb != NULL) {
		/* Store packet length. */
		skb->len = len;
		/* Round up packet length to number of words. */
		uint32_t len_in_words = DIV_ROUND_UP(len, 4);
		/* Allocate enough memory for being able to copy chucks or 4 bytes. */
		skb->data = (uint8_t*)malloc(len_in_words*4);
		if (skb->data != NULL) {
			/* Copy the packet content. We might copy some residual data here [safe]. */
			ar9170_usb_memcpy((uint32_t*)(skb->data), (uint32_t*)buf, len_in_words);
			/* Add the packet in the list of pending received packets. */
			if(!ar9170_op_add_pending_pkt(ar, &ar->rx_pending_pkts, skb, false)) {
				
				printf("ERROR: received packet could not be added in the pending RX packets queue.\n");
			}
		} else {
			
			printf("ERROR: Could not allocate memory for packet contents.\n");
		}	
	
	} else {
		printf("ERROR: Could not allocate memory for socket buffer.\n");
	}
	
	#if AR9170_RX_DEBUG_DEEP
	printf("DEBUG: AR9170 MAIN; Scheduled received packet processing.\n");
	#endif	
}


/*
 * If the frame alignment is right (or the kernel has
 * CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS), and there
 * is only a single MPDU in the USB frame, then we could
 * submit to mac80211 the SKB directly. However, since
 * there may be multiple packets in one SKB in stream
 * mode, and we need to observe the proper ordering,
 * this is non-trivial.
 */

/* This function is now called inside interrupt context. */
void ar9170_handle_mpdu(struct ar9170 *ar, U8 *buf, int len)
{	
	/* Immediately drop packets with size larger than some maximum. */
	if (len > AR9170_RX_MAX_PACKET_LENGTH) {
		#if AR9170_RX_DEBUG_DEEP
		printf("WARNING: RX Packet Large [%u].\n",len);
		#endif
		return;
	}
	struct ar9170_rx_head *head;
	struct ar9170_rx_macstatus *mac;
	struct ar9170_rx_phystatus *phy = NULL;
	struct ieee80211_rx_status status;
	struct sk_buff *skb = NULL;
	int mpdu_len;
	U8 mac_status;
		
	if (!IS_STARTED(ar)) {
		printf("WARNING: Can not accept MPDUs while not started.\n");
		return;
	}

	if (not_expected(len < sizeof(*mac))) {
		printf("WARNING: Received MPDU too short: %d.\n",len);
		goto drop;
	}
	
	/* Before processing the received data, we need to check 
	 * whether the response from the AR9170 device contains
	 * 2 added padding bytes. If it does, we need to remove
	 * them, as they may be a source of an error. Usually, 
	 * these bytes are either 00 00 or ff ff; in the later, 
	 * they cause the MPDU to be considered as MIDDLE, and
	 * this cause an error - so the packet is dropped. 
	 *
	 * If the padding is 00 00, the packet is surprisingly
	 * processed correctly. This probably occurs because of
	 * the upper layer header that contains the length, so
	 * Contiki does not process more bytes that it needs.
	 * 
	 * Finally, we must check whether there exist even more
	 * padded bytes. For that we rely on the 3f 3f pattern,
	 * which we believe that exists always and is used as
	 * our reference. Actually, it indicates the decryption
	 * field; so if we do not use encryption, this field is
	 * safe to use.	 Additionally, we could check the last
	 * 13 bytes of the PHY control header; should be all 0.
	 */
	
	/* Start from the back and try to locate the 3f 3f pattern.
	 * Once done check whether there are 2 or 4 bytes after it;
	 * remove the last 2 in the later case.
	 */
	
	unsigned int i, match, counter = len - sizeof(struct ar9170_rx_head), forward_counter = 0;
	/* The constant patterns. */
	const uint8_t _mac_status_pattern[2] = {0x3f, 0x3f};
	const uint8_t _mac_phy_status_pattern[13] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};				
	/* Flag indicating the search result. */
	bool pattern_found = false;
	
	while (counter > 0) {
		/* Compare the short pattern with the tail of the response. */
		match = memcmp(&_mac_status_pattern[0], &buf[counter+sizeof(struct ar9170_rx_head)-sizeof(struct ar9170_rx_macstatus)], 2);
		if (match == 0) {
			#if AR9170_RX_DEBUG_DEEP
			printf("DEBUG: AR9170_RX; Padding length: %u.\n",forward_counter);
			#endif
			/* Short pattern found. Proceed with phy comparison. 
			 * If the phy comparison fails, we signal a warning,
			 * but do not drop the packet.
			 */
			pattern_found = true;
			unsigned int _index = counter - 13;
			match = memcmp(&_mac_phy_status_pattern[0], &buf[_index+sizeof(struct ar9170_rx_head)-sizeof(struct ar9170_rx_macstatus)], 13);
			if (match != 0) {
				#if AR9170_RX_DEBUG_DEEP
				printf("WARNING: Short pattern matched but long pattern not.\n");
				#endif
			}
			/* Remove the padded bytes, by adjusting the response length. */
			len -=  forward_counter;
			/* Break the loop*/
			break;			
			
		} else {
			/* Short pattern not found. Shift to the left. */
			counter--;
			forward_counter++;
		}			
	}
	
	/* If the short pattern was not found, we could ignore the packet, right? */
	if (pattern_found == false) {
		#if AR9170_RX_DEBUG_DEEP
		printf("ERROR: the MAC status pattern was not found.\n");
		#endif
		//goto drop;
	}				
	
	/* Extracting the MAC status header from the received MPDU */
	mpdu_len = len - sizeof(*mac);
	
	mac = (void *)(buf + mpdu_len);
	mac_status = mac->status;
	
	switch (mac_status & AR9170_RX_STATUS_MPDU) {
	case AR9170_RX_STATUS_MPDU_FIRST:
		/* Aggregated MPDUs start with an PLCP header */
		if (likely(mpdu_len >= sizeof(struct ar9170_rx_head))) {
			head = (void *) buf;

			/*
			 * The PLCP header needs to be cached for the
			 * following MIDDLE + LAST A-MPDU packets.
			 *
			 * So, if you are wondering why all frames seem
			 * to share a common RX status information,
			 * then you have the answer right here...
			 */
			memcpy(&ar->rx_plcp, (void *) buf,
			       sizeof(struct ar9170_rx_head));

			mpdu_len -= sizeof(struct ar9170_rx_head);
			buf += sizeof(struct ar9170_rx_head);

			ar->rx_has_plcp = true;
		} else {
		/*
			if (net_ratelimit()) {
				wiphy_err(ar->hw->wiphy, "plcp info "
					"is clipped.\n");
			}
		*/
			goto drop;
		}
		break;

	case AR9170_RX_STATUS_MPDU_LAST:
		/*
		 * The last frame of an A-MPDU has an extra tail
		 * which does contain the phy status of the whole
		 * aggregate.
		 */

		if (likely(mpdu_len >= sizeof(struct ar9170_rx_phystatus))) {
			mpdu_len -= sizeof(struct ar9170_rx_phystatus);
			phy = (void *)(buf + mpdu_len);
		} else {
		/*
			if (net_ratelimit()) {
				wiphy_err(ar->hw->wiphy, "frame tail "
					"is clipped.\n");
			}
		*/	
			goto drop;
		}

	case AR9170_RX_STATUS_MPDU_MIDDLE:
		/*  These are just data + mac status */
		if (unlikely(!ar->rx_has_plcp)) {
			/*
			if (!net_ratelimit())
				return;
			*/
			#if AR9170_RX_DEBUG_DEEP
			printf("ERROR: rx stream does not start with a first_mpdu frame tag.\n");
			#endif

			goto drop;
		}

		head = &ar->rx_plcp;
		break;

	case AR9170_RX_STATUS_MPDU_SINGLE:
		#if AR9170_RX_DEBUG_DEEP
		printf("Single MPDU received [%u].\n",len);
		#endif
		/* single mpdu has both: plcp (head) and phy status (tail) */
		head = (void *) buf;

		mpdu_len -= sizeof(struct ar9170_rx_head);
		mpdu_len -= sizeof(struct ar9170_rx_phystatus);

		buf += sizeof(struct ar9170_rx_head);
		phy = (void *)(buf + mpdu_len);
		break;

	default:
		printf("ERROR: RX_STATUS is not any of Single/Middle/Start/Tail.\n");
		break;
	}
	
	/* FC + DU + RA + FCS */
	if (not_expected(mpdu_len < (2 + 2 + ETH_ALEN + FCS_LEN))) {
		#if AR9170_RX_DEBUG_DEEP
		printf("ERROR: MPDU does not have sufficient length.\n");
		#endif
		goto drop;
	}		

	memset(&status, 0, sizeof(status));
	if (not_expected(ar9170_rx_mac_status(ar, head, mac, &status))) {
		#if AR9170_RX_DEBUG_DEEP
		printf("ERROR: MPDU has bad MAC status [%u].\n",len);		
		int j;
		for(j=0; j<len; j++)
			printf("%02x ",buf[j]);
		printf(" \n");	
		#endif
		
		goto drop;
		
	} else {	
		/* Print packet dump from selected node. Note that if it is a single
		 * MPDU, the PLCP header is already removed. This is just debugging.
		 */
		#if AR9170_RX_DEBUG_DEEP
		int j;
		printf("CORRECT PACKET [%u]: ",len-sizeof(struct ar9170_rx_head));
		for(j=0; j<len-sizeof(struct ar9170_rx_head); j++)
			printf("%02x ",buf[j]);
		printf(" \n");
		#endif		
	}		

	if (!ar9170_ampdu_check(ar, buf, mac_status))
		goto drop;

	if (phy)
		ar9170_rx_phy_status(ar, phy, &status);

	ar9170_ps_beacon(ar, buf, mpdu_len);
/*
	carl9170_ba_check(ar, buf, mpdu_len);
*/
	/* We may want to drop packets due to overhearing, 
	 * already inside the interrupt context, in order
	 * not to overload the AR9170 scheduler. So, only
	 * broad-casted MAC frames and frames destined for
	 * us are stored.
	 */
	const uint8_t broadcast_ethernet_addr[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};	
	struct ieee80211_hdr* pkt_head = (struct ieee80211_hdr*)buf;
	
	if (ether_addr_equal(pkt_head->addr1, unique_vif->addr) || 
	    ether_addr_equal(pkt_head->addr1, broadcast_ethernet_addr)) {
		/* Schedule packet processing outside the interrupt context. */
		ar9170_schedule_handle_mpdu(ar, buf, mpdu_len);	
	
	} else {		
		/* Do not store. It is not for us. */
		#if AR9170_RX_DEBUG_DEEP
		printf("Dropping frame for: %02x:%02x:%02x:%02x:%02x:%02x\n",
				pkt_head->addr1[0],
				pkt_head->addr1[1],
				pkt_head->addr1[2],
				pkt_head->addr1[3],
				pkt_head->addr1[4],
				pkt_head->addr1[5]
				);
		#endif		
		goto drop;		
	}
	
	/* Create a shallow copy of the socket buffer. 
	 * This is freed, after packet processing is 
	 * completed.
	 */
/*
	skb = ar9170_rx_copy_data(buf, mpdu_len);
	if (!skb) {
		printf("ERROR: Could not allocate memory for MPDU [handleMPDU].\n");
		goto drop;			
	} else {
		if (skb->data == NULL) {
			printf("ERROR: Could not allocate memory data for MPDU [handleMPDU].\n");
		}
	}		
*/		
	/* At this point we need to handle the received packet. 
	 * This function needs to clear the packet content.
	 */
	
/*	__ieee80211_rx(skb);
*/
/*
	memcpy(IEEE80211_SKB_RXCB(skb), &status, sizeof(status));
	ieee80211_rx(ar->hw, skb);
*/
	/* If the data has not been freed, we signal an error. */
/*	
	if (skb->data == NULL) {
		printf("ERROR: Someone has freed the packet content.\n");
	}
	
	/* Free the socket buffer created by ar9170_rx_copy_data. */
/*	
	if (skb != NULL) {
		sfree(skb);
		skb = NULL;
	}
*/	
	return;

drop:
	#if AR9170_RX_DEBUG_DEEP
	printf("WARNING: Dropping received MPDU.\n");
	#endif
	ar->rx_dropped++;	
}



/*
 * NOTE:
 *
 * The firmware is in charge of waking up the device just before
 * the AP is expected to transmit the next beacon.
 *
 * This leaves the driver with the important task of deciding when
 * to set the PHY back to bed again.
 */
void ar9170_ps_beacon(struct ar9170 *ar, void *data, unsigned int len)
{
	struct ieee80211_hdr *hdr = data;
	struct ieee80211_tim_ie *tim_ie;
	U8 *tim;
	U8 tim_len;
	bool cam = false;

	if (likely(!(ar->hw->conf.flags & IEEE80211_CONF_PS))) {
		#if AR9170_RX_DEBUG_DEEP
		printf("DEBUG: Station not in PS mode.\n");
		#endif
		return;
	}		

	/* check if this really is a beacon */
	if (!ieee80211_is_beacon(hdr->frame_control)) {
		#if AR9170_RX_DEBUG_DEEP
		printf("[%llu] Station in PS mode but what was received was not a beacon.\n", RTIMER_NOW());
		#endif
		return;
	}
	
	/* min. beacon length + FCS_LEN */
	if (len <= 40 + FCS_LEN) {
		#if AR9170_RX_DEBUG_DEEP
		printf("DEBUG: Station in PS mode but received beacon less than 40 bytes.\n");
		#endif
		return;
	}
	
	/* and only beacons from the associated BSSID, please */
	if (!ether_addr_equal(hdr->addr3, ar->common.curbssid) ){// || (!ar->common.curaid)) {
		#if AR9170_RX_DEBUG_DEEP
		printf("[%llu] Station in PS mode but received beacon is not from the associated BSSID.\n", RTIMER_NOW());
		#endif
		return;
	}		
#if 0 // FIXME - check what is important here for the IBSS implementation 
	ar->ps.last_beacon = jiffies;

	tim = carl9170_find_ie(data, len - FCS_LEN, WLAN_EID_TIM);
	if (!tim)
		return;

	if (tim[1] < sizeof(*tim_ie))
		return;

	tim_len = tim[1];
	tim_ie = (struct ieee80211_tim_ie *) &tim[2];

	if (!WARN_ON_ONCE(!ar->hw->conf.ps_dtim_period))
		ar->ps.dtim_counter = (tim_ie->dtim_count - 1) %
			ar->hw->conf.ps_dtim_period;

	/* Check whenever the PHY can be turned off again. */

	/* 1. What about buffered unicast traffic for our AID? */
	cam = ieee80211_check_tim(tim_ie, tim_len, ar->common.curaid);

	/* 2. Maybe the AP wants to send multicast/broadcast data? */
	cam |= !!(tim_ie->bitmap_ctrl & 0x01);


	if (!cam) {
		/* back to low-power land. */
		ar->ps.off_override &= ~PS_OFF_BCN;
		/* FIXME 
		 * Here I call directly the ps_update instead of the ps_check.
		 * The ps_check would queue the work to do, i.e. to send the 
		 * command, while the ps_update is doing it directly which may
		 * cause some difference in behavior.
		 */
		ar9170_ps_update(ar);//ar9170_ps_check(ar);
	} else {
		/* force CAM */
		ar->ps.off_override |= PS_OFF_BCN;
	}
#endif	
}


int ar9170_rx_mac_status(struct ar9170 *ar,
struct ar9170_rx_head *head, struct ar9170_rx_macstatus *mac,
struct ieee80211_rx_status *status)
{
	struct ieee80211_channel *chan;
	U8 error, decrypt;

	if (sizeof(struct ar9170_rx_head) != 12) {
		printf("BUG: Size of RX head is not 12.\n");
	}
	if (sizeof(struct ar9170_rx_macstatus) != 4) {
		printf("BUG: Size of RX MAC status is not 4.\n");
	}

	error = mac->error;

	if (error & AR9170_RX_ERROR_WRONG_RA) {
		if (!ar->sniffer_enabled) {
			#if AR9170_RX_DEBUG_DEEP
			printf("ERROR: RX with wrong RA although sniffer is not enabled.\n");
			#endif
			return -EINVAL;
		}			
	}

	if (error & AR9170_RX_ERROR_PLCP) {
		if (!(ar->filter_state & FIF_PLCPFAIL)) {
			#if AR9170_RX_DEBUG_DEEP
			printf("ERROR: Rx with Fail PLCP although these are not filtered.");
			#endif
			return -EINVAL;
		}			

		status->flag |= RX_FLAG_FAILED_PLCP_CRC;
	}

	if (error & AR9170_RX_ERROR_FCS) {
		ar->tx_fcs_errors++;

		if (!(ar->filter_state & FIF_FCSFAIL))
			return -EINVAL;

		status->flag |= RX_FLAG_FAILED_FCS_CRC;
	}

	decrypt = ar9170_get_decrypt_type(mac);
	if (!(decrypt & AR9170_RX_ENC_SOFTWARE) &&
	decrypt != AR9170_ENC_ALG_NONE) {
		if ((decrypt == AR9170_ENC_ALG_TKIP) &&
		(error & AR9170_RX_ERROR_MMIC))
		status->flag |= RX_FLAG_MMIC_ERROR;

		status->flag |= RX_FLAG_DECRYPTED;
	}

	if (error & AR9170_RX_ERROR_DECRYPT && !ar->sniffer_enabled)
		return -ENODATA;

	error &= ~(AR9170_RX_ERROR_MMIC |
	AR9170_RX_ERROR_FCS |
	AR9170_RX_ERROR_WRONG_RA |
	AR9170_RX_ERROR_DECRYPT |
	AR9170_RX_ERROR_PLCP);

	/* drop any other error frames */
	if (unlikely(error)) {
		/* TODO: update netdevice's RX dropped/errors statistics */

		//if (net_ratelimit())
		#if AR9170_RX_DEBUG_DEEP
		printf("WARNING: received frame with suspicious error code (%#x).\n", error);
		#endif
		return -EINVAL;
	}

	chan = ar->channel;
	if (chan) {
		status->band = chan->band;
		status->freq = chan->center_freq;
	}

	switch (mac->status & AR9170_RX_STATUS_MODULATION) {
		case AR9170_RX_STATUS_MODULATION_CCK:
		if (mac->status & AR9170_RX_STATUS_SHORT_PREAMBLE)
			status->flag |= RX_FLAG_SHORTPRE;
		switch (head->plcp[0]) {
			case AR9170_RX_PHY_RATE_CCK_1M:
			status->rate_idx = 0;
			break;
			case AR9170_RX_PHY_RATE_CCK_2M:
			status->rate_idx = 1;
			break;
			case AR9170_RX_PHY_RATE_CCK_5M:
			status->rate_idx = 2;
			break;
			case AR9170_RX_PHY_RATE_CCK_11M:
			status->rate_idx = 3;
			break;
			default:
			//if (net_ratelimit()) {
				#if AR9170_RX_DEBUG_DEEP
				printf("ERROR: invalid plcp cck rate (%x).\n", head->plcp[0]);
				#endif
			//}

			return -EINVAL;
		}
		break;

		case AR9170_RX_STATUS_MODULATION_DUPOFDM:
		case AR9170_RX_STATUS_MODULATION_OFDM:
		switch (head->plcp[0] & 0xf) {
			case AR9170_TXRX_PHY_RATE_OFDM_6M:
			status->rate_idx = 0;
			break;
			case AR9170_TXRX_PHY_RATE_OFDM_9M:
			status->rate_idx = 1;
			break;
			case AR9170_TXRX_PHY_RATE_OFDM_12M:
			status->rate_idx = 2;
			break;
			case AR9170_TXRX_PHY_RATE_OFDM_18M:
			status->rate_idx = 3;
			break;
			case AR9170_TXRX_PHY_RATE_OFDM_24M:
			status->rate_idx = 4;
			break;
			case AR9170_TXRX_PHY_RATE_OFDM_36M:
			status->rate_idx = 5;
			break;
			case AR9170_TXRX_PHY_RATE_OFDM_48M:
			status->rate_idx = 6;
			break;
			case AR9170_TXRX_PHY_RATE_OFDM_54M:
			status->rate_idx = 7;
			break;
			default:
			//if (net_ratelimit()) {
				#if AR9170_RX_DEBUG_DEEP
				printf("ERROR: invalid plcp ofdm rate (%x).\n", head->plcp[0]);
				#endif
			//}

			return -EINVAL;
		}
		if (status->band == IEEE80211_BAND_2GHZ)
			status->rate_idx += 4;
		break;

		case AR9170_RX_STATUS_MODULATION_HT:
		if (head->plcp[3] & 0x80)
			status->flag |= RX_FLAG_40MHZ;
		if (head->plcp[6] & 0x80)
			status->flag |= RX_FLAG_SHORT_GI;

		status->rate_idx = clamp(0, 75, head->plcp[3] & 0x7f);
		status->flag |= RX_FLAG_HT;
		break;

		default:
			printf("BUG: Reached default case in switch.\n");
		return -ENOSYS;
	}

	return 0;
}


bool ar9170_ampdu_check(struct ar9170 *ar, U8 *buf, U8 ms)
{
	le16_t fc;

	if ((ms & AR9170_RX_STATUS_MPDU) == AR9170_RX_STATUS_MPDU_SINGLE) {
		/*
		 * This frame is not part of an aMPDU.
		 * Therefore it is not subjected to any
		 * of the following content restrictions.
		 */
		return true;
	}

	/*
	 * "802.11n - 7.4a.3 A-MPDU contents" describes in which contexts
	 * certain frame types can be part of an aMPDU.
	 *
	 * In order to keep the processing cost down, I opted for a
	 * stateless filter solely based on the frame control field.
	 */

	fc = ((struct ieee80211_hdr *)buf)->frame_control;
	if (ieee80211_is_data_qos(fc) && ieee80211_is_data_present(fc))
		return true;

	if (ieee80211_is_ack(fc) || ieee80211_is_back(fc) ||
	    ieee80211_is_back_req(fc))
		return true;

	if (ieee80211_is_action(fc))
		return true;
		
	if (ieee80211_is_atim(fc))
		return true;	

	return false;
}


void ar9170_rx_phy_status(struct ar9170 *ar,
struct ar9170_rx_phystatus *phy, struct ieee80211_rx_status *status)
{
	int i;

	if(sizeof(struct ar9170_rx_phystatus) != 20) {
		printf("BUG: sizeof(struct ar9170_rx_phystatus) != 20.\n");
	}

	for (i = 0; i < 3; i++)
		if (phy->rssi[i] != 0x80)
			status->antenna |= BIT(i);

	/* post-process RSSI */
	for (i = 0; i < 7; i++)
		if (phy->rssi[i] & 0x80)
			phy->rssi[i] = ((phy->rssi[i] & 0x7f) + 1) & 0x7f;

	/* TODO: we could do something with phy_errors */
	status->signal = ar->noise[0] + phy->rssi_combined;
}

struct sk_buff *ar9170_rx_copy_data(U8 *buf, int len)
{
	struct sk_buff *skb = NULL;
	int reserved = 0;
	struct ieee80211_hdr *hdr = (void *) buf;

	if (ieee80211_is_data_qos(hdr->frame_control)) {
		U8 *qc = ieee80211_get_qos_ctl(hdr);
		reserved += NET_IP_ALIGN;

		if (*qc & IEEE80211_QOS_CTL_A_MSDU_PRESENT)
		reserved += NET_IP_ALIGN;
	}

	if (ieee80211_has_a4(hdr->frame_control))
		reserved += NET_IP_ALIGN;

	reserved = 32 + (reserved & NET_IP_ALIGN);

	skb = (struct sk_buff*)smalloc(sizeof(struct sk_buff));
	if (skb != NULL) {
		/* Create a shallow copy, i.e. the actual 
		 * packet content is NOT copied!! 
		 */
		skb->len = len;
		skb->data = buf;
	
	} else {
		printf("ERROR: Could not allocate memory for socket buffer [inside copying mpdu].\n");
	}
/*	
	skb->data = NULL;
	skb->data = malloc(len);
	if(skb->data != NULL) 
		memcpy(skb->data, buf, len);
*/		
/*
	skb = dev_alloc_skb(len + reserved);
	if (likely(skb)) {
		skb_reserve(skb, reserved);
		memcpy(skb_put(skb, len), buf, len);
	}
*/
	return skb;
}
