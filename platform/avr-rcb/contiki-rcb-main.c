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

#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"

#if RF230BB           //radio driver using contiki core mac
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/sicslowpan.h"
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
		.low = 0xe2,
		.high = 0x99,
		.extended = 0xff,
	};
	
PROCESS(rcb_leds, "RCB leds process");

#if RF230BB
PROCINIT(&etimer_process, &tcpip_process, &rcb_leds);
#else
PROCINIT(&etimer_process, &mac_process, &tcpip_process, &rcb_leds);
#endif

/* Put default MAC address in EEPROM */
uint8_t mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};

#define LED1 (1<<PE2)
#define LED2 (1<<PE3)
#define LED3 (1<<PE4)

#define LEDOff(x) (PORTE |= (x))
#define LEDOn(x) (PORTE &= ~(x))


void
init_lowlevel(void)
{
  /* Second rs232 port for debugging */
  rs232_init(RS232_PORT_1, USART_BAUD_57600,
             USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);

  /* Redirect stdout to second port */
  rs232_redirect_stdout(RS232_PORT_1);
  
  DDRE |= LED1 | LED2 | LED3;
}


static struct etimer et;
PROCESS_THREAD(rcb_leds, ev, data)
{
  u8_t error;

  PROCESS_BEGIN();
  
  if((error = icmp6_new(NULL)) == 0) {
    while(1) {
      PROCESS_YIELD();
      
#if UIP_CONF_IPV6	  
	  if (ev == ICMP6_ECHO_REQUEST) {
#else
 		if (1) {
#endif        
		LEDOn(LED2);
		etimer_set(&et, CLOCK_SECOND/10);
	  } else {
		LEDOff(LED2);
	  }
    }
  }
  PROCESS_END();
}


int
main(void)
{
  //calibrate_rc_osc_32k(); //CO: Had to comment this out

  /* Initialize hardware */
  init_lowlevel();

  /* Clock */
  clock_init();

  LEDOff(LED1 | LED2);

  /* Process subsystem */
  process_init();

  /* Register initial processes */
  procinit_init();

  /* Autostart processes */
  autostart_start(autostart_processes);
  
  printf_P(PSTR("\n********BOOTING CONTIKI*********\n"));

  printf_P(PSTR("System online.\n"));

  /* Main scheduler loop */
  while(1) {
    process_run();
  }

  return 0;
}
