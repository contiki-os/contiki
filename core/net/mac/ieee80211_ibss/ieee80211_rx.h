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
#include "skbuff.h"
#include "ieee80211.h"

#ifndef IEEE80211_RX_H_
#define IEEE80211_RX_H_

#define BROADCAST_80211_ADDR {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

void __ieee80211_rx( struct sk_buff * skb );
void ieee80211_rx_handle_ATIM_pkt(struct ar9170* ar, struct ieee80211_mgmt* mgmt);
void ieee80211_handle_ATIM_status_rsp(bool success);
void ieee80211_rx_process_mgmt_mpdu(struct sk_buff* skb);
void ieee80211_rx_process_atim(struct ieee80211_mgmt* mgmt);
void ieee80211_rx_process_beacon(struct ieee80211_mgmt* mgmt, size_t len);
void ieee80211_rx_process_data_mpdu(struct sk_buff* skb);
#endif /* IEEE80211_RX_H_ */