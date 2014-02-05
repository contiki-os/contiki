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
#include "string.h"
#include "ar9170.h"
#include "ar9170_psm.h"
#include "ieee80211_psm.h"
#include "ieee80211_debug.h"
#include "ieee80211_rx.h"
#include "ieee80211_ibss.h"
#include "etherdevice.h"
#include "ibss_util.h"
#include "ieee80211_mh_psm.h"
#include "mac80211.h"
#include "if_ether.h"
#include "netstack.h"
#include "packetbuf.h"
#include "rtimer.h"
#include "ieee80211.h"
#include "rimeaddr.h"
#include "interrupt\interrupt_sam_nvic.h"
#include <stdint-gcc.h>



void __ieee80211_rx( struct sk_buff * skb )
{	
	/* Branch control handover to the different handlers, 
	 * depending on whether the packet is a management or
	 * a data frame.
	 */
	#if IBSS_RX_DEBUG_DEEP
	int i;
	uint8_t* test = (uint8_t*)skb->data;
	printf("IBSS received MPDU [%d].\n", (unsigned int)(skb->len));
	for (i=0; i<skb->len; i++)
		printf("%02x ", test[i]);
	printf(" \n");
	#endif	
	
	/* First process data frames, then management frames. */
	struct ieee80211_hdr* pkt_head = (struct ieee80211_hdr*)skb->data;	
	
	//Debug
/*
	uint8_t test_eth_addr[ETH_ALEN] = {0x24, 0x65, 0x11, 0xc0, 0x77, 0x92};
		
	if (ether_addr_equal(test_eth_addr, pkt_head->addr2) && !ieee80211_is_beacon(le16_to_cpu(pkt_head->frame_control))) {
		printf("From selected node [%u].\n",skb->len);
		int k;
		for(k=0; k<skb->len; k++) {
			printf("%02x ", skb->data[k]);
		}
		printf(" \n");
	}		
*/		
	if (ieee80211_is_mgmt(le16_to_cpu(pkt_head->frame_control))) {
		
		ieee80211_rx_process_mgmt_mpdu(skb);
	
	} else if (ieee80211_is_data(le16_to_cpu(pkt_head->frame_control))) {
		
		ieee80211_rx_process_data_mpdu(skb);
	
	} else {
		printf("WARNING: Received something not data or management.\n");
	}
}


void ieee80211_rx_process_mgmt_mpdu(struct sk_buff* skb) 
{
	/* Branch control handover to the different handlers, 
	 * depending on whether the packet is either ATIM or  
	 * a BCN frame. For the moment we do not handle the 
	 * other types of management frames.
	 */
	#if IBSS_RX_DEBUG_DEEP
	printf("DEBUG: IBSS; Processing MGMT frame.\n");
	#endif
			
	struct ieee80211_mgmt* mgmt = (struct ieee80211_mgmt*)(skb->data);
		
	if (ieee80211_is_beacon(cpu_to_le16(mgmt->frame_control))) {
		
		ieee80211_rx_process_beacon(mgmt, skb->len);
	
	} else if (ieee80211_is_atim(cpu_to_le16(mgmt->frame_control))) {

		ieee80211_rx_process_atim(mgmt);
	
	} else if (ieee80211_is_probe_req(cpu_to_le16(mgmt->frame_control)) || 
			   ieee80211_is_probe_resp(cpu_to_le16(mgmt->frame_control))) {
		
		// Do not process probe requests and responses
		
	} else {	
		/* Do not handle other management packets. */
		#if IBSS_RX_DEBUG_DEEP
		printf("WARNING: Other kind of management packet: %04x\n", mgmt->frame_control);
		#endif
	}
}


