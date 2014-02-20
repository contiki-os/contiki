/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
#include "ar9170_scheduler.h"
#include "ar9170.h"
#include "ar9170_psm.h"
#include "ieee80211_psm.h"
#include "compiler.h"
#include "usb_cmd_wrapper.h"
#include "contiki-main.h"
#include "ar9170_debug.h"
#include "ieee80211_ibss.h"
#include "fwcmd.h"
#include "delay.h"
#include "cc.h"
#include "usb_wrapper.h"
#include "wire_digital.h"
#include "linked_list.h"
#include "rtimer.h"


void ar9170_sch_erase_nodes_check( struct ar9170* ar )
{
	if (ar->erase_awake_nodes_flag == true) {
		ar->erase_awake_nodes_flag = false;
		if (ar->ps_mgr.psm_state == AR9170_PRE_TBTT) {
			/* We should only erase the neighbor list inside the pre-TBTT period. */
			ieee80211_psm_erase_awake_neighbors(ar);
		} else {
			/* If we reach here, it means that we did not have the time to erase
			* the list before the ATIM Window begins. We need to signal a warning
			* since the node will erroneously assume that some nodes are awake, 
			* while they may be not.
			*/
			printf("WARNING: Asking to delete neighbors outside TBTT.\n");
		}
	}
}

void ar9170_sch_powersave_check( struct ar9170* ar )
{
	if (ar->ps.update_mask & AR9170_PS_UPDATE_TRANSITION_FLAG) {
		/* Transition requested. */
		if (likely(ar->ps.update_mask & AR9170_PS_UPDATE_ACTION_SLEEP)) {
			
			/* Set device state to power-save. This should stop or cancel 
			 * asynchronous packet transmissions, to prevent sending frames
			 * while on power-save. This is happening a bit earlier, but it
			 * is anyway safe.
			 */
			if (not_expected(ar->ps.state == true)) {
				printf("ERROR: Device RF state already put to sleep.\n");
			}
			/* Set to power-save. Consider moving it to the handle_ps. */
			ar->ps.state = true;
			
			if (not_expected(ar->ps_mgr.psm_transit_to_sleep == false)) {
				printf("ERROR: Why the transit_to_sleep flag is already off?\n");
			}
			
			/* 
			 * Device powers-down now.
			 */
			ar9170_powersave(ar, true);		
			
			#ifdef WITH_LED_DEBUGGING
			digital_write(DOZE_ACTIVE_PIN, HIGH);
			#endif
		
		} else {			
			
			if (not_expected(ar->ps_mgr.psm_transit_to_wake == false)) {
				printf("ERROR: Why the transit_to_wake flag is already off?\n");
			}
			
			/* Wake up device, sending an RF wake-up command. */
			ar9170_powersave(ar, false);
			
			/* Device PHY is not on yet; we can only be sure when we get the
			 * command response back from the AR9170 device. So we do not yet
			 * clear the power-save state.
			 */
			//ar->ps.state = false; // FIXME - remove this from here			
		}
				
		/* Clear update mask */
		ar->ps.update_mask = 0;		
	}
}

void ar9170_sch_beacon_ctrl_check( struct ar9170* ar )
{
	if (not_expected(ar->beacon_ctrl == true)) {
		
		if (ar->cmd_async_lock == false) {
		
			#if AR9170_SCHEDULER_DEBUG_DEEP
			rtimer_clock_t current_time;
			current_time = RTIMER_NOW();
			printf("[%llu] BCN CTRL.\n",current_time);
			#endif
			
			/* Send beacon control command down */
			ar9170_update_beacon(ar, true);
			
			/* Clear beacon control flag, so it is not called again. */
			ar->beacon_ctrl = false;
		}		
		
	}
}

