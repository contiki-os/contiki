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
#include "compiler.h"
#include "regd.h"
#include "ath.h"
#include "ar9170.h"
#include "mac80211.h"
#include "ar9170_main.h"
#include "ieee80211_ibss.h"
#include "usb_cmd_wrapper.h"
#include "usb_fw_wrapper.h"
#include "clock.h"
#include "ieee80211_iface_setup_process.h"
#include <sys\errno.h>
#include "platform-conf.h"
#include "ar9170_led.h"
#include "smalloc.h"
#include "ibss_setup_process.h"
#include "net_scheduler_process.h"
#include "netstack.h"

#define DEBUG_PROC	1
#include "contiki-main.h"


PROCESS(ieee80211_iface_setup_process, "Network Setup Process");

int add_network_interface(struct ar9170* ar) {
	
	U32 bss_info_changed_flag = 0;
		
	PRINTF("IEEE80211_IFACE_SETUP_PROCESS: Enable 802.11 wireless networking.\n");
	
	/* Parse the given device firmware */
	if (ar9170_parse_firmware(ar)) {
		printf("ERROR: Parsing device firmware returned errors.\n");
		return -EINVAL;
	}
	
	/* Initialize AR9170 device */
	if (!ar9170_init_device(ar)) {
		printf("ERROR: Device could not be initialized.\n");
		return -ENXIO;
	}
	
	/* Register device */
	if (!ar9170_register_device(ar)) {
		printf("ERROR: Device could not be registered.\n");
		return -ENXIO;
	}

	// EXTRA FIXME - put it somewhere better than just here
	ar->common.regulatory.country_code = CTRY_SWITZERLAND;
	ar->common.regulatory.regpair = smalloc(sizeof(struct reg_dmn_pair_mapping ));
	ar->common.regulatory.regpair->reg_2ghz_ctl = 48; // XXX XXX This is an ugly hack and we should fix it better
	
	/* Initially the PS state is OFF, as well as the OFF override flag */
	ar->ps.state = 0;
	ar->ps.off_override = 0;
		
	/* Start device */
	if (!ar9170_op_start(ar)) {
		printf("ERROR: Device could not start!\n");
		return -ENXIO;
	}
		
	/* Add interface */
	if (ar9170_op_add_interface(hw, unique_vif)) {
		printf("ERROR: Interface could not be added!\n");
		return -1;
	}
	
	// Call bss_info_changed for modifying slot time
	bss_info_changed_flag |= BSS_CHANGED_ERP_SLOT;
	ar9170_op_bss_info_changed(hw, unique_vif, 
						&unique_vif->bss_conf, 
						bss_info_changed_flag);
						
	/* Inform the uIP that the MAC address is now registered. 
	 * We do this by re-calling NETSTACK_INIT for the network
	 * module.
	 */
	NETSTACK_NETWORK.init();					
	/* All OK */					
	return 0; 
}
/*---------------------------------------------------------------------------*/

/* Configure device so network setup is possible. 
 * The procedure is as follows: the device will 
 * attempt to scan for the default IBSS. If it is 
 * not possible to join this IBSS, the device 
 * shall proceed with creating the default IBSS
 */
int start_network_setup_operation() 
{	
	U32 config_flag_changed = 0;
	int err;
	
	#if AR9170_MAIN_DEBUG
	printf("DEBUG: __start network operation.\n");
	#endif
	
	/* Update TX QoS */		
	struct ieee80211_tx_queue_params params[4];
	
	CARL9170_FILL_QUEUE(params[3], 7, 31, 1023,	0);
	CARL9170_FILL_QUEUE(params[2], 3, 31, 1023,	0);
	CARL9170_FILL_QUEUE(params[1], 2, 15, 31,	188);
	CARL9170_FILL_QUEUE(params[0], 2, 7,  15,	102);	
	
	/* Configure tx queues */
	 if (ar9170_op_conf_tx(hw, unique_vif, 0, &params[0]) ||
		 ar9170_op_conf_tx(hw, unique_vif, 1, &params[1]) ||
		 ar9170_op_conf_tx(hw, unique_vif, 2, &params[2]) ||
		 ar9170_op_conf_tx(hw, unique_vif, 3, &params[3])) {
		 printf("ERROR: Could not set TX parameters.\n");
		 return -1;
	 }
	 
	/* BSS Change notification: change device to idle. */
	config_flag_changed = BSS_CHANGED_IDLE;
	ar9170_op_bss_info_changed(hw,unique_vif, &unique_vif->bss_conf, config_flag_changed);
	
	/* Flush device if necessary. Drain beacon.*/
	ar9170_op_flush(hw, false);
	
	config_flag_changed = 0;
	config_flag_changed |= IEEE80211_CONF_CHANGE_CHANNEL |
	IEEE80211_CONF_CHANGE_POWER |
	IEEE80211_CONF_CHANGE_LISTEN_INTERVAL;// |	IEEE80211_CONF_CHANGE_PS;
	
	/* Configure network hardware parameters */ 
	err =  __ieee80211_hw_config(hw, config_flag_changed);
	
	if (err) {
		printf("ERROR: IEEE80211 HW configuration returned errors.\n");
		return -1;
	}		
	
	/* Everything is fine */  
	return 0;
}
/*---------------------------------------------------------------------------*/


