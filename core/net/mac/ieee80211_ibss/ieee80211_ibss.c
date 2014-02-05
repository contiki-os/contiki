/*
 * IBSS mode implementation
 * Copyright 2003-2008, Jouni Malinen <j@w1.fi>
 * Copyright 2004, Instant802 Networks, Inc.
 * Copyright 2005, Devicescape Software, Inc.
 * Copyright 2006-2007  Jiri Benc <jbenc@suse.cz>
 * Copyright 2007, Michael Wu <flamingice@sourmilk.net>
 * Copyright 2009, Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
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
#include "if_ether.h"
#include <stdint-gcc.h>
#include "ieee80211_ibss.h"
#include <sys\errno.h>
#include "nl80211.h"
#include "string.h"
#include "mac80211.h"
#include "cfg80211.h"
#include "skbuff.h"
#include "hw.h"
#include "ieee80211.h"
#include "ar9170_debug.h"
#include "ar9170_wlan.h"
#include "compiler.h"
#include "ar9170.h"
#include "usb_cmd_wrapper.h"
#include "ar9170_main.h"
#include "delay.h"
#include "ar9170_mac.h"
#include "ibss_util.h"
#include "etherdevice.h"
#include "ar9170_psm.h"
#include "ieee80211_rx.h"
#include "ieee80211_iface_setup_process.h"
#include "ieee80211_psm.h"
#include "ieee80211_debug.h"
#include "process.h"
#include "ibss_setup_process.h"
#include "fwcmd.h"
#include "smalloc.h"
#include "ar9170_debug.h"
#include "ieee80211_mh_psm.h"
#include "platform-conf.h"

/* Flag indicating whether the default IBSS is operating [joined or created] */
static volatile bool ieee80211_is_ibss_joined_flag;

static volatile bool ieee80211_has_ibss_started_beaconing_flag = false;

int __ieee80211_sta_init_hw() 
{
	/* Allocate the channel structure */
	ibss_info->ibss_channel = smalloc(sizeof(struct ieee80211_channel));
	
	if (!ibss_info->ibss_channel) {
		printf("ERROR: Could not allocate memory for channel structure.\n");
		return -ENOMEM;
	}
	
	ibss_info->ibss_channel->band = IEEE80211_BAND_2GHZ;
	channel_type = NL80211_CHAN_NO_HT;
	ibss_info->ibss_channel->center_freq = 2412;
	ibss_info->ibss_channel->hw_value = 0;
	ibss_info->ibss_channel->max_power = 18;
	
	/* Allocate the hardware structure */
	hw = (struct ieee80211_hw*)smalloc(sizeof(struct ieee80211_hw));
	if (!hw) {
		printf("ERROR: In ibss.c: could not allocate memory for the IEEE80211_HW struct.\n");
		return -ENOMEM;
	}
	/* Set PS mode on startup.  */
	if (ibss_info->ps_mode != IBSS_NULL_PSM) {
		printf("INFO: IBSS operates in Power-Save Mode.\n");	
		hw->conf.flags = IEEE80211_CONF_PS; 
	}		
	/* FIXME - We have a single TX queue */
	hw->queues = __AR9170_NUM_TXQ;
	
	/* Store the channel information on the hw structure */
	hw->conf.channel = ibss_info->ibss_channel;
	hw->conf.channel_type = NL80211_CHAN_NO_HT; // TODO - check if this is correct to do it here
	
	/* Store the reference to the AR9170 device if this is initialized */
	struct ar9170* ar = ar9170_get_device();
	
	if (ar == NULL) {
		printf("ERROR: AR9170 device structure is not allocated / initialized.\n");
		return -ENOMEM;
	}
	// otherwise, this is how the hw structure can access the ar9170 structure
	hw->priv = ar;
	
	/* Allocate memory for the wireless interface structure */
	hw->wiphy = (struct wiphy*)smalloc(sizeof(struct wiphy));
	
	if(!hw->wiphy) {
		printf("ERROR: No memory for the wireless interface initialization.\n");
		return -ENOMEM;
	}
	
	/* Initialize the tx parameter set manually. These parameters can change later. */
	ibss_tx_params.aifs = 2;
	ibss_tx_params.cw_max = 1023;
	ibss_tx_params.cw_min = 31;
	ibss_tx_params.txop = 0;
	ibss_tx_params.uapsd = 0;
	
	return 0;
}


