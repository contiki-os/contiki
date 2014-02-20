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
#include "ieee80211_debug.h"
#include "ieee80211_mh_psm.h"
#include "ieee80211_ibss.h"
#include "etherdevice.h"
#include "ieee80211_tx.h"
#include "cc.h"
#include "if_ether.h"

static ieee80211_mh_psm_address_map* mh_psm_addr_map;

ieee80211_mh_psm_address_map* ieee80211_mh_psm_get_address_map() {
	
	return mh_psm_addr_map;
} 

/* 
 * Initialize the address mapping for multi-hop PSM. 
 * The function parses the source routing info, which
 * is hard-coded in the program flash and resolves the
 * next-hop MAC address for the [fixed] end-node MAC
 * address. 
 */
bool ieee80211_mh_psm_init_address_map() {
		
	if (mh_psm_addr_map != NULL) {
		
		#if IBSS_MH_PSM_DEBUG
		printf("WARNING: List is already initialized.\n");
		#endif
		return true;
	}
	
	/* Initialize the linked list. */
	mh_psm_addr_map = linked_list_init(mh_psm_addr_map);
	
	if (mh_psm_addr_map == NULL) {		
		printf("ERROR: MH_PSM Address Map could not be initialized.\n");
		return false;
	}
	
	/* TODO: If we want to use MH-PSM we must implement this function, 
	 * e.g. with duplicate address detection. For compatibility we just
	 * return false here.
	 */
	return false;
}


bool ieee80211_mh_psm_atim_list_contains_A3( struct ar9170* ar, U8* bssid ) 
{
	int i;
	/* Extract the ATIM queue for the considered AR9170 device. */
	ar9170_tx_queue* atim_queue = ar->tx_pending_atims;
	
	if (atim_queue == NULL) {
		/* The list should normally be initialized, so we signal a warning. */
		#if IBSS_MH_PSM_DEBUG
		printf("WARNING: PSM; ATIM queue is null. Should not occur.\n");
		#endif
		ar->tx_pending_atims = linked_list_init(ar->tx_pending_atims);
		return false;
	}		
	
	if (linked_list_is_empty(ar->tx_pending_atims)) {
		/* If the list is empty, we can immediately return. */
		#if IBSS_MH_PSM_DEBUG_DEEP
		printf("DEBUG: PSM; The ATIM list is empty.\n");
		#endif
		return false;
	} 
	/* The ATIM list is non-empty at this point. */
	
	/* Walk through the list of ATIM frames. If an ATIM has already 
	 * been created for the requested A3, we do not need to create
	 * a new one.
	 */
	for (i=0; i<linked_list_get_len(ar->tx_pending_atims); i++) {
		
		/* Extract packet */
		struct sk_buff* atim_packet = linked_list_get_element_at(atim_queue, i);
		
		/* Extract data from packet */
		struct ieee80211_hdr* atim_header = (struct ieee80211_hdr*)(atim_packet->data);
				
		/* Extract A3 address */
		U8* atim_a3 = atim_header->addr3;
		
		if (ether_addr_equal(atim_a3, bssid)) {
			#if IBSS_MH_PSM_DEBUG_DEEP
			printf("DEBUG: PSM; ATIM for current DA is already pending.\n");
			#endif
			return true;
		}
	}
	/* The A3 was not found among the ones of the pending ATIMS. Signal "false",
	 * so the ATIM is constructed.
	 */
	return false;
}




U8* ieee80211_mh_psm_resolve_next_hop( U8* bssid ) 
{
	if (mh_psm_addr_map == NULL) {
		/* The address map should be initialized during setup, so we should signal a warning here. */
		printf("WARNING: MH-PSM Address Map is NULL. This should not happen.\n");
		return NULL;
	}
		
	if (linked_list_is_empty(mh_psm_addr_map)) {
		/* If the list is empty, we should signal a warning message. */
		#if IBSS_MH_PSM_DEBUG
		printf("WARNING: MH-PSM; The address map is empty. Cannot resolve next-hop.\n");
		#endif
		return NULL;		
	}
	
	/* Otherwise walk through the list and if the list contains the right end address,
	 * return the corresponding next hop mac address, otherwise, return NULL.
	 */
	int i;
	for (i=0; i<linked_list_get_len(mh_psm_addr_map); i++) {
		
		struct ieee80211_mh_psm_address_mapping* addr_mapping = 
			(struct ieee80211_mh_psm_address_mapping*)linked_list_get_element_at(mh_psm_addr_map,i);
		if (addr_mapping == NULL) {
			/* Signal an error, since the map should not contain NULL elements. */
			printf("ERROR: The map return a NULL element.\n");
			return NULL;
			
		} else {
			/* The map returned a non-null element. Check if this is the right one. */
			if ( ether_addr_equal(addr_mapping->end_addr, bssid) ) {
				/* The correct address was found. */
				return addr_mapping->next_hop_addr;
			}
		}
	}
	/* If we have reached this point, it means that we could not find the address in the 
	 * map and we should signal a warning. 
	 */
	printf("WARNING: The next-hop address could not be resolved. The end address is not in the map.\n");
	return NULL;
}




