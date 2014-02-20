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
#include "string.h"
#include "if_ether.h"
#include "ieee80211_psm.h"
#include "ar9170_psm.h"
#include "ieee80211_debug.h"
#include "etherdevice.h"
#include "ieee80211_ibss.h"
#include "ieee80211_tx.h"
#include "ieee80211_mh_psm.h"
#include "linked_list.h"
#include "ar9170.h"
#include "smalloc.h"


static bool ieee80211_psm_atim_list_contains_DA(struct ar9170* ar, U8* da) {
	
	int i;
	/* Extract the ATIM queue for the considered AR9170 device. */
	ar9170_tx_queue* atim_queue = ar->tx_pending_atims;
	
	if (atim_queue == NULL) {
		/* The list should normally be initialized, so we signal a warning. */
		#if IBSS_PSM_DEBUG
		printf("WARNING: PSM; ATIM queue is null. Should not occur.\n");
		#endif
		ar->tx_pending_atims = linked_list_init(ar->tx_pending_atims);
		return false;
	}		
	
	if (linked_list_is_empty(ar->tx_pending_atims)) {
		/* If the list is empty, we can immediately return. */
		#if IBSS_PSM_DEBUG_DEEP
		printf("DEBUG: PSM; The ATIM list is empty.\n");
		#endif
		return false;
	} 
	/* The ATIM list is non-empty at this point. */
	
	/* Walk through the list of ATIM frames. If an ATIM has already 
	 * been created for the requested DA, we do not need to create
	 * a new one.
	 */
	for (i=0; i<linked_list_get_len(ar->tx_pending_atims); i++) {
		
		/* Extract packet */
		struct sk_buff* atim_packet = linked_list_get_element_at(atim_queue, i);
		
		if (atim_packet == NULL || atim_packet->data == NULL) {
			printf("WARNING: Got null ATIM from the list.\n");
			return false;
		} 
		/* Extract data from packet */
		struct ieee80211_hdr* atim_header = (struct ieee80211_hdr*)(atim_packet->data);
				
		/* Extract DA address */
		U8* atim_da = atim_header->addr1;
		
		if (ether_addr_equal(atim_da, da)) {
			#if IBSS_PSM_DEBUG_DEEP
			printf("DEBUG: PSM; ATIM for current DA is already pending.\n");
			#endif
			return true;
		}
	}
	/* The DA was not found among the ones of the pending ATIMS. Signal "false",
	 * so the ATIM is constructed.
	 */
	return false;
}


void ieee80211_psm_add_awake_node(struct ar9170* ar, U8* sa) {
	
	U8* awake_node = (U8*)smalloc(ETH_ALEN);
	if (!awake_node) {
		printf("ERROR: No memory for allocation of SA for awake list.\n");
		return;
	}
	memcpy(awake_node, sa, ETH_ALEN);
	
	if (ar->ps_mgr.wake_neighbors_list == NULL) {
		printf("WARNING: Neighbors' List is Null. Should not occur.\n");
		ar->ps_mgr.wake_neighbors_list = linked_list_init(ar->ps_mgr.wake_neighbors_list);
	}
	/* We do not need to protect this list from interrupts, as no interrupt access it. */
	int position = linked_list_add_tail(ar->ps_mgr.wake_neighbors_list, awake_node,
			AR9170_PSM_MAX_AWAKE_NODES_LIST_LEN, true);
	
	if (position < 0) {
		printf("WARNING: IBSS; Awake not could not be added to the intended list.\n");
	}
	#if IBSS_PSM_DEBUG_DEEP
	printf("DEBUG: PSM; Node is added to the list in position: %d.\n", position);
	#endif	
}

/* 
 * Important! The functions erases the list elements but also the 
 * possible contents of the elements. It is important that if the
 * elements are references to heap memory, we must also free them
 * so we do not have  memory leaking.
 */
void ieee80211_psm_erase_awake_neighbors(struct ar9170* ar) {
		
	if (!linked_list_is_empty(ar->ps_mgr.wake_neighbors_list))	{
		
		irqflags_t _flags = cpu_irq_save();		
		linked_list_erase_deep(&ar->ps_mgr.wake_neighbors_list);
		cpu_irq_restore(_flags);
	}		
}


/* 
 * This function is called outside the interrupt context,
 * mainly because it might take a long time to create all
 * the required ATIM frames.
 */