void ar9170_sch_beacon_cancel_check( struct ar9170* ar )
{
	if (not_expected(ar->beacon_cancel == true)) {
		/* Clear beacon control flag, so it is not called again. */
		ar->beacon_cancel = false;
		
		/* Send command to cancel beacon transmission. */
		ar9170_flush_cab(ar, unique_cvif->id);
		
	}
}

void ar9170_sch_rx_filter_disable_check( struct ar9170* ar )
{
	if (not_expected(ar->clear_filtering == true)) {
		/* Clear the flag, so we do not need to enter this if clause again. */
		ar->clear_filtering = false;
		/* We can now start receiving packets; both management and data. */
		ar9170_rx_filter(ar, AR9170_RX_FILTER_OTHER_RA |
							AR9170_RX_FILTER_CTL_OTHER |
							AR9170_RX_FILTER_BAD |
							AR9170_RX_FILTER_CTL_BACKR |
							//AR9170_RX_FILTER_MGMT |
							//AR9170_RX_FILTER_DATA |
							AR9170_RX_FILTER_DECRY_FAIL);
	}
}

void ar9170_sch_async_cmd_check( struct ar9170* ar )
{
	if (not_expected(ar->cmd_list->buffer != NULL)) {
		
		/* There are pending commands to be sent to the device.
		 * So check whether we are allowed to proceed with the 
		 * following command transfer [checking the asynchronous
		 * command lock flag].
		 */

		delay_us(10); /* How can we get rid of this artificial delay? TODO */
		
		if (ar->cmd_async_lock == false) {			
			
			/* Asynchronous command lock is free, so we can 
			 * transfer a command down, if there is any.
			 */
			if (ar->cmd_list->buffer != NULL) {
				/* Normally, this should hot happen, since the commands are rare, 
				 * and are usually handled directly, without need for queuing.
				 */
				printf("WARNING: AR9170 Scheduler sends pending command.\n");
				
				if (!ar9170_usb_send_cmd(&ar->cmd_async_lock, ar->cmd_list->buffer, 
					ar->cmd_list->send_chunk_len)) {
					printf("ERROR: Submitting next command returned errors.\n");
				}
			}					
		} 
	}
}

void ar9170_sch_async_rx_check( struct ar9170* ar )
{
	 /*
	 * If the pending RX queue is non-empty, we should proceed
	 * with the handling of the first-in-queue packet. Notice,
	 * that we only handle one packet and then continue with 
	 * other asynchronous operations. We will handle the next
	 * packet once we get back here.
	 */
	if (!linked_list_is_empty(ar->rx_pending_pkts)) {
		
		/* We want to protect the pre-TBTT window, 
		 * leaving it free for critical operations
		 * so we postpone packet processing, until
		 * we reach the ATIM Window.
		 */
		if (ar->ps_mgr.psm_state == AR9170_PRE_TBTT) {
			return;
		}
		
		#if AR9170_SCHEDULER_DEBUG_DEEP
		printf("DEBUG: AR9170 Scheduler; Pending RX packets: %d.\n", linked_list_get_len(ar->rx_pending_pkts));
		#endif
		/* Process the first in line received packet. */
		ar9170_async_rx(ar);
	}
}

