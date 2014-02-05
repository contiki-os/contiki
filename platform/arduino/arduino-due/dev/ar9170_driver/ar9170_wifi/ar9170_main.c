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
#include "ar9170_main.h"
#include "ar9170.h"
#include "usb_wrapper.h"
#include "fwcmd.h"
#include "usb_cmd_wrapper.h"
#include "delay.h"
#include "common.h"
#include "string.h"
#include <sys\errno.h>
#include "usb_fw_wrapper.h"
#include "ar9170_led.h"
#include "ar9170_mac.h"
#include "ar9170_debug.h"
#include "conf_ar9170.h"
#include "ar9170_state.h"
#include "hw.h"
#include "ar9170_wlan.h"
#include "ar9170eeprom.h"
#include "carl9170_v197.h"
#include "ar9170_phy.h"
#include "ieee80211_ibss.h"
#include "mac80211.h"
#include "nl80211.h"
#include <sys\lock.h>
#include "compiler.h"
#include "list.h"
#include "platform-conf.h"
#include "ar9170_psm.h"
#include <stdio.h>
#include "skbuff.h"
#include "usb_lock.h"
#include "ath.h"
#include "core_cmFunc.h"
#include "linked_list.h"
#include "dsc.h"
#include "ar9170.h"
#include "smalloc.h"
#include "cc.h"
#include "bitops.h"
#include "clock.h"
#include "ieee80211_rx.h"
#include "interrupt\interrupt_sam_nvic.h"
#include "ar9170_scheduler.h"
#include "rtimer.h"
#include "ieee80211_psm.h"
#include "if_ether.h"
#include "etherdevice.h"



/* Global AR9170 device structure */
static struct ar9170* ar = NULL;

#define RATE(_bitrate, _hw_rate, _txpidx, _flags) {	\
.bitrate	= (_bitrate),			\
.flags		= (_flags),			\
.hw_value	= (_hw_rate) | (_txpidx) << 4,	\
}

