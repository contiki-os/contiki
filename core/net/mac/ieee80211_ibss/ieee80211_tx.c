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
#include "ieee80211_tx.h"
#include "skbuff.h"
#include "compiler.h"
#include "ieee80211.h"
#include "if_ether.h"
#include "ieee80211_ibss.h"
#include "bitops.h"
#include "cfg80211.h"
#include <stdio.h>
#include "ar9170.h"
#include "ieee80211_debug.h"
#include "smalloc.h"
#include "ieee80211_psm.h"
#include "cc.h"

/* Global counter for sequence number generation */
volatile le16_t tx_packets_sent = 0;



int ieee80211_frame_duration(enum ieee80211_band band, size_t len,
			     int rate, int erp, int short_preamble)
{
	int dur;

	/* calculate duration (in microseconds, rounded up to next higher
	 * integer if it includes a fractional microsecond) to send frame of
	 * len bytes (does not include FCS) at the given rate. Duration will
	 * also include SIFS.
	 *
	 * rate is in 100 kbps, so divident is multiplied by 10 in the
	 * DIV_ROUND_UP() operations.
	 */

	if (band == IEEE80211_BAND_5GHZ || erp) {
		/*
		 * OFDM:
		 *
		 * N_DBPS = DATARATE x 4
		 * N_SYM = Ceiling((16+8xLENGTH+6) / N_DBPS)
		 *	(16 = SIGNAL time, 6 = tail bits)
		 * TXTIME = T_PREAMBLE + T_SIGNAL + T_SYM x N_SYM + Signal Ext
		 *
		 * T_SYM = 4 usec
		 * 802.11a - 17.5.2: aSIFSTime = 16 usec
		 * 802.11g - 19.8.4: aSIFSTime = 10 usec +
		 *	signal ext = 6 usec
		 */
		dur = 16; /* SIFS + signal ext */
		dur += 16; /* 17.3.2.3: T_PREAMBLE = 16 usec */
		dur += 4; /* 17.3.2.3: T_SIGNAL = 4 usec */
		dur += 4 * DIV_ROUND_UP((16 + 8 * (len + 4) + 6) * 10,
					4 * rate); /* T_SYM x N_SYM */
		// XXX XXX
		dur += 20;
		
	} else {
		/*
		 * 802.11b or 802.11g with 802.11b compatibility:
		 * 18.3.4: TXTIME = PreambleLength + PLCPHeaderTime +
		 * Ceiling(((LENGTH+PBCC)x8)/DATARATE). PBCC=0.
		 *
		 * 802.11 (DS): 15.3.3, 802.11b: 18.3.4
		 * aSIFSTime = 10 usec
		 * aPreambleLength = 144 usec or 72 usec with short preamble
		 * aPLCPHeaderLength = 48 usec or 24 usec with short preamble
		 */
		dur = 10; /* aSIFSTime = 10 usec */
		dur += short_preamble ? (72 + 24) : (144 + 48);

		dur += DIV_ROUND_UP(8 * (len + 4) * 10, rate);
		
		//XXX XXX
		//dur += 200;
	}

	return dur;
}


le16_t ieee80211_duration( struct sk_buff* skb, int group_addr )
{
	int dur, rate, mrate, erp;
	UNUSED(mrate);
	
	struct ieee80211_hdr* hdr = (struct ieee80211_hdr*)skb->data;
	
	if (ieee80211_is_ctl(hdr->frame_control)) {
		printf("WARNING: Ctrl frame. No duration calculation.\n");
		return 0;
	}
	
	if (group_addr) {
		printf("WARNING: Multi-cast frame. No ACK --> duration calculation.\n");
		return 0;
	}
	
	/* Manually setting the rate FIXME - automate it TODO */
	rate = 10;
	erp = 0;	
	
	/* Don't calculate ACKs for QoS Frames with NoAck Policy set */
	if (ieee80211_is_data_qos(hdr->frame_control) &&
			*(ieee80211_get_qos_ctl(hdr)) & IEEE80211_QOS_CTL_ACK_POLICY_NOACK) {
				printf("WARNING: No ACKs for QoS frames with NoAck Policy.\n");
				dur = 0;
	} else {
		dur = ieee80211_frame_duration(ibss_info->ibss_channel->band, 10, rate, erp, unique_vif->bss_conf.use_short_preamble);
	}
	
	#if IBSS_TX_DEBUG_DEEP
	printf("DEBUG: Duration id: %04x.\n", cpu_to_le16(dur));
	#endif
	/* TODO - for fragmented packets we should update the duration. But we do not support it for now. */
	
	return cpu_to_le16(dur);	
}



