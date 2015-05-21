/*
 * Copyright (c) 2006, Technical University of Munich
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
 * @(#)$$
 */

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"

#define ANNOUNCE_BOOT 0    //adds about 600 bytes to program size
#define DEBUG 0
#if DEBUG
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#define PRINTSHORT(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#define PRINTSHORT(...)
#endif

#if RF230BB           //radio driver using contiki core mac
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/ipv6/sicslowpan.h"
#else                 //radio driver using Atmel/Cisco 802.15.4'ish MAC
#include <stdbool.h>
#include "mac.h"
#include "sicslowmac.h"
#include "sicslowpan.h"
#include "ieee-15-4-manager.h"
#endif /*RF230BB*/

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#include "dev/rs232.h"
#include "dev/serial-line.h"
#include "dev/slip.h"

#include "sicslowmac.h"

FUSES =
	{
		.low = (FUSE_CKSEL0 & FUSE_CKSEL2 & FUSE_CKSEL3 & FUSE_SUT0), // 0xe2,
		.high = (FUSE_BOOTSZ0 /*& FUSE_BOOTSZ1*/ & FUSE_SPIEN & FUSE_JTAGEN), //0x9D,
		.extended = 0xff,
	};
	
#if RF230BB
#if NETSTACK_CONF_WITH_IPV6 || NETSTACK_CONF_WITH_IPV4
//PROCINIT(&etimer_process, &tcpip_process );
#else
//PROCINIT(&etimer_process );
#endif
#else
#if NETSTACK_CONF_WITH_IPV6 || NETSTACK_CONF_WITH_IPV4
PROCINIT(&etimer_process, &mac_process, &tcpip_process );
#else
PROCINIT(&etimer_process, &mac_process );
#endif
#endif
/* Put default MAC address in EEPROM */
#if MY_NODE_ID
uint8_t mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 
  MY_NODE_ID};
#else
uint8_t mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
#endif

void
init_lowlevel(void)
{

  /* Second rs232 port for debugging */
  rs232_init(RS232_PORT_1, USART_BAUD_38400,
             USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);

  /* Redirect stdout to second port */
  rs232_redirect_stdout(RS232_PORT_1);
  
  /* Clock */
  clock_init();
 
 /* rtimers needed for radio cycling */
  rtimer_init();

 /* Initialize process subsystem */
  process_init();
 /* etimers must be started before ctimer_init */
  process_start(&etimer_process, NULL);
  
#if RF230BB

  ctimer_init();
  /* Start radio and radio receive process */
  NETSTACK_RADIO.init();

  /* Set addresses BEFORE starting tcpip process */

  linkaddr_t addr;
  memset(&addr, 0, sizeof(linkaddr_t));
  eeprom_read_block ((void *)&addr.u8,  &mac_address, 8);
 
#if NETSTACK_CONF_WITH_IPV6
  memcpy(&uip_lladdr.addr, &addr.u8, 8);
#endif  
  rf230_set_pan_addr(IEEE802154_PANID, 0, (uint8_t *)&addr.u8);
#ifdef CHANNEL_802_15_4
  rf230_set_channel(CHANNEL_802_15_4);
#else
  rf230_set_channel(26);
#endif

  linkaddr_set_node_addr(&addr); 

  PRINTF("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n",addr.u8[0],addr.u8[1],addr.u8[2],addr.u8[3],addr.u8[4],addr.u8[5],addr.u8[6],addr.u8[7]);

  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

#if ANNOUNCE_BOOT
  printf_P(PSTR("%s %s, channel %u"),NETSTACK_MAC.name, NETSTACK_RDC.name,rf230_get_channel());
  if (NETSTACK_RDC.channel_check_interval) {//function pointer is zero for sicslowmac
    unsigned short tmp;
    tmp=CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval == 0 ? 1:\
                                   NETSTACK_RDC.channel_check_interval());
    if (tmp<65535) printf_P(PSTR(", check rate %u Hz"),tmp);
  }
  printf_P(PSTR("\n"));
#endif

#if UIP_CONF_ROUTER
#if ANNOUNCE_BOOT
  printf_P(PSTR("Routing Enabled\n"));
#endif
  //rime_init(rime_udp_init(NULL));
  //uip_router_register(&rimeroute);
#endif
#if NETSTACK_CONF_WITH_IPV6 || NETSTACK_CONF_WITH_IPV4
  process_start(&tcpip_process, NULL);
#endif
#else
/* mac process must be started before tcpip process! */
  process_start(&mac_process, NULL);
#if NETSTACK_CONF_WITH_IPV6 || NETSTACK_CONF_WITH_IPV4
  process_start(&tcpip_process, NULL);
#endif
#endif /*RF230BB*/

}


int
main(void)
{
  //calibrate_rc_osc_32k(); //CO: Had to comment this out

  /* Initialize hardware */
  init_lowlevel();

  /* Register initial processes */
//  procinit_init();

  printf_P(PSTR("\n********BOOTING CONTIKI*********\n"));

  printf_P(PSTR("System online.\n"));

  /* Autostart processes */
  autostart_start(autostart_processes);

  /* Main scheduler loop */
  while(1) {
    process_run();
  }

  return 0;
}