int __ieee80211_sta_init_config() 
{
	/* Initialize "join" flag to false. */
	ieee80211_is_ibss_joined_flag = false;
	
	/* Initialize structure for the STA information */
	unique_sta = (struct ieee80211_sta*)smalloc(sizeof(struct ieee80211_sta));
	if(!unique_sta) {
		printf("ERROR: NO memory for STA info initialization.\n");
		return -ENOMEM;
	}
	/* Zeroing*/
	memset(unique_sta, 0, sizeof(struct ieee80211_sta));	
	
	/* Initialize structure for IBSS info */
	ibss_info = (struct ieee80211_ibss_info*)smalloc(sizeof(struct ieee80211_ibss_info));
	if(!ibss_info) {
		printf("ERROR: NO memory for IBSS info initialization.\n");
		return -ENOMEM;
	}
	memset(ibss_info, 0, sizeof(struct ieee80211_ibss_info)),
	
	/* Set to non-initialized */
	ibss_info->is_initialized = false;
	ibss_info->tbtt_counter = 0;
	/* Choose a BSSID for the network. SSID and Name */
	
	// Set the BSSID manually
	ibss_info->ibss_bssid = (uint8_t*)smalloc(ETH_ALEN);
	
	const uint8_t default_bssid[ETH_ALEN] = DEFAULT_BSSID; 
	ibss_info->ibss_bssid[0] = default_bssid[0];
	ibss_info->ibss_bssid[1] = default_bssid[1];
	ibss_info->ibss_bssid[2] = default_bssid[2];
	ibss_info->ibss_bssid[3] = default_bssid[3];
	ibss_info->ibss_bssid[4] = default_bssid[4];
	ibss_info->ibss_bssid[5] = default_bssid[5];
	
	ibss_info->ibss_name = (uint8_t*)smalloc(ETH_ALEN);
	
	/* The network name is Disney :) */
	ibss_info->ibss_name[0] = 0x44;
	ibss_info->ibss_name[1] = 0x69;
	ibss_info->ibss_name[2] = 0x73;
	ibss_info->ibss_name[3] = 0x6e;
	ibss_info->ibss_name[4] = 0x65;
	ibss_info->ibss_name[5] = 0x79;
	
	#ifdef WITH_STANDARD_PSM
	ibss_info->ps_mode = IBSS_STD_PSM;
	#else
	#ifdef WITH_MULTI_HOP_PSM
	ibss_info->ps_mode = IBSS_MH_PSM;
	#else
	#warning Program will run in non-ps mode!
	ibss_info->ps_mode = IBSS_NULL_PSM;
	#endif
	#endif
	
	/* Initialize the unique virtual interface structure */
	unique_vif = (struct ieee80211_vif*)smalloc(sizeof(struct ieee80211_vif));	
	if(!unique_vif) {
		printf("ERROR: NO memory for virtual interface initialization.\n");
		return -ENOMEM;
	}
	 /* set operating mode [ad-hoc]*/
	unique_vif->type = NL80211_IFTYPE_ADHOC;
	
	/* zeroing the bss_conf memory */
	memset(&unique_vif->bss_conf,0,sizeof(struct ieee80211_bss_conf));
	
	unique_vif->bss_conf.use_short_slot = false; // TODO - check if this needs to be changed at any point.
	
	/* Set the bssid on the configuration structure */
	unique_vif->bss_conf.bssid = ibss_info->ibss_bssid;
	
	/* Beacon period */
	unique_vif->bss_conf.beacon_int = BEACON_INTERVAL;
	
	/* ATIM Window Duration */
	unique_vif->bss_conf.atim_window = ATIM_WINDOW_DURATION;
	
	/* Soft Beacon Period. */
	unique_vif->bss_conf.soft_beacon_int = SOFT_BEACON_PERIOD;
	
	/* 29 units of beacon interval IBSS DTIM period [DEBUG]. */
	unique_vif->bss_conf.bcn_ctrl_period = 29;
	
	/* IBSS has not DTIM period */
	unique_vif->bss_conf.dtim_period = 0;
	
	/* enable beaconing [default] */
	unique_vif->bss_conf.enable_beacon = true;
	
	/* set the NO_HT channel type [default] */
	unique_vif->bss_conf.channel_type = NL80211_CHAN_NO_HT;
	
	/* Basic rates */
	unique_vif->bss_conf.basic_rates = 1;
	
	/* Short preamble */
	unique_vif->bss_conf.use_short_preamble = false;
	
	/* set to unprepared to force initialization */
	unique_vif->prepared = false;
	
	unique_cvif = smalloc(sizeof(struct ar9170_vif_info));
	
	unique_cvif->active = false;
	unique_cvif->beacon = NULL;
	unique_cvif->id = 0;
			
	return(__ieee80211_sta_init_hw());
	/* TODO - perhaps more data should be initialized? */
			
}


