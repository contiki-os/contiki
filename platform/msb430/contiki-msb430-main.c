/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * \file
 *	Main system file for the MSB-430 port.
 * \author
 * 	Michael Baar <baar@inf.fu-berlin.de>, Nicolas Tsiftes <nvt@sics.se>
 */

#include <io.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>

#include "contiki-msb430.h"

#include "sys/procinit.h"
#include "sys/autostart.h"
#include "sys/rtimer.h"

#include "dev/adc.h"
#include "dev/dma.h"
#include "dev/sht11.h"
#include "dev/slip.h"

#include "net/mac/nullmac.h"
#include "net/mac/xmac.h"

SENSORS(NULL);

#if WITH_UIP
static struct uip_fw_netif slipif =
  {UIP_FW_NETIF(172,16,0,1, 255,255,255,0, slip_send)};
#else
int
putchar(int c)
{
  rs232_send(c);
  return c;
}
#endif /* WITH_UIP */

static void
set_rime_addr(void)
{
  rimeaddr_t addr;

  addr.u16[0] = node_id;
  rimeaddr_set_node_addr(&addr);
}

static void
msb_ports_init(void)
{
  P1DIR = 0x00; P1SEL = 0x00; P1OUT = 0x00;
  P2DIR = 0x1A; P2SEL = 0x00; P2OUT = 0x18;
  P3DIR = 0x21; P3SEL = 0x00; P3OUT = 0x09;
  P4DIR = 0x00; P4SEL = 0x00; P4OUT = 0x07;
  P5DIR = 0xFD; P5SEL = 0x0E; P5OUT = 0xF9;
  P6DIR = 0xC8; P6SEL = 0x07; P6OUT = 0x00;
}

int
main(void)
{
  msp430_cpu_init();	

  /* Platform-specific initialization. */
  msb_ports_init();
  adc_init();
  dma_init();

  clock_init();
  leds_init();
  leds_on(LEDS_ALL);
  sht11_init();

  // low level
  irq_init();
  process_init();

  // serial interface
  rs232_init();

#ifdef WITH_SDC
  spi_init();
#endif

  uart_lock(UART_MODE_RS232);
  uart_unlock(UART_MODE_RS232);
#if WITH_UIP
  slip_arch_init(BAUD2UBR(115200));
#endif

  rtimer_init();
  ctimer_init();

  energest_init();
  node_id_restore();

  /* System services */
  process_start(&etimer_process, NULL);
#if 0
  process_start(&sensors_process, NULL);
#endif

  /* Radio driver */
  cc1020_init(cc1020_config_19200);

  /* Network configuration */
#if WITH_UIP
  uip_init();
  uip_sethostaddr(&slipif.ipaddr);
  uip_setnetmask(&slipif.netmask);
  /* Point-to-point, no default router. */
  uip_fw_default(&slipif);
  tcpip_set_forwarding(0);

  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);
  process_start(&slip_process, NULL);
#endif /* WITH_UIP */

  nullmac_init(&cc1020_driver);
  rime_init(&nullmac_driver);
  set_rime_addr();
 
  printf(CONTIKI_VERSION_STRING " started. Node id %d.\n", node_id);

  printf("Autostarting processes\n");
  autostart_start((struct process **) autostart_processes);

  leds_off(LEDS_ALL);
  lpm_on();

  for (;;) {
    while (process_run() > 0);
    if (process_nevents() == 0) {
      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);
      LPM_SLEEP();
      ENERGEST_OFF(ENERGEST_TYPE_LPM);
      ENERGEST_ON(ENERGEST_TYPE_CPU);
    }
  }

  return 0;
}