static void __ieee80211_tx_atim(struct sk_buff* atim) {
	
	/* Obtain a reference to the AR9170 structure */
	struct ar9170* ar = ar9170_get_device();
	
	if (ar != NULL) {
		if(!ar9170_op_add_pending_pkt(ar, &(ar->tx_pending_atims), atim, true)) {
			printf("ERROR: Adding a new ATIM to the AR9170 transmit queue returned errors.\n");
				
		} else {
			/* Everything is OK. */
		}
	} else {
		printf("WARNING: AR9170 device is not initialized! ATIM not added. \n");
	}	
}

static bool __ieee80211_tx(struct sk_buff* skb) {
	
	/* Obtain a reference to the AR9170 structure */
	struct ar9170* ar = ar9170_get_device();
	
	#if IBSS_TX_DEBUG_DEEP
	printf("SKB: [%u]",skb->len);
	int i;
	for (i=0; i<skb->len; i++) {
		printf("%02x ", (skb->data)[i]);
	}
	printf(" \n");
	#endif
	
	if (ar != NULL) {		
		if(!ar9170_op_add_pending_pkt(ar, &(ar->tx_pending_pkts), skb, true)) {
			printf("ERROR: Adding a new packet to the AR9170 transmit queue returned errors.\n");
			goto err_free;
		} else {
			/* Everything is OK. */
			return true;
		}
	} else {
		printf("WARNING: AR9170 device is not initialized! Packet not added. \n");
		goto err_free;
	}
err_free:
	if(skb->data != NULL) {
		free(skb->data);
		skb->data = NULL;	
	}	
	if (skb != NULL)	
		free(skb);
		
	return false;	
}

bool ieee80211_tx( struct sk_buff * skb ) 
{	
	/*Reference to the header in order to modify selected fields */
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(skb->data);
	
	/* Update the sequence number for the MAC packet from the global counter */
	hdr->seq_ctrl = tx_packets_sent + 0x10;
	/* Increment sequence counter */
	tx_packets_sent += 0x10;
	
	/* Update the frame control. FIXME - how is this automated? */
	//hdr->frame_control |= 0x4000; This is only for encrypted packets
	
	/* Update the duration of the frame */
	hdr->duration_id = ieee80211_duration(skb, unique_vif->bss_conf.use_short_preamble);
	
	/* Queue the prepared packet in the buffer of pending MAC packets.
	 * Notice that the IEEE80211 scheduler is responsible for acting
	 * accordingly on a non-empty packet buffer.
	 */		
	return __ieee80211_tx(skb);
}


/* Create and buffer an ATIM packet for the requested DA */
void ieee80211_create_atim_pkt(struct ar9170* ar, uint8_t* da, uint8_t* a3) {
	
	#if IBSS_TX_DEBUG_DEEP
	printf("DEBUG: PSM; Creating ATIM for DA: %02x:%02x:%02x:%02x:%02x:%02x.\n", 
			da[0], da[1], da[2], da[3], da[4], da[5]);
	#endif	
	
	/* Allocate socket buffer memory */
	struct sk_buff* atim_packet = (struct sk_buff*)smalloc(sizeof(struct sk_buff));
	if (!atim_packet) {
		printf("ERROR: No memory for ATIM packet creation.\n");
		return;
	}
	/* Allocate header memory [actual data] */
	struct ieee80211_hdr_3addr* atim_header = 
		(struct ieee80211_hdr_3addr*)smalloc(sizeof(struct ieee80211_hdr_3addr));
	
	if (!atim_header) {
		printf("ERROR: No memory for ATIM header creation.\n");
		return;
	}
	
	/* Zero the packet memory. */
	memset(atim_header, 0, sizeof(struct ieee80211_hdr_3addr));
		
	/* Append the destination address to the header */
	memcpy(atim_header->addr1, da, ETH_ALEN);
	
	/* Append the source address to the header */
	memcpy(atim_header->addr2, unique_vif->addr, ETH_ALEN); 
	
	if (ibss_info->ps_mode == IBSS_MH_PSM) {
		
		if (a3 == NULL) {
			printf("WARNING: The A3 field is NULL, although the node is in MH-PSM.\n");
			memset(atim_header->addr3, 0, ETH_ALEN);
		} else {
			/* For advanced PSM append the given MAC address in the Address-3 field. */
			memcpy(atim_header->addr3, a3, ETH_ALEN);
		}
				
	} else {
		/* For standard [or NULL] PSM append the BSSID in the Address-3 field. */
		memcpy(atim_header->addr3, ibss_info->ibss_bssid, ETH_ALEN);
	}
	
	/* Set up the frame control for ATIM packet */
	atim_header->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ATIM);
	
	/* Sequence control */
	atim_header->seq_ctrl = tx_packets_sent + 0x10;
	/* Increment sequence counter */
	tx_packets_sent += 0x10;
	/* Update the duration of the frame */
	
	/* Assign packet to the socket buffer */
	atim_packet->data = (uint8_t*)atim_header;
	/* Calculate duration */
	atim_header->duration_id = ieee80211_duration(atim_packet, unique_vif->bss_conf.use_short_preamble);
		
	/* Assign packet length */
	atim_packet->len = sizeof(struct ieee80211_hdr_3addr);
	
	#if IBSS_TX_DEBUG_DEEP
	int ii;
	for(ii=0; ii<24; ii++) 
		printf("%02x ",atim_packet->data[ii]);
	printf(" \n");
	#endif
	/* Queue the prepared ATIM in the buffer of pending ATIM packets.
	 * Notice that the IEEE80211 scheduler is responsible for acting
	 * accordingly on a non-empty packet buffer.
	 */
	__ieee80211_tx_atim(atim_packet);

}


