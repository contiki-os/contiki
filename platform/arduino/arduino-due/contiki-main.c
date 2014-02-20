/**
 * \addtogroup arduino-due-platform
 *
 * @{
 */
/*
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*          Contiki main file for Arduino Due.
* \author
*          Ioannis Glaropoulos
*          
*/
/*---------------------------------------------------------------------------*/
#include "compiler.h"
#include "sleepmgr.h"
#include "arduino_due_x.h"
#include <stdint-gcc.h>
#include "uhc.h"
#include "ar9170.h"
#include "if_ether.h"
#include "mac80211.h"
#include "contiki.h"
#include <sys\errno.h>
#include <stddef.h>
#include <asf.h>
#include "watchdog.h"
#include "leds.h"
#include "sensors.h"
#include "uart1.h"
#include "serial-line.h"
/* Include network library */
#include "contiki-net.h"

#define DEBUG 1
#include "uip-debug.h"
#include "contiki-main.h"

#ifdef WITH_AR9170_WIFI_SUPPORT
#ifdef WITH_USB_SUPPORT
#include "ieee80211_iface_setup_process.h"
#include "ibss_setup_process.h"
#include "net_scheduler_process.h"
#endif
#endif

#include "pmc.h"
#include "pio.h"
#include "pio\pio_sam3x8e.h"
#include "pio_handler.h"

#include "conf_uart_serial.h"

#ifdef WITH_SLIP
#include "slip-radio.h"
#endif

#include "contiki-conf.h"
#include "conf_board.h"

/* Include sensors' header files. */
#ifdef WITH_TILT_SENSOR
#include "tilt-sensor.h"
#endif
#include "wire_digital.h"

/* Register the existing sensors. We should add our sensors here.
 * This isn't optimal; considers a standard order of definitions.
 */
#ifdef WITH_TILT_SENSOR
#ifdef WITH_G_SENSOR
#ifdef WITH_SOUND_SENSOR
SENSORS(&g_sensor, &sound_sensor, &tilt_sensor);
#else
SENSORS(&g_sensor, &tilt_sensor);
#endif
#else
SENSORS(&tilt_sensor);
#endif
#endif

/* 
 * Commented out the TCP IP process. We do
 * not want the process to start now since
 * we have not configured the link layer
 * interface yet. Instead, we start the UIP
 * after the MAC is configured.
 */
#if UIP_CONF_IPV6
PROCINIT(&sensors_process);
//PROCINIT(&tcpip_process, &sensors_process);
#else
PROCINIT(&sensors_process); 
#warning "No TCP/IP process!"
#endif

#ifdef WITH_AR9170_WIFI_SUPPORT
#ifndef WITH_USB_SUPPORT
#error WIFI is requested without enabling USB!
#endif
#endif 

#ifdef WITH_MULTI_HOP_PSM
#ifdef WITH_STANDARD_PSM
#error MULTI_HOP_PSM does not combine with standard PSM!
#endif
#else
#ifndef WITH_STANDARD_PSM
#warning Device runs in non-psm mode
#endif
#endif

#ifdef WITH_SOFT_BEACON_GENERATION
#ifndef WITH_ADVANCED_PSM
#ifndef WITH_STANDARD_PSM
#error SOFT generation without psm!
#endif
#endif
#endif

#ifdef WITH_LED_DEBUGGING
#ifdef WITH_AR9170_WIFI_SUPPORT
static void configure_ar9170_disconnect_pins() {
	
	/* Pins 16-18 will change state when the USB disconnects. */
	configure_output_pin(USB_DISCONNECT_ACTIVE_PIN, LOW, DISABLE, DISABLE);
	configure_output_pin(USB_DISCONNECT_PASSIVE_PIN, HIGH, DISABLE, DISABLE);
}
#endif

