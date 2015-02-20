/*
 * Copyright (c) 2015, 3B Scientific GmbH.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \addtogroup platform
 * @{
 *
 * \file
 *   Main module for the cc3200-launchxl platform
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include "dev/leds.h"
#include "dev/watchdog.h"
// #include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
// #include "dev/cc2520/cc2520.h"

#include "lib/random.h"
#include "net/netstack.h"
#include "net/queuebuf.h"
#include "net/ip/tcpip.h"
#include "net/ip/uip.h"
#include "net/mac/frame802154.h"

#include "net/wifi.h"
#include "net/ip64/ip64.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#if STARTUP_CONF_VERBOSE
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if UART_CONF_ENABLE
#define PUTS(s) puts(s)
#else
#define PUTS(s)
#endif

#ifdef CC2520_RF_CONF_CHANNEL
#define CC2520_RF_CHANNEL CC2520_RF_CONF_CHANNEL
#else
#define CC2520_RF_CHANNEL 18
#endif

/*---------------------------------------------------------------------------*/
#if UIP_CONF_LOGGING
void
uip_log(char *msg)
{
	PUTS(msg);
}
#endif /* UIP_CONF_LOGGING */

/*---------------------------------------------------------------------------*/
#if LOG_CONF_ENABLED
void
log_message(char *m1, char *m2)
{
	PRINTF("%s%s\n", m1, m2);
}
#endif /* LOG_CONF_ENABLED */

/*---------------------------------------------------------------------------*/
static void
fade(unsigned char l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 4000; ++k) {
    j = k > 2000 ? 4000 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      asm("nop");
    }
    leds_off(l);
    for(i = 0; i < 2000 - j; ++i) {
      asm("nop");
    }
  }
}

/*---------------------------------------------------------------------------*/
static void
set_rf_params(void)
{
	uint16_t short_addr;
	uint8_t ext_addr[8];

	// Generate ZigBee address from Wifi MAC address
	memset(&ext_addr[0], 0x3B, sizeof(ext_addr));
	memcpy(&ext_addr[1], &wifi_mac_addr[0], sizeof(wifi_mac_addr));

	short_addr = ext_addr[7];
	short_addr |= ext_addr[6] << 8;

	/* Populate linkaddr_node_addr. Maintain endianness */
	memcpy(&linkaddr_node_addr, &ext_addr[8 - LINKADDR_SIZE], LINKADDR_SIZE);

	#if STARTUP_CONF_VERBOSE
	{
		int i;
		PRINTF("Rime configured with address ");
		for(i = 0; i < LINKADDR_SIZE - 1; i++) {
			PRINTF("%02x:", linkaddr_node_addr.u8[i]);
		}
		PRINTF("%02x\n", linkaddr_node_addr.u8[i]);
	}
	#endif

	NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, IEEE802154_PANID);
	NETSTACK_RADIO.set_value(RADIO_PARAM_16BIT_ADDR, short_addr);
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CC2520_RF_CHANNEL);
	NETSTACK_RADIO.set_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Contiki main routine for the cc3200-launchxl platform
 */
void
contiki_main(void *pv_parameters)
{
	leds_init();

	// Output platform information
	PUTS(CONTIKI_VERSION_STRING);
	PRINTF("%s\n\n", PLATFORM_STRING);

	// Initialize cc32xx wireless driver and fade red led
	wifi_init();
	fade(LEDS_RED);

	// Initialize watch dog subsystem
	watchdog_init();

	// Initialize clock system
	clock_init();
	rtimer_init();

	// Initialize process subsystem
	process_init();

	// Event timers must be started before ctimer_init
	process_start(&etimer_process, NULL);
	ctimer_init();

	// button_sensor_init();
	serial_line_init();
	fade(LEDS_YELLOW);

	// Setup ZigBee stack
	PRINTF("Starting ZigBee Network\n");
	PRINTF(" Net: ");
	PRINTF("%s\n", NETSTACK_NETWORK.name);
	PRINTF(" MAC: ");
	PRINTF("%s\n", NETSTACK_MAC.name);
	PRINTF(" RDC: ");
	PRINTF("%s\n\n", NETSTACK_RDC.name);

	// Initialize random number engine.
	random_init(0);

	// Setup network stack
	set_rf_params();
	netstack_init();

#if NETSTACK_CONF_WITH_IPV6
	// Set IPv6 address
	memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
	queuebuf_init();

	// Start TCP/IP stack
	process_start(&tcpip_process, NULL);

	// Start IPv6 <--> IPv4 translator
	// for CC32xx wireless network interface
	ip64_init();
#endif /* NETSTACK_CONF_WITH_IPV6 */

  // process_start(&sensors_process, NULL);

	autostart_start(autostart_processes);

	watchdog_start();
	fade(LEDS_GREEN);

	while(1)
	{
		// Reset watchdog
		watchdog_periodic();

		// Handle polls and events
		process_run();
	}
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
