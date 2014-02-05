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
#include "ar9170_psm.h"
#include "platform-conf.h"
#include <stdio.h>
#include "ieee80211_ibss.h"
#include "ar9170.h"
#include <stdint-gcc.h>
#include "string.h"
#include "ar9170_debug.h"
#include "hw.h"
#include "etherdevice.h"
#include "dsc.h"
#include "ieee80211_tx.h"
#include "ieee80211_psm.h"
#include "ar9170_mac.h"
#include "usb_lock.h"
#include "cc.h"
#include "rtimer.h"
#include "linked_list.h"
#include "contiki-main.h"
#include "compiler.h"
#include "wire_digital.h"

#define RTIMER_MILLISECOND	RTIMER_SECOND / 1000
/* Real time timer to schedule real time events */
struct rtimer* real_time_timer;


/* This is a call-back function fire when the Soft Beacon transmission time expires. */
static void ar9170_psm_soft_beacon_tx(struct rtimer* timer, void* ptr) {
	
	rtimer_clock_t current_time = RTIMER_NOW();
	UNUSED(timer);
	UNUSED(ptr);
	
	#if AR9170_PSM_DEBUG_DEEP
	printf("[%llu] Soft TBTT.\n",current_time);
	#endif
	
	struct ar9170* ar = ar9170_get_device(); 
	
	if (not_expected(ar == NULL)) {
		printf("ERROR: Reference to AR9170 device is NULL. Disconnected?\n");
		return;
	}
	if (ar->ps_mgr.psm_state == AR9170_SOFT_BCN_WINDOW) {
		
		/* Enable the flag that tells the scheduler to transmit a soft beacon. */
		ar->ps_mgr.send_soft_bcn_flag = true;
		
		/* Schedule next soft beacon transmission time interrupt. */
		if (rtimer_set(real_time_timer, current_time +
			RTIMER_MILLISECOND * (unique_vif->bss_conf.soft_beacon_int),
			0,
			ar9170_psm_soft_beacon_tx, NULL)) {
			
			printf("ERROR: PSM; Could not set timer for Soft beaconing.\n");
		}
		
	} else {
		
		printf("WARNING: The device woke up for soft BCN outside the respective window.\n");
	}	
}

/* This function is called inside interrupt context. */
void ar9170_psm_start_soft_beaconing( struct ar9170* ar, rtimer_clock_t current_time ) 
{
	#if AR9170_PSM_DEBUG_DEEP
	printf("DEBUG: PSM; Soft beaconing.\n");
	#endif
	
	/* Change device PSM state to Soft Beaconing. This 
	 * will prevent the node from transmitting data 
	 * packets.
	 */
	ar->ps_mgr.psm_state = AR9170_SOFT_BCN_WINDOW;
	
	
	/* Schedule next soft-beacon transmission. */
	if (rtimer_set(real_time_timer, current_time +
		RTIMER_MILLISECOND * (unique_vif->bss_conf.soft_beacon_int),
		0,
		ar9170_psm_soft_beacon_tx, NULL)) {
			
			printf("ERROR: PSM; Could not set timer for Soft beaconing.\n");
	}
	
	/* FIXME - schedule immediate power-saving. */
	//ar9170_psm_schedule_powersave(ar, true);
}