static void configure_led_debug_pins() {
	
	configure_output_pin(PRE_TBTT_ACTIVE_PIN, LOW, DISABLE, DISABLE);
	configure_output_pin(PRE_TBTT_PASSIVE_PIN, LOW, DISABLE, DISABLE);
	configure_output_pin(DOZE_ACTIVE_PIN, LOW, DISABLE, DISABLE);
	configure_output_pin(DOZE_PASSIVE_PIN, LOW, DISABLE, DISABLE);
	configure_output_pin(TX_ACTIVE_PIN, LOW, DISABLE, DISABLE);
	configure_output_pin(TX_PASSIVE_PIN, LOW, DISABLE, DISABLE);	
}
#endif



/*! \brief Main function. Execution starts here.
 */
int main(void)
{	
	sysclk_init();
	irq_initialize_vectors();
	cpu_irq_enable();

	/* Initialize the sleep manager */
	sleepmgr_init();
	
	/* Initialize the SAM board */
	board_init();
	
	/* Serial line [UART] initialization */
	uart1_init(CONF_UART_BAUDRATE);
	
	#if WITH_SERIAL_LINE_INPUT
	/* If SLIP-radio is enabled, the handler is overridden. */
	uart1_set_input(serial_line_input_byte);
	#endif
	
	while(!uart_is_tx_ready(CONSOLE_UART));
	/* PRINT Contiki Entry String */
	PRINTF("Starting ");	
	PRINTF(CONTIKI_VERSION_STRING);	
	
	/* Configure sys-tick for 1 ms */
	clock_init(); 
	
	/* Initialize Contiki Process function */
	process_init();
	
	/* rtimer and ctimer should be initialized before radio duty cycling layers*/
	rtimer_init();
	
	/* etimer_process should be initialized before ctimer */
	process_start(&etimer_process, NULL);
	
	/* Initialize the ctimer process */ 
	ctimer_init();	
#ifdef WITH_LED_DEBUGGING
	configure_led_debug_pins();
#ifdef WITH_AR9170_WIFI_SUPPORT
	configure_ar9170_disconnect_pins();
#endif
#endif		
	
	/* rtimer initialization */
	rtimer_init();
	
	/* Network protocol stack initialization */
	netstack_init();
	
	/* Process init initialization */
	procinit_init();
	
	/* Initialize energy estimation routines */
	energest_init();
		
	/* Initialize watch-dog process */
	watchdog_start();  

#ifdef WITH_AR9170_WIFI_SUPPORT
#ifdef WITH_USB_SUPPORT
	/* Start network-related system processes. */
	#if WITH_UIP6	
	#ifdef WITH_SLIP
	#warning SLIP_RADIO enabled!
	process_start(&slip_radio_process, NULL);
	#endif
	#endif		
#else
#error USB support must be enabled.
#endif
#endif

#ifdef WITH_USB_SUPPORT
	/* Start ARM Cortex-M3 USB Host Stack */
	uhc_start();
	configure_ar9170_disconnect_pins();
#endif	

	/* Autostart all declared [not system] processes */
	//autostart_start(autostart_processes);

	#if UIP_CONF_IPV6
	printf("Tentative link-local IPv6 address: ");
	{
		uip_ds6_addr_t *lladdr;
		int i;
		lladdr = uip_ds6_get_link_local(-1);
		for(i = 0; i < 7; ++i) {
			printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
			lladdr->ipaddr.u8[i * 2 + 1]);
		}
		printf("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
	}

	if(!UIP_CONF_IPV6_RPL) {
		uip_ipaddr_t ipaddr;
		int i;
		uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
		uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
		uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
		printf("Tentative global IPv6 address ");
		for(i = 0; i < 7; ++i) {
			printf("%02x%02x:",
			ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
		}
		printf("%02x%02x\n",
		ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
	}
	#endif /* UIP_CONF_IPV6 */

	PRINTF("Starting Contiki OS main loop...\n");	

	while(true) {
		
		/* Contiki Polling System */
		process_run();
	}	
}