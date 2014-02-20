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

#include "contiki.h"
#include "usb_lock.h"
#include "ar9170.h"
#include "ieee80211_ibss.h"
#include "ibss_main.h"
#include "ibss_setup_process.h"
#include "compiler.h"
#include "tcpip.h"
#include "resolv.h"
#include "platform-conf.h"
#include "ieee80211_iface_setup_process.h"
#include "uart1.h"

#define DEBUG_PROC	1
#include "contiki-main.h"

/*---------------------------------------------------------------------------*/
PROCESS(net_scheduler_process, "Network Scheduler Process");



/*---------------------------------------------------------------------------*/
static void net_scheduler_process_poll_handler(void)
{	
	/* Check first if the device has been plugged. */
	if (ar9170_is_wlan_device_plugged()) {
		
		/* AR9170 driver scheduler */
		ar9170_op_scheduler(ar9170_get_device());
		
	} else {
		/* Maybe the AR9170 has just been disconnected. We can 
		 * check this by examining whether the device is still
		 * allocated. In such a case we signal an event to all 
		 * relevant processes and free the resources, if it is
		 * not done already.
		 */
		if (ar9170_is_wlan_device_allocated()) {
			
			PRINTF("NET_SCHEDULER_PROCESS: INFO; AR9170 UNPLUGGED!\n");
			if (ar9170_get_device() != NULL) {
				sfree(ar9170_get_device());
				*ar9170_get_device_pt() = NULL;
			}
		
			#ifdef WITH_STATISTICS_COLLECTION
			/* Signal an exit of statistics display, as well. FIXME - move 
			 * this to the scheduler, so the USB host does not need to care
			 * about statistics display.
			 */
			process_post(&stats_display_process, PROCESS_EVENT_EXIT, NULL); 
			#endif
			process_post(&net_scheduler_process, PROCESS_EVENT_EXIT, NULL);
			
			/* Set the device status to non-allocated. */
			ar9170_set_wlan_device_non_allocated();
			
			/* If the device was added, it must now be removed. */
			if (ar9170_is_wlan_device_added()) {
				
				PRINTF("NET_SCHEDULER_PROCESS: Removing AR9170 interface.\n");
				ar9170_set_wlan_device_not_added();
				
				/* Inform the IBSS scheduler that the network is now down. */
				if (ieee80211_is_ibss_joined()) {
					/* Signal IBSS disconnect event. */
					ieee80211_set_ibss_join_state(false);
				}
			}		
		
		} else {
			printf("Device unplugged but not allocated.\n");
		}
	}
	
	
	/* Check scheduler if the default IBSS has been joined */
	if (ieee80211_is_ibss_joined()) {
		
		/* Post an event to the IBSS_SETUP_PROCESS, that the 
		 * initialization phase is now completed.
		 */		
		if (not_expected(ieee80211_has_ibss_just_started_beaconing())) {
							
			//PRINTF("NET_SCHEDULER_PROCESS: Signal Traffic Generation start.\n");
			PRINTF("NET_SCHEDULER_PROCESS: Signal Upper-Layer Networking start.\n");
			/* Signal an event to the traffic generator to start generating packets,
			 * [if configured as such]. Notice that we always need a traffic generator
			 * regardless of whether we generate packets, as the relaying is handled by
			 * the same Contiki process.
			 */
			#ifndef WITH_SLIP
			#if WITH_UIP6			
			process_start(&tcpip_process, NULL);
			process_start(&resolv_process, NULL);
			#endif
			#else /* WITH_SLIP */
			/* For SLIP-radio, we now enable the interrupting on the 
			 * UART receiving line; we have not done this before, as
			 * the link-layer was still in the initialization phase.
			 */
			PRINTF("For SLIP: Enabling RX UART interrupts.\n");
			uart1_enable_rx_interrupt();
			#endif /* WITH_SLIP */			
		}
		
		/* IEEE80211 net scheduler */
		ieee80211_op_scheduler(ar9170_get_device());
	
	} else {
		/* TODO - Operations performed upon IBSS disconnection. 
		 * Contiki is informed through the IBSS connection flag
		 *  but we need to de-allocate all resources related to
		 * the IBSS network.
		 */
		
	} 
	
	/* Poll process to run continuously, unless there are reasons not to do it. */
	if (ar9170_is_wlan_device_added() || ieee80211_is_ibss_joined()) {
	
		process_poll(&net_scheduler_process);	
	
	} else {
		/* Before this poll-handler de-registers, it re-starts the 
		 * IEEE80211_IFACE_SETUP process, so the AR9170 is able to
		 * restart again. */
		process_start(&ieee80211_iface_setup_process, NULL);
		
		PRINTF("NET_SCHEDULER_PROCESS: Poll-handler de-registers.\n");
	}	
}


/*---------------------------------------------------------------------------*/
static void net_scheduler_process_exit_handler(void)
{
	PRINTF("NET_SCHEDULER_PROCESS terminates.\n");
}



/*---------------------------------------------------------------------------*/
PROCESS_THREAD(net_scheduler_process, ev, data)
{	
	/* Declare the poll handler for the network scheduler process. */
	PROCESS_POLLHANDLER(net_scheduler_process_poll_handler());
	
	/* Declare process exit handler */
	PROCESS_EXITHANDLER(net_scheduler_process_exit_handler());
	
	/* Start process */
	PROCESS_BEGIN();
	
	PRINTF("NET_SCHEDULER_PROCESS\n");
	
	process_poll(&net_scheduler_process);
	
	/* Wait until the process needs to terminate (triggered externally). */
	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT);
	
	PRINTF("NET_SCHEDULER_PROCESS ended.\n");
	
	PROCESS_END();	
}


	
	