void ieee80211_rx_process_atim(struct ieee80211_mgmt* mgmt) {	
	
	/* If the device is not in the ATIM window, or pre-TBTT
	 * window, this ATIM frame should be discarded. This is
	 * a big question, whether this is accurate. We are not
	 * supposed to transmit ATIM packets outside the ATIM 
	 * window but nothing says we are not allowed to receive.
	 */
	if (ar9170_get_device()->ps_mgr.psm_state != AR9170_ATIM_WINDOW &&
	   ar9170_get_device()->ps_mgr.psm_state != AR9170_PRE_TBTT) {
			
		#if IBSS_RX_DEBUG_DEEP
		printf("WARNING: IBSS; ATIM frame arrived in wrong state.\n");
		#endif
		return;
	}
			
	#if IBSS_RX_DEBUG_DEEP
	printf("DEBUG: IBSS; Received ATIM frame.\n");
	#endif	
	
	/* An ATIM frame that is not destined for us, is not handled.
	 * This is because in the current implementation we do not 
	 * consider group-addressed ATIM frames, just the individually 
	 * addressed ones. XXX A good question regarding overhearing:
	 * do we add the transmitted of this ATIM frame to the list
	 * of awake neighbors, or does this decision depend on the
	 * response status [ACK]?
	 */
	
	/* Create a temporary variable for the broadcast Ethernet address. */
	uint8_t eth_broadcast_addr[ETH_ALEN] = BROADCAST_80211_ADDR;
	
	if ((!ether_addr_equal(mgmt->da, unique_vif->addr)) &&
		(!ether_addr_equal(mgmt->da, eth_broadcast_addr))) {
		
		#if IBSS_RX_DEBUG
		printf("DEBUG: IBSS; ATIM packet not for me. Drop.\n");
		#endif
		return;
	} 
	
	/*
	 * ATIM Frames have no body; just MAC header. We handle the 
	 * ATIM frames depending on whether we are in PSM or MH-PSM
	 * mode. The control is handled to the PSM manager if PSM is
	 * enabled, otherwise we just drop the packet. XXX Check if 
	 * this is actually correct [Ask Vladimir].
	 */
	
	/* Obtain a reference to the AR9170 device. */
	struct ar9170* ar = ar9170_get_device();
	
	/* Check whether the device is in PS Mode. */
	if (ar->hw->conf.flags & IEEE80211_CONF_PS) {
		
		/* 
		 * ATIM receptions will be handled by the PSM manager
		 */
		ieee80211_rx_handle_ATIM_pkt(ar, mgmt);
				
	} else {
				
		/* Packet should not be handled. */
	}
}