void ieee80211_mh_psm_handle_atim_packet( struct ar9170* ar, struct ieee80211_mgmt* mgmt )
{
	#if IBSS_MH_PSM_DEBUG_DEEP
	printf("DEBUG: MH-PSM; Handling ATIM packet.\n");
	#endif	
	
	/* Check whether the Address-3 field of the ATIM header contains something. */
	if(mgmt->bssid == NULL) {
		/* It is possible that the A3 field contains the BSSID, but is it not 
		 * possible that it contains nothing. So we signal a warning here.
		 */
		printf("WARNING: BSSID Field of ATIM packet is NULL. Should not happen.\n");
		return;
	}
	
	/* Check whether the Address-3 field of the ATIM header contains the SSID of 
	 * the default network. If yes, this means that the sending device does not 
	 * implement the advanced PSM scheme, so the current node can not continue 
	 * with ATIM forwarding. 
	 */
	if(ether_addr_equal(mgmt->bssid, ibss_info->ibss_bssid)) {
		
		#if IBSS_MH_PSM_DEBUG
		printf("DEBUG: MH-PSM; ATIM Address-3 contains the BSSID. Can not continue.\n");
		#endif	
		return;
	}
	
	/* We now extract the Address-3 field and check, first, whether the stored 
	 * MAC address is our own MAC address; in this case we can not continue, as
	 * the ATIM's final destination is, certainly, us.
	 */
	if(ether_addr_equal(mgmt->bssid, unique_vif->addr)) {
		
		#if IBSS_MH_PSM_DEBUG_DEEP
		printf("DEBUG: MH-PSM; ATIM Address-3 field contains our MAC address. Can not continue.\n");
		#endif
		return;
	}
	
	/* We now need to handle valid Address-3 fields, filled-in by stations in 
	 * multi-hop power-save mode. We, first, check, whether the Station with 
	 * the MAC address stored in the Address-3 field is among the awake nodes
	 * list of the current node. If yes, the ATIM forwarding is redundant.
	 */
	if (ieee80211_psm_is_remote_da_awake(ar, mgmt->bssid)) {
	
		#if IBSS_MH_PSM_DEBUG_DEEP
		printf("DEBUG: MH-PSM; Address-3 field contains awake node. ATIM forwarding is redundant.\n");
		#endif
		return;			
	}		
	 
	/* We, now, check whether an identical ATIM frame is already contained
	 * in the local queue of pending ATIM frames. Identical means, for the 
	 * same Address-3 field. In such case the ATIM creation is redundant. 
	 */ 
	if (ieee80211_mh_psm_atim_list_contains_A3(ar, mgmt->bssid)) {
		
		#if IBSS_MH_PSM_DEBUG_DEEP
		printf("DEBUG: MH-PSM; An ATIM with the same A3 field is already pending.\n");
		#endif
		return;		
	} 
	 
	/* At this point we know that the remote station is not in the list of STAs
	 * known to be awake, so we should attempt to wake him up through the multi-
	 * hop ATIM forwarding scheme. We extract the stored MAC address and we try 
	 * to resolve the respective next-hop MAC address. Note that in the current
	 * implementation, the whole thing is handled in the MAC layer, but as it 
	 * is, clearly, a cross-layer design, we should handle the next-hop address
	 * resolve in the routing layer. TODO
	 */
	U8* next_hop_mac_addr = ieee80211_mh_psm_resolve_next_hop(mgmt->bssid);
	
	/* If the result of the next-hop resolve is NULL, we can not continue, but
	 * we signal a warning, as it should not, normally, happen, in a scenario,
	 * where routing and Multi-Hop MAC resolution are implemented correctly.
	 */
	if (next_hop_mac_addr == NULL) {
		/* Print warning. TODO - handle network routing discovery. */
		printf("WARNING: Next-hop MAC resolution failed.\n");
		return;
	}
		
	/* We must now proceed with an ATIM frame generation with the Address-3 
	 * field resolved above. The ATIM frame is pushed in the tail of the 
	 * pending ATIMs queue, and the scheduler will send it at the earliest 
	 * occasion.
	 */
	#if IBSS_MH_PSM_DEBUG_DEEP
	printf("MH_PSM; ATIM forward\n");
	printf("To: %02x:%02x:%02x:%02x:%02x:%02x\n",next_hop_mac_addr[0],
		next_hop_mac_addr[1],
		next_hop_mac_addr[2],
		next_hop_mac_addr[3],
		next_hop_mac_addr[4],
		next_hop_mac_addr[5]);
	#endif
	printf("FA\n");
	ieee80211_create_atim_pkt(ar, next_hop_mac_addr, mgmt->bssid);
	
	return;
}