struct ieee80211_rate __carl9170_ratetable[] = {
	RATE(10, 0, 0, 0),
	RATE(20, 1, 1, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(55, 2, 2, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(110, 3, 3, IEEE80211_RATE_SHORT_PREAMBLE),
	RATE(60, 0xb, 0, 0),
	RATE(90, 0xf, 0, 0),
	RATE(120, 0xa, 0, 0),
	RATE(180, 0xe, 0, 0),
	RATE(240, 0x9, 0, 0),
	RATE(360, 0xd, 1, 0),
	RATE(480, 0x8, 2, 0),
	RATE(540, 0xc, 3, 0),
};
#undef RATE

struct ieee80211_supported_band carl9170_band_2GHz = {
	.channels	= carl9170_2ghz_chantable,
	.n_channels	= ARRAY_SIZE(carl9170_2ghz_chantable),
	.bitrates	= carl9170_g_ratetable,
	.n_bitrates	= carl9170_g_ratetable_size,
	.ht_cap		= CARL9170_HT_CAP,
};

static struct ieee80211_supported_band carl9170_band_5GHz = {
	.channels	= carl9170_5ghz_chantable,
	.n_channels	= ARRAY_SIZE(carl9170_5ghz_chantable),
	.bitrates	= carl9170_a_ratetable,
	.n_bitrates	= carl9170_a_ratetable_size,
	.ht_cap		= CARL9170_HT_CAP,
};



struct ar9170* ar9170_get_device()
{
	/* Return the unique AR9170 structure pointer. */
	return ar;
}

struct ar9170** ar9170_get_device_pt()
{
	/* Return the address of the unique AR9170 structure pointer. */
	return &ar;
}

struct ar9170* ar9170_alloc(struct ar9170** ar_pt)
{
	int i;
	#if AR9170_MAIN_DEBUG
	printf("DEBUG allocating %d bytes for the AR9170 structure.\n",sizeof(struct ar9170));
	#endif
	
	// Allocate memory for the global AR9170 device
	struct ar9170* athr = (struct ar9170*)smalloc(sizeof(struct ar9170));
	
	if (!athr) {
		printf("ERROR: Could not allocate memory for the AR9170 device.\n");
		return NULL;
	}
	//Zeroing memory
	memset(athr, 0, sizeof(struct ar9170));
	
	// Zeroing locks	
	athr->fw_boot_wait = 0;
	athr->cmd_lock = 0;
	athr->cmd_async_lock = 0;
	athr->cmd_wait = 0;
	athr->cmd_buf_lock = 0;
	athr->state_lock = 0;
	athr->mutex_lock = 0;
	athr->tx_async_lock = 0;
	athr->tx_buf_lock = 0;
	athr->tx_atim_wait = false;
	athr->tx_data_wait = false;
	athr->clear_tx_data_wait_at_next_tbtt = false;
	athr->clear_cmd_async_lock_at_next_tbtt = false;
	athr->clear_tx_async_lock_at_next_tbtt = false;
	athr->atim_wait_cleared_count = 0;
	
	/*Initialize command list structure */
	athr->cmd_list = (struct ar9170_send_list*)smalloc(sizeof(struct ar9170_send_list));
	athr->cmd_list->buffer = NULL;
	athr->cmd_list->next_send_chunk = NULL;
	
	/*Initialize tx data list structure */
	athr->tx_list = (struct ar9170_send_list*)smalloc(sizeof(struct ar9170_send_list));
	athr->tx_list->buffer = NULL;
	athr->tx_list->next_send_chunk = NULL;
	
	/* Initialize TX pending ATIM queue structure */
	athr->tx_pending_atims = linked_list_init(athr->tx_pending_atims);
	
	/* Initialize TX pending packet queue structure */
	athr->tx_pending_pkts = linked_list_init(athr->tx_pending_pkts);
	
	/* Initialize TX pending packet queue structure */
	athr->tx_pending_soft_beacon = linked_list_init(athr->tx_pending_soft_beacon);
	
	/* Initialize RX pending packet queue structure */
	athr->rx_pending_pkts = linked_list_init(athr->rx_pending_pkts);
	
	/* Initialize AR9170 list of wake neighbors*/
	athr->ps_mgr.wake_neighbors_list = linked_list_init(athr->ps_mgr.wake_neighbors_list);
	
	/* Initially the AR9170 device is in a TX Period. */
	athr->ps_mgr.psm_state = AR9170_TX_WINDOW;
	
	/* Clear the ATIM creation flag. */
	athr->ps_mgr.create_atims_flag = false;
	
	/* Clear flag of awake node erase. */
	athr->erase_awake_nodes_flag = false;
	
	/* Clear the soft beacon transmission flag. */
	athr->ps_mgr.send_soft_bcn_flag = false;
	
	/* Zero DTIM counter */
	athr->ps.dtim_counter = 0;
	
	// Initialize RNG
	athr->rng.initialized = false;
	athr->vifs = 0;
	athr->vif_bitmap = 0;		
	
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: ar9170_alloc: Set state to stopped.\n");
	#endif
	ar9170_set_state(athr, AR9170_STOPPED);

	for (i=0; i<AR9170_MAX_VIRTUAL_MAC; i++) {
		athr->vif_priv[i].id = 0;
		athr->vif_priv[i].vif = NULL;
	}		
	
	athr->rx_software_decryption = false;
	athr->sniffer_enabled = 0;
	
	athr->ps.update_mask = 0;
	athr->beacon_ctrl = false;
	athr->beacon_cancel = false;
	
	athr->clear_filtering = false;

	// Allocate memory for the firmware struct
	struct firmware *firmw = smalloc(sizeof(struct firmware));
	if (!firmw) {
		printf("ERROR: Could not allocate memory for the AR9170 firmware struct.\n");
		return NULL;
	}
	/* Firmware setting */
	firmw->data = firmware_image;
	firmw->size = firmware_image_len;
	athr->fw.fw = firmw;

	/* Initialize off_override flag to zero [so we force PS if necessary] */
	memset(&athr->ps.off_override, 0,sizeof(unsigned int));
	athr->ps.state = false; //initially power-saving is deactivated 
	
	/* Zero the PS transition pending flags. */
	athr->ps_mgr.psm_transit_to_sleep = false;
	athr->ps_mgr.psm_transit_to_wake = false;
	/* Assign pointer to global variable */
	*ar_pt = athr;
	
	return *ar_pt;
}



int ar9170_parse_eeprom( struct ar9170* ar )
{
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: Parsing the EEPROM...\n");
	#endif
	
	struct ath_regulatory *regulatory = &ar->common.regulatory;
	unsigned int rx_streams, tx_streams, tx_params = 0;
	int bands = 0;
	int chans = 0;
	
	if (ar->eeprom.length == cpu_to_le16(0xffff)) {
		printf("ERROR: EEPROM Empty.\n");
		return -ENODATA;
	}
		
	rx_streams = hweight8(ar->eeprom.rx_mask);
	tx_streams = hweight8(ar->eeprom.tx_mask);
		
	if (rx_streams != tx_streams) {
		tx_params = IEEE80211_HT_MCS_TX_RX_DIFF;

		if(!(tx_streams >= 1 && tx_streams <= IEEE80211_HT_MCS_TX_MAX_STREAMS)) {
			printf("WARNING: tx_streams has wrong value.\n");
		}			

		tx_params = (tx_streams - 1) <<
			    IEEE80211_HT_MCS_TX_MAX_STREAMS_SHIFT;

		carl9170_band_2GHz.ht_cap.mcs.tx_params |= tx_params;
		carl9170_band_5GHz.ht_cap.mcs.tx_params |= tx_params;
	}

	if (ar->eeprom.operating_flags & AR9170_OPFLAG_2GHZ) {
		ar->hw->wiphy->bands[IEEE80211_BAND_2GHZ] =
			&carl9170_band_2GHz;
		chans += carl9170_band_2GHz.n_channels;
		bands++;
	}

	if (ar->eeprom.operating_flags & AR9170_OPFLAG_5GHZ) {
		ar->hw->wiphy->bands[IEEE80211_BAND_5GHZ] =
			&carl9170_band_5GHz;
		chans += carl9170_band_5GHz.n_channels;
		bands++;
	}

	if (!bands)
		return -EINVAL;

	ar->survey = smalloc(sizeof(struct survey_info) * chans); //kzalloc(sizeof(struct survey_info) * chans, GFP_KERNEL);
	memset(ar->survey, 0, sizeof(struct survey_info) * chans);
	if (!ar->survey)
		return -ENOMEM;
	ar->num_channels = chans;

	/*
	 * I measured this, a bandswitch takes roughly
	 * 135 ms and a frequency switch about 80.
	 *
	 * FIXME: measure these values again once EEPROM settings
	 *	  are used, that will influence them!
	 */
	if (bands == 2)
		ar->hw->channel_change_time = 135 * 1000;
	else
		ar->hw->channel_change_time = 80 * 1000;

	regulatory->current_rd = le16_to_cpu(ar->eeprom.reg_domain[0]);

	/* second part of wiphy init */
	SET_IEEE80211_PERM_ADDR(ar->hw, ar->eeprom.mac_address);
	
	/* Copy the permanent address to the virtual interface */
	memcpy(unique_vif->addr, hw->wiphy->perm_addr, ETH_ALEN); 

	return true;
	
}


int ar9170_register_device(struct ar9170* ar)
{
	bool result = false;
	int i, err;
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: Registering AR9170 device...\n");
	#endif
	struct ath_regulatory *regulatory = &ar->common.regulatory;
	
	ar->mem_bitmap = smalloc(sizeof(unsigned long));
	
	if (!ar->mem_bitmap)
		return -ENOMEM;
	
	/* try to read EEPROM, init MAC addr */
	result = ar9170_read_eeprom(ar);
	if (result == false) {
		printf("ERROR: Read EEPROM returned errors.\n");
		return result;
	}		
		
	/* parse the EEPROM */
	result = ar9170_parse_eeprom(ar);	
	if (result == false) {
		printf("ERROR: Parse EEPROM returned errors.\n");
		return result;
	}			
		
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: MAC Address: ");
	for (i=0; i<ETH_ALEN; i++) {
		printf("%02x ",(ar->hw->wiphy->perm_addr)[i]);
	}
	printf(" \n");	
	#endif
	
#if 0 //FIXME - check what is important here	
	err = ath_regd_init(regulatory, ar->hw->wiphy,
	carl9170_reg_notifier);
	if (err)
		return err;

	if (modparam_noht) {
		carl9170_band_2GHz.ht_cap.ht_supported = false;
		carl9170_band_5GHz.ht_cap.ht_supported = false;
	}

	for (i = 0; i < ar->fw.vif_num; i++) {
		ar->vif_priv[i].id = i;
		ar->vif_priv[i].vif = NULL;
	}

	err = ieee80211_register_hw(ar->hw);
	if (err)
	return err;

	/* mac80211 interface is now registered */
	ar->registered = true;

	if (!ath_is_world_regd(regulatory))
	regulatory_hint(ar->hw->wiphy, regulatory->alpha2);

	#ifdef CONFIG_CARL9170_DEBUGFS
	carl9170_debugfs_register(ar);
	#endif /* CONFIG_CARL9170_DEBUGFS */
#endif

	result = ar9170_led_init(ar);
	if (result == false) {
		printf("ERROR: Could not initialize LEDS.\n");
		goto err_unreg;
	}
	
#ifdef CONFIG_CARL9170_LEDS // TODO-enable this
	result = carl9170_led_register(ar);
	if (result == false	) {
		printf("ERROR: Could not register LEDS.\n");
		goto err_unreg;
	}		
#endif /* CONFIG_CARL9170_LEDS */	

#ifdef CONFIG_CARL9170_WPC // TODO-enable this
	result = carl9170_register_wps_button(ar);
	if (result == false)
	goto err_unreg;
#endif /* CONFIG_CARL9170_WPC */

#ifdef CONFIG_AR9170_HWRNG
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: Registering hw rng.\n");
	#endif
	err = ar9170_register_hwrng(ar);
	if (err) {
		printf("ERROR: Registering hw rng returned errors.\n");
		result = false;
		goto err_unreg;
	}		
#endif /* CONFIG_CARL9170_HWRNG */	
	
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: AR9170 device is now registered.\n");
	#endif
	
	return result;
err_unreg:	
	printf("ERROR: Must now unregister device.\n");
	return result;
}



bool ar9170_read_eeprom( struct ar9170* ar )
{	
#define RW	8 /* Number of words to read at once */
#define RB	(sizeof(U32) * RW)
	U8 *eeprom = (void*) &ar->eeprom;
	le32_t offsets[RW];
	int i, j;
	bool result;
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: Target eeprom size: %d. Total commands to send: %d.\n",sizeof(ar->eeprom), sizeof(ar->eeprom)/RB);	
	#endif
	if (sizeof(ar->eeprom) & 3) {
		printf("BUG: EEPROM Size is not correct.\n");
	}
	
	if (RB > INTR_ENDPOINT_MAX_SIZE - AR9170_CMD_HDR_LEN ) {
		printf("BUG: Read block exceeds maximum endpoint transfer size.\n");
	}
	
	if (sizeof(ar->eeprom) % RB ) {
		printf("BUG: RB does not perfectly multiply the EEPROM size.\n");
	}
	
	for (i = 0; i < sizeof(ar->eeprom) / RB; i++) {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: Reading block: %d.\n",i);
		#endif
		for (j = 0; j < RW; j++)
			offsets[j] = cpu_to_le32(AR9170_EEPROM_START + RB * i + 4 * j);

		result = ar9170_exec_cmd(ar, CARL9170_CMD_RREG, RB, (U8 *) &offsets, RB, eeprom + RB * i);
		if (result == false) {
			printf("ERROR: Execute command returned error.\n");
			return result;
		}
		
	}

#undef RW
#undef RB
	return result;
}


bool ar9170_init_device(struct ar9170* ar)
{
	int result;
	
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: Initializing USB device...\n");
	#endif
	
	
	/*
	 * The carl9170 firmware let's the driver know when it's
	 * ready for action. But we have to be prepared to gracefully
	 * handle all spurious [flushed] messages after each (re-)boot.
	 * Thus the command response counter remains disabled until it
	 * can be safely synchronized.
	 */
	ar->cmd_seq = -2;
	
	/*
	 * Initialize listening on the bulk endpoint.
	 * TODO perhaps try to initialize more than
	 * one parallel listening USB requests.
	 */
	ar9170_listen_on_bulk_in();
	
	delay_ms(10);
	
	/*
	 * Initialize listening on the interrupt endpoint.
	 */
	ar9170_listen_on_interrupt_in();
			
	delay_ms(10);
	
	ar9170_usb_open(ar);

	// Start routine for firmware upload
	result = ar9170_upload_firmware(ar);
	if (!result) {
		printf("ERROR: Uploading firmware returned errors.\n");
		return result;
	}
	
	delay_ms(100);
	
	/* Now, start the command response counter */
	ar->cmd_seq = -1;
	
	// Attempt echo test [synchronous]
	result = ar9170_echo_test(ar); 
	if(!result) {
		printf("ERROR: Wrong ECHO!\n");
		return result;
	}
/*	
	// Set device state to stopped; TODO - check if a better place can be found
	
	#if USB_MAIN_DEBUG
	printf("DEBUG: Set state to stopped.\n");
	#endif
	ar9170_set_state(ar, AR9170_STOPPED);	
*/	
	return result;
} 


bool ar9170_usb_open(struct ar9170* ar) 
{
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: USB open. Device transits from Stopped to Idle.\n");
	#endif
	
	ar9170_set_state_when(ar, AR9170_STOPPED, AR9170_IDLE);
	
	return false;
}

bool ar9170_op_start(struct ar9170* ar) // FIXME - the argument should be of type ieee80211_hw, although not really important...
{
	int err,i;
	
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: ar9170_op_start().\n");
	printf("DEBUG: Initial device state: %d.\n",ar->state);
	#endif
	
	__lock_acquire(&ar->mutex_lock);

	//carl9170_zap_queues(ar); TODO - check if this is necessary and why

	/* reset QoS defaults */
	CARL9170_FILL_QUEUE(ar->edcf[AR9170_TXQ_VO], 2, 31, 1023,  0);
	CARL9170_FILL_QUEUE(ar->edcf[AR9170_TXQ_VI], 2, 31, 1023,  0);
	CARL9170_FILL_QUEUE(ar->edcf[AR9170_TXQ_BE], 2, 31, 1023,  0);
	CARL9170_FILL_QUEUE(ar->edcf[AR9170_TXQ_BK], 2, 31, 1023,  0);
	CARL9170_FILL_QUEUE(ar->edcf[AR9170_TXQ_SPECIAL], 2, 3, 7, 0);
	
	ar->filter_state = 0;
	ar->usedkeys = 1;
	ar->rx_software_decryption = false;
	ar->disable_offload = false;
	// TODO - complete this initialization [there are more]
	
	ar->mem_allocs = 0;
	
	/* Open the USB device */	
	err = ar9170_usb_open(ar);
	if (err) {
		printf("ERROR: Opening USB device returned errors.\n");
		goto out;
	}
	
	/* Initialize MAC */	
	err = ar9170_init_mac(ar);
	if (err) {
		printf("ERROR: Initializing MAC returned errors.\n");
		goto out;
	}
	
	/* Resetting MAC QoS */
	err = ar9170_set_qos(ar);
	if (err) {
		printf("ERROR: Setting QoS returned errors.\n");
		goto out;
	}
	
	/* Initializing RX filtering */
	if (ar->fw.rx_filter) {
		#if AR9170_MAIN_DEBUG
		printf("DEBUG: Filter RX.\n");
		#endif
		/* In the initialization phase we do not need to handle packet notifications. */
		err = ar9170_rx_filter(ar,  AR9170_RX_FILTER_OTHER_RA | 
									AR9170_RX_FILTER_CTL_OTHER |
									AR9170_RX_FILTER_BAD | 
									AR9170_RX_FILTER_CTL_BACKR | 
									AR9170_RX_FILTER_DATA | 
									AR9170_RX_FILTER_MGMT |
									AR9170_RX_FILTER_DECRY_FAIL);
		if (err) {
			printf("ERROR: Setting RX filter returned errors.\n");
			goto out;
		}	
	}	
	/* Initialize DMA */
	err = !ar9170_write_rgstr(ar, AR9170_MAC_REG_DMA_TRIGGER,
	AR9170_DMA_TRIGGER_RXQ);
	if (err) {
		printf("ERROR: Triggering DMA returned errors.\n");
		goto out;
	}
	
	/* Clear key-cache */
	for (i = 0; i < AR9170_CAM_MAX_USER + 4; i++) {
		err = ar9170_upload_key(ar, i, NULL, AR9170_ENC_ALG_NONE,
		0, NULL, 0);
		if (err) {
			printf("ERROR: Uploading key returned errors.\n");	
			goto out;	
		}
		
		err = ar9170_upload_key(ar, i, NULL, AR9170_ENC_ALG_NONE,
		1, NULL, 0);
		if (err) {
			printf("ERROR: Uploading key returned errors.\n");
			goto out;
		}
		
		if (i < AR9170_CAM_MAX_USER) {
			err = ar9170_disable_key(ar, i);
			if (err) {
				printf("ERROR: Disabling key returned errors.\n");
				goto out;
			}		
		}
	}
	/* Set device state to STARTED */	
	ar9170_set_state_when(ar, AR9170_IDLE, AR9170_STARTED);	
	
	/* Initialize real timer for ATIM interruptions */
	ar9170_psm_init_rtimer();
	
#if 0	// FIXME - decide what to do with it. This code does not send commands to the device.
	ieee80211_queue_delayed_work(ar->hw, &ar->stat_work,
	round_jiffies(msecs_to_jiffies(CARL9170_STAT_WORK)));

	ieee80211_wake_queues(ar->hw);
#endif
	err = 0;
	
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: ar9170_op_start() completed.\n");
	#endif
out: 
	__lock_release(&ar->mutex_lock);
	if (err)
		return false;
	else
		return true;	
}


#ifdef CONFIG_AR9170_HWRNG
int ar9170_rng_get(struct ar9170 *ar)
{
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: Getting hw rng...\n");
	#endif
	#define RW	(AR9170_MAX_CMD_PAYLOAD_LEN / (3*sizeof(U32))) // XXX - /3 in order to fit
	#define RB	(AR9170_MAX_CMD_PAYLOAD_LEN / 3) // XXX - /3 in order to fit

	static const le32_t rng_load[RW] = {
	[0 ... (RW - 1)] = cpu_to_le32(AR9170_RAND_REG_NUM)};

	U32 buf[RW];

	unsigned int i, off = 0, transfer, count;
	int result;
	
	if (RB > AR9170_MAX_CMD_PAYLOAD_LEN) {
		printf("BUG: RB > AR9170_MAX_CMD_PAYLOAD_LEN.\n");//BUILD_BUG_ON(RB > CARL9170_MAX_CMD_PAYLOAD_LEN);	
	}
	
	if (!IS_ACCEPTING_CMD(ar) || !ar->rng.initialized) {
		printf("ERROR: RNG-get shall be called again.\n");
		return -EAGAIN;	
	}	

	count = ARRAY_SIZE(ar->rng.cache);
	
	while (count) {
		printf("count: %d.\n",count);
		result = ar9170_exec_cmd(ar, CARL9170_CMD_RREG,
		RB, (U8 *) rng_load,
		RB, (U8 *) buf);
		if (result == false) {
			printf("ERROR: Could not get the hw rng.\n");
			return -1;	
		}
		
		transfer = min((unsigned int)count, (unsigned int)RW);
		for (i = 0; i < transfer; i++)
		ar->rng.cache[off + i] = buf[i];

		off += transfer;
		count -= transfer;
	}

	ar->rng.cache_idx = 0;

	#undef RW
	#undef RB
	return 0;
}

int ar9170_rng_read(struct hwrng *rng, U32 *data)
{
	struct ar9170 *ar = (struct ar9170 *)rng->priv;
	int ret = -EIO;

	//mutex_lock(&ar->mutex);
	__lock_acquire(&ar->mutex);
	if (ar->rng.cache_idx >= ARRAY_SIZE(ar->rng.cache)) {
		ret = ar9170_rng_get(ar);
		if (ret) {
			//mutex_unlock(&ar->mutex);
			__lock_release(&ar->mutex);
			return ret;
		}
	}

	*data = ar->rng.cache[ar->rng.cache_idx++];
	__lock_release(&ar->mutex);//mutex_unlock(&ar->mutex);

	return sizeof(U16);
}

void ar9170_unregister_hwrng(struct ar9170 *ar)
{
	if (ar->rng.initialized) {
		// hwrng_unregister(&ar->rng.rng); FIXME
		ar->rng.initialized = false;
	}
}

int ar9170_register_hwrng(struct ar9170 *ar)
{
	int err;

	//snprintf(ar->rng.name, ARRAY_SIZE(ar->rng.name),"%s_%s", KBUILD_MODNAME, wiphy_name(ar->hw->wiphy));
	ar->rng.rng.name = "GLARIOAN";//ar->rng.name;
	ar->rng.rng.data_read = ar9170_rng_read;
	ar->rng.rng.priv = (unsigned long)ar;

	if (ar->rng.initialized) {
		printf("WARNING: RNG already initialized.\n");
		return -EALREADY;
	}	

	err = false; //hwrng_register(&ar->rng.rng); FIXME
	if (err) {
		printf("ERROR: Failed to register the random number generator (%d)\n", err);
		return err;
	}

	ar->rng.initialized = true;

	err = ar9170_rng_get(ar);
	if (err) {
		ar9170_unregister_hwrng(ar);
		return err;
	}

	#if AR9170_MAIN_DEBUG
	printf("DEBUG: hw rng registered successfully.\n");
	#endif
	return 0;
}

#endif /* CONFIG_CARL9170_HWRNG */



int ar9170_op_add_interface(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif)
{
	#if AR9170_MAIN_DEBUG 
	printf("DEBUG: Adding an ar9170 interface.\n");
	#endif
	
	struct ar9170_vif_info *vif_priv = (void *) vif->drv_priv;
	struct ieee80211_vif *main_vif = NULL;
	struct ar9170 *ar = hw->priv;
	int vif_id = -1, err = 0;

	/* XXX - this was necessary. Otherwise I do not know how to pass this reference. */
	if (!vif_priv) {
		
		vif_priv = unique_cvif; //(struct ar9170_vif_info*)malloc(sizeof(struct ar9170_vif_info));
		vif_priv->active = false;
	} else {
		vif_priv->active = false;
	}
	
	__lock_acquire(&ar->mutex_lock);
	__read_lock();
	
	if (vif_priv->active == true) {
		#if AR9170_MAIN_DEBUG
		printf("DEBUG: Virtual interface info already active.\n");
		#endif
		/*
		 * Skip the interface structure initialization,
		 * if the vif survived the _restart call.
		 */
		vif_id = vif_priv->id;
		vif_priv->enable_beacon = false;

		__lock_acquire(&ar->beacon_lock);
		
		if (vif_priv->beacon) {
			if(vif_priv->beacon->data) {
				free(vif_priv->beacon->data);
			}
			free(vif_priv->beacon);	
		}		
		vif_priv->beacon = NULL;
		__lock_release(&ar->beacon_lock);

		goto init;
	}

	main_vif = ar9170_get_main_vif(ar); 

	if (main_vif && main_vif->prepared) { // Altered by John, to add the prepared flag
		switch (main_vif->type) {
		case NL80211_IFTYPE_STATION:
			#if AR9170_MAIN_DEBUG
			printf("DEBUG: Device is a managed station.\n");
			#endif
			if (vif->type == NL80211_IFTYPE_STATION)
				break;

			err = -EBUSY;
			__read_unlock();

			goto unlock;

		case NL80211_IFTYPE_AP:
			if ((vif->type == NL80211_IFTYPE_STATION) ||
			    (vif->type == NL80211_IFTYPE_WDS) ||
			    (vif->type == NL80211_IFTYPE_AP))
				break;

			err = -EBUSY;
			__read_unlock();
			goto unlock;

		default:
			__read_unlock();
			goto unlock;
		}
	}

	vif_id = 0;// FIXME later this is easy: bitmap_find_free_region(&ar->vif_bitmap, ar->fw.vif_num, 0);

	if (vif_id < 0) {
		__read_unlock();
		printf("ERROR: No space available!.\n");
		err = -ENOSPC;
		goto unlock;
	}

	if(ar->vif_priv[vif_id].id != vif_id)
		printf("BUG: The virtual interface ids should match.\n");

	vif_priv->active = true;
	vif_priv->id = vif_id;
	vif_priv->enable_beacon = false;
	ar->vifs++;
	
	//list_add_tail_rcu(&vif_priv->list, &ar->vif_list); Instead of this we do:
	ar->vif_list.next = (struct list_head*)&vif_priv; // This is dangerous XXX
	
	// rcu_assign_pointer(ar->vif_priv[vif_id].vif, vif); Instead of this, we do:
	ar->vif_priv[vif_id].vif = vif;

init:
	if (ar9170_get_main_vif(ar) == vif) {
		ar->beacon_iter = vif_priv;
		//rcu_assign_pointer(ar->beacon_iter, vif_priv);
		__read_unlock();

		#if AR9170_MAIN_DEBUG
		printf("DEBUG: Will initialize interface.\n");
		#endif
		
		err = ar9170_init_interface(ar, vif);
		if (err) {
			printf("ERROR: ar9170_init_interface returned errors.\n");
			goto unlock;
		}			
	} else {
		__read_unlock();
		printf("ERROR: Could not find the virtual interface.\n");
		//err = carl9170_mod_virtual_mac(ar, vif_id, vif->addr); TODO- add support for this, although, probably not urgent

		if (err)
			goto unlock;
	}

	if (ar->fw.tx_seq_table) {
		#if AR9170_MAIN_DEBUG
		printf("DEBUG: Writing on the tx_seq_table: %d.\n",vif_id);
		#endif
		err = !ar9170_write_rgstr(ar, ar->fw.tx_seq_table + vif_id * 4, 0);
		if (err)
			goto unlock;
	}

unlock: // XXX These are all dangerous here- but hopefully we do not need to enter
	if (err && (vif_id >= 0)) {
		printf("ERROR: Adding interface resulted in errors.\n");
		vif_priv->active = false;
		//bitmap_release_region(&ar->vif_bitmap, vif_id, 0);
		ar->vifs--;
		//RCU_INIT_POINTER(ar->vif_priv[vif_id].vif, NULL);
		//list_del_rcu(&vif_priv->list);
		__lock_release(&ar->mutex);
		//synchronize_rcu();
	} else {
		if (ar->vifs > 1) {
			#if AR9170_MAIN_DEBUG
			printf("DEBUG: Number of virtual interfaces are more than one.\n");
			#endif
			
			// Reason to override PS-off is having more than one virtual interfaces
			ar->ps.off_override |= PS_OFF_VIF; 
		}
		__lock_release(&ar->mutex);
	}
	
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: Adding interface returns: %d.\n",err);
	#endif
	return err;
}


struct ieee80211_vif *ar9170_get_main_vif(struct ar9170 *ar)
{
	return unique_vif; // TODO - allow more virtual interfaces than just the global
}


int ar9170_init_interface(struct ar9170 *ar, struct ieee80211_vif *vif)
{
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: ar9170_init_interface().\n");
	#endif
	bool modparam_nohwcrypt = true; // FIXME - did this manually
	
	struct ath_common *common = &ar->common;
	int err,i;

	if (!vif) {
		if(IS_STARTED(ar))
			printf("WARNING: Device has already started!\n");
		return 0;
	}
	/* Copy MAC address */
	memcpy(common->macaddr, vif->addr, ETH_ALEN); 

	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: Copied permanent address: \n");
	for (i=0; i<ETH_ALEN; i++)
		printf("%02x : ",common->macaddr[i]);
	printf(" \n");
	#endif
		
	if (modparam_nohwcrypt ||
	((vif->type != NL80211_IFTYPE_STATION) &&
	(vif->type != NL80211_IFTYPE_AP))) {
		ar->rx_software_decryption = true;
		ar->disable_offload = true;
	}
	/* Set MAC operating mode */
	err = ar9170_set_operating_mode(ar);
	return err;
}




int ar9170_op_conf_tx(struct ieee80211_hw *hw,struct ieee80211_vif *vif, U16 queue,
const struct ieee80211_tx_queue_params *param)
{
	#if AR9170_MAIN_DEBUG_DEEP
	printf("carl9170_op_conf_tx. queue: %d, parameters: %d, %d, %d, %d, %d.\n",queue, param->aifs, param->cw_max,
	param->cw_min, param->txop, param->uapsd);
	#endif
	
	struct ar9170 *ar = hw->priv;
	int ret;

	__lock_acquire(&ar->mutex_lock);
	if (queue < ar->hw->queues) {
		memcpy(&ar->edcf[ar9170_qmap[queue]], param, sizeof(*param));
		ret = ar9170_set_qos(ar);
	} else {
		printf("ERROR: Invalid argument.\n");
		ret = -EINVAL;
	}
	__lock_release(&ar->mutex_lock);
	return ret;
}


void ar9170_op_flush(struct ieee80211_hw *hw, bool drop)
{
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: ar9170_op_flush.\n");
	#endif
	
	struct ar9170 *athr = hw->priv;
	unsigned int vid;

	__lock_acquire(&athr->mutex_lock);
	
	//FIXME for_each_set_bit(vid, &ar->vif_bitmap, ar->fw.vif_num) - but we have only one
	vid = 0;
	ar9170_flush_cab(athr, vid);

	// carl9170_flush(ar, drop); FIXME XXX important!
	__lock_release(&athr->mutex_lock);
}



int ar9170_op_config(struct ieee80211_hw *hw, U32 changed)
{
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: ar9170_op_config.\n");
	#endif
	/* Reference the ar9170 structure */
	struct ar9170 *ar = hw->priv;
	
	int err = 0;

	__lock_acquire(&ar->mutex_lock);
	if (changed & IEEE80211_CONF_CHANGE_LISTEN_INTERVAL) {
		/* TODO */
		#if AR9170_MAIN_DEBUG
		printf("DEBUG: Should change the listen interval.\n");
		#endif
		err = 0;
	}

	if (changed & IEEE80211_CONF_CHANGE_PS) {
		err = ar9170_ps_update(ar);
		if (err) {
			printf("ERROR: PS update returned errors.\n");
			goto out;
		}			
	}

	if (changed & IEEE80211_CONF_CHANGE_SMPS) {
		/* TODO */
		err = 0;
	}

	if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: op_confic: Change channel.\n");
		#endif
		/* adjust slot time for 5 GHz */
		err = ar9170_set_slot_time(ar);
		if (err)
			goto out;

		err = ar9170_update_survey(ar, true, false);
		if (err) {
			printf("ERROR: ar9170 update survey returned errors.\n");
			goto out;	
		}		

		err = ar9170_set_channel(ar, hw->conf.channel, hw->conf.channel_type, CARL9170_RFI_NONE);
		if (err) {
			printf("ERROR: PHY could not set the channel.\n");	
			goto out;
		}
		
		err = ar9170_update_survey(ar, false, true);
		if (err) {
			printf("ERROR: ar9170 update survey returned errors.\n");
			goto out;
		}

		err = ar9170_set_dyn_sifs_ack(ar);
		if (err) {
			printf("ERROR: ar9170 setting dynamic SIFS/ACK returned errors.\n");
			goto out;
		}

		err = ar9170_set_rts_cts_rate(ar);
		if (err) {
			printf("ERROR: ar9170 setting RTS/CTS returned errors.\n");
			goto out;
		}
		
	}

	if (changed & IEEE80211_CONF_CHANGE_POWER) {
		err = ar9170_set_mac_tpc(ar, ar->hw->conf.channel);
		if (err) {
			printf("ERROR: ar9170 setting MAC TX power returned errors.\n");
			goto out;
		}
	}

	out:
	__lock_release(&ar->mutex_lock);
	return err;
}

/* This function is called inside interrupt context. */
int ar9170_ps_update(struct ar9170 *ar)
{
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: PS State update requested. Current state: %d.\n", ar->ps.state);
	#endif
		
	bool ps = false;
	/* Check if there is any reason to remain awake. */
	if (!ar->ps.off_override) {
		/* If there is not reason to remain awake, check if the device operates in PSM. */
		ps = (ar->hw->conf.flags & IEEE80211_CONF_PS);
		/* If the device is in PSM and there is no reason to override it, the ps flag is true. */
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: No reason to override OFF the PS state request. New requested state: %d.\n", ps);
		#endif		
	} else {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: OFF override flag is true. No power save.\n");
		#endif
	}			
	/* If the ps flag is set and the ps state is false, means that we are
	 * going to switch to PS, since currently the device RF is ON.
	 */	
	if (ps != ar->ps.state) {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: PS State must change to: %d.\n", ps);
		#endif
		ar9170_psm_schedule_powersave(ar, ps);
	
		if (ar->ps.state && !ps) {
			//FIXME ar->ps.sleep_ms = jiffies_to_msecs(jiffies - ar->ps.last_action);
		}
		if (ps) {}
			//FIXME ar->ps.last_slept = jiffies;
		//FIXME ar->ps.last_action = jiffies;
		
		/* This updates already the PS state of the AR9170. */
	/*	
		ar->ps.state = ps;
	*/
		
		if (ps == true) {
			/* We choose NOT to set the device's RF state to OFF 
			 * at this point, since the actual transfer has not 
			 * occurred yet. But we set the transit flag, which 
			 * will prevent this node from attempting any frame
			 * transition already from now.
			 */
			if (ar->ps_mgr.psm_transit_to_sleep == false) {
				
				ar->ps_mgr.psm_transit_to_sleep = true;
			
			} else {
				printf("ERROR: We already expect a transit to SLEEP!\n");
			}
		
		} else {
			/* We choose NOT to set the device's RF state to ON 
			 * at this point, since the actual transfer has not 
			 * occurred yet. But we set the transit flag, which 
			 * will prevent this node from attempting any frame
			 * transition until the power-save has been enabled.
			 */
			if (ar->ps_mgr.psm_transit_to_wake == false) {
				
				ar->ps_mgr.psm_transit_to_wake = true;
				
			} else {
				printf("ERROR: We already expect a transit!\n");
			}
		}		
	}
	return 0;
}



int ar9170_update_survey(struct ar9170 *ar, bool flush, bool noise)
{
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: ar9170_update_survey.\n");
	#endif
	
	int err = 0;

	if (noise) {
		
		err = ar9170_get_noisefloor(ar);
		
		if (err) {
			printf("ERROR: Noise floor could not be extracted.\n");
			return err;
		}			
	}

	if (ar->fw.hw_counters) {
		
		err = ar9170_collect_tally(ar);
		if (err) { 
			printf("ERROR: ar9170_collect_tally returned errors.\n");
			return err;
		}		
	}

	if (flush) {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: Flushing [zeroing].\n");
		#endif
		memset(&ar->tally, 0, 5*sizeof(uint64_t));//sizeof(ar->tally));
	}
	
	return 0;
}


void ar9170_op_configure_filter(struct ieee80211_hw *hw,
					 unsigned int changed_flags,
					 unsigned int *new_flags,
					 U64 multicast)
{
	#if AR9170_MAIN_DEBUG
	printf("ar9170_op_configure_filter_start.\n");
	#endif
	
	struct ar9170 *ar = hw->priv;

	/* mask supported flags */
	*new_flags &= FIF_ALLMULTI | ar->rx_filter_caps;

	if (!IS_ACCEPTING_CMD(ar)) {
		printf("WARNING: Not accepting commands.\n");
		return;
	}		

	__lock_acquire(&ar->mutex_lock);

	ar->filter_state = *new_flags;
	/*
	 * We can support more by setting the sniffer bit and
	 * then checking the error flags, later.
	 */

	if (*new_flags & FIF_ALLMULTI)
		multicast = ~0ULL;

	printf("Multicast: %llu.\n", multicast);
	
	if (multicast != ar->cur_mc_hash)
		if(ar9170_update_multicast(ar, multicast)) {
			printf("WARNING: ar9170_update_multicast returned error.\n");
		}

	if (changed_flags & (FIF_OTHER_BSS | FIF_PROMISC_IN_BSS)) {
		ar->sniffer_enabled = !!(*new_flags &
			(FIF_OTHER_BSS | FIF_PROMISC_IN_BSS));

		if(ar9170_set_operating_mode(ar)) {
			printf("WARNING: Inside rx config filters: set_operating_mode returned errors.\n");
		}
	}

	if (ar->fw.rx_filter && changed_flags & ar->rx_filter_caps) {
		U32 rx_filter = 0;

		if (!ar->fw.ba_filter)
			rx_filter |= AR9170_RX_FILTER_CTL_OTHER;

		if (!(*new_flags & (FIF_FCSFAIL | FIF_PLCPFAIL)))
			rx_filter |= AR9170_RX_FILTER_BAD;

		if (!(*new_flags & FIF_CONTROL))
			rx_filter |= AR9170_RX_FILTER_CTL_OTHER;

		if (!(*new_flags & FIF_PSPOLL))
			rx_filter |= AR9170_RX_FILTER_CTL_PSPOLL;

		if (!(*new_flags & (FIF_OTHER_BSS | FIF_PROMISC_IN_BSS))) {
			rx_filter |= AR9170_RX_FILTER_OTHER_RA;
			rx_filter |= AR9170_RX_FILTER_DECRY_FAIL;
		}

		if(ar9170_rx_filter(ar, rx_filter)) {
			printf("WARNING: update filter returned errors.\n");
		}
	}

	__lock_release(&ar->mutex_lock);
}


void ar9170_op_bss_info_changed(struct ieee80211_hw *hw,
					 struct ieee80211_vif *vif,
					 struct ieee80211_bss_conf *bss_conf,
					 U32 changed)
{
	#if AR9170_MAIN_DEBUG_DEEP
	printf("ar9170_op_bss_info_changed begins. Change Flag: %08x.\n",(unsigned int)changed);
	#endif
	
	struct ar9170 *ar = hw->priv;
	struct ath_common *common = &ar->common;
	int err = 0;
	struct ar9170_vif_info *vif_priv;
	struct ieee80211_vif *main_vif;

	__lock_acquire(&ar->mutex_lock);
	vif_priv = (void *) vif->drv_priv;
	main_vif = ar9170_get_main_vif(ar);
	if (!main_vif) {
		printf("WARNING: Main virtual interface is null.\n");
		goto out;
	}
	
	if (changed & BSS_CHANGED_BEACON_ENABLED) {
		
		#if AR9170_MAIN_DEBUG_DEEP
		printf("bss_changed_beacon_enabled_flag set.\n");
		#endif
		
		//struct ar9170_vif_info *iter;
		int i = 0;

		vif_priv->enable_beacon = bss_conf->enable_beacon;
		__read_lock();
	/* FIXME . what is this exactly?	
		list_for_each_entry_rcu(iter, &ar->vif_list, list) {
			if (iter->active && iter->enable_beacon)
				i++;

		}
	*/
		ar->beacon_enabled = true; // Changed from -i-
		__read_unlock();		
	}

	if (changed & BSS_CHANGED_BEACON) {
		
		#if AR9170_MAIN_DEBUG_DEEP
		printf("bss_changed_beacon_flag set. Will now update beacon.\n");
		#endif
		
		/* Update beacon content but do not submit it to device yet */
		err = ar9170_update_beacon(ar, false);
		if (err) {
			printf("ERROR: update beacon returned errors.\n");
			goto out;
		}
		
		/* Submit beacon control command down XXX we do not do it yet. */			
		//ar9170_send_beacon_cmd(ar);
	}
	
	if (changed & (BSS_CHANGED_BEACON_ENABLED | BSS_CHANGED_BEACON |
		       BSS_CHANGED_BEACON_INT)) {

		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: At least one of bss_changed_beacon_enabled/beacon/beacon_int_flags set.\n");
		#endif
		
		if (main_vif != vif) {
			bss_conf->beacon_int = main_vif->bss_conf.beacon_int;
			bss_conf->bcn_ctrl_period = main_vif->bss_conf.bcn_ctrl_period;
			bss_conf->dtim_period = main_vif->bss_conf.dtim_period;
		}

		/*
		 * Therefore a hard limit for the broadcast traffic should
		 * prevent false alarms.
		 */
		if (vif->type != NL80211_IFTYPE_STATION &&
		    (bss_conf->beacon_int * bss_conf->dtim_period >=
		     (AR9170_QUEUE_STUCK_TIMEOUT / 2))) {
			err = -EINVAL;
			goto out;
		}

		err = ar9170_set_beacon_timers(ar);
		if (err) {
			printf("ERROR: Beacon timers could not be set.\n");
			goto out;
		}			
	}
	
	if (changed & BSS_CHANGED_HT) {
		/* TODO */
		err = 0;
		if (err)
			goto out;
	}

	if (main_vif != vif)
		goto out;

	/*
	 * The following settings can only be changed by the
	 * master interface.
	 */

	if (changed & BSS_CHANGED_BSSID) {
		#if AR9170_MAIN_DEBUG
		printf("bss_changed_bssid_flag set.\n");
		#endif
		
		memcpy(common->curbssid, bss_conf->bssid, ETH_ALEN);
		err = ar9170_set_operating_mode(ar);
		if (err)
			goto out;
	}

	if (changed & BSS_CHANGED_ASSOC) {
		#if AR9170_MAIN_DEBUG
		printf("bss_changed_assoc_flag set.\n");
		#endif
		
		ar->common.curaid = bss_conf->aid;
		err = ar9170_set_beacon_timers(ar);
		if (err)
			goto out;
	}

	if (changed & BSS_CHANGED_ERP_SLOT) {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("bss_changed_erp_slot_flag set.\n");
		#endif
		
		err = ar9170_set_slot_time(ar);
		if (err)
			goto out;
	}

	if (changed & BSS_CHANGED_BASIC_RATES) {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("bss_changed_rates_flag set.\n");
		#endif
		
		err = ar9170_set_mac_rates(ar);
		if (err)
			goto out;
	}

out:
	if(err && IS_STARTED(ar)) {
		printf("ERROR: BSS info changed returned error on a started device.\n");
	}
	__lock_release(&ar->mutex_lock);
	
}




bool ar9170_op_add_pending_pkt( struct ar9170* ar, ar9170_tx_queue** queue_pt, 
	struct sk_buff* skb, bool atomic )
{		
	/* The packet is added to the queue given in the arguments' list. The queue
	 * should not be NULL, as it is expected to have been allocated during the 
	 * initialization. If it is found NULL we print a warning.
	 */		
	if ((*queue_pt) == NULL) {
		
		#if AR9170_MAIN_DEBUG
		printf("WARNING: Queue is null.\n");
		#endif
		
		/* Initialize the queue */
		*queue_pt = linked_list_init(*queue_pt);
		
		if ((*queue_pt) == NULL) {
			printf("ERROR: Could not allocate memory for transmission queue initialization.\n");
			goto err_free;
		}	
	} /* The queue is now non-null. */
			