void ieee80211_rx_process_beacon(struct ieee80211_mgmt* mgmt, size_t len) {
	
	
	/* If the device is not in the pre-TBTT window,
	 * this beacon frame should be discarded. Note, 
	 * however, that this applies only in the case
	 * after network initialization, since before 
	 * it the device is by default in the transmit
	 * period.
	 */
	if (ar9170_get_device()->ps_mgr.psm_state != AR9170_PRE_TBTT) {
			
		if (ieee80211_is_ibss_joined()) {
			
			#if IBSS_RX_DEBUG_DEEP
			printf("WARNING: IBSS; BCN arrived not in pre-TBTT state.\n");
			#endif
			return;
		}		
	}
	
	#if IBSS_RX_DEBUG_DEEP
	printf("DEBUG: Processing received BCN frame.\n");	
	printf("DEBUG: IBSS; advertised BCN Interval: %04x\n",le16_to_cpu(mgmt->u.beacon.beacon_int));
	printf("DEBUG: IBSS; advertized BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n", mgmt->bssid[0], mgmt->bssid[1],
		mgmt->bssid[2],mgmt->bssid[3],mgmt->bssid[4],mgmt->bssid[5]);
	printf("DEBUG: IBSS; SA: %02x:%02x:%02x:%02x:%02x:%02x\n", mgmt->sa[0], mgmt->sa[1],
		mgmt->sa[2], mgmt->sa[3], mgmt->sa[4], mgmt->sa[5]);
	printf("DEBUG: IBSS; DA: %02x:%02x:%02x:%02x:%02x:%02x\n", mgmt->da[0], mgmt->da[1],
		mgmt->da[2], mgmt->da[3], mgmt->da[4], mgmt->da[5]);		
	#endif
	
	/* Process --ONLY-- beacons from the default IBSS [BSSID]. */
	if(!ether_addr_equal(mgmt->bssid, unique_vif->bss_conf.bssid)) {
		
		/* Beacon Not from the default IBSS. */
		#if IEEE80211_IBSS_DEBUG_DEEP
		printf("DEBUG: IBSS; BCN not from default network.\n");
		#endif
		return;
	}
	
	size_t baselen;
	struct ieee802_11_elems elems;
	
	/* Process beacon from the current [default] IBSS */
	baselen = (U8 *) mgmt->u.beacon.variable - (U8 *) mgmt;
	if (baselen > len) {
		printf("WARNING: Received beacon too small.\n");
		return;	
	}
	/* Parse the elements sent with the currently received beacon. */		
	ieee802_11_parse_elems(mgmt->u.beacon.variable, len - baselen, &elems);
	
	U8* ssid = elems.ssid;
	#if IBSS_RX_DEBUG_DEEP	
	printf("SSID: %02x:%02x:%02x:%02x:%02x:%02x\n",ssid[0],ssid[1],ssid[2],ssid[3],ssid[4],ssid[5]);	
	int i;
	printf("DEBUG: IBSS; IBSS Params: ");
	for (i=0; i<elems.ibss_params_len; i++) {
		printf("%02x ", elems.ibss_params[i]);
	}		
	printf(" \n");
	#endif	
	
	/* Check if we have joined the default IBSS and join now if not already.
	 * [Joining involves a call to the _sta_join_ibss function.]
	 */
	if (not_expected(ieee80211_is_ibss_joined() == false)) {
		
		#if IBSS_RX_DEBUG_DEEP	
		printf("DEBUG: IBSS; joining default IBSS now...\n");
		#endif
		
		/* Join the default IBSS with the overridden parameters:
		 *
		 * - Beacon Interval
		 * - ATIM Window 
		 * - SSID 
		 */
		ieee80211_sta_join_ibss(le16_to_cpu(mgmt->u.beacon.beacon_int), 
				((U16*)elems.ibss_params)[0], ssid);
				
	} else {
		
		#if IEEE80211_IBSS_DEBUG_DEEP
		printf("DEBUG: IBSS; Already initialized. Checking for changes...\n");
		#endif
		
		/* Check and update BSS and AR9170 configuration: BCN & ATIM Window. */
		if ((unique_vif->bss_conf.beacon_int !=  mgmt->u.beacon.beacon_int) ||
			(unique_vif->bss_conf.atim_window != ((U16*)elems.ibss_params)[0])) {
				
				/* Store the updated values. */
				unique_vif->bss_conf.beacon_int =  mgmt->u.beacon.beacon_int;
				unique_vif->bss_conf.atim_window = ((U16*)elems.ibss_params)[0];
				
				/* Notify the driver for the network changes */
				__ieee80211_bss_change_notify(BSS_CHANGED_BEACON_INT);
				
				#if IBSS_RX_DEBUG_DEEP
				printf("DEBUG: Received updated beaconing information.\n");
				#endif
			}
		
		
		/* 
		 * If the device is in PS mode, add the sender SA of
		 * the received beacon to the list of awake nodes for
		 * the current beacon interval, if not there already.
		 *
		 * Due to soft beaconing operation, this is dangerous,
		 * so for the moment we do not update the neighbors'
		 * list due to beacon receptions.
		 */
		struct ar9170* ar = ar9170_get_device();
		
		if (ar->hw->conf.flags & IEEE80211_CONF_PS) {
		/*	
			if (!ieee80211_psm_is_remote_da_awake(ar, mgmt->sa))
				ieee80211_psm_add_awake_node(ar, mgmt->sa);				
		*/
		}			
	}
}





