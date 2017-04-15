/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file rndis_task.c *********************************************************
 *
 * \brief
 *      Manages the RNDIS Dataclass for the USB Device
 *
 * \addtogroup usbstick
 *
 * \author
 *        Colin O'Flynn <coflynn@newae.com>
 *
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
/**
 \addtogroup RNDIS
 @{
 */

//_____  I N C L U D E S ___________________________________________________


#include "contiki.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include "rndis/rndis_task.h"
#include "rndis/rndis_protocol.h"
#if RF230BB
#include "rf230bb.h"
#endif
#include "uip.h"
#include "sicslow_ethernet.h"
#include <stdio.h>

#include <avr/pgmspace.h>
#include <util/delay.h>
#include "watchdog.h"

#include "rndis/cdc_ecm.h"
#include "rndis/cdc_eem.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define PRINTF printf
#define PRINTF_P printf_P

//_____ M A C R O S ________________________________________________________





//_____ D E F I N I T I O N S ______________________________________________


#define IAD_TIMEOUT_DETACH 400
#define IAD_TIMEOUT_ATTACH 800

#define RNDIS_TIMEOUT_DETACH 900
#define RNDIS_TIMEOUT_ATTACH 1000

#define PBUF ((rndis_data_packet_t *) usb_eth_data_buffer)

//! Temp data buffer when adding RNDIS headers
uint8_t usb_eth_data_buffer[64];

uint64_t usb_ethernet_addr = 0x020000000002ULL;

//_____ D E C L A R A T I O N S ____________________________________________


//! Timers for LEDs
uint8_t led1_timer, led2_timer;

uint8_t usb_eth_is_active = 1;


uint8_t usb_eth_packet_is_available() {
	Usb_select_endpoint(RX_EP);
	return Is_usb_read_enabled();
}


uint8_t usb_eth_ready_for_next_packet() {
#ifdef USB_ETH_HOOK_IS_READY_FOR_INBOUND_PACKET
	return USB_ETH_HOOK_IS_READY_FOR_INBOUND_PACKET();
#else
	return 1;
#endif

	return 1;
}

#if USB_ETH_CONF_MASS_STORAGE_FALLBACK
static void
usb_eth_setup_timeout_fallback_check() {
	extern uint8_t fingerPresent;
		/* Device is Enumerated but RNDIS not loading. We might
		   have a system that does not support IAD (winXP). If so
		   count the timeout then switch to just network interface. */
	static uint16_t iad_fail_timeout, rndis_fail_timeout;	
	if (usb_mode == rndis_debug) {
		//If we have timed out, detach
		if (iad_fail_timeout == IAD_TIMEOUT_DETACH) {
		
			//Failed - BUT we are using "reverse logic", hence we force device
			//into this mode. This is used to allow Windows Vista have time to
			//install the drivers
			if (fingerPresent && (rndis_state != rndis_data_initialized) && Is_device_enumerated() ) {
				iad_fail_timeout = 0;
			} else {
					stdout = NULL;
					Usb_detach();
					doInit = 1; //Also mark system as needing intilizing
			}
			
		//Then wait a few before re-attaching
		} else if (iad_fail_timeout == IAD_TIMEOUT_ATTACH) {
		
			if (fingerPresent) {
				usb_mode = mass_storage;
			} else {
				usb_mode = rndis_only;
			}
			Usb_attach();
		}

		//Increment timeout when device is not initializing, OR we have already detached,
		//OR the user had their finger on the device, indicating a reverse of logic
		if ( ( (rndis_state != rndis_data_initialized) && Is_device_enumerated() ) ||
		  (iad_fail_timeout > IAD_TIMEOUT_DETACH) || 
		   (fingerPresent) ) {
			iad_fail_timeout++;
		} else {	
		iad_fail_timeout = 0;
		}
	} //usb_mode == rndis_debug


	 /* Device is Enumerated but RNDIS STIL not loading. We just
		have RNDIS interface, so obviously no drivers on target.
		Just go ahead and mount ourselves as mass storage... */
	if (usb_mode == rndis_only) {
		//If we have timed out, detach
		if (rndis_fail_timeout == RNDIS_TIMEOUT_DETACH) {
			Usb_detach();
		//Then wait a few before re-attaching
		} else if (rndis_fail_timeout == RNDIS_TIMEOUT_ATTACH) {
			usb_mode = mass_storage;
			Usb_attach();
		}

		//Increment timeout when device is not initializing, OR we are already
		//counting to detach
		if ( ( (rndis_state != rndis_data_initialized)) ||
		  (rndis_fail_timeout > RNDIS_TIMEOUT_DETACH) ) {
			rndis_fail_timeout++;
		} else {	
		rndis_fail_timeout = 0;
		}
	}//usb_mode == rnids_only
}
#endif

PROCESS(usb_eth_process, "USB Ethernet process");

/**
 * \brief RNDIS Process
 *
 *   This is the link between USB and the "good stuff". In this routine data
 *   is received and processed by RNDIS, CDC-ECM, or CDC-EEM
 */
