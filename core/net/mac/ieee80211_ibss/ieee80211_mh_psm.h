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
#include "ar9170.h"

#include "ieee80211.h"
#include "compiler.h"
#include "linked_list.h"

#ifndef IEEE80211_MH_PSM_H_
#define IEEE80211_MH_PSM_H_


/*
 * Mapping end MAC addresses to next-hop MAC addresses,
 * used for replacing the cross-layer functionality in
 * our test-bed design. TODO - replace this with a real
 * implementation in the network [routing] layer.
 */
struct ieee80211_mh_psm_address_mapping {
	
	U8* end_addr;
	U8* next_hop_addr;
};

/* Linked list implementation for the address map. */
typedef struct linked_list_t ieee80211_mh_psm_address_map;


void ieee80211_mh_psm_handle_atim_packet(struct ar9170* ar, struct ieee80211_mgmt* mgmt);
bool ieee80211_mh_psm_atim_list_contains_A3( struct ar9170* ar, U8* bssid );
U8* ieee80211_mh_psm_resolve_next_hop( U8* bssid );


//************************************
// Method:    ieee80211_mh_psm_get_address_map
// FullName:  ieee80211_mh_psm_get_address_map
// Access:    public 
// Returns:   ieee80211_mh_psm_address_map* The unique MAC-resolve address map
// Qualifier:
//************************************
ieee80211_mh_psm_address_map* ieee80211_mh_psm_get_address_map();

//************************************
// Method:    ieee80211_mh_psm_init_address_map
// FullName:  ieee80211_mh_psm_init_address_map
// Access:    public 
// Returns:   bool True if the initialization was successful
// Qualifier:
//************************************
bool ieee80211_mh_psm_init_address_map();
#endif /* IEEE80211_MH_PSM_H_ */