void ieee80211_rx_process_data_mpdu(struct sk_buff* skb)
{
	#if IBSS_RX_DEBUG_DEEP
	int i;
	uint8_t* test = (uint8_t*)skb->data;
	printf("IBSS received MPDU [%d].\n", (unsigned int)(skb->len));
	for (i=0; i<skb->len; i++)
	printf("%02x ", test[i]);
	printf(" \n");
	#endif
	#if IEEE80211_IBSS_DEBUG_DEEP
	printf("DEBUG: IBSS; Processing DATA frame.\n");
	#endif
		
	/* Implement some consistency checking. */
	if (skb == NULL) {
		printf("WARNING: data frame is NULL!\n");
		return;
	}
	if (skb->data == NULL || skb->len == 0) {
		printf("WARNING: Zero or NULL Content!\n");
		return;
	}	
	/* Create a stack copy of the Ethernet broadcast address. */
	static const uint8_t broadcast_ethaddr[ETH_ALEN] = BROADCAST_80211_ADDR;
	
	/* Extract the header of the packet. Check whether the
	 * packet is destined for the local node [MAC address].
	 * Normally this check should be done of the Contiki OS
	 * RDC driver, as it is done, e.g. in ContikiMac.
	 */
	struct ieee80211_hdr_3addr* pkt_head = (struct ieee80211_hdr_3addr*)(skb->data);
		
	if ((!ether_addr_equal(pkt_head->addr1, unique_vif->addr)) && 
		(!ether_addr_equal(pkt_head->addr1, broadcast_ethaddr))) {
		/* The received packet is for some other station. */
		#if IBSS_RX_DEBUG_DEEP
		printf("DEBUG: IBSS_RX; Packet is not destined for us.\n");
		#endif
		return;
	}
	/* The packet is for us. Check whether the BSSID is the default one. */
	if (!ether_addr_equal(pkt_head->addr3, unique_vif->bss_conf.bssid)) {
		/* The packet is in fact for us, however, from an un-known IBSS. */
		#if IBSS_RX_DEBUG_DEEP
		printf("DEBUG: IBSS_RX; Packet is not from the default IBSS. Strange. \n");
		printf("%02x %02x %02x %02x %02x %02x\n", 
			pkt_head->addr3[0], 
			pkt_head->addr3[1], 
			pkt_head->addr3[2], 
			pkt_head->addr3[3], 
			pkt_head->addr3[4], 
			pkt_head->addr3[5]);
		#endif
		return;
	}
	
	#if IBSS_RX_DEBUG_DEEP
	int i;
	uint8_t* test = (uint8_t*)skb->data;
	printf("IBSS received MPDU destined for us [%u]:\n", (unsigned int)(skb->len));
	for (i=0; i<skb->len; i++) {
		printf("%02x ", test[i]);
	}		
	printf(" \n");
	#endif
	
	
	/* The packet is destined for us. Extract the packet payload. 
	 * This means that we need to remove the header content that
	 * has a fixed length of 24+2+6 bytes.
	 */
	
	/* From TX: Header [24+2] + Encaps [6] + Payload [...] */
	COMPILER_WORD_ALIGNED uint8_t* pkt_payload = skb->data + 32;		
		
	if(skb->len-32-4 > 0) {
		/* For now we send the packet up to the packet recorder, 
		 * which implements multi-hop relaying. We plan to link
		 * the ieee80211_rx module to uIP, so the processing of
		 * received packets traverses the Contiki IP stack.
		 */
		
		/* Hand-in the packet to the traffic recorder. 
		 * We have removed this; now we link it with 
		 * the Contiki OS standard procedure.
		 */		
		//trec_record_packet(pkt_payload, skb->len-32-4);
		
		/* We must copy the packet to the packet buffer. 
		 * This is normally done on the Radio level in 
		 * Contiki OS. Here we do it on the MAC level.
		 * FYI, we copied the implementation from the 
		 * cc2420.c file.
		 */
		
		/* Clear the packet buffer in case of existing trash. */
		packetbuf_clear();

		/* Register the reception time-stamp as an attribute
		 * in the packet buffer. 
		 */
		packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, RTIMER_NOW());
		
		/* Register the packet id [sequence number] for the 
		 * received packet. This is be done to avoid having
		 * duplicate receptions.
		 */
		packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, pkt_head->seq_ctrl);
				
		/* This is a workaround in our effort to merge Contiki OS with 
		 * 802.11. The Contiki OS uses rimeaddr_t structures to store
		 * MAC level addresses, which are 8-bytes long, while we only
		 * use Ethernet addresses, which are 6 bytes. We thus build the
		 * rime addresses by setting the last two bytes to zero, and 
		 * adding the Ethernet address in the beginning of the rimeaddr.
		 */
		
		/* Register the MAC source and destination addresses. */
		rimeaddr_t sa_address, da_address;
		
		memset(sa_address.u8,0,RIMEADDR_SIZE);
		memset(sa_address.u8,0,RIMEADDR_SIZE);
		memcpy(sa_address.u8, pkt_head->addr2, ETH_ALEN);
		memcpy(da_address.u8, pkt_head->addr1, ETH_ALEN);
				
		/* Register the packet MAC source address. */
		packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &sa_address);
		
		/* Register the packet MAC destination address. */
		packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &da_address);
		
		/* Print the addresses [debug]. */
		

		
		/* Copy the data to the packet buffer removing the last 4 bytes of FCS. */
		uint8_t* packet_buffer_ptr = packetbuf_dataptr();	

		memcpy(packet_buffer_ptr, skb->data, skb->len-FCS_LEN);

		/* Set the data length at the packet buffer. This 
		 * includes the MAC buffer but not the last 4 bytes. 
		 */		
		packetbuf_set_datalen(skb->len - FCS_LEN);		
				
		/* Hand in control to the MAC module, on the top of this
		 * receiver. By default, this is the ieee80211 driver.
		 */
		NETSTACK_MAC.input();
	
	} else {
		printf("WARNING: Packet contained no data payload.\n");
	}	
}



