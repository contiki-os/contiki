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
#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "contiki-msb430.h"
#include "dev/adc.h"
#include "dev/sd.h"
#include "dev/serial-line.h"
#include "dev/sht11.h"
#include "dev/watchdog.h"

extern volatile bool uart_edge;

extern void init_net(void);

SENSORS(NULL);

static void
msb_ports_init(void)
{
  P1SEL = 0x00; P1OUT = 0x00; P1DIR = 0x00;
  P2SEL = 0x00; P2OUT = 0x18; P2DIR = 0x1A;
  P3SEL = 0x00; P3OUT = 0x09; P3DIR = 0x21;
  P4SEL = 0x00; P4OUT = 0x00; P4DIR = 0x00;
  P5SEL = 0x0E; P5OUT = 0xF9; P5DIR = 0xFD;
  P6SEL = 0x07; P6OUT = 0x00; P6DIR = 0xC8;
}

int
main(void)
{
#if WITH_SD
  int r;
#endif /* WITH_SD */

  msp430_cpu_init();	
  watchdog_stop();

  /* Platform-specific initialization. */
  msb_ports_init();
  adc_init();

  clock_init();
  rtimer_init();

  sht11_init();
  leds_init();
  leds_on(LEDS_ALL);

  process_init();

  /* serial interface */
  rs232_set_input(serial_line_input_byte);
  rs232_init();
  serial_line_init();

  uart_lock(UART_MODE_RS232);
  uart_unlock(UART_MODE_RS232);
#if WITH_UIP
  slip_arch_init(BAUD2UBR(115200));
#endif


#if WITH_SD
  r = sd_initialize();
  if(r < 0) {
    printf("Failed to initialize the SD driver: %s\n", sd_error_string(r));
  } else {
    sd_offset_t capacity;
    printf("The SD driver was successfully initialized\n");
    capacity = sd_get_capacity();
    if(capacity < 0) {
      printf("Failed to get the SD card capacity: %s\n", sd_error_string(r));
    } else {
      printf("SD card capacity: %u MB\n",
	(unsigned)(capacity / (1024UL * 1024)));
    }
  }
#endif

  node_id_restore();

  /* System timers */
  process_start(&etimer_process, NULL);
  ctimer_init();

  /* Networking stack. */
  NETSTACK_RADIO.init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();
  {
    rimeaddr_t rimeaddr;
  
    rimeaddr.u8[0] = node_id & 0xff;
    rimeaddr.u8[1] = node_id >> 8;
    rimeaddr_set_node_addr(&rimeaddr);
  }

  energest_init();
 
#if PROFILE_CONF_ON
  profile_init();
#endif /* PROFILE_CONF_ON */
 
  leds_off(LEDS_ALL);

  printf("Node %d.%d: %s %s, channel check rate %u Hz\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ?
		1 : (unsigned)NETSTACK_RDC.channel_check_interval()));

  autostart_start(autostart_processes);

  /*
   * This is the scheduler loop.
   */
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  while (1) {
    int r;
#if PROFILE_CONF_ON
    profile_episode_start();
#endif /* PROFILE_CONF_ON */
    do {
      /* Reset watchdog. */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);

#if PROFILE_CONF_ON
    profile_episode_end();
#endif /* PROFILE_CONF_ON */

    /*
     * Idle processing.
     */
    int s = splhigh();		/* Disable interrupts. */
    if (process_nevents() != 0) {
      splx(s);			/* Re-enable interrupts. */
    } else {
      static unsigned long irq_energest = 0;
      /* Re-enable interrupts and go to sleep atomically. */
      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);
     /*
      * We only want to measure the processing done in IRQs when we
      * are asleep, so we discard the processing time done when we
      * were awake.
      */
      energest_type_set(ENERGEST_TYPE_IRQ, irq_energest);

      if (uart_edge) {
	_BIC_SR(LPM1_bits + GIE);
      } else {
	_BIS_SR(LPM1_bits + GIE);
      }

      /*
       * We get the current processing time for interrupts that was
       * done during the LPM and store it for next time around. 
       */
      dint();
      irq_energest = energest_type_time(ENERGEST_TYPE_IRQ);
      eint();
      ENERGEST_OFF(ENERGEST_TYPE_LPM);
      ENERGEST_ON(ENERGEST_TYPE_CPU);
#if PROFILE_CONF_ON
      profile_clear_timestamps();
#endif /* PROFILE_CONF_ON */
    }
  }

  return 0;
}