	/* Check the packet for consistency. */	
	if (skb == NULL) {
		/* Request to add a NULL packet. */
		printf("WARNING: Request to add a NULL packet.\n");
		goto err_free;
	
	} else {		
		if (skb->data == NULL) {			
			/* Socket buffer is not NULL but packet content is not set. */
			printf("WARNING: Request to add an empty packet.\n");
			goto err_free;
		}
		if (skb->len <= 0) {
			/* Socket buffer is not NULL but packet content is not set. */
			printf("WARNING: Request to add a packet with wrong length [%u].\n",skb->len);
			goto err_free;
		}
		
	} 
	/* The packet is OK. */
	
	/* If required, lock access to the queue, disabling the global interrupt. */
	irqflags_t _flags;
	if(atomic) {	
		_flags = cpu_irq_save();	
	}	
	
	/* Add the packet in the selected queue. Notice: if the whole
	 * operation is atomic, we do not force atomic inside the list
	 * operation [redundant and also it will re-enable interrupts]
	 * 
	 * If the whole operation is not atomic, this is because we are
	 * already inside the interrupt context [TODO - how to ensure]
	 * and so atomic call of linked_list_add_tail is redundant and
	 * dangerous.
	 */
	int position;
	if (ar->rx_pending_pkts == (*queue_pt)) {
		/* This is the reception queue. */
		position = linked_list_add_tail(*queue_pt, (void*)skb, AR9170_MAX_PENDING_RX_PKT_QUEUE_LEN, false);
	
	} else {
		/* This is a transmission queue (Beacon, Atim, Data)*/
		position = linked_list_add_tail(*queue_pt, (void*)skb, AR9170_MAX_PENDING_TX_PKT_QUEUE_LEN, false);
		#if AR9170_MAIN_DEBUG_DEEP
		int k;
		printf("ADD PKT [%u]:",skb->len);
		for (k=0; k<skb->len; k++) {
			printf("%02x ", (skb->data)[k]);
		}
		printf(" \n");
		#endif
	}
	
	
	/* Check if the packet has been correctly added. */
	if (position < 0) {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("WARNING: Packet was not added in the queue.\n");
		#endif
		goto err_unlock;
	
	} else {
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: Packet added at position: %d.\n", position);
		#endif
		goto unlock;
	}
		
err_unlock:
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: Packet NOT added.\n");
	#endif
	if(atomic) {
		cpu_irq_restore(_flags);
	}		
err_free:	
	return false;	
unlock:
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: Packet added.\n");
	#endif
	if(atomic) {		
		cpu_irq_restore(_flags);
	}		
	return true;		
}


bool ar9170_async_tx_soft_beacon(struct ar9170* ar) {
	
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: AR9170 Scheduler will attempt soft beacon transmission.\n");
	#endif
	
	/* Attempt a beacon transmission, only if the 
	 * underlying USB bulk-out endpoint is available.
	 */
	delay_ms(1); /* How can we get rid of this artificial delay? TODO */
	
	if (ar->tx_async_lock == false) {
		
		if ((ibss_info->ibss_beacon_buf == NULL) || (ibss_info->ibss_beacon_buf->len <= 0)) {
			/* Signal an error; there should always be a stored beacon buffer. */
			printf("ERROR: There is no beacon buffer.\n");
			return false;
		}
		
		struct sk_buff* beacon_buffer = (struct sk_buff*)smalloc(sizeof(struct sk_buff));
		if (beacon_buffer == NULL) {
			printf("ERROR: Could not allocate memory for beacon buffer.\n");
			return false;
		}
		
		/* Assign buffer length */
		beacon_buffer->len = ibss_info->ibss_beacon_buf->len;
		
		/* Allocate memory for the soft beacon. */
		beacon_buffer->data = smalloc(ibss_info->ibss_beacon_buf->len);
		if (beacon_buffer->data == NULL) {
			printf("ERROR: Could not allocate memory for beacon buffer data.\n");
			return false;
		}
		
		/* Copy packet to new memory. This is freed later. */
		ar9170_usb_memcpy((uint32_t*)(beacon_buffer->data), 
			(uint32_t*)(ibss_info->ibss_beacon_buf->data), DIV_ROUND_UP(ibss_info->ibss_beacon_buf->len,4));
					
		/* Prepare and transmit the soft beacon. */		
		if (ar9170_op_tx(hw, beacon_buffer) == false) {
			
			printf("WARNING: Packet could not be prepared/transmitted.\n");
			return false;
		
		} else {
			
			return true;
		}
		
	} else {
		/* We can not transmit it yet. Line seems to be busy. */
		return false;
	}			
}


/* FIXME - This function prints sufficient error messages, but needs 
 * to be completed so it is also robust against null pointer errors.
 */
ar9170_tx_queue* ar9170_async_tx( struct ar9170* ar, ar9170_tx_queue* tx_queue )
{
	#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: AR9170 Scheduler will attempt packet transmission.\n");
	#endif
	
	bool result;
	/* Attempt a packet transmission, only if the 
	 * underlying USB bulk-out endpoint is available.
	 */
	
	/* Set a flag for updating stats after transmission. */
	bool is_atim_queue = (tx_queue == ar->tx_pending_atims);
	bool is_data_queue = (tx_queue == ar->tx_pending_pkts);
	
	if (ar->tx_async_lock == false) {
		
		/* Pull the first packet from the given queue [DATA or ATIM]. */		
		struct sk_buff* next_skb = (struct sk_buff*)linked_list_get(tx_queue);
		if (!next_skb) {
			printf("ERROR: Queue returned a NULL element.\n");
		}
		
		if (!next_skb->data) {
			printf("ERROR: Queue returned a NULL content element.\n");
		}
		
		if (ar->ps.state == true || (ar->ps_mgr.psm_state != AR9170_TX_WINDOW && ar->ps_mgr.psm_state != AR9170_ATIM_WINDOW)) {
			/* We can not transmit, so return the queue as it is. */
			printf("We lost race.\n");
			return tx_queue;
		}
		
		/* XXX This is an ugly hack. If we are in MH-PSM 
		 * we must restore the A3 field, so it will again
		 * contain th default BSSID. It is only necessary
		 * for data frames. XXX XXX
		 */
		if (ibss_info->ps_mode == IBSS_MH_PSM && is_data_queue == true) {
			
			struct ieee80211_hdr* data_pkt_hdr = (struct ieee80211_hdr*)(next_skb->data);
			
			if (ether_addr_equal(data_pkt_hdr->addr3, unique_vif->bss_conf.bssid)) {
				printf("WARNING: Why the data packet already contains the BSSID?\n");
			
			} else {
				memcpy(data_pkt_hdr->addr3, unique_vif->bss_conf.bssid, ETH_ALEN);
			}			
		}			
						
		/* Prepare and transmit the first packet */
		result = ar9170_op_tx(hw, next_skb);
		
		/* We have now transmitted the packet. However, this still remains
		 * in the packet pending queue and it is dangerous if an interrupt
		 * occurs that browses this queue. So the whole operation shall be
		 * protected.
		 */
		irqflags_t _flags = cpu_irq_save();
		
		if (next_skb->data != NULL) {
			/* Socket buffer will be freed when the list element will be removed. */
			free(next_skb->data);
			next_skb->data = NULL;
			
		
		} else {
			printf("Why is it already NULL?\n");
		}
		/* If there are more packets in the queue, point the root
		 * to the next packet, i.e. remove the first element. We 
		 * protect the whole list operation. BUT I think this is
		 * redundant, since the driver's TX Queue is not accessed 
		 * inside interrupt context.
		 */				
		if (tx_queue == ar->tx_pending_atims) {
			
			ar->tx_pending_atims = linked_list_remove_first(tx_queue);
		
		} else if (tx_queue == ar->tx_pending_pkts) {
			
			ar->tx_pending_pkts =  linked_list_remove_first(tx_queue);
		}		
		cpu_irq_restore(_flags);
				
		#if AR9170_MAIN_DEBUG_DEEP
		printf("DEBUG: Removed packet from queue. Current length: %u.\n",linked_list_get_len(tx_queue));
		#endif
		#if AR9170_MAIN_DEBUG
		if(tx_queue != ar->tx_pending_pkts && tx_queue != ar->tx_pending_atims) {
			
		}
		#endif
		if (result == false) {
			printf("WARNING: Packet could not be prepared/transmitted.\n");
		}
		/* Register event with the statistics evaluator. Inspect the actual 
		 * queue where the packet was transmitted from, and increment the 
		 * respective counter.
		 */
		if(is_atim_queue) {
			#if AR9170_MAIN_DEBUG_DEEP
			printf("DEBUG: AR9170 MAIN; Increment total ATIM transmissions.\n");
			#endif

		
		} else if(is_data_queue) {
			#if AR9170_MAIN_DEBUG_DEEP
			printf("DEBUG: AR9170 MAIN; Increment total packet transmissions.\n");
			#endif

		
		} else {
			printf("WARNING: Packet was not transmitted from the ATIM or the TX Queue.\n");
		}
	/*				
		if (next_skb->data != NULL) {
			printf("WARNING: Need to free the packet data! Why was it not done already?\n");
			free(next_skb->data);
		}		
		if (next_skb != NULL) {
			#if AR9170_MAIN_DEBUG_DEEP
			
			printf("DEBUG: AR9170 MAIN; Will not free socket buffer.\n");
			#endif
			//free(next_skb);
			//next_skb = NULL;
		}			
	*/	
		
	} else {
		#if AR9170_MAIN_DEBUG
		printf("DEBUG: AR9170 MAIN; TX still ongoing. Wait.\n");
		#endif
		/* We must wait for the current data chunk transfer
		 * to complete, before we can sent down a new one.
		 */	
	}
	
	/* Finally, we return the [perhaps updated] transmission queue. */
	if (is_atim_queue) {
		return ar->tx_pending_atims;
	} else if (is_data_queue) {
		return ar->tx_pending_pkts;
	} else {
		printf("ERROR: unrecognized returning queue.\n");
		return NULL;
	}	
}


void ar9170_async_rx( struct ar9170* ar) 
{
	#if AR9170_MAIN_DEBUG_DEEP
	printf("DEBUG: AR9170 Scheduler will attempt packet processing.\n");
	#endif
		
	//delay_us(100); /* How can we get rid of this artificial delay? TODO */
	
	/* Pull the first packet from the list of pending received packets. */
	struct sk_buff* next_skb = (struct sk_buff*)linked_list_get(ar->rx_pending_pkts);
	
	if (next_skb != NULL) {
		/* 
		 *	Process the packet in the list.
		 */
		__ieee80211_rx(next_skb);
	/*	
		ar9170_handle_mpdu(ar, next_skb->data, next_skb->len);
	*/		
		if (next_skb->data != NULL ) {
			/* Free the packet content [only the content,
			 * not the socket buffer]. 
			 */
			sfree(next_skb->data);
			next_skb->data = NULL;
		
		} else {
			printf("ERROR: The packet content is already NULL.\n");
		}
		/* Do not free the socket buffer for this packet. 
		 * The pointer is called by value, so there is no
		 * way the list to know whether the data is NULL.
		 */
		//free(next_skb);
		//next_skb = NULL;
	
	} else {
		/* Signal a warning that the packet was NULL. */
		printf("WARNING: Socket buffer was NULL: Why?\n");
	}
		
	/* Remove packet from the pending buffer and free resources. 
	 * This is maybe dangerous, so we do it in a safe way, thus,
	 * by disabling and re-enabling the processor's interrupts.
	 */
	irqflags_t _flags = cpu_irq_save();	
	ar->rx_pending_pkts = linked_list_remove_first(ar->rx_pending_pkts);
	cpu_irq_restore(_flags);	
	
	#if AR9170_MAIN_DEBUG_DEEP
	if (linked_list_is_empty(ar->rx_pending_pkts)) {
		printf("Scheduler processed packet and now the queue is empty again.\n");
	}
	#endif
}




/************************************************************************/
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++  AR9170 Scheduler for Asynchronous Operations  ++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/************************************************************************/
void ar9170_op_scheduler(struct ar9170* ar) {
	
	/* Sequential check of required operations. The order 
	 * of the operations should NOT matter, so feel free  
	 * to populate this function with new operations.
	 */
		
	/* -------- Check for erasing the awake nodes -------*/
	ar9170_sch_erase_nodes_check(ar);
		
	/* -------- Check for power-save transition -------- */
	ar9170_sch_powersave_check(ar);		
	
	/* -------- Check beacon control command -------- */
	ar9170_sch_beacon_ctrl_check(ar);
	
	/* -------- Check beacon cancellation command -------- */
	ar9170_sch_beacon_cancel_check(ar);		
	
	/* -------- Check filter disabling after beaconing started -------- */
	ar9170_sch_rx_filter_disable_check(ar);	
	
	/* -------- Check for any other asynchronous write command -------- */
	ar9170_sch_async_cmd_check(ar);
		
	/* -------- TX Routines -------- */	
	ar9170_sch_async_tx_check(ar);
	
	/* 
	 * -------- RX Routines -------- 
	 *
	 * If the pending RX queue is non-empty, we should proceed
	 * with the handling of the next packet immediately.
	 */
	ar9170_sch_async_rx_check(ar);			
}