void _ieee80211_sta_update_info() {
	memcpy(unique_sta->addr, unique_vif->addr, ETH_ALEN);	
}

int __ieee80211_hw_config(struct ieee80211_hw* _hw , U32 changed_flag) 
{		
	/* Configure device */
	int err = ar9170_op_config(_hw,changed_flag);
	
	if(err) {
		printf("ERROR: configuring AR9170 returned errors.\n");
	}
	return err;
}


void __driver_conf_tx() 
{
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("DEBUG: __driver_conf_tx.\n");
	#endif
	struct ieee80211_tx_queue_params _params[4];
	
/*	
	CARL9170_FILL_QUEUE(_params[3], 7, 15, 1023, 0);
	CARL9170_FILL_QUEUE(_params[2], 3, 15, 1023, 0);
	CARL9170_FILL_QUEUE(_params[1], 2, 7, 15, 94);
	CARL9170_FILL_QUEUE(_params[0], 2, 3, 7,  47);
*/	
	/* This is a workaround for MAC TX QoS. We set 
	 * all TX queue QoS to the maximum possible values
	 * for the minimum and maximum contention window.
	 * This will add a small delay in the transmission
	 * of packets when the network load is moderate, but
	 * will enable fast packet flooding when traffic
	 * conditions are heavy.
	 */
	CARL9170_FILL_QUEUE(_params[3], 7, 15, 1023, 0);
	CARL9170_FILL_QUEUE(_params[3], 7, 15, 1023, 0);
	CARL9170_FILL_QUEUE(_params[3], 7, 15, 1023, 0);
	CARL9170_FILL_QUEUE(_params[3], 7, 15, 1023, 0);
	
	/* Configure tx queues */
	if (ar9170_op_conf_tx(hw, unique_vif, 0, &_params[0]) ||
	    ar9170_op_conf_tx(hw, unique_vif, 1, &_params[1]) ||
		ar9170_op_conf_tx(hw, unique_vif, 2, &_params[2]) ||
		ar9170_op_conf_tx(hw, unique_vif, 3, &_params[3])) {
		printf("ERROR: Could not set TX parameters.\n");				
	}	
	
}

void __driver_conf_rx_filter() 
{
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("Driver: Configure RX filter.\n");
	#endif
	
	struct ar9170* ar = ar9170_get_device();
	
	// TODO - automate the following
	unsigned int new_flags = 0x1001;
	unsigned int new_flags_e = 0x0001;
	ar->rx_filter_caps = 0xed;
	ar->cur_mc_hash = 1;
/*	
	ar9170_op_configure_filter(hw, 0x80000100, &new_flags, 0x80001001);
	mdelay(2);
	ar9170_op_configure_filter(hw, 0x80000100, &new_flags_e, 0x80000001);
	mdelay(2);
	ar9170_op_configure_filter(hw, 0x80000100, &new_flags, 0x800001001);
	mdelay(2);
	ar9170_op_configure_filter(hw, 0x80000100, &new_flags, 0xc00001001);
*/	
	ar9170_update_multicast_mine(ar, 0x80000000, 0x00001001);
	delay_ms(3);
	ar9170_update_multicast_mine(ar, 0x80000000, 0x00000001);
	delay_ms(3);
	ar9170_update_multicast_mine(ar, 0x80000000, 0x00001001);
	delay_ms(3);
	ar9170_update_multicast_mine(ar, 0xc0000000,0x00001001);
	delay_ms(3);
}