/* This is a call-back function fired when the ATIM Window expires. */
static void ar9170_psm_atim_window_end(struct rtimer* timer, void* ptr) {
	
	rtimer_clock_t current_time = RTIMER_NOW();	
	UNUSED(timer);
	UNUSED(ptr);
	
	#if AR9170_PSM_DEBUG_DEEP
	printf("[%llu] AE\n",current_time);
	#endif	
		
	// TODO - move ar9170 pointer to the arguments' list.
	struct ar9170* ar = ar9170_get_device(); 
	
	if (not_expected(ar == NULL)) {
		printf("ERROR: Reference to the AR9170 device is NULL. Disconnected?\n");
		return;
	}	
#ifdef WITH_SOFT_BEACON_GENERATION	
	/* The device has Soft beaconing mode enabled, which affects the 
	 * power-saving operation, so we check here whether we need to 
	 * enable soft beaconing in this beacon interval.
	 */
	if (ar->ps.off_override == PS_OFF_BCN) {
		/* We would need to stay awake only because of the beacon 
		 * transmission, so we are allowed to transit to the soft
		 * beaconing state [semi-doze].
		 */
		ar9170_psm_start_soft_beaconing(ar, current_time);
		return;
	} 
	
#endif

	/* Set device PSM state to "Data Transmission Period. 
	 * Note that this is done, REGARDLESS of whether the 
	 * node will transit to the doze state or not.
	 */
	if (ar->ps_mgr.psm_state != AR9170_ATIM_WINDOW) {
		printf("WARNING: AR9170 device should have been in ATIM Window [%u].\n",ar->ps_mgr.psm_state);
	}
	ar->ps_mgr.psm_state = AR9170_TX_WINDOW;	

	/* Schedule power-save transition. Notice that this MIGHT NOT be 
	 * executed due to beacon transmission in this beacon interval, 
	 * or because ATIM frames have been sent / received and properly 
	 * acknowledged.
	 */
	ar9170_ps_update(ar);
}

/* Interrupt handler, fired when the ATIM Window starts, after the pre-
 * TBTT period. Note that this transition may have already occurred, in
 * response to a "BCN sent" response from the AR9170 device, or a BCN,
 * already received in this TBTT period.
 */
static void ar9170_psm_atim_window_start(struct rtimer* timer, void* ptr)
{
	UNUSED(ptr);
	/* Store the current time. */
	rtimer_clock_t current_time = RTIMER_NOW();
	
	#if AR9170_PSM_DEBUG_DEEP	
	printf("[%llu] ATIMW Start.\n",current_time);
	#endif
	
	struct ar9170* ar = ar9170_get_device(); 
	
	if (not_expected(ar == NULL)) {
		/* Probably the device had just been disconnected. (?)*/
		printf("ERROR: Reference to AR9170 device is NULL. Disconnected?\n");
		return;
	}
	/* Set device PSM state to "ATIM Window Period, if not already there. */
	if (ar->ps_mgr.psm_state == AR9170_PRE_TBTT) {
		
		/* Device PSM state transits to the ATIM Window. */
		ar->ps_mgr.psm_state = AR9170_ATIM_WINDOW;	
	
	} else if (ar->ps_mgr.psm_state == AR9170_ATIM_WINDOW) {
		
		/* Device has been already moved to the ATIM Window due to a beacon send notification. */
	
	} else {
	
		printf("WARNING: AR9170 device should have only been in either pre-TBTT or ATIM Window!\n");
	}
	
	/* Set rtimer to the due time of the ATIM Window End. */
	if(rtimer_set(real_time_timer, current_time + 
			RTIMER_MILLISECOND * (unique_vif->bss_conf.atim_window-AR9170_ATIM_WINDOW_OFFSET_KUS),
			0, 
			ar9170_psm_atim_window_end, NULL)) {
				
		printf("ERROR: PSM; Could not set timer for ATIM Window End.\n");
	}
	
	/* Update the override flag, if there is some data to transmit in the current beacon interval. */
	if (!linked_list_is_empty(ar->tx_pending_pkts)) {
		
		ar->ps.off_override |= PS_OFF_DATA;
		#ifdef WITH_LED_DEBUGGING
		/* Fire-up traffic LED indicator. */
		digital_write(TX_ACTIVE_PIN, HIGH);
		#endif
	}	
	
	/* Cancel beacon transmission in this frame. Do we really need to do this? */
	//ar9170_schedule_bcn_cancel(ar);
}