static int allocate_mac_resources()
{
	struct ar9170* ar;
	/* Allocate resources for AR9170 */
	struct ar9170** ar_pt = ar9170_get_device_pt();
	
	if ((*ar_pt) != NULL) {
		PRINTF("ERROR: AR9170 device already initialized when allocation is called.\n");
		return -EINVAL;
		
	} else {
		ar = ar9170_alloc(ar_pt);
	}
	/* Check the result of memory allocation */
	if (ar == NULL) {
		PRINTF("ERROR: Could not allocate memory for the AR9170 struct.\n");
		return -ENOMEM;
	}
	/* Allocate resource for IEEE80211 HW structure */
	if(__ieee80211_sta_init_config()) {
		PRINTF("ERROR: Could not allocate memory for the IEEE80211 HW struct.\n");
		return -ENOMEM;
	}
	/* Copy the network SSID */
	memcpy(ar->common.curbssid, ibss_info->ibss_bssid, ETH_ALEN);
	
	/* Assign pointers: AR9170 has a pointer to the unique hw structure */
	ar->hw = hw;
	ar->channel = hw->conf.channel;
	if (!ar->hw->wiphy) {
		PRINTF("ERROR: Could not allocate memory for the WIPHY struct.\n");
		return -ENOMEM;
	}
	// AR9170 obtains a virtual interface of id 0.
	ar->vif_priv[0].id = 0;
	
	/* AR9170 struct gets a pointer to the virtual interface info */
	ar->beacon_iter = unique_cvif;
	
	if(ar->beacon_iter == NULL) {
		return -1;
	}
	/* Everything is fine */
	return 0;
}
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
static void ieee80211_iface_setup_process_poll_handler(void)
{
	if (ar9170_is_mass_storage_device_plugged()) {
		/* The MSC device has been plugged-in. Check whether the WLAN USB 
		 * device is also plugged-in and if yes, signal an error, otherwise
		 * continue with device EJECT procedure.
		 */
		if (!ar9170_is_wlan_device_plugged()) {
			
			/* The Mass Storage device is plugged while WLAN is unplugged. */
			PRINTF("IEEE80211_IFACE_SETUP_PROCESS: Mass Storage device has been plugged.\n");
						
			/* Send the EJECT command if not sent. The command will be sent only once. */
			if (ar9170_is_eject_command_sent()) {
				/* Do nothing. Needs to wait for MSC device eject. */
				
			} else {
				/* The EJECT command needs to be sent now. */
				if (ar9170_eject_mass_storage_device_cmd()) {
			
					/* Yield process. Process shall wait for the appearance of the WLAN
					 * device. 
					 */			
					PRINTF("IEEE80211_IFACE_SETUP_PROCESS: EJECT command was sent.\n");	
				
				} else {
					PRINTF("ERROR: IEEE80211_IFACE_SETUP_PROCESS; EJECT command could not be sent.\n");
					ar9170_set_eject_command_not_sent();
				}
			}
									
		} else {
			
			PRINTF("ERROR: IEEE80211_IFACE_SETUP_PROC; We have not sent an EJECT command, but WLAN device is connected!\n");
			
			/* Post an even for process termination */
			process_post(&ieee80211_iface_setup_process, PROCESS_EVENT_EXIT, NULL);
			
			/* Do not poll the process again. This is a serious error. */
			return;
		}	
	
	} else if (ar9170_is_wlan_device_plugged()) {
		
		/* The WLAN device has been plugged-in. */
		if (ar9170_is_mass_storage_device_plugged()) {
			
			PRINTF("ERROR: WLAN device is started, while Mass Storage Device is still plugged.\n");
			
			/* Post an even for process termination */
			process_post(&ieee80211_iface_setup_process, PROCESS_EVENT_EXIT, NULL);
			
			/* Do not poll the process again. */
			return;
		
		} else {
	
			/* MSC Device is not plugged. This is fine. */
			PRINTF("IEEE80211_IFACE_SETUP_PROCESS: AR9170 WLAN device is now plugged-in.\n");
			
			/* We do the same things, regardless of this being 
			 * a cold or warm reset. Perhaps we could just do a
			 * printout here.
			 */	
	
			/* We should now allocate the required resources and run the default 
			 * network configuration. 
			 */	
			if(allocate_mac_resources()) {
	
				PRINTF("ERROR: MAC resources allocation returned errors.\n");
				goto _err_out;		
			}
			/* Update the flag indicating that the device is allocated. */
			ar9170_set_wlan_device_allocated();
				
			/* We can now enable networking by booting the AR9170 device. */
	
			/* Obtain the reference to the AR9170 device */
			struct ar9170* ar = ar9170_get_device();
	
			if (!ar) {
				PRINTF("ERROR: Device not properly initialized. Network initialization failed.\n");
				goto _err_out;
			}

			/* Initialize and add the network interface. */
			if (add_network_interface(ar)) {
				printf("ERROR: AR9170 device could not be added!\n");
				goto _err_out;
			}
	
			/* Start network setup operation. */
			if (!start_network_setup_operation()) {
				
				/* Start the processes of IBSS initialization
				 * and networking schedulers. 
				 */
				process_start(&ibss_setup_process, NULL);
				process_start(&net_scheduler_process, NULL);
				
				/* If the network operation was successfully 
				 * started, we can update the AR9170 status
				 * to ADDED. 
				 */
				ar9170_set_wlan_device_added();
			}				
			/* Signal an event for the process to continue. */
			process_post(&ieee80211_iface_setup_process, PROCESS_EVENT_CONTINUE, NULL);
			
			/* Do not poll the process again. */
			return;
		_err_out:
			process_post(&ieee80211_iface_setup_process, PROCESS_EVENT_EXIT, NULL);
			/* Do not poll the process again. */
			return;	
		}		
	
	} else {
		
		/* Device has not be plugged yet. Nothing to be done yet. */		
	}				
			
	/* Poll the process again so it runs continuously. */
	process_poll(&ieee80211_iface_setup_process);
}
/*---------------------------------------------------------------------------*/