void __ieee80211_sta_join_ibss(U32 basic_rates, U16 capability, __le64 tsf) 
{
	int kk;
	uint8_t* test;
	int i, err;
	U32 bss_change = 0;
	U16 supp_rates[] = {10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540};
	
	UNUSED(test);
	UNUSED(err);
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("IBSS: Attempt to join the IBSS...\n");
	#endif
		
	/* Until the default IBSS network has been created, and beaconing
	 * has started, we filter-out management frames, as they are not
	 * needed. The following command actually filters out everything.
	 */
	ar9170_rx_filter(ar9170_get_device(), AR9170_RX_FILTER_OTHER_RA | 
										  AR9170_RX_FILTER_CTL_OTHER |
										  AR9170_RX_FILTER_BAD | 
										  AR9170_RX_FILTER_CTL_BACKR | 
										  AR9170_RX_FILTER_MGMT | 
										  AR9170_RX_FILTER_DATA |	
										  AR9170_RX_FILTER_DECRY_FAIL); 
										  
	/*
	 * Call the configuration function for the underlying
	 * AR9170 device.
	 */
	if(__ieee80211_hw_config(hw, IEEE80211_CONF_CHANGE_CHANNEL))
		return;				

										  
	/* One of the basic responsibilities of this function is
	 * to create the structure for the beacon that the 
	 * device will periodically transmit.
	 */
	ibss_info->ibss_beacon_buf = smalloc(sizeof(struct sk_buff));
	if (ibss_info->ibss_beacon_buf == NULL) {
		printf("ERROR: Could not allocate memory for beacon buffer creation.\n");
		return;
	}
	ibss_info->ibss_beacon_buf->data = smalloc(AR9170_MAC_BCN_LENGTH_MAX);
	if (ibss_info->ibss_beacon_buf->data == NULL) {
		printf("ERROR: Could not allocate memory for beacon data transmission.\n");
		return;
	}
	ibss_info->ibss_beacon_buf->len = 0;
	memset(ibss_info->ibss_beacon_buf->data, 0, AR9170_MAC_BCN_LENGTH_MAX);
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("DEBUG: Maximum fw-supported beacon length: %u; hw: %u.\n",ar->fw.beacon_max_len, AR9170_MAC_BCN_LENGTH_MAX);
	#endif
	
	/* Build IBSS probe response [BCN as well :)] */
	
	struct ieee80211_mgmt* mgmt = (struct ieee80211_mgmt*)(ibss_info->ibss_beacon_buf->data);
	/* Update length */
	ibss_info->ibss_beacon_buf->len += 36; 
	/* The frame control for the beacon frame */
	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);
	/* Address 1: Destination MAC address is set to broadcast */
	memset(mgmt->da, 0xff, ETH_ALEN);
	/* Address 2: Source MAC address is the local address */
	memcpy(mgmt->sa, unique_vif->addr, ETH_ALEN);
	/* Address 3: Contains the BSSID of the network */
	memcpy(mgmt->bssid, ibss_info->ibss_bssid, ETH_ALEN);
	/* The beacon interval duration */
	mgmt->u.beacon.beacon_int = cpu_to_le16(unique_vif->bss_conf.beacon_int);
	/* Time stamp */
	mgmt->u.beacon.timestamp = 0;// FIXME cpu_to_le64(tsf);
	/* Privacy capability flag */
	mgmt->u.beacon.capab_info = cpu_to_le16(capability);
	
	/* Add the name of the network */
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = WLAN_EID_SSID;
	ibss_info->ibss_beacon_buf->len += 1;
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = ETH_ALEN;
	ibss_info->ibss_beacon_buf->len += 1;
	memcpy(&(ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len]), ibss_info->ibss_name, ETH_ALEN);
	ibss_info->ibss_beacon_buf->len += ETH_ALEN;
	
	
	/* Add supported rates. */
	// Build supported rates array - FIXME automate this!
	
	for (i = 0; i < 12; i++) {
		int rate = supp_rates[i];
		U8 basic = 0;
		if (basic_rates & BIT(i))
			basic = 0x80;
		supp_rates[i] = basic | (U8) (rate / 5);
	}
	
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = WLAN_EID_SUPP_RATES;
	ibss_info->ibss_beacon_buf->len += 1;
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 8; // FIXME - could be even less...
	ibss_info->ibss_beacon_buf->len += 1;
	for (i=0; i<8; i++) {
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = (U8)supp_rates[i];
		ibss_info->ibss_beacon_buf->len += 1;
	}		
	
	
	/* Add DS parameters */
	if (channel_type == IEEE80211_BAND_2GHZ) {
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = WLAN_EID_DS_PARAMS;
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 1;
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = ibss_info->ibss_channel->hw_value + 1; // FIXME 
		ibss_info->ibss_beacon_buf->len += 1;
	}	
	
	
	/* Add IBSS parameter set */
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = WLAN_EID_IBSS_PARAMS;
	ibss_info->ibss_beacon_buf->len += 1;
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 2;
	ibss_info->ibss_beacon_buf->len += 1;
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 
		(U8)(unique_vif->bss_conf.atim_window);
	ibss_info->ibss_beacon_buf->len += 1;
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 0;
	ibss_info->ibss_beacon_buf->len += 1;
	
	
	/* Add the extra rates */
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = WLAN_EID_EXT_SUPP_RATES;
	ibss_info->ibss_beacon_buf->len += 1;
	ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 4;
	ibss_info->ibss_beacon_buf->len += 1;
	for(i=0; i<4; i++) {
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = (U8)supp_rates[8+i];
		ibss_info->ibss_beacon_buf->len += 1;
	}		
		
	
	/* Add Vendor Specific data */
	if (hw->queues >= IEEE80211_NUM_ACS) {
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = WLAN_EID_VENDOR_SPECIFIC;
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 7; /* len */
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 0x00; /* Microsoft OUI 00:50:F2 */
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 0x50;
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 0xf2;
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 2; /* WME */
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 0; /* WME info */
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 1; /* WME ver */
		ibss_info->ibss_beacon_buf->len += 1;
		ibss_info->ibss_beacon_buf->data[ibss_info->ibss_beacon_buf->len] = 0; /* U-APSD no in use */
		ibss_info->ibss_beacon_buf->len += 1;
	}
	
	#if IEEE80211_IBSS_DEBUG_DEEP
	test = (uint8_t*)(ibss_info->ibss_beacon_buf->data);
	printf("Current BCN [%d]: ",(unsigned int)(ibss_info->ibss_beacon_buf->len));
	for (kk=0; kk<ibss_info->ibss_beacon_buf->len; kk++)
		printf("%02x ",test[kk]);
	printf(" \n");
	#endif
	
	/* Notify the AR9170 */
	bss_change |= BSS_CHANGED_BEACON_INT;
	//bss_change |= BSS_CHANGED_BSSID;
	bss_change |= BSS_CHANGED_BEACON;
	bss_change |= BSS_CHANGED_BEACON_ENABLED;
	bss_change |= BSS_CHANGED_BASIC_RATES;
	bss_change |= BSS_CHANGED_HT;
	bss_change |= BSS_CHANGED_IBSS;
	bss_change |= BSS_CHANGED_ERP_SLOT;
	
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("DEBUG: Will now notify the driver for BSS change.\n");
	#endif
	
	struct ar9170* ar = ar9170_get_device();
	/* Enable beacon flag */
	unique_cvif->enable_beacon = true;
	ar->beacon_enabled = true;
	 
	/* Notify the driver for the network changes */
	__ieee80211_bss_change_notify(bss_change);	
	
	/* Run TX configuration */
	__driver_conf_tx();
	
	/* Run RX filter configuration */
	__driver_conf_rx_filter();
	
}