void ieee80211_psm_create_atim_pkts(struct ar9170* ar) {
	
	#if IBSS_PSM_DEBUG_DEEP
	printf("DEBUG: IBSS PSM; Create ATIMS.\n");
	#endif
	/* Extract the transmission queue for the AR910 device. */
	ar9170_tx_queue* tx_queue = ar->tx_pending_pkts;
	
	/* Signal a warning if the queue is NULL, as it should 
	 * have been initialized when the AR9170 interface is 
	 * added.
	 */
	if (tx_queue == NULL) {		
		printf("WARNING: PSM; Null list of pending frames. Should not occur.\n");
		ar->tx_pending_pkts = linked_list_init(ar->tx_pending_pkts); 
		return;
	}
	/* If the list of pending packets is empty, we do not need 
	 * to proceed with creating more ATIM frames. Additionally, 
	 * we might want to erase any pending frames as well - TODO
	 */
	if (linked_list_is_empty(tx_queue)) {		
		#if IBSS_PSM_DEBUG_DEEP
		printf("DEBUG: PSM: Empty list. No ATIM packets created.\n");
		#endif
		return;
	}	
	
	/* The ATIM packet creation depends on the implemented 
	 * power-save mode at the Station so the execution will
	 * branch accordingly. 
	 */
	if (ibss_info->ps_mode == IBSS_MH_PSM) {
		/* Walk through the list of pending data packets For
		 * each A3, construct an ATIM frame and place it in
		 * the pending ATIM queue. 
		 */
		int i;
		for (i=0; i<linked_list_get_len(tx_queue); i++) {
		
			/* Extract packet from the list */
			struct sk_buff* packet = linked_list_get_element_at(tx_queue, i);	
			/* Check */
			if (packet == NULL || packet->data == NULL) {
				printf("WARNING: List returned null packet.\n");
				continue;
			}	
			/* Extract data from packet [socket] buffer */
			struct ieee80211_hdr_3addr* pkt_header = (struct ieee80211_hdr_3addr*)packet->data;		
			/* Extract DA address */
			U8* da = pkt_header->addr1;		
			/* Extract the A3 address. */
			U8* a3 = pkt_header->addr3;
			/* If an ATIM for this A3 has not been added to the pending list,
			 * the packet needs to be created. 
			 */
			if (!ieee80211_mh_psm_atim_list_contains_A3(ar,a3)) {
				/* Handle control to the IEEE80211 module, that will
				 * create and push the ATIM to the ATIM queue
				 */
				#if IBSS_MH_PSM_DEBUG_DEEP
				printf("DEBUG: PSM; Creating ATIM packet for a final destination.\n");
				#endif
				ieee80211_create_atim_pkt(ar, da, a3);			
			}
		}
	
	} else if (ibss_info->ps_mode == IBSS_STD_PSM) {
		
		/* Walk through the list of packets that are pending.
		 * For each DA, construct an ATIM frame and place it
		 * in the pending ATIM queue.
		 */
		int i;
		for (i=0; i<linked_list_get_len(tx_queue); i++) {
		
			/* Extract packet from the list. */
			struct sk_buff* packet = linked_list_get_element_at(tx_queue, i);	
			#if IBSS_PSM_DEBUG_DEEP
			int j;
			printf("IBSS: [%u]",packet->len);
			for (j=0; j<packet->len; j++)
				printf("%02x ", (packet->data)[j]);
			printf(" \n");
			#endif	
			/* Extract data from packet [socket] buffer */
			struct ieee80211_hdr* pkt_header = (struct ieee80211_hdr*)(packet->data);		
			/* Extract DA address */
			U8* da = pkt_header->addr1;		
			/* If an ATIM for this DA has not been added to the pending list,
			 * the packet needs to be created. 
			 */
			if (!ieee80211_psm_atim_list_contains_DA(ar,da)) {
				/* Handle control to the IEEE80211 module, that will
				 * create and push the ATIM to the ATIM queue
				 */
				ieee80211_create_atim_pkt(ar, da, NULL);			
				
			} else {
				/* No need to create an ATIM, because such an ATIM 
				 * is already there. This is the case when multiple
				 * packets for the same DA arrive in the same beacon
				 * interval.
				 */
				#if IBSS_MH_PSM_DEBUG_DEEP
				printf("ATIM for this DA Contained.\n");
				#endif
			}
		}	
		
	} else {
		/* Signal an error, as the function is supposed to generate ATIMS for a 
		 * Station that is not in PS mode.
		 */
		printf("WARNING: Device is not in PS mode. Why do we need to create ATIMS?\n");
		return;
	}	
}	