/*
static void ar9170_bcn_control(struct rtimer* timer, void* ptr)
{
	UNUSED(timer);
	UNUSED(ptr);
	rtimer_clock_t current_time;
	#if AR9170_PSM_DEBUG
	current_time = RTIMER_NOW();
	printf("[%llu] BCN CTRL.\n",current_time);
	#endif
	ar9170_update_beacon(ar, true);
}
*/


/* Schedule an interrupt to mark the ATIM Window End. 
 * The function is called from interrupt context, so 
 * it needs to be executed fast.
 */
void ar9170_psm_schedule_atim_window_start_interrupt(rtimer_clock_t pre_tbtt_time)
{	
	#if AR9170_PSM_DEBUG_DEEP
	printf("DEBUG: PSM; Scheduling ATIM start interrupt...\n");
	#endif
	
	/* Set rtimer to the due time of the ATIM Window Start. Note that this 
	 * may well be canceled by an earlier "BCN Sent" response or a Beacon 
	 * reception from a neighbor.
	 */ 
	int err = rtimer_set(real_time_timer, pre_tbtt_time + (RTIMER_MILLISECOND  *
								(AR9170_PRETBTT_KUS+AR9170_ATIM_WINDOW_OFFSET_KUS)), 
								0, 
								ar9170_psm_atim_window_start, NULL); 
	if (err) {
		printf("ERROR: PSM; Could not set timer for ATIM Window End.\n");
	}	
}


void ar9170_psm_init_rtimer() {
	real_time_timer = smalloc(sizeof(struct rtimer));
	memset(real_time_timer,0,sizeof(struct rtimer));
}

/* This function is called within interrupt context. */
void ar9170_psm_schedule_powersave(struct ar9170* ar, bool new_state) {
	
	/* If the new state is different from the current one, 
	 * schedule a power-save transition at the earliest 
	 * occasion.
	 */
	if (new_state != ar->ps.state) {
		
		/* PS state needs to transit. */
		#if AR9170_PSM_DEBUG_DEEP
		printf("DEBUG: PS transition to {--> %d} will be scheduled.\n",new_state);
		#endif
		
		ar->ps.update_mask = AR9170_PS_UPDATE_TRANSITION_FLAG;
		if (new_state == true) {
			/* Schedule transition to power-save */
			ar->ps.update_mask |= AR9170_PS_UPDATE_ACTION_SLEEP;
		} else {
			/* Schedule transition to non power-save */
			ar->ps.update_mask |= AR9170_PS_UPDATE_ACTION_WAKE;			
		}
		#if AR9170_PSM_DEBUG_DEEP
		printf("DEBUG: Mask: %02x.\n", ar->ps.update_mask);
		#endif
	
	} else {
		/* No change required */
		#if AR9170_PSM_DEBUG
		printf("DEBUG: NO PS transition.\n");
		#endif
	}
}


void ar9170_psm_async_tx_data( struct ar9170* ar )
{		
	/*
	 * We need to check whether the station is currently
	 * in the ATIM Window, or in the Data Transmission
	 * Period. Execution proceeds based on the state.
	 */
	if (ar->ps_mgr.psm_state == AR9170_ATIM_WINDOW) {
		
		/*
		 * STA lies in the ATIM Window, so we can not
		 * handle transmission of data packets yet.
		 */
		#if AR9170_PSM_DEBUG
		/* This is possible since a TBTT interrupt could occur right here */
		printf("DEBUG: Transmission was called while in ATIM Window.\n");
		#endif
		
	} else if (ar->ps_mgr.psm_state == AR9170_TX_WINDOW) {		
		
		/* STA lies in the transmission period, so we 
		 * check first whether we are done with the 
		 * previous transmission.
		 */
		if ((ar->tx_data_wait == true) || (ar->tx_atim_wait == true)) {
			/* Still waiting for an ACK / NACK. */
			return;			
		}
		/* Otherwise we can safely begin. We do this, 
		 * because overloading the AR9170 with multiple
		 * packets may end up with us having a burst of
		 * command status responses which we can not 
		 * handle with our small CPU.
		 */
		
		/*
		 * STA lies in the transmission period, so we
		 * need to check whether we can find a packet
		 * whose intended receiver is awake. We move
		 * this packet in the beginning of the list.
		 */	
		ar9170_tx_queue* queue = ieee80211_psm_can_send_first_pkt(ar);
		
		if ( queue != NULL) {
			
			__start(&(ar->tx_data_wait));		
			
			/* Assign the list reference to the pending packets queue. */
			if (queue != ar->tx_pending_pkts) {
				printf("NE\n");
			} 
			ar->tx_pending_pkts = queue;
			/*
			 * The intended receiver is AWAKE, so we 
			 * can proceed with the packet transmission
			 * We always send the first in line, so it
			 * is the responsibility of the previous
			 * method to re-arrange the packets' order. 
			 */
			ar->tx_pending_pkts = ar9170_async_tx(ar, ar->tx_pending_pkts);	
			
		} else {			
			/* Can not send packets, as receivers are not AWAKE */
		}
	}
}