PROCESS_THREAD(usb_eth_process, ev, data_proc)
{
	static struct etimer et;

	PROCESS_BEGIN();

	while(1) {
		if(usb_mode == mass_storage) {
			etimer_set(&et, CLOCK_SECOND);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			continue;
		}

#if USB_ETH_CONF_MASS_STORAGE_FALLBACK
		usb_eth_setup_timeout_fallback_check();
#endif
		
		if(!Is_device_enumerated()) {
			usb_eth_reset();
			while(!Is_device_enumerated()) {
				etimer_set(&et, CLOCK_SECOND);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et) || Is_device_enumerated());
			}
			USB_ETH_HOOK_INACTIVE();
		}

		switch(usb_configuration_nb) {
			case USB_CONFIG_RNDIS_DEBUG:
			case USB_CONFIG_RNDIS:
				if(rndis_process()) {
					USB_ETH_HOOK_READY();
					etimer_set(&et, CLOCK_SECOND/80);
				} else {
					USB_ETH_HOOK_INACTIVE();
					etimer_set(&et, CLOCK_SECOND/8);
				}
				break;
			case USB_CONFIG_EEM:
				cdc_eem_process();
				USB_ETH_HOOK_READY();
				etimer_set(&et, CLOCK_SECOND/80);
				break;
			case USB_CONFIG_ECM:
			case USB_CONFIG_ECM_DEBUG:
				if(cdc_ecm_process()) {
					USB_ETH_HOOK_READY();
					etimer_set(&et, CLOCK_SECOND/80);
				} else {
					USB_ETH_HOOK_INACTIVE();
					etimer_set(&et, CLOCK_SECOND/8);
				}
				break;
			default:
				etimer_set(&et, CLOCK_SECOND/4);
				break;
		}


		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)||(usb_eth_packet_is_available()&&usb_eth_ready_for_next_packet()));
	} // while(1)

	PROCESS_END();
}

/**
 \brief Sends a single ethernet frame over USB using appropriate low-level protocol (EEM or RNDIS)
 \param senddata Data to send
 \param sendlen Length of data to send
 \param led Should the LED be light up for this frame?
 */
uint8_t usb_eth_send(uint8_t * senddata, uint16_t sendlen, uint8_t led)
{
	uint8_t ret = 0;
	
	if(!usb_eth_is_active) {
		USB_ETH_HOOK_TX_ERROR("Inactive");
		goto bail;
	}

	//Check device is set up
	if (Is_device_enumerated() == 0) {
		USB_ETH_HOOK_TX_ERROR("Device not enumerated");
		goto bail;
	}

	switch(usb_configuration_nb) {
		case USB_CONFIG_RNDIS_DEBUG:
		case USB_CONFIG_RNDIS:
			ret = rndis_send(senddata, sendlen, led);
			break;
		case USB_CONFIG_EEM:
			ret = eem_send(senddata, sendlen, led);
			break;
		case USB_CONFIG_ECM:
		case USB_CONFIG_ECM_DEBUG:
			ret = ecm_send(senddata, sendlen, led);
			break;
	}

bail:

	if(!ret) // Hit the watchdog if we have a successful send.
		watchdog_periodic();

	return ret;
}

uint8_t
usb_eth_set_active(uint8_t active) {
	if(usb_eth_is_active!=active) {	
		switch(usb_configuration_nb) {
			case USB_CONFIG_RNDIS_DEBUG:
			case USB_CONFIG_RNDIS:
				usb_eth_is_active = active;
				rndis_send_interrupt();
				break;
			case USB_CONFIG_EEM:
				break;
			case USB_CONFIG_ECM:
			case USB_CONFIG_ECM_DEBUG:
				cdc_ecm_set_active(active);
				usb_eth_is_active = active;
				break;
		}
	}
	return 0;
}

void
usb_eth_get_mac_address(uint8_t dest[6]) {
	memcpy(dest,&usb_ethernet_addr,6);
}

void
usb_eth_set_mac_address(const uint8_t src[6]) {
	memcpy(&usb_ethernet_addr,src,6);
}

#include "watchdog.h"

void
usb_eth_reset() {
	rndis_state = rndis_uninitialized;
}

void
usb_eth_switch_to_windows_mode() {
	Usb_detach();

	usb_mode = rndis_debug;
	rndis_state = 	rndis_uninitialized;

	// Reset the USB configuration
	usb_user_endpoint_init(0);

	Leds_off();

	// Wait a few seconds, displaying
	// a pretty LED light pattern.
	int i;
	for(i = 0; i < 5; i++) {
		Led0_on();
		_delay_ms(100);
		watchdog_periodic();
		Led0_off();
		Led1_on();
		_delay_ms(100);
		watchdog_periodic();
		Led1_off();
		Led2_on();
		_delay_ms(100);
		watchdog_periodic();
		Led2_off();
		Led3_on();
		_delay_ms(100);
		watchdog_periodic();
		Led3_off();
	}

	Leds_off();

	//Attach USB
	Usb_attach();
}

/** @}  */