void ieee80211_rx_handle_ATIM_pkt(struct ar9170* ar, struct ieee80211_mgmt* mgmt) {
	
	/*
	 * The current ATIM packet is for us, and we are in power-saving
	 * mode so we need to extract the source address of the packet and
	 * push it to the list of AWAKE neighbors for the current beacon
	 * interval.
	 */
	
	/* Extract the source address of the packet */
	U8* source_address = mgmt->sa;
	
	/* Receiving an ATIM frame means that we need to stay awake in 
	 * the following Beacon interval. So we update the PS flag.
	 */
	ar->ps.off_override |= PS_OFF_ATIM;
	
	/* Although, we might not need to send data to this station, we
	 * anyway place its SA in the list of AWAKE receivers for the 
	 * current Beacon interval, if it is not there already. We don't
	 * skip this even for the broad casted ATIM frames.
	 */	
		
	if (!ieee80211_psm_is_remote_da_awake(ar, source_address)) {
		ieee80211_psm_add_awake_node(ar, source_address);
	}	
	
	
	/* Create a temporary variable for the broadcast Ethernet address. */
	uint8_t eth_broadcast_addr[ETH_ALEN] = BROADCAST_80211_ADDR;

	if (ether_addr_equal(eth_broadcast_addr, mgmt->da)) {
		#if IBSS_RX_DEBUG_DEEP
		printf("Do not handle broadcast ATIMs.\n");
		#endif
		return;
	}		
	
	/* Check if the device is in the Multi-Hop Power-Save Mode, and 
	 * handler the control to the respective function. Notice that 
	 * this operation is complementary to the above function call, 
	 * not supplementary!
	 */
	if (ibss_info->ps_mode == IBSS_MH_PSM) {
		/* Extra functionality due to advanced power-save mode. 
		 * This will normally forward the received ATIM.
		 */
		ieee80211_mh_psm_handle_atim_packet(ar, mgmt);
	}
}

