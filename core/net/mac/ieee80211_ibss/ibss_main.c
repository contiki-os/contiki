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
#include <stdio.h>
#include "skbuff.h"
#include "ieee80211_tx.h"
#include "ar9170.h"
#include "ibss_main.h"
#include "ieee80211_psm.h"
#include "smalloc.h"
#include "rtimer.h"
#include "platform-conf.h"
#include "mac.h"
#include "packetbuf.h"
#include "ieee80211_ibss.h"


void ieee80211_drv_tx(mac_callback_t sent, void* ptr)
{
	/* The packet is already stored in the packet buffer. It 
	 * contains the network layer payload and the buffer also
	 * contains the length and the MAC destination address. 
	 */
	
	if (!ieee80211_is_ibss_joined()) {
		printf("ERROR: IBSS not joined. Cannot send packet.\n");
		goto _err;
	}
	
	struct sk_buff* skb = smalloc(sizeof(struct sk_buff));
	if (skb == NULL) {
		printf("ERROR: Could not allocate memory for socket buffer creation.\n");
		goto _err;
	}	
	/* Extract packet and assign it to the socket buffer. */
	skb->data = packetbuf_dataptr(); 
	skb->len =  packetbuf_datalen();
	
	#if IEEE80211_IBSS_DEBUG_DEEP
	int i;
	printf("ieee80211_tx_pkt\n");
	for (i=0; i<skb->len; i++) {
		printf("%02x ", (skb->data)[i]);
	}
	printf(" \n");
	#endif
	
	/* Attempt packet transmission. Notice that for MH-PSM we will need
	 * both the next-hop destination address, as well as the final MAC 
	 * destination address. For that we need to enhance the packet buffer
	 * structure. TODO
	 */
	const rimeaddr_t* dest_addr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
	const rimeaddr_t* next_addr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
	
	if (dest_addr == NULL || next_addr == NULL) {
		printf("ERROR: Next-hop or final destination address is null.\n");
		goto _err;
	}	
	/* Send packet to MAC processing and eventually down to the driver queue. */
	bool tx_result = ieee80211_start_xmit(skb, dest_addr->u8, next_addr->u8, false);
/*	
	struct sk_buff* dup_skb = smalloc(sizeof(struct sk_buff));
	if (!dup_skb){
		printf("ERROR:\n");
	}
	dup_skb->len = skb->len;
	dup_skb->data = smalloc(skb->len);
	if (!dup_skb->data) {
		printf("ERROR:\n");
	}
	memcpy(dup_skb->data, skb->data, skb->len);
	tx_result = ieee80211_start_xmit(dup_skb, dest_addr->u8, next_addr->u8, false);
*/	
	if (!tx_result) {
		/* Inform the UIP that the packet was not attempted. */
		printf("ERROR: Packet was not attempted. Inform UIP.\n");
		sent(NULL, false, 0);
		
	} else {
		/* Inform the UIP that the packet was stored in the driver queue. */
		sent(NULL, true, 0);
	}
	return;	
_err:	
	/* Inform the UIP that the packet was not attempted. */
	sent(NULL, false, 0);
	return;			
}



/*
 * Core implementation of the IEEE80211 for Contiki OS.
 * It currently includes an operation scheduler.
 */
void ieee80211_op_scheduler(struct ar9170* ar) {
	
	
	/* If the device is ready to populate the ATIM frame 
	 * buffer, we do it here, outside the interrupt 
	 * context. Normally this is called after each TBTT.
	 */
	if (ar->ps_mgr.create_atims_flag == true) {
				
		/* Clear the "create ATIMs" flag so the ATIM creation 
		 * is not called twice in the same TBTT period. 
		 */
		ar->ps_mgr.create_atims_flag = false;
		ieee80211_psm_create_atim_pkts(ar);
	}
						
	
}