bool ieee80211_psm_is_remote_da_awake(struct ar9170* ar, U8* da ) 
{
	#if IBSS_PSM_DEBUG_DEEP
	printf("DEBUG: PSM; DA: %02x:%02x:%02x:%02x:%02x:%02x_\n", da[0], da[1], da[2], da[3], da[4], da[5]);
	#endif
	
	/* If we are already in pre-TBTT, do not proceed */
	if (ar9170_get_device()->ps_mgr.psm_state == AR9170_PRE_TBTT) {
		return false;
	
	}
	/* Protect from the TBTT interrupt that will erase the list. 
	 * If, however, this interrupt is handled right after we are
	 * exiting, we need to re-check for the ps state.
	 */
	irqflags_t _flags = cpu_irq_save();
		
	/* Walk through the vector of awake nodes. Return false, 
	 * if the corresponding DA is not found there.
	 */	
	int i;
	for (i=0; i< linked_list_get_len(ar->ps_mgr.wake_neighbors_list); i++) {
		/* Get the element at position -i- */
		U8* address_to_check = (U8*)linked_list_get_element_at(ar->ps_mgr.wake_neighbors_list, i);
		
		if (address_to_check != NULL) {
			#if IBSS_PSM_DEBUG_DEEP
			printf("DEBUG: PSM; DA: %02x:%02x:%02x:%02x:%02x:%02x.\n", address_to_check[0],
				address_to_check[1], address_to_check[2], address_to_check[3], address_to_check[4], address_to_check[5]);
			#endif
			if (ether_addr_equal(address_to_check, da)) {
				/* Found */
				cpu_irq_restore(_flags);
				return true;
			}
		} 		
	}
	cpu_irq_restore(_flags);
	return false;
}


bool ieee80211_psm_is_DA_awake(struct sk_buff* packet )
{
	#if IBSS_PSM_DEBUG_DEEP
	printf("DEBUG: PSM; ar9170_psm_is_DA_awake.\n");
	#endif
	
	/* Extract packet from the socket buffer. */
	struct sk_buff* skb = packet;
	
	if (skb == NULL) {
		printf("ERROR: Socket buffer is NULL.\n");
		return false;
	}	
	if (skb->data == NULL) {
		/* Signal a warning if the packet has NULL content. */
		printf("WARNING: No packet content or corrupted socket buffer.\n");
		return false;
	}
	
	/* Extract data from packet. */
	struct ieee80211_hdr* pkt_header = (struct ieee80211_hdr*)skb->data;
	
	/* Extract DA address from the header. */
	U8* da = pkt_header->addr1;
	
	/* Obtain a reference to the AR9170 device. */
	struct ar9170* ar = ar9170_get_device();
	
	/* Check whether the corresponding node is awake, by investigating 
	 * the node's list of awake neighbors.
	 */
	if (ieee80211_psm_is_remote_da_awake(ar,da)) {
		#if IBSS_PSM_DEBUG_DEEP
		printf("DEBUG: PSM; Receiver is awake.\n");
		#endif
		if (ar9170_get_device()->ps_mgr.psm_state == AR9170_PRE_TBTT) {
			return false; // prevet from sending when already in pre-tbtt.
		}
		return true;
		
	} else {
		/* The remote neighbor is not known to be awake. */
		return false;
	}
}