void ar9170_psm_async_tx_mgmt( struct ar9170* ar )
{
	/* 
	 * Pick the first ATIM frame from the buffer. 
	 * Extract the DA of the packet and check if
	 * the intended received is already awake, so
	 * the packet transmission is redundant.
	 */
	ar9170_tx_queue* queue = ieee80211_psm_can_send_first_atim(ar);
	
	if (queue != NULL) {
		/* Assign the returned reference to the ATIM queue of the device. */
		ar->tx_pending_atims = queue;
		/*
		 * The returned queue is not null, 
		 * so we should try to send the 
		 * first ATIM packet in the queue.
		 * Before that we must set the ATIM
		 * waiting flag. TODO - release the
		 * flag in the status response of the
		 * ATIM *OR* at the expiration of the
		 * ATIM Window, since it is possible,
		 * that the window expires before status
		 * response is received [_unlikely_] 
		 */
		if ((ar->tx_data_wait == true) || (ar->tx_atim_wait == true)) {
			
			/* Device still busy with previous ATIM / DATA transmission, 
			 * so we need to try again later. 
			 */
			#if AR9170_PSM_DEBUG
			printf("DEBUG: PSM; Still busy with previous ATIM.\n");
			#endif
			return;
		}
		/* Set the waiting flag for ATIM response. */
		__start(&(ar->tx_atim_wait));
		
		/* Save the current DA under ATIM transmission. It
		 * will be used for updating the list of neighbors,
		 * that are AWAKE, after ACK reception.
		 */
		struct sk_buff* atim_packet = (struct sk_buff*)linked_list_get(queue);
		struct ieee80211_hdr_3addr* atim_header = (struct ieee80211_hdr_3addr*)(atim_packet->data);
		memcpy(&ar->ps_mgr.last_ATIM_DA[0], atim_header->addr1, ETH_ALEN); 
		
		if (ibss_info->ps_mode == IBSS_MH_PSM) {
			
			/* Save the current A3 under ATIM transmission. It
			 * will be used for updating the list of neighbors,
			 * that are AWAKE, after ACK reception. FIXME - the
			 * driver should not know the PS mode of the IBSS!
			 */
			#if AR9170_PSM_DEBUG_DEEP
			printf("DEBUG: PSM; Add also the pending A3 field.\n");
			#endif
			memcpy(&ar->ps_mgr.last_ATIM_A3[0], atim_header->addr3, ETH_ALEN); 
		}		
		
		/* Send packet and return the updated ATIM queue */
		ar->tx_pending_atims = ar9170_async_tx(ar, queue);	
		
		/* Update the ps_off_override flags, so after ATIM 
		 * expiration the STA remains in the AWAKE state. 
		 * XXX Check with the standard, whether we indeed
		 * need to stay awake, even without the response 
		 * from the intended receiver. 
		 */
		ar->ps.off_override |= PS_OFF_ATIM;
	
	} else {		
		/* The ATIM queue is null, nothing to be done. */
	}	
}
