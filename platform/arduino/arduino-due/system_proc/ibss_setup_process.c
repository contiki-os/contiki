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
#include "ibss_setup_process.h"
#include "ieee80211_ibss.h"
#include "ar9170_led.h"
#include "ar9170.h"
#include "platform-conf.h"
#include "ieee80211_mh_psm.h"
#include "ieee80211_iface_setup_process.h"

#define DEBUG_PROC	1
#include "contiki-main.h"

PROCESS(ibss_setup_process, "80211 IBSS Setup Process");


/* Declare the etimer. */
struct etimer et_ibss_setup_proc;
	
/* Flag indicating whether the IBSS setup [join/create] has been completed. */
static volatile bool ibss_setup_process_completed_flag;

/*---------------------------------------------------------------------------*/
bool is_ibss_setup_completed() {
	
	return ibss_setup_process_completed_flag;
}

/*---------------------------------------------------------------------------*/
static void ibss_setup_process_poll_handler()
{
	if (ar9170_is_wlan_device_added()) {
		
		if(ieee80211_is_ibss_joined()) {
			PRINTF("ERROR: IBSS joined flag is already set before network setup.\n");
			/* Signal an error to the process for premature exiting. */
			process_post(&ibss_setup_process, PROCESS_EVENT_EXIT, NULL);
		
		} else {
			#ifdef WITH_STATISTICS_COLLECTION
			/* Signal an event to the statistics collector to start displaying statistics. */
			process_post(&stats_display_process, PROCESS_EVENT_CONTINUE, NULL);
			#endif
			/* Network setup is completed successfully. Time to schedule the 
			 * IBSS creation, unless join comes first. 
			 */
			process_post(&ibss_setup_process, PROCESS_EVENT_CONTINUE, NULL);
		} 
		/* Do not poll any more. */
		return; 
	}
	
	/* Poll processes so it runs continuously, until the network setup completes. */
	process_poll(&ibss_setup_process);
}


/*---------------------------------------------------------------------------*/
static void ibss_setup_process_exit_handler(void)
{
	PRINTF("IBSS_SETUP_PROCESS terminates.\n");
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ibss_setup_process, ev, data)
{	
	/* Declare the poll handler for the IBSS setup process. */
	PROCESS_POLLHANDLER(ibss_setup_process_poll_handler());
	
	/* Declare the exit handler for the IBSS setup process. */
	PROCESS_EXITHANDLER(ibss_setup_process_exit_handler());
	
	/* Start process */
	PROCESS_BEGIN();
	
	PRINTF("IBSS_SETUP_PROCESS\n");
	
	/* Initialize indicator flag that IBSS can start. */
	ibss_setup_process_completed_flag = false;
	
	/* Poll the process for the first time. */
	process_poll(&ibss_setup_process);
	
	/* Wait until the process needs to continue, or to terminate. */
	PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_EXIT)||(ev == PROCESS_EVENT_CONTINUE));
	
	if (ev == PROCESS_EVENT_CONTINUE) {
		// Do nothing	
	} else if (ev == PROCESS_EVENT_EXIT) {
	
		/* Exit process. Nothing to be done. */
		PRINTF("IBSS_SETUP_PROCESS: ERROR; Premature exiting.\n");
		goto err_exit;
	}
	
	/* Wait some time until we can start scanning for the IBSS. */
	etimer_set(&et_ibss_setup_proc, IBSS_PROC_INIT_DELAY);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_ibss_setup_proc));
	/* Enable scanning. */
	ieee80211_enable_scanning();
		
	PRINTF("INFO: IBSS; Start scanning for default IBSS.\n");
	/* Passive scanning for the default network. Implemented as follows:
	 * The device is waiting for parsing beacons from the default IBSS;
	 * If no beacons arrive, the node proceeds with creating the default
	 * IBSS. Otherwise, the device PASSIVELY joins the default IBSS.
	*/
	etimer_set(&et_ibss_setup_proc, IBSS_PROC_SCAN_TIMEOUT);
	
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_ibss_setup_proc)|| (ev == PROCESS_EVENT_CONTINUE));
	
	/* Timer expired or default IBSS joined [posts a "CONTINUE" event]. */
	if (ieee80211_is_ibss_joined()) {
	
		PRINTF("IBSS_SETUP_PROCESS: The node has joined the default IBSS.\n");
		
	} else {			
		/* Proceed with creating an IBSS network. */
		PRINTF("IBSS_SETUP_PROCESS: Need to create the default IBSS.\n");
				
		ieee80211_sta_create_ibss();
	}
	
	/* Wait until the network is joined / created. 
	 * Normally, the driver should signal network 
	 * setup completion to the net scheduler, and
	 * the net scheduler should post us an event.
	 */
	
	while(!ieee80211_is_ibss_joined()) {
	
	}
	
	/* If in advanced power-save mode, initialized the respecting MAC resolve mapping list. */
	#ifdef WITH_MULTI_HOP_PSM
	ieee80211_mh_psm_init_address_map();
	#endif
				
	/* Set ORANGE led signaling network joining. */
	ar9170_led_set_state(ar9170_get_device(), AR9170_LED_ORANGE | AR9170_LED_GREEN);
err_exit:		
	PRINTF("IBSS_SETUP_PROCESS END\n");
	PROCESS_END();
}	
/*---------------------------------------------------------------------------*/