void ieee80211_sta_create_ibss()
{
	int i;
	#if IEEE80211_IBSS_DEBUG
	printf("IBSS: Creating default Ad-hoc network...\n");
	#endif
	
	_ieee80211_sta_update_info();
	
	uint16_t capability; 
			
	#if IEEE80211_IBSS_DEBUG
	printf("IBSS: Network name is 'Disney'. \n");
	printf("BSSID: ");
	for(i=0; i<ETH_ALEN; i++)
		printf("%02x:",ibss_info->ibss_bssid[i]);
	printf("\r\n");
	#endif
		
	/* Privacy capabilities */
	capability = WLAN_CAPABILITY_IBSS | WLAN_CAPABILITY_PRIVACY ;
	
	/* Join this IBSS */
	__ieee80211_sta_join_ibss(1, capability, 0);
	
	/* The IBSS is now initialized */
	ibss_info->is_initialized = true;
	
	/* Update indicator flag that IBSS is joined */
	ieee80211_is_ibss_joined_flag = true;
}


void ieee80211_sta_join_ibss(U16 _beacon_interval, U16 _atim_window, U8* _name) 
{
	int i;
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("IBSS: Joining default Ad-hoc network...\n");
	#endif
	
	_ieee80211_sta_update_info();
	
	uint16_t capability; 
	
	/* Privacy capabilities */
	capability = WLAN_CAPABILITY_IBSS | WLAN_CAPABILITY_PRIVACY ;
	
	/* Override & Accept the received beacon interval */
	unique_vif->bss_conf.beacon_int = _beacon_interval; 
	/* Override & Accept the received ATIM window */
	unique_vif->bss_conf.atim_window = _atim_window;
	/* Override & Accept the received network name */
	memcpy(ibss_info->ibss_name, _name , ETH_ALEN);	
	
	/* Join this IBSS */
	__ieee80211_sta_join_ibss(1, capability, 0);
	
	/* The IBSS is now initialized */
	ibss_info->is_initialized = true;
	
	/* Update indicator flag that IBSS is joined */
	ieee80211_is_ibss_joined_flag = true;
	
	/* We could let the IBSS_SETUP_PROCESS event timer
	 * to simply expire, and update the status of the 
	 * IBSS, however, we prefer to force it to stop 
	 * waiting and update the state now.
	 */
	process_post(&ibss_setup_process, PROCESS_EVENT_CONTINUE, NULL);
}