void ar9170_sch_async_tx_check( struct ar9170* ar )
{
	/* Enter the TX routines only if the device RF is awake. */
	if ((ar->ps.state != false) || (ar->ps_mgr.psm_transit_to_sleep == true)) {
		return;
	}
	
	/* Enter the TX routines only if there are pending packets 
	 * in the ATIM or DATA queue.
	 */	
	if ((!linked_list_is_empty(ar->tx_pending_pkts)) || (!linked_list_is_empty(ar->tx_pending_atims))) {
		
		/*
		*
		* Depending on the state of the protocol, we schedule 
		* transmissions of management or data packets. For now
		* we just attempt transmissions down, immediately.
		*/
				
		/* Otherwise check and transit depending on the state. */
		if (ar->ps_mgr.psm_state == AR9170_ATIM_WINDOW) {
		
			/* If the RF is not powered-on, we signal a warning. 
			 * It is not an error since the state might have just
			 * changed.
			 */
			if(ar->ps.state == true) {
				printf("WARNING: In ATIM Window the device should be ON.\n");
				return;
			}
			/*
			 * We are currently in the ATIM Window, and we can
			 * only send (and receive??) ATIM Packets 
			 */
			if (!linked_list_is_empty(ar->tx_pending_atims)) {
			
				#if AR9170_SCHEDULER_DEBUG_DEEP
				printf("DEBUG: Scheduler; Pending ATIM packets.\n");
				#endif
			
				/* It is implied that the device is in PSM, so the
				 * execution control will be handled to the PS 
				 * manager for checking and sending ATIM frames
				 */
			
				if ((ar->tx_data_wait == false) && (ar->tx_atim_wait == false)) {
					/*
					 * We are not waiting for another status response,
					 * locally, from the AR9170 device, so we proceed
					 * with the next ATIM frame, if there is any.
					 */
					ar9170_psm_async_tx_mgmt(ar);
				
				} else {
				
					/* AR9170 device still busy with the previous frame. */
					//printf("%u %u \n", ar->tx_atim_wait, ar->tx_data_wait);
				}				
			}		
	
		} else if (ar->ps_mgr.psm_state == AR9170_TX_WINDOW) {
		
			/* If the RF is not powered-on, we do not transmit. */
			if(ar->ps.state == true) {
				return;
			}
			delay_us(100);
			/*
			 * We are currently in the TX Window, and we can
			 * only send and receive Data Packets. 
			 */
			if (!linked_list_is_empty(ar->tx_pending_pkts)) {
				/* Currently, there are pending data packets. */
				#if AR9170_SCHEDULER_DEBUG_DEEP
				printf("DEBUG: AR9170 Scheduler; DATA packets pending.\n");
				#endif
				
				/* If we are done with the previous transmission
				 * we proceed with the following one.
				 */
				if ((ar->tx_data_wait == false) && (ar->tx_atim_wait == false)) {
				
					/* We are not waiting for a status response. */
					/*
					 * If the device is in PSM, the execution control
					 * shall be handed in to the PS implementation for 
					 * checking whether the packet can be sent to the 
					 * intended receiver.
					 */			
					if (hw->conf.flags & IEEE80211_CONF_PS) {
						#if AR9170_SCHEDULER_DEBUG_DEEP
						printf("DEBUG: Packet transmission handled by PSM manager.\n");
						#endif
						/* Hand in control to the PSM core that will
						 * decide whether the first packet can be 
						 * transmitted or not.
						 */
						ar9170_psm_async_tx_data(ar);		
		
					} else {
					/*
					 * If the device is not in PSM, the first packet 
					 * should be attempted immediately.
					 */
					 ar->tx_pending_pkts = ar9170_async_tx(ar, ar->tx_pending_pkts);	
							
					}	
						
				} else {
					/* AR9170 device still busy with the previous frame. */
					//printf("%u %u \n", ar->tx_atim_wait, ar->tx_data_wait);	
				}				
			}		
	
		} else if (ar->ps_mgr.psm_state == AR9170_SOFT_BCN_WINDOW) {
		
			/* We check whether we need to transmit soft beacons. */
			if (not_expected(ar->ps_mgr.send_soft_bcn_flag == true)) {
											
				/* We must send a soft beacon now. */
				bool result = ar9170_async_tx_soft_beacon(ar);
						
				if (result) {
					/* Clear the flag so we do not enter again. */
					ar->ps_mgr.send_soft_bcn_flag = false;
				}				
			}
	
		} else if (ar->ps_mgr.psm_state == AR9170_PRE_TBTT) {
	
			/* Nothing is transmitted in pre-TBTT window. */	
	
		} else {
			/* Signal an error on unrecognized device. */
			printf("ERROR: Scheduler can not recognize device state.\n");
		}
		
	}
		
}
