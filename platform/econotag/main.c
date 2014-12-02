/*
 * Copyright (c) 2012, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
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

#include <string.h>

/* debug */
#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"

/* contiki */
#include "contiki.h"
#include "dev/button-sensor.h"
#include "net/linkaddr.h"
#include "net/netstack.h"

/* mc1322x */
#include "mc1322x.h"
#include "config.h"
#include "contiki-uart.h"

/* econotag */
#include "platform_prints.h"

#ifndef OWN_SENSORS_DEFINITION
SENSORS(&button_sensor, &button_sensor2);
#endif

#ifndef M12_CONF_SERIAL
#define M12_SERIAL 0x000000
#else
#define M12_SERIAL M12_CONF_SERIAL
#endif

int main(void) {

	mc1322x_init();

	/* m12_init() flips the mux switch */

	/* trims the main crystal load capacitance */
	if (!FORCE_ECONOTAG_I && CRM->SYS_CNTLbits.XTAL32_EXISTS) {
		/* M12 based econotag */
		PRINTF("trim xtal for M12\n\r");
		CRM->XTAL_CNTLbits.XTAL_CTUNE = (M12_CTUNE_4PF << 4) | M12_CTUNE;
		CRM->XTAL_CNTLbits.XTAL_FTUNE = M12_FTUNE;

		/* configure pullups for low power */
		GPIO->FUNC_SEL.GPIO_63 = 3;
		GPIO->PAD_PU_SEL.GPIO_63 = 0;
		GPIO->FUNC_SEL.SS = 3;
		GPIO->PAD_PU_SEL.SS = 1;
		GPIO->FUNC_SEL.VREF2H = 3;
		GPIO->PAD_PU_SEL.VREF2H = 1;
		GPIO->FUNC_SEL.U1RTS = 3;
		GPIO->PAD_PU_SEL.U1RTS = 1;

	} else {
		/* econotag I */
		PRINTF("trim xtal for Econotag I\n\r");
		CRM->XTAL_CNTLbits.XTAL_CTUNE = (ECONOTAG_CTUNE_4PF << 4) | ECONOTAG_CTUNE;
		CRM->XTAL_CNTLbits.XTAL_FTUNE = ECONOTAG_FTUNE;
	}

	/* create mac address if blank*/
	if (mc1322x_config.eui == 0) {
		/* mac address is blank */
		/* construct a new mac address based on IAB or OUI definitions */

		/* if an M12_SERIAL number is not defined */
		/* generate a random extension in the Redwire experimental IAB */
		/* The Redwire IAB (for development only) is: */
		/* OUI: 0x0050C2 IAB: 0xA8C */
		/* plus a random 24-bit extension */
		/* Otherwise, construct a mac based on the M12_SERIAL */
		/* Owners of an Econotag I (not M12 based) can request a serial number from Redwire */
		/* to use here */

		/* M12 mac is of the form "EC473C4D12000000" */
		/* Redwire's OUI: EC473C */
		/* M12: 4D12 */
		/* next six nibbles are the M12 serial number as hex */
		/* e.g. if the barcode reads: "12440021" = BDD1D5 */
		/* full mac is EC473C4D12BDD1D5 */

#if (M12_SERIAL == 0)
                /* use random mac from experimental range */
		mc1322x_config.eui = (0x0050C2A8Cull << 24) | (*MACA_RANDOM & (0xffffff));
#else
		/* construct mac from serial number */
		mc1322x_config.eui = (0xEC473C4D12ull << 24) | M12_SERIAL;
#endif
		mc1322x_config_save(&mc1322x_config);		
	} 
	
	/* configure address on maca hardware and RIME */
	contiki_maca_set_mac_address(mc1322x_config.eui);

#if NETSTACK_CONF_WITH_IPV6
	memcpy(&uip_lladdr.addr, &linkaddr_node_addr.u8, sizeof(uip_lladdr.addr));
	queuebuf_init();
	NETSTACK_RDC.init();
	NETSTACK_MAC.init();
	NETSTACK_NETWORK.init();
  #if DEBUG_ANNOTATE
	print_netstack();
  #endif
	process_start(&tcpip_process, NULL);
  #if DEBUG_ANNOTATE
	print_lladdrs();
  #endif
#endif /* endif NETSTACK_CONF_WITH_IPV6 */

	process_start(&sensors_process, NULL);

	print_processes(autostart_processes); 
	autostart_start(autostart_processes);

	/* Main scheduler loop */
	while(1) {
		check_maca();

		if(uart1_input_handler != NULL) {
			if(uart1_can_get()) {
				uart1_input_handler(uart1_getc());
			}
		}
		
		process_run();
	}
	
	return 0;
}
