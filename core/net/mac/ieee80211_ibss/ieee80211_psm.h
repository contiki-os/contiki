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
#include "compiler.h"
#include "ar9170.h"
#include "skbuff.h"

#ifndef IEEE80211_PSM_H_
#define IEEE80211_PSM_H_

enum ieee80211_ibss_ps_mode {
	
	/* No PSM */
	IBSS_NULL_PSM,
	/* Standard PSM */
	IBSS_STD_PSM,
	/* Advanced PSM */
	IBSS_MH_PSM,
};


void ieee80211_psm_add_awake_node(struct ar9170* ar, U8* sa);
void ieee80211_psm_erase_awake_neighbors(struct ar9170* ar);
void ieee80211_psm_create_atim_pkts(struct ar9170* ar);
bool ieee80211_psm_is_DA_awake(struct sk_buff* pkt);
bool ieee80211_psm_is_remote_da_awake(struct ar9170* ar, U8* da );
ar9170_tx_queue* ieee80211_psm_can_send_first_atim(struct ar9170* ar);
ar9170_tx_queue* ieee80211_psm_can_send_first_pkt(struct ar9170* ar);


#endif /* IEEE80211_PSM_H_ */