bool ieee80211_start_xmit( struct sk_buff *skb, U8* da, U8* next_hop, bool free_buf )
{
	
	#if IBSS_TX_DEBUG_DEEP
	printf("SKB: [%u]",skb->len);
	int i;
	for (i=0; i<skb->len; i++) {
		printf("%02x ", (skb->data)[i]);
	}
	printf(" \n");
	#endif
	
	/* Frame Control of the IEEE80211 MAC packet [FC] */
	le16_t fc;
	
	/* MAC header length */
	uint16_t hdrlen;
	
	/* IEEE80211 Header Structure initialization */
	struct ieee80211_hdr_3addr hdr;
	memset(&hdr, 0, sizeof(struct ieee80211_hdr_3addr));
	
	/* Set up frame control info for data packets */
	fc = cpu_to_le16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA);
	
	/* Append the next-hop destination address to the header */
	memcpy(hdr.addr1, next_hop, ETH_ALEN);
	/* Append the source address to the header */
	memcpy(hdr.addr2, unique_vif->addr, ETH_ALEN);
	/* Depending on the PS Mode, the Address-3 field may hold different info. */
	if (ibss_info->ps_mode == IBSS_MH_PSM) {
		/* Append the final destination address. Note that here the Layer 3 and 
		 * Layer 2 addresses are identical, but normally the IEEE80211 module 
		 * should ask the Routing Layer to resolve the MAC address of the final 
		 * destination. */
		memcpy(hdr.addr3, da, ETH_ALEN);
	} else {
		/* Append the SSID to the header for the standard or null PSM. */
		memcpy(hdr.addr3, unique_vif->bss_conf.bssid, ETH_ALEN);
	}
	
	/* Header length will be with 3 address fields, i.e. the basic length is 24 */
	hdrlen = 24;
	
	/* receiver and we are QoS enabled, use a QoS type frame */
	fc |= cpu_to_le16(IEEE80211_STYPE_QOS_DATA);
	/* Note that we must zero this memory later */
	hdrlen += 2;
	
	/* Set-up the frame control, duration and sequence control fields.
	 * Notice that they may all change later [Sure?]
	 */
	hdr.frame_control = fc;
	hdr.duration_id = 0;
	hdr.seq_ctrl = 0;
	
	/* Set-up encapsulation field info [TODO - check if required] */	
	U8 encaps_data[ENCAPS_LEN] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
	
	/* Header + Encaps + Payload */	
	U8* mac_pkt = smalloc(hdrlen + skb->len + ENCAPS_LEN);
	if (!mac_pkt) {
		printf("ERROR: No memory for packet allocation.\n");
		goto error_free;
	}
	/* Copy packet content */
	memcpy(mac_pkt, &hdr, hdrlen);
	memset(mac_pkt + hdrlen - 2, 0, 2);
	memcpy(mac_pkt + hdrlen, &encaps_data[0], 6);
	memcpy(mac_pkt + hdrlen + 6, skb->data, skb->len);
	
	/* Total length */
	uint16_t mac_pkt_len = skb->len + ENCAPS_LEN + hdrlen;
	
	#if IBSS_TX_DEBUG_DEEP
	printf("SKB: [%u]",skb->len);
	int i;
	for (i=0; i<skb->len; i++) {
		printf("%02x ", (skb->data)[i]);
	}
	printf(" \n");
	#endif
	
	/* Free payload buffer content if requested. */
	if (free_buf) {
		/* For Contiki OS  UIP, we should not normally need to delete this buffer. */
		free(skb->data);
	}
	
	
	/* Attach MAC buffer*/
	skb->data = mac_pkt;
	skb->len = mac_pkt_len;
	
	#if IBSS_TX_DEBUG_DEEP
	printf("SKB: [%u]",skb->len);
	for (i=0; i<skb->len; i++) {
		printf("%02x ", (skb->data)[i]);
	}
	printf(" \n");
	#endif
	
	/* Send a prepared IEEE80211 packet */	
	return ieee80211_tx(skb);

error_free:
	free(skb->data);
	free(skb);
	return false;
}