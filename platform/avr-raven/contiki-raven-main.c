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

//#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"
//#include <stdbool.h>

#ifdef RF230BB
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#include "net/sicslowpan.h"
#include "net/uip-netif.h"
#include "net/mac/sicslowmac.h"

#else
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

#ifdef RAVEN_LCD_INTERFACE
#include "raven-lcd.h"
#endif

#include "sicslowmac.h"
#if UIP_CONF_ROUTER
#include "net/routing/rimeroute.h"
#include "net/rime/rime-udp.h"
#endif /* UIP_CONF_ROUTER*/
#include "net/rime.h"
//#include "node-id.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...) do {} while (0)
#endif

typedef struct
{
    unsigned char B2;
    unsigned char B1;
    unsigned char B0;
} __signature_t;


/* Put the MCU signature in the .elf file */
#define SIGNATURE __signature_t __signature __attribute__((section (".signature")))

SIGNATURE =
{
/* Older AVR-GCCs may not define the SIGNATURE_n bytes so use explicit 1284p values */
  .B2 = 0x05,//SIGNATURE_2,
  .B1 = 0x97,//SIGNATURE_1,
  .B0 = 0x1E,//SIGNATURE_0,
};

/* Set the fuses in the .elf file */
FUSES =
{
  .low = 0xe2,
  .high = 0x99,
  .extended = 0xff,
};

/* Put default MAC address in EEPROM */
uint8_t mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};

#ifdef RAVEN_LCD_INTERFACE
#ifdef RF230BB
PROCINIT(&etimer_process, &tcpip_process, &raven_lcd_process);
#else
PROCINIT(&etimer_process, &mac_process, &tcpip_process, &raven_lcd_process);
#endif /*RF230BB*/
#else
#ifdef RF230BB
PROCINIT(&etimer_process, &tcpip_process);
#else
PROCINIT(&etimer_process, &mac_process, &tcpip_process);
#endif /*RF230BB*/
#endif /*RAVEN_LCD_INTERFACE*/

void
init_lowlevel(void)
{
#ifdef RAVEN_LCD_INTERFACE
  /* First rs232 port for Raven 3290 port */
  rs232_init(RS232_PORT_0, USART_BAUD_38400,
             USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);

  /* Set input handler for 3290 port */
  rs232_set_input(0,raven_lcd_serial_input);
#endif

  /* Second rs232 port for debugging */
  rs232_init(RS232_PORT_1, USART_BAUD_57600,
             USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);

  /* Redirect stdout to second port */
  rs232_redirect_stdout(RS232_PORT_1);
}
#ifdef RF230BB
/* Address initialization when using the core MAC layer */
/*---------------------------------------------------------------------------*/
static void
set_addresses(void)
{
  rimeaddr_t addr;
  
  memset(&addr, 0, sizeof(rimeaddr_t));
  AVR_ENTER_CRITICAL_REGION();
  eeprom_read_block ((void *)&addr.u8,  &mac_address, 8);
  AVR_LEAVE_CRITICAL_REGION();
 
  memcpy(&uip_lladdr.addr, &addr.u8, 8);	
  rf230_set_pan_addr(IEEE802154_PANID, 0, (uint8_t *)&addr.u8);

  rf230_set_channel(24);
  rimeaddr_set_node_addr(&addr); 
  PRINTF("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n",addr.u8[0],addr.u8[1],addr.u8[2],addr.u8[3],addr.u8[4],addr.u8[5],addr.u8[6],addr.u8[7]);

 // uip_ip6addr(&ipprefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
 // uip_netif_addr_add(&ipprefix, UIP_DEFAULT_PREFIX_LEN, 0, AUTOCONF);
 // uip_nd6_prefix_add(&ipprefix, UIP_DEFAULT_PREFIX_LEN, 0);
 // PRINTF("Prefix %x::/%u\n",ipprefix.u16[0],UIP_DEFAULT_PREFIX_LEN);

#if UIP_CONF_ROUTER
  rime_init(rime_udp_init(NULL));
  uip_router_register(&rimeroute);
#endif
}

#if DEBUG
int pingtimer1=0;pingtimer2=0
extern int rf230_interrupt_flag;
#endif
#endif /*RF230BB*/

int
main(void)
{
  //calibrate_rc_osc_32k(); //CO: Had to comment this out

  /* Initialize serial ports */
  init_lowlevel();

  /* Clock */
  clock_init();
 
   printf_P(PSTR("\n********BOOTING CONTIKI*********\n"));

 /* Process subsystem */
  process_init();

#ifdef RF230BB
  /* Start radio and radio receive process */
  rf230_init();
  sicslowpan_init(sicslowmac_init(&rf230_driver));
//  ctimer_init();
//  sicslowpan_init(lpp_init(&rf230_driver));
//  rime_init(sicslowmac_driver.init(&rf230_driver));
//  rime_init(lpp_init(&rf230_driver));
  
  /* Set addresses BEFORE starting tcpip process */
  set_addresses(); 
  PRINTF(PSTR("Driver: %s, Channel: %u\n"), sicslowmac_driver.name, rf230_get_channel()); 
#endif /*RF230BB*/

  /* Register initial processes */
  procinit_init(); 

  /* Autostart processes */
  autostart_start(autostart_processes);

  //Give ourselves a prefix
 // init_net();

  printf_P(PSTR(CONTIKI_VERSION_STRING" online\n"));

  /* Main scheduler loop */
  while(1) {

    process_run();

#if DEBUG
    if (rf230_interrupt_flag) {
      if (rf230_interrupt_flag!=11) {
        PRINTF("*****Radio interrupt %u\n",rf230_interrupt_flag);
        rf230_interrupt_flag=0;
      }
    }
    if (pingtimer1++==10000) {
      pingtimer1=0;
      if (pingtimer2++==1000) { 
        PRINTF("-------Ping\n");
        pingtimer2=0;
        raven_ping6();
      }
    }
#endif

  }

  return 0;
}
