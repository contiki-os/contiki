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
#include "usb_cmd_wrapper.h"
#include "usb_wrapper.h"
#include "uhi_vendor.h"
#include "usb_lock.h"
#include "string.h"
#include "fwcmd.h"
#include <sys\errno.h>
#include "ar9170_debug.h"
#include "cfg80211.h"
#include "ar9170.h"
#include "compiler.h"
#include <time.h>
#include "delay.h"


COMPILER_WORD_ALIGNED uint8_t echo_test_command[4] = {0x4a, 0x11, 0x01, 0x23};
COMPILER_WORD_ALIGNED uint8_t reboot_test_command[4] = { 0x04, 0x04, 0x00, 0x00};		 


COMPILER_WORD_ALIGNED uint8_t scsi_stop_unit[31] = {
	0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x06, 0x1B,	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


bool ar9170_echo_test(struct ar9170* ar) {
	
	bool echo_result;
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Attempting an ECHO test...\n");
	#endif
	uint32_t echoed_bytes;
	
	echo_result = ar9170_exec_cmd(ar, CARL9170_CMD_ECHO, 4, echo_test_command, 4, (uint8_t*)&echoed_bytes);
	
	if (echo_result == false) {
		printf("ERROR: ECHO command returned error.\n");
		return false;
	}
	
	if (memcmp(&echoed_bytes, &echo_test_command[0], 4)) {
		printf("ERROR: Wrong ECHO!\n");
		return false;
	}
	
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Correct echo reply received.\n");
	#endif
	
	return true;
}


void ar9170_eject_command_done (usb_add_t add, usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered)
{
	if (status != UHD_TRANS_NOERROR) {
		printf("DEBUG : Error sending MAGIC COMMAND!\n");
		return; // Error during transfer
	}
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG : Eject Command Sent!\n");
	printf("DEBUG : Address: 0x%02x  EndPoint: 0x%02x Status: %d  Bytes Transfered: %u\n", add, ep, status, (unsigned int)nb_transfered);
	#endif
}



bool ar9170_eject_mass_storage_device_cmd( void )
{		
	/* Will attempt to EJECT the Mass Storage Device now */
	printf("Attempting to eject the 84FF [CD-ROM] device...\n");
		
	/* Update EJECT command status to sent. In case it will not be
	 * sent we need to update back to not_sent. We do this so as not
	 * to lose race, if the unplug interrupt comes too early.
	 */
	ar9170_set_eject_command_sent();
	
	bool eject_result;		
	// Check if bulk endpoints are available and send the command
	if (uhi_vendor_bulk_is_available()) {
		// Send data through bulk OUT endpoint
		eject_result = uhi_vendor_bulk_out_run(scsi_stop_unit, 31, ar9170_eject_command_done);
		// Receive data through bulk IN endpoint
		//uhi_vendor_bulk_in_run(my_in_buffer, sizeof(my_in_buffer), my_callback_bulk_in_done);
	} else {
		return false;
	}
	
	/* Indicate whether the EJECT command was sent successfully. */
	return eject_result;
}



bool __ar9170_exec_cmd( struct ar9170_cmd* cmd )
{
	int i;
	uint8_t* cmd_buf;
		
	cmd_buf = smalloc(cmd->hdr.len + AR9170_CMD_HDR_LEN); 
	if (cmd_buf == NULL) {
		
		printf("ERROR: No memory for __execute_command.\n");
		return false;
	}
	cmd_buf[0] = cmd->hdr.len;
	cmd_buf[1] = cmd->hdr.cmd;
	cmd_buf[2] = cmd->hdr.seq;
	cmd_buf[3] = cmd->hdr.ext;
	for (i=0; i<cmd->hdr.len; i++)
		cmd_buf[i+4] = *(cmd->data + i);
	
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("CMD [%u]: ", cmd->hdr.len + AR9170_CMD_HDR_LEN);
	for(i=0; i<cmd->hdr.len + AR9170_CMD_HDR_LEN; i++)
		printf("%02x ",cmd_buf[i]);
	printf(" \n");
	#endif
	
	struct ar9170* ar = ar9170_get_device();
		
	return ar9170_usb_write_reg(&ar->cmd_async_lock, cmd_buf,cmd->hdr.len + AR9170_CMD_HDR_LEN);
}

bool ar9170_exec_cmd(struct ar9170* ar, const enum carl9170_cmd_oids cmd, unsigned int plen, void *payload, unsigned int outlen, void* out )
{	
	bool result;	
	
	ar->cmd.hdr.len = plen;
	ar->cmd.hdr.cmd = cmd;
	ar->cmd.hdr.seq = 0;	
	ar->cmd.hdr.ext = 0;
	 
	memcpy(ar->cmd.data, (uint8_t*)payload, plen);
		
	__start(&ar->cmd_lock);
	ar->readbuf = out;
	ar->readlen = outlen;	
	__complete(&ar->cmd_lock);
	
	__start(&ar->cmd_wait);

	result = __ar9170_exec_cmd(&ar->cmd);
		
	if ( !(cmd & CARL9170_CMD_ASYNC_FLAG) ) {
		
		__wait_for_completion(&ar->cmd_wait);
		//Can continue
				
	} else {
		__complete(&ar->cmd_wait);
	}				
		
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Command executed.\n");
	#endif
	
	return result; 
}

void ar9170_cmd_callback( struct ar9170 *ar, uint32_t len, void *buffer )
{
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Command received callback.\n");
	#endif
	if (unlikely(ar->readlen != len - AR9170_CMD_HDR_LEN)) {
		printf("WARNING: Received invalid command response length! Read buffer size: %d, received length: %d.\n",
				(unsigned int)ar->readlen, (unsigned int)len - AR9170_CMD_HDR_LEN);		
		//ar9170_restart(ar, CARL9170_RR_INVALID_RSP); TODO
	}
	__start(&ar->cmd_lock);
	
	if (ar->readbuf != NULL) {
		if(len >= AR9170_CMD_HDR_LEN) {
			memcpy(ar->readbuf, buffer + AR9170_CMD_HDR_LEN, len - AR9170_CMD_HDR_LEN);
		
		} else {
			#if USB_CMD_WRAPPER_DEBUG_DEEP
			printf("DEBUG: Response had no data.\n");
			#endif
		}			
	} else {
		#if USB_CMD_WRAPPER_DEBUG_DEEP
		printf("WARNING: The readbuf was NULL; can not copy.\n");
		#endif
	}	
	
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG: Releasing command lock and wait.\n");
	#endif
	__complete(&ar->cmd_lock);
	__complete(&ar->cmd_wait);
}


int ar9170_write_rgstr(struct ar9170 *ar, const U32 reg, const U32 val)
{
	const le32_t buf[2] = {
		cpu_to_le32(reg),
		cpu_to_le32(val),
	};
	int result;

	result = ar9170_exec_cmd(ar, CARL9170_CMD_WREG, sizeof(buf), (U8 *) buf, 0, NULL);
	if (result == false) {
		printf("ERROR: Write Register command returned errors.\n");
	/*	
		if (net_ratelimit()) {
			wiphy_err(ar->hw->wiphy, "writing reg %#x "
			"(val %#x) failed (%d)\n", reg, val, result);
		}
	*/
	}
	return result;
}


int ar9170_read_reg(struct ar9170 *ar, U32 reg, U32 *val)
{
	return ar9170_read_mreg(ar, 1, &reg, val);
}


int ar9170_read_mreg(struct ar9170 *ar, const int nregs, const U32 *regs, U32 *out)
{
	int i, result;
	le32_t *offs, *res;

	/* abuse "out" for the register offsets, must be same length */
	offs = (le32_t *)out;
	for (i = 0; i < nregs; i++)
	offs[i] = cpu_to_le32(regs[i]);

	/* also use the same buffer for the input */
	res = (le32_t *)out;

	result = ar9170_exec_cmd(ar, CARL9170_CMD_RREG, 4 * nregs, (U8 *)offs,	4 * nregs, (U8 *)res);
	if (result == false) {
		printf("ERROR: Read Register command returned errors.\n");
	/* FIXME - what is this and how it is used?
		if (net_ratelimit()) {
			wiphy_err(ar->hw->wiphy, "reading regs failed (%d)\n",
			result);
		}
	*/	
		return result;
	}

	/* convert result to cpu endian */
	for (i = 0; i < nregs; i++)
	out[i] = le32_to_cpu(res[i]);

	return result;
}


struct ar9170_cmd *ar9170_cmd_buf(struct ar9170 *ar, const enum carl9170_cmd_oids cmd, const unsigned int len)
{
	struct ar9170_cmd *tmp;

	tmp = smalloc(sizeof(struct ar9170_cmd_head) + len);
		
	if (tmp) {
		memset(tmp,0,sizeof(struct ar9170_cmd_head) + len);
		tmp->hdr.cmd = cmd;
		tmp->hdr.len = len;
	} else {
		printf("ERROR: ar9170_cmd_buf could not allocate memory.\n");
	} 

	return tmp;
}


int ar9170_reboot(struct ar9170 *ar)
{
	struct ar9170_cmd *cmd;
	int result;

	cmd = ar9170_cmd_buf(ar, CARL9170_CMD_REBOOT_ASYNC, 0);
	if (!cmd) {
		return -ENOMEM;	
	}

	result = __ar9170_exec_cmd(cmd);//__ar9170_exec_cmd(ar, cmd, true);
	
	free(cmd);
	return result;
}


int ar9170_mac_reset(struct ar9170 *ar)
{
	return ar9170_exec_cmd(ar, CARL9170_CMD_SWRST, 0, NULL, 0, NULL);
}


int ar9170_rx_filter(struct ar9170* ar, const unsigned int _rx_filter)
{
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG: RX Filtering.\n");
	#endif
	
	le32_t rx_filter = cpu_to_le32(_rx_filter);
	
	return !ar9170_exec_cmd(ar, CARL9170_CMD_RX_FILTER, sizeof(rx_filter), (U8*)&rx_filter, 0, NULL);
}


int ar9170_flush_cab(struct ar9170 *ar,const unsigned int vif_id)
{
	return ar9170_bcn_ctrl(ar, vif_id, AR9170_BCN_CTRL_DRAIN, 0, 0);
}


int ar9170_bcn_ctrl(struct ar9170 *ar, const unsigned int vif_id, const U32 mode, const U32 addr, const U32 len)
{
	int err = 0;
	
	struct ar9170_cmd *cmd = NULL;
	
	cmd = ar9170_cmd_buf(ar, CARL9170_CMD_BCN_CTRL_ASYNC, sizeof(struct ar9170_bcn_ctrl_cmd));
	if (!cmd) {
		printf("ERROR: No memory for bcn_ctrl command.\n");
		return -ENOMEM;
	}		

	cmd->bcn_ctrl.vif_id = cpu_to_le32(vif_id);
	cmd->bcn_ctrl.mode = cpu_to_le32(mode);
	cmd->bcn_ctrl.bcn_addr = cpu_to_le32(addr);
	cmd->bcn_ctrl.bcn_len = cpu_to_le32(len);

	err =  !__ar9170_exec_cmd(cmd); //__ar9170_exec_cmd(ar, cmd, true);
	
	if (err) {
		printf("ERROR: bcn_ctrl returned errors.\n");
	}
	free(cmd);
	
	return err;
}

int ar9170_collect_tally(struct ar9170 *ar)
{
	struct ar9170_tally_rsp tally;
	struct survey_info *info;
	unsigned int tick;
	int err;

	err = !ar9170_exec_cmd(ar, CARL9170_CMD_TALLY, 0, NULL, sizeof(tally), (U8 *)&tally);
	if (err) {
		printf("ERROR: Collect TALLY command returned errors.\n");
		return err;	
	}	

	tick = le32_to_cpu(tally.tick);
	if (tick) {
		ar->tally.active += le32_to_cpu(tally.active) / tick;
		ar->tally.cca += le32_to_cpu(tally.cca) / tick;
		ar->tally.tx_time += le32_to_cpu(tally.tx_time) / tick;
		ar->tally.rx_total += le32_to_cpu(tally.rx_total);
		ar->tally.rx_overrun += le32_to_cpu(tally.rx_overrun);

		if (ar->channel) {
			info = &ar->survey[ar->channel->hw_value];
			info->channel_time = ar->tally.active;
			info->channel_time_busy = ar->tally.cca;
			info->channel_time_tx = ar->tally.tx_time;
			//do_div(info->channel_time, 1000);
			//do_div(info->channel_time_busy, 1000);
			//do_div(info->channel_time_tx, 1000);
			//FIXME - perhaps it is important
			info->channel_time = info->channel_time / 1000;
			info->channel_time_busy = info->channel_time_busy / 1000;
			info->channel_time_tx = info->channel_time_tx / 1000;
		}
	}
	return 0;
}



int ar9170_powersave(struct ar9170 *ar, const bool ps)
{
	int err = 0;
	
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	printf("DEBUG: executing ar9170_powersave with value: %d.\n", ps);
	#endif
	
	struct ar9170_cmd *cmd;
	U32 state;

	cmd = ar9170_cmd_buf(ar, CARL9170_CMD_PSM_ASYNC,sizeof(struct ar9170_psm));
	if (!cmd) {
		printf("ERROR: No memory for the command buffering.\n");
		return -ENOMEM;	
	}	

	if (ps) {
		/* Sleep until next TBTT */
		state = AR9170_PSM_SLEEP | 1;
	} else {
		/* wake up immediately */
		state = 1;
	}
	
	/* Set desired state */
	cmd->psm.state = cpu_to_le32(state);
	
	/* XXX comment-out to disable actual power-save command. [DEBUG] */
	//err = !__ar9170_exec_cmd(cmd); 
	/* XXX If commented out, we need to disable, here, the transit flags. */
	
	if (ar->ps_mgr.psm_transit_to_sleep == true) {
		ar->ps_mgr.psm_transit_to_sleep = false;
		
	} else if (ar->ps_mgr.psm_transit_to_wake == true) {
		ar->ps_mgr.psm_transit_to_wake = true;
		
	} else {
		printf("ERROR: Why the ps transit flag is already off?\n");
	}
	
	#if USB_CMD_WRAPPER_DEBUG_DEEP
	if(state == 1) {		
		printf("WILL WAKEUP!\n");
	} else {
		printf("WILL SLEEP!\n");
	}	
	#endif
	
	/* Free command buffer */
	free(cmd);
	
	if (err) {
		printf("ERROR: Power-save command returned error status.\n");
	}
	return err;
}