/* This function is called inside interrupt context. */
void ieee80211_handle_ATIM_status_rsp(bool success) {
	
	if(!success) {
		
		/* Check if the ATIM was for a broad-casted frame. 
		 * This is a workaround and shall be re-designed, 
		 * for better implementation. Currently we demand
		 * broad-casted frames to be acknowledged just as 
		 * all the remaining frames. Since stations do not
		 * send ACKs for broadcast frames, we have to handle
		 * this situation here.
		 */
		const uint8_t broadcast_eth_addr[ETH_ALEN] =  BROADCAST_80211_ADDR;
		if (ether_addr_equal(broadcast_eth_addr, ar9170_get_device()->ps_mgr.last_ATIM_DA)) {
			/* ATIM status is for broadcast frame. This is not a problem.*/
		
		} else {
		
		#if IBSS_RX_DEBUG
		/* Print a warning? */	
		printf("ANA\n");
		#endif
		return;
		}
				
	}		
	
	#if IBSS_RX_DEBUG_DEEP
	printf("INFO: ATIM sent successfully.\n");
	#endif
	
	/* The handling of a successful ATIM status response relies on the 
	 * actual PS mode implemented by the node. In standard PS mode, the
	 * device moves the saved DA in the list of awake neighbors for the
	 * current BCN interval.
	 */
	if ((ibss_info->ps_mode == IBSS_STD_PSM) || (ibss_info->ps_mode == IBSS_MH_PSM)) {
		
		/* Obtain the reference for the AR9170 device. */
		struct ar9170* ar = ar9170_get_device();
		
		/* There exists a rare scenario that a [late] beacon reception 
		 * has already populate the list of neighbors with this address
		 * so we check before adding a redundant element.
		 */
		if (ieee80211_psm_is_remote_da_awake(ar, ar->ps_mgr.last_ATIM_DA)) {
			/* Already there. */
			printf("DEBUG: DA is already in the list.\n");			
			return;
		}
		
		U8* awake_da = (U8*)malloc(ETH_ALEN);
		if (!awake_da) {
			printf("ERROR: No memory for DA allocation in the list of neighbors.\n");
		}
		
		if (ar) {
			memcpy(awake_da, ar->ps_mgr.last_ATIM_DA, ETH_ALEN);
			memset(ar->ps_mgr.last_ATIM_DA, 0, ETH_ALEN);
			
		} else {
			
			printf("ERROR: AR9170 device is NULL.\n");
		}
		
		
		/* Add the new DA in the list of awake neighbors. */
		int position = linked_list_add_tail(ar->ps_mgr.wake_neighbors_list, awake_da,
			AR9170_PSM_MAX_AWAKE_NODES_LIST_LEN, false);
		
		#if IBSS_RX_DEBUG_DEEP
		printf("DEBUG: IBSS; Adding %02x:%02x:%02x:%02x:%02x:%02x in the list of awake nodes in position %d Length: %d.\n",
			awake_da[0],
			awake_da[1],
			awake_da[2],
			awake_da[3],
			awake_da[4],
			awake_da[5],
			position,
			linked_list_get_len(ar->ps_mgr.wake_neighbors_list));
		#endif
		
		if (position < 0) {
			printf("ERROR: DA could not be added in the list of AWAKE nodes.\n");
		}
	
	} else {
		
		/* The device is non in PS-Mode. So we need to signal an error, 
		 * since we received an ATIM status response.
		 */
		printf("ERROR: ATIM status response while not in PSM.\n");
	}
	
	if (ibss_info->ps_mode == IBSS_MH_PSM) {
		/* 
		 * In the Multi-Hop PS Mode, a status response from the next-hop Station 
		 * implies that also the final destination can be pushed into the list 
		 * of awake nodes for the current beacon interval. This is of course a 
		 * convention, in order to defer from sending duplicate ATIMs for the 
		 * same reason. 
		 *
		 * We could as well add all intermediate relays to the list of awake 
		 * neighbors, as a result of this successful status response. Of course
		 * we rely on the next-hop station to successfully propagate the ATIM
		 * frame to the final destination.
		 */
		#if IBSS_RX_DEBUG_DEEP
		printf("DEBUG: IBSS_RX; Got ATIM Status response while in MH-PSM.\n");
		#endif
		
		U8* awake_a3 = (U8*)malloc(ETH_ALEN);
		if (!awake_a3) {
			printf("ERROR: NO memory for A3 allocation in the list of neighbors.\n");
		}
		/* Obtain the reference for the AR9170 device. */
		struct ar9170* ar = ar9170_get_device();
		if (ar) {
			memcpy(awake_a3, ar->ps_mgr.last_ATIM_A3, ETH_ALEN);
			memset(ar->ps_mgr.last_ATIM_A3, 0, ETH_ALEN);
			
		} else {
			
			printf("ERROR: AR9170 device is NULL.\n");
			return;
		}
		/* Add the new DA in the list of awake neighbors. */
		int position = linked_list_add_tail(ar->ps_mgr.wake_neighbors_list, awake_a3,
			AR9170_PSM_MAX_AWAKE_NODES_LIST_LEN, false);
		
		#if IBSS_RX_DEBUG_DEEP
		printf("DEBUG: IBSS; Adding a new A3 in the list of awake nodes in position %d Length: %d.\n",position,
		linked_list_get_len(ar->ps_mgr.wake_neighbors_list));
		#endif
		
		if (position < 0) {
			printf("ERROR: A3 could not be added in the list of AWAKE nodes.\n");
		}		
	}	
}