struct sk_buff * ieee80211_beacon_get_tim( struct ieee80211_hw * hw, struct ieee80211_vif* vif, 
void* tim_offset, void* tim_length )
{	
	int kk;
	uint8_t* test;
	
	UNUSED(kk);
	UNUSED(test);
	
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("DEBUG: IBSS beacon get tim.\n");
	#endif
	
	struct sk_buff *presp = ibss_info->ibss_beacon_buf;
	
	if (vif->type == NL80211_IFTYPE_AP) {
		/*
		 *  TODO - just ignore for now
		 *
		 */
	} else if (vif->type == NL80211_IFTYPE_ADHOC) {
		
		#if IEEE80211_IBSS_DEBUG_DEEP
		printf("DEBUG: Beacon will be tweaked assuming AD-HOC mode.\n");
		#endif
		
		if (!presp) {	
			printf("WARNING: Beacon data could not be retrieved.\n");
			goto out;
		}
		
		/* Beacon frame control is now changed. */
		le16_t frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_BEACON);
		
		memcpy(presp->data, &frame_control,sizeof(le16_t));
	
	} else {		
		printf("WARNING: Neither ad-hoc nor access point.\n");
	}

out:	
	return ibss_info->ibss_beacon_buf;
}




void __ieee80211_bss_change_notify(U32 flag_changed) {
	
	ar9170_op_bss_info_changed(hw, unique_vif, &unique_vif->bss_conf, flag_changed);
}



bool ieee80211_is_ibss_joined()
{
	return ieee80211_is_ibss_joined_flag;	
}


void ieee80211_set_ibss_join_state(bool new_state) {
	
	ieee80211_is_ibss_joined_flag = new_state;
}

void ieee80211_set_ibss_started_beaconing() {
	
	#if IEEE80211_IBSS_DEBUG
	printf("INFO: IBSS started beaconing.\n");
	#endif
	if (ieee80211_has_ibss_started_beaconing_flag == false) {
		ieee80211_has_ibss_started_beaconing_flag = true;
	} else {
		printf("ERROR: The beaconing flag is set. Why?\n");
	}	
}


bool ieee80211_has_ibss_just_started_beaconing()
{
	if (ieee80211_has_ibss_started_beaconing_flag == true) {
		/* So this function will return true only on transitions. */
		ieee80211_has_ibss_started_beaconing_flag = false;
		return true;
		
	} else {
		return false;
	}
}

void ieee80211_enable_scanning() {
	
	ar9170_rx_filter(ar9170_get_device(), AR9170_RX_FILTER_OTHER_RA |
		AR9170_RX_FILTER_CTL_OTHER |
		AR9170_RX_FILTER_BAD |
		AR9170_RX_FILTER_CTL_BACKR |
		//AR9170_RX_FILTER_MGMT |
		AR9170_RX_FILTER_DATA |
		AR9170_RX_FILTER_DECRY_FAIL);
}