static void ieee80211_iface_setup_process_exit_handler(void)
{
	PRINTF("IEEE80211_IFACE_SETUP_PROCESS terminates.\n");
}
/*---------------------------------------------------------------------------*/



static void ieee80211_iface_setup_process_init() {
	
	/* Initialize flags while waiting for device connection. */
	ar9170_set_device_off();
	
	/* Release the CTRL endpoint lock flag. */
	ar9170_usb_ctrl_out_init_lock();
}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(ieee80211_iface_setup_process, ev, data)
{ 	
	/* Declare the poll handler for the network setup process. */	
	PROCESS_POLLHANDLER(ieee80211_iface_setup_process_poll_handler());
	
	/* Declare the exit handler for the network setup process. */
	PROCESS_EXITHANDLER(ieee80211_iface_setup_process_exit_handler());
	
	/* Start process */
	PROCESS_BEGIN();
	
	PRINTF("IEEE80211_IFACE_SETUP_PROCESS: Waiting for device connection...\n");
	
	ieee80211_iface_setup_process_init();
				
	/* Poll the process for the first time. */
	process_poll(&ieee80211_iface_setup_process);
	
	/* Wait until the process can either continue or terminate. */
	PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_EXIT)||(ev == PROCESS_EVENT_CONTINUE));

	if (ev == PROCESS_EVENT_CONTINUE) {	
		
		/* Exit normally. */
		PRINTF("IEEE80211_IFACE_SETUP_PROCESS: Network setup completed successfully!\n");
		/* Set GREEN led signaling network setup completion. */
		ar9170_led_set_state(ar9170_get_device(), AR9170_LED_GREEN);
				
		goto out;
			
	} else if (ev == PROCESS_EVENT_EXIT) {	
		/* Exit is here implying an error in the process. */
		PRINTF("IEEE80211_IFACE_SETUP_PROCESS needs to exit due to some erron while setting up networking.\n");
		goto err_out;
	}
	
err_out:	
	PRINTF("Network Initialization completed with errors.\n");
out:
	
	PRINTF("IEEE80211_IFACE_SETUP_PROCESS END.\n");
	PROCESS_END();	
}
/*---------------------------------------------------------------------------*/
				
				