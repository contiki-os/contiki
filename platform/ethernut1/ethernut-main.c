
/* Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
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
 * $Id: ethernut-main.c,v 1.5 2010/12/03 21:39:33 dak664 Exp $
 *
 */

#include "contiki.h"
#include "contiki-net.h"

#include "dev/rs232.h"
#include "dev/rtl8019-drv.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

PROCINIT(&etimer_process, &tcpip_process, &rtl8019_process);

static const struct uip_eth_addr ethaddr = {{0x00,0x06,0x98,0x01,0x02,0x29}};

int
main(void)
{
  uip_ipaddr_t addr;

  /*
   * GCC depends on register r1 set to 0.
   */
  asm volatile ("clr r1");

  /*
   * No interrupts used.
   */
  cli();

  /*
   * Enable external data and address
   * bus.
   */
  MCUCR = _BV(SRE) | _BV(SRW);

  clock_init();
  rs232_init(RS232_PORT_0, USART_BAUD_57600,USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);

  sei();


  process_init();

  uip_ipaddr(&addr, 193,10,67,152);
  uip_sethostaddr(&addr);

  uip_setethaddr(ethaddr);

  procinit_init();

  autostart_start(autostart_processes);

  rs232_print(RS232_PORT_0, "Initialized\n");

  while(1) {
    process_run();
  }

  return 0;
}

int
putchar(int c)
{
  rs232_send(RS232_PORT_0, c);
  return c;
}
