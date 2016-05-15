/*
 * Copyright (c) 2015, Copyright Robert Olsson / Radio Sensors AB  
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
 *
 * Author  : Robert Olsson robert@radio-sensors.com
 * Created : 2015-12-30
 */

/**
 * \file
 *         An application for monitoring current use on Atmel 
 *         radios also supporting the RPC function on RFR2 chips.
 *         Current needs to be measured with amp-meter.
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "powertrace.h"
#include <stdio.h>
#include "dev/leds.h"
#include "rf230bb.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"
#include <avr/sleep.h>
#include <dev/watchdog.h>

#define TEST_PERIOD 6

static int ps = 0;      /* Power-save false */
static char buf[110];
static struct etimer et;

PROCESS(power_use_process, "Power use test");
PROCESS(sleep_process,   "Sleep process");
AUTOSTART_PROCESSES(&power_use_process, &sleep_process);

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;


PROCESS_THREAD(power_use_process, ev, data)
{
  static int i, j;
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
    PROCESS_BEGIN();

  /* powertrace_start(CLOCK_SECOND * 2); */
  broadcast_open(&broadcast, 129, &broadcast_call);
  leds_init();
  rf230_set_channel(25);  

  for(i=0; i < 2; i++) {   /* Loop over min and max rpc settings  */

    NETSTACK_RADIO.off(); /* Radio off for rpc change */
    NETSTACK_RADIO.off();
    etimer_set(&et, CLOCK_SECOND * 8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if(i == 0) 
      rf230_set_rpc(0x0); /* Disbable all RPC features */
    else 
      rf230_set_rpc(0xFF); /* Enable all RPC features. Only XRFR2 radios */
    NETSTACK_RADIO.on();

    /*  Loop over the different TX power settings 0-15  */
    
    for(j=15; j >= 0; j--) {
      NETSTACK_RADIO.on();
      rf230_set_txpower(j);
      ps = 0;
      printf("TX with PWR=%d RPC=0x%02x\n", rf230_get_txpower(), rf230_get_rpc());
      ps = 1;
      
      etimer_set(&et, CLOCK_SECOND * TEST_PERIOD);
      
      while(1) {
	if(etimer_expired(&et))
	  break;
	packetbuf_copyfrom(buf, sizeof(buf)); /* Dummy data */
	broadcast_send(&broadcast);
	PROCESS_PAUSE();
	//leds_on(LEDS_RED);
	}
      /* Barrier so we can see next run */
      NETSTACK_RADIO.off();
      etimer_set(&et, CLOCK_SECOND * TEST_PERIOD);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }

    /*  RX */
    NETSTACK_RADIO.on();
    ps = 0;
    printf("RX radio on RPC=0x%02x\n",  rf230_get_rpc());
    ps = 1;
    etimer_set(&et, CLOCK_SECOND * TEST_PERIOD);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    NETSTACK_RADIO.off();
  }

  /* Last just RX OFF*/
  ps = 0;
  printf("RX radio off\n");
  ps = 1;
  etimer_set(&et, CLOCK_SECOND * TEST_PERIOD);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  PROCESS_END();
}

/* Sleep process is just put MCU in sleep so we minimiza MCU impact */
PROCESS_THREAD(sleep_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    watchdog_periodic();
    if(ps) 
      set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    else
      set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    sleep_enable();
    sei();  
    sleep_cpu();
    sleep_disable();
    sei();
    PROCESS_PAUSE();
  }
  PROCESS_END();
}

