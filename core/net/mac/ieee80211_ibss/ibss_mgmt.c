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
#include <stdlib.h>
#include "ibss_mgmt.h"
#include "ieee80211.h"
#include <stdint-gcc.h>
#include "skbuff.h"


struct sk_buff* ibss_create_atim(struct ieee80211_vif vif)
{
	struct ieee80211_hdr_3addr* atim;	
	printf("DEBUG: Creating ATIM MGMT frame.\n");
	
	/* Reserve space for  IBSS ATIM frame */
	atim = (struct ieee80211_hdr_3addr*)malloc(sizeof(struct ieee80211_hdr_3addr));
	if (atim == NULL) {
		printf("ERROR: Could not allocate memory for ATIM creation.\n");
		return NULL;
	}
	/* Zeroing memory*/
	memset(atim, 0, sizeof(struct ieee80211_hdr_3addr));
	
	/* Frame Control shall be the frame control type of ATIM packets */
	atim->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ATIM);
	
	/* Destination address. This shall be overwritten with the individual destination MAC address */
	memset(atim->addr1, 0xff, ETH_ALEN);
	
	/* Source (local) MAC address [must be stored on the virtual interface] */
	memcpy(atim->addr2, vif.addr, ETH_ALEN);
	
	/* BSSID [must be stored on the ibss_info object]. This shall be overwritten in the MH-PSM mode */
	memcpy(atim->addr3, vif.bss_conf.bssid, ETH_ALEN);
	
	struct sk_buff* atim_buffer = (struct sk_buff*)malloc(sizeof(struct sk_buff));
	
	if (atim_buffer == NULL) {
		printf("ERROR: Could not allocate memory for ATIM Buffer creation.\n");
		return NULL;
	}
	atim_buffer->len = sizeof(struct ieee80211_hdr_3addr);
	atim_buffer->data = (uint8_t*)atim;
		
	return atim_buffer;
}

struct sk_buff* ibss_atim_set_da(struct sk_buff* atim_buf, uint8_t* da )
{
	if (atim_buf != NULL) {
		if (atim_buf->data != NULL) {
			struct ieee80211_hdr* atim = (struct ieee80211_hdr*)atim_buf->data;
			memcpy(atim->addr1, da, ETH_ALEN);
		}
	}
	
	return atim_buf;
}

struct sk_buff* ibss_atim_set_sa( struct sk_buff* atim_buf, uint8_t* sa )
{
	if (atim_buf != NULL) {
		if (atim_buf->data != NULL) {
			struct ieee80211_hdr* atim = (struct ieee80211_hdr*)atim_buf->data;
			memcpy(atim->addr2, sa, ETH_ALEN);
		}
	}
	
	return atim_buf;
}

struct sk_buff* ibss_atim_set_address3( struct sk_buff* atim_buf, uint8_t* addr3 )
{
	if (atim_buf != NULL) {
		if (atim_buf->data != NULL) {
			struct ieee80211_hdr* atim = (struct ieee80211_hdr*)atim_buf->data;
			memcpy(atim->addr3, addr3, ETH_ALEN);
		}
	}
	
	return atim_buf;
}


