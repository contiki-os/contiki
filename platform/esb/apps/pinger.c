/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: pinger.c,v 1.3 2010/10/19 18:29:05 adamdunkels Exp $
 */

#include "contiki-esb.h"

#include <stdio.h>

PROCESS(pinger, "Pinger");

static struct uip_udp_conn *conn;

struct data {
  uint8_t dummy_data[20];
  uint16_t id;
  uint16_t seqno;
  uint8_t pingpong;
#define PING 0
#define PONG 1
};

static unsigned char pingeron;
static struct etimer etimer;

static unsigned short sent_seqno, last_seqno;

#define PORT 9145

static int place_id = 0, packet_count = 0;


/*---------------------------------------------------------------------------*/
static void
quit(void)
{
  process_exit(&pinger);
  LOADER_UNLOAD();
}
/*---------------------------------------------------------------------------*/
static void
udp_appcall(void *arg)
{
  struct data *d;
  /*  char buf[50];*/
  d = (struct data *)uip_appdata;
  
  if(uip_newdata()) {
    leds_toggle(LEDS_YELLOW);
    /*    beep();*/

    /*    if(uip_htons(d->seqno) != last_seqno + 1) {
      leds_toggle(LEDS_RED);
      beep_quick(2);
      }*/    
    /*    last_seqno = uip_htons(d->seqno);*/
    /*    uip_udp_send(sizeof(struct data));*/
    /*    snprintf(buf, sizeof(buf), "Packet received id %d signal %u\n",
	     d->id, tr1001_sstrength());

	     rs232_print(buf);*/
    /*    if(d->pingpong == PING) {
      d->pingpong = PONG;
    } else {
      d->pingpong = PING;
      d->seqno = uip_htons(uip_htons(d->seqno) + 1);
      }*/
    /*    uip_udp_send(sizeof(struct data));
	  timer_restart(&timer);*/
  } else if(uip_poll()) {
    if(pingeron && etimer_expired(&etimer) && packet_count > 0) {
      --packet_count;
      d->id = place_id;
      d->pingpong = PING;
      d->seqno = uip_htons(sent_seqno);
      ++sent_seqno;
      uip_udp_send(sizeof(struct data));
      etimer_reset(&etimer);
      leds_toggle(LEDS_GREEN);
    }

    if(packet_count == 0) {
      pingeron = 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(config_thread(struct pt *pt, process_event_t ev, process_data_t data))
{
  static struct etimer pushtimer;
  static int counter;
  
  PT_BEGIN(pt);

  
  while(1) {
    
    PT_WAIT_UNTIL(pt, ev == sensors_event && data == &button_sensor);

    beep();
    
    leds_on(LEDS_YELLOW);

    etimer_set(&pushtimer, CLOCK_SECOND);
    for(counter = 0; !etimer_expired(&pushtimer); ++counter) {
      etimer_restart(&pushtimer);
      PT_YIELD_UNTIL(pt, (ev == sensors_event && data == &button_sensor) ||
		     etimer_expired(&pushtimer));
    }

    place_id = counter;

    beep_quick(place_id);

    pingeron = 1;

    packet_count = 20;

    etimer_set(&etimer, CLOCK_SECOND / 2);

    leds_off(LEDS_YELLOW);

    leds_on(LEDS_RED);
    PT_WAIT_UNTIL(pt, packet_count == 0);
    
    pingeron = 0;
    leds_off(LEDS_RED);    
  }
  
  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pinger, ev, data)
{
  static struct pt config_pt;
  
  PROCESS_BEGIN();

  pingeron = 0;
  
  conn = udp_broadcast_new(UIP_HTONS(PORT), NULL);
  
  PT_INIT(&config_pt);

  button_sensor.configure(SENSORS_ACTIVE, 1);
    
  
  while(1) {

    config_thread(&config_pt, ev, data);    
        
    PROCESS_WAIT_EVENT();

    printf("Event %d\n", ev);
    
    beep();
    
    if(ev == tcpip_event) {
      udp_appcall(data);
    }
    
    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&etimer)) {
      tcpip_poll_udp(conn);
    }
   
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