ar9170_tx_queue* ieee80211_psm_can_send_first_atim(struct ar9170* ar) {
	
	#if IBSS_PSM_DEBUG_DEEP
	printf("DEBUG: IBSS_PSM; Check and send first atim.\n");
	#endif
	
	int i,j;
	int found_atim_at_index = -1;
	/*
	 * Walk through the list of pending ATIM frames;
	 * Remove the pending ATIMS for STAs that are 
	 * already known to be awake for the following
	 * Data Transmission period. Stop and return
	 * the list when an ATIM transmission is valid
	 */
	
	struct sk_buff* atim_packet = NULL;
	/* Obtain a reference to the ATIM Queue of the considered AR9170 device. */
	ar9170_tx_queue* the_atim_queue = ar->tx_pending_atims;
	
	if (linked_list_is_empty(the_atim_queue)) {
		/* There are no pending ATIMs. */
		#if IBSS_PSM_DEBUG
		printf("WARNING: There are no pending ATIMs!\n");
		#endif
		return NULL;
		
	} else {
		/* Walk along the non-empty queue. */
		for (i=0; i<linked_list_get_len(the_atim_queue); i++) {
			
			atim_packet = linked_list_get_element_at(the_atim_queue, i);
			
			if (atim_packet == NULL) {
				
				printf("ERROR: The ATIM queue is empty before it finishes!\n");
				return NULL;
			
			} else {
					
				/* Check whether the receiver is awake */
				if (!ieee80211_psm_is_DA_awake(atim_packet)) {
					
					/* We found a valid ATIM frame */
					found_atim_at_index = i;
					break;
				}
			}	
		}		
		/* If valid ATIM packet is found */
		if (found_atim_at_index >= 0) {
			
			#if IBSS_MH_PSM_DEBUG_DEEP
			printf("DEBUG: PSM; ATIM at position %d must be sent.\n", found_atim_at_index);
			#endif
			/* Erase [remove] all ATIM packets in front of this position */
			for (j=0; j< found_atim_at_index; j++) {
				#if IBSS_PSM_DEBUG
				printf("DEBUG: PSM: Removing ATIM frame at position: %u.n", j);
				#endif
				struct sk_buff* an_atim = linked_list_get(the_atim_queue);
				if (an_atim->data != NULL) {
					sfree(an_atim->data);
					an_atim->data = NULL;
				}
				the_atim_queue = linked_list_remove_first(the_atim_queue);
			}
			return the_atim_queue;				
			
		} else {
			#if IBSS_PSM_DEBUG_DEEP
			printf("DEBUG: No ATIM needs to be sent. Shall erase the whole list.\n");
			#endif
		
			for (j=0; j<linked_list_get_len(the_atim_queue); j++) {
				
				struct sk_buff* an_atim = linked_list_get(the_atim_queue);
				
				if ((an_atim != NULL) && (an_atim->data != NULL)) {
					sfree(an_atim->data);
					an_atim->data = NULL;
				}				
			}
			linked_list_erase(the_atim_queue);
		
			return NULL;
		}		
	}		
}



ar9170_tx_queue* ieee80211_psm_can_send_first_pkt(struct ar9170* ar) 
{	
	/*
	 * Walk through the list of pending DATA packets.
	 * Check whether the corresponding DAs are awake.
	 * If yes, move the first eligible packet in the
	 * beginning of the queue and return true. 
	 */
	int found_packet_at_index = -1;
	int i;
	
	struct sk_buff* packet = NULL;
	/* Obtain a reference to the pending packet queue. */
	ar9170_tx_queue* the_tx_queue = ar->tx_pending_pkts;
	
	/* Return immediately if the list is empty. */
	if (linked_list_is_empty(the_tx_queue)) {
		printf("WARNING: There are no pending packets!\n");
		return NULL;	
		
	} else {
		/* The list contains packets. Walk along the queue */
		for (i=0; i<linked_list_get_len(the_tx_queue); i++) {
			
			packet = linked_list_get_element_at(the_tx_queue, i);
			
			if (packet == NULL) {
				/* This should not happen, i.e. the queue should not 
				 * contain packets with NULL content, so we signal an
				 * error, and return NULL.
				 */				
				printf("ERROR: The TX queue is empty before it finishes!\n");
				return NULL;
				
			} else {
				
				/* Check whether the intended receiver of the packet is 
				 * known to be awake in the current beacon interval.
				 */
				if (ieee80211_psm_is_DA_awake(packet)) {
					
					/* We found the packet to send first! Store the index. */
					found_packet_at_index = i;
					break;
				}				
			}
		}
	}		
	/* If a packet candidate is found we move it at the beginning of the queue. */
	if (found_packet_at_index >= 0) {
		
		#if IBSS_PSM_DEBUG_DEEP
		printf("DATA at position %u can be sent. Total length: %u.\n", found_packet_at_index,
				linked_list_get_len(the_tx_queue));
		#endif
		/* Swap packet position if required. */
		if (found_packet_at_index == 0) {
			return the_tx_queue;

		} else {
			
			return linked_list_move_to_front(the_tx_queue, found_packet_at_index);
		}
		
	
	} else {
		/* No candidate packet was found to be eligible for sending. This is, 
		 * probably because none of the intended receivers are known to be 
		 * awake. 
		 */
		#if IBSS_PSM_DEBUG_DEEP
		printf("DEBUG: No packet can be sent. Receivers not in the AWAKE list.\n");
		#endif
		return NULL;
	}		
}