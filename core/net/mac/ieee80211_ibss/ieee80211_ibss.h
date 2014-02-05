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
#include "mac80211.h"
#include "skbuff.h"
#include "cfg80211.h"
#include <stdint-gcc.h>
#include "ar9170.h"
#include "ieee80211_psm.h"


#ifndef IEEE80211_IBSS_H_
#define IEEE80211_IBSS_H_



/* The unique global virtual interface
 * struct.
 */
struct ieee80211_vif* unique_vif;

/* The unique globally visible
 * ieee80211 hardware struct.
 */
struct ieee80211_hw* hw;

/* The TX parameter set */
struct ieee80211_tx_queue_params ibss_tx_params;

/* The unique vif_info structure */
struct ar9170_vif_info *unique_cvif;

/* Wireless channel type*/
uint8_t channel_type;

struct ieee80211_ibss_info {
	
	bool	is_initialized;
	/*
	 * The static array containing the 
	 * name representation of the IBSS
	 * Network. 
	 */
	uint8_t*	ibss_name;
	/*
	 * The static array containing the 
	 * BSSID of the IBSS Network.
	 */
	uint8_t*	ibss_bssid;
	/* The unique globally visible
	 * ieee80211 channel struct.
	 */
	struct ieee80211_channel* ibss_channel;
	
	/* The unique beacon buffer */
	struct sk_buff *ibss_beacon_buf;
	
	/* A debug counter to play with the beaconing */
	uint8_t tbtt_counter;
	
	enum ieee80211_ibss_ps_mode ps_mode;
};	

/* The unique global IBSS information struct */
struct ieee80211_ibss_info* ibss_info;

/* The unique global ieee80211 STA information struct */
struct ieee80211_sta* unique_sta;



struct sk_buff * ieee80211_beacon_get_tim( struct ieee80211_hw * hw, struct ieee80211_vif* vif, void* tim_offset, void* tim_length );
void ieee80211_sta_create_ibss();
int __ieee80211_sta_init_config();
int __ieee80211_sta_init_hw();
void __ieee80211_sta_join_ibss(U32, uint16_t, __le64);
void __ieee80211_bss_change_notify(U32 flag_changed);
int __ieee80211_hw_config(struct ieee80211_hw* _hw , U32 changed_flag);



void _ieee80211_sta_update_info();
void ieee80211_sta_join_ibss(U16 _beacon_interval, U16 _atim_window, U8* _name);
bool ieee80211_is_ibss_joined();
void ieee80211_set_ibss_join_state(bool new_state);
void ieee80211_set_ibss_started_beaconing();
bool ieee80211_has_ibss_just_started_beaconing();
void ieee80211_enable_scanning();
#endif /* IEEE80211_IBSS_H_ */