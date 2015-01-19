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
// #include "dev/leds.h"
#include "dev/watchdog.h"
// #include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
// #include "dev/cc2520/cc2520.h"

#include "net/wifi-drv.h"

#include "lib/random.h"
#include "net/netstack.h"
#include "net/queuebuf.h"
#include "net/ip/tcpip.h"
#include "net/ip/uip.h"
#include "net/mac/frame802154.h"

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
/*
static void
fade(unsigned char l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 800; ++k) {
    j = k > 400 ? 800 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      asm("nop");
    }
    leds_off(l);
    for(i = 0; i < 400 - j; ++i) {
      asm("nop");
    }
  }
}
*/
/*---------------------------------------------------------------------------*/
//static void
//set_rf_params(void)
//{
//  uint16_t short_addr;
//  uint8_t ext_addr[8];
//
//  ieee_addr_cpy_to(ext_addr, 8);
//
//  short_addr = ext_addr[7];
//  short_addr |= ext_addr[6] << 8;
//
//  /* Populate linkaddr_node_addr. Maintain endianness */
//  memcpy(&linkaddr_node_addr, &ext_addr[8 - LINKADDR_SIZE], LINKADDR_SIZE);
//
//#if STARTUP_CONF_VERBOSE
//  {
//    int i;
//    printf("Rime configured with address ");
//    for(i = 0; i < LINKADDR_SIZE - 1; i++) {
//      printf("%02x:", linkaddr_node_addr.u8[i]);
//    }
//    printf("%02x\n", linkaddr_node_addr.u8[i]);
//  }
//#endif
//
//  NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, IEEE802154_PANID);
//  NETSTACK_RADIO.set_value(RADIO_PARAM_16BIT_ADDR, short_addr);
//  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CC2520_RF_CHANNEL);
//  NETSTACK_RADIO.set_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8);
//
//}
/*---------------------------------------------------------------------------*/
/**
 * \brief Contiki main routine for the cc3200-launchxl platform
 */
void
contiki_main(void *pv_parameters)
{
	// leds_init();
	//fade(LEDS_YELLOW);

	process_init();

	watchdog_init();
	// button_sensor_init();

	serial_line_init();
	// fade(LEDS_GREEN);

	PUTS(CONTIKI_VERSION_STRING);
	PUTS(PLATFORM_STRING);

	PRINTF(" Net: ");
	PRINTF("%s\n", NETSTACK_NETWORK.name);
	PRINTF(" MAC: ");
	PRINTF("%s\n", NETSTACK_MAC.name);
	PRINTF(" RDC: ");
	PRINTF("%s\n\n", NETSTACK_RDC.name);

	/* Initialise the H/W RNG engine. */
	random_init(0);

	clock_init();
	rtimer_init();

	process_start(&etimer_process, NULL);
	ctimer_init();

	// set_rf_params();
	// netstack_init();

//#if NETSTACK_CONF_WITH_IPV6
//  memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
//  queuebuf_init();
//  process_start(&tcpip_process, NULL);
//#endif /* NETSTACK_CONF_WITH_IPV6 */

  // process_start(&sensors_process, NULL);

	process_start(&wifi_process, NULL);
	autostart_start(autostart_processes);

	watchdog_start();
	// fade(LEDS_ORANGE);

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
