/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: energest-demo.c,v 1.3 2008/01/22 14:58:41 zhitao Exp $
 */

/**
 * \file
 *         Contiki application sending estimated energy to a sink node
 * \author
 *         Zhitao He <zhitao@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/mac/mac.h"
#include "net/mac/nullmac.h"
#include "dev/button-sensor.h"

#include "dev/simple-cc2420.h"

#include "dev/leds.h"
#include "node-id.h"
#include <stdio.h>

#define SINK_ID 41
/*---------------------------------------------------------------------------*/
PROCESS(output_process, "output energest");
PROCESS(user_process, "user input");
AUTOSTART_PROCESSES(
		    &output_process,
		    &user_process
		    );
/*---------------------------------------------------------------------------*/
static struct abc_conn abc;
static u16_t read = 0;
static u16_t send = 0;

int lpm_en;
static int radio_off;
static int computing;
static int sending;

enum states{
  RADIO_OFF = 1,
  RADIO_LOW,
  RADIO_MID,
  RADIO_FULL,
  LPM_OFF,
  SENDING1k,
  SENDING12k,
};

struct energy_time {
  unsigned short source;
  long cpu;
  long lpm;
  long transmit;
  long listen;
};

static struct energy_time last;
static struct energy_time diff;

// FIXME: workaround to turn on/off radio. Rime should export an MAC on/off interface to avoid forcing the user to do this explicitly
static const struct mac_driver *mac = &nullmac_driver;

static clock_time_t sleep_cycles;
static const char send_string[] = "I am a sending string.";
static int send_length = 10;
static int send_amount = 10;
/*---------------------------------------------------------------------------*/
static void
abc_recv(struct abc_conn *c)
{
  struct energy_time *incoming= (struct energy_time *)rimebuf_dataptr();
  read++;
  if(node_id == SINK_ID) {
    printf("%i SICS %i %u %li %li %li %li\n", node_id, read,
	   incoming->source, incoming->cpu, incoming->lpm,
	   incoming->transmit, incoming->listen);
  }
}
/*---------------------------------------------------------------------------*/
const static struct abc_callbacks abc_call = {abc_recv};
/*---------------------------------------------------------------------------*/
static void
do_computing(void)
{
  int i;
  for(i = 0; i < 100; i++) {
    clock_delay(1000);
  }
}
/*---------------------------------------------------------------------------*/
static void
do_sending(void)
{
  int i;
  simple_cc2420_set_chan_pan_addr(11, 0x2024, node_id, NULL);
  simple_cc2420_set_txpower(1);

  for(i = 0;i < send_amount; i++) {
    rimebuf_copyfrom(send_string, send_length);

    mac->on();
    abc_send(&abc);
    mac->off();
  }
  simple_cc2420_set_txpower(31);
  simple_cc2420_set_chan_pan_addr(RF_CHANNEL, 0x2024, node_id, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(output_process, ev, data)
{
  static struct etimer et, et2;
  
  PROCESS_EXITHANDLER(abc_close(&abc);)
    
  PROCESS_BEGIN();
  
  etimer_set(&et, 5*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  /* Energy time init */
  last.cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last.lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last.listen = energest_type_time(ENERGEST_TYPE_LISTEN);

  abc_open(&abc, 128, &abc_call);

  if (node_id == SINK_ID) {
    while(1) {
      PROCESS_YIELD();
      printf("I'm a sink. I'm doing nothing...");
    }
  }

  etimer_set(&et, UPDATE_TICKS);
  
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    
    if(computing) {
      do_computing();
    }

    /* stop-start ongoing time measurements to retrieve the diffs
       during last interval */
    ENERGEST_OFF(ENERGEST_TYPE_CPU);
    ENERGEST_ON(ENERGEST_TYPE_CPU);
    mac->on();
    mac->off();

    /* Energy time diff */
    diff.source = node_id;
    diff.cpu = energest_type_time(ENERGEST_TYPE_CPU) - last.cpu;
    diff.lpm = energest_type_time(ENERGEST_TYPE_LPM) - last.lpm;
    diff.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) - last.transmit;
    diff.listen = energest_type_time(ENERGEST_TYPE_LISTEN) - last.listen;
    last.cpu = energest_type_time(ENERGEST_TYPE_CPU);
    last.lpm = energest_type_time(ENERGEST_TYPE_LPM);
    last.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    last.listen = energest_type_time(ENERGEST_TYPE_LISTEN);

    send++;
/*     printf("%i SICS %i %i %li %li %li %li\n", node_id, send, */
/* 	   diff.source, diff.cpu, diff.lpm, diff.transmit, diff.listen); */

    rimebuf_copyfrom((char*)&diff, sizeof(diff));

    mac->on();
    abc_send(&abc);

    if(sending) {
      mac->off();
      do_sending();
    } else if(radio_off) {
      mac->off();
    } else {
      mac->off();
      etimer_set(&et2, sleep_cycles);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
      mac->on();
    }

    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static enum states
next_state(enum states current_state)
{
  return current_state == 7 ? current_state = 1 : current_state + 1;
}
/*---------------------------------------------------------------------------*/
static void
show_state(enum states current_state)
{
  printf("state = %d\n", current_state);
  switch(current_state) {
  case 1:
    leds_off(LEDS_BLUE);leds_off(LEDS_GREEN);leds_on(LEDS_RED);
    break;

  case 2:
    leds_off(LEDS_BLUE);leds_on(LEDS_GREEN);leds_off(LEDS_RED);
    break;

  case 3:
    leds_off(LEDS_BLUE);leds_on(LEDS_GREEN);leds_on(LEDS_RED);
    break;

  case 4:
    leds_on(LEDS_BLUE);leds_off(LEDS_GREEN);leds_off(LEDS_RED);
    break;

  case 5:
    leds_on(LEDS_BLUE);leds_off(LEDS_GREEN);leds_on(LEDS_RED);
    break;

  case 6:
    leds_on(LEDS_BLUE);leds_on(LEDS_GREEN);leds_off(LEDS_RED);
    break;

  case 7:
    leds_on(LEDS_BLUE);leds_on(LEDS_GREEN);leds_on(LEDS_RED);
    break;

  default:
    printf("unknown state\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
run_state(enum states current_state)
{
  switch(current_state) {
  case RADIO_OFF:
    lpm_en = 1;
    radio_off = 1;
    sleep_cycles = 0;
    computing = 0;
    sending = 0;
    break;

  case RADIO_LOW:
    lpm_en = 1;
    radio_off = 0;
    sleep_cycles = UPDATE_TICKS * 99 / 100;
    computing = 0;
    sending = 0;
    break;

  case RADIO_MID:
    lpm_en = 1;
    radio_off = 0;
    sleep_cycles = UPDATE_TICKS * 90 / 100;
    computing = 0;
    sending = 0;
    break;

  case RADIO_FULL:
    lpm_en = 1;
    radio_off = 0;
    sleep_cycles = 0;
    computing = 0;
    sending = 0;
    break;

  case LPM_OFF:
    lpm_en = 0;
    radio_off = 0;
    sleep_cycles = UPDATE_TICKS * 90 / 100;
    computing = 0;
    sending = 0;
    break;

  case SENDING1k:
    lpm_en = 1;
    radio_off = 0;
    sleep_cycles = UPDATE_TICKS * 99 / 100;
    computing = 0;
    sending = 1;
    send_amount = 10;
    send_length = 100;
    break;

  case SENDING12k:
    lpm_en = 1;
    radio_off = 0;
    sleep_cycles = UPDATE_TICKS * 90 / 100;
    computing = 0;
    sending = 1;
    send_amount = 100;
    send_length = 100;
    break;
    
  default:
    ;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(user_process, ev, data)
{
  static enum states state;

  PROCESS_BEGIN();

  printf("Node id %d\n", node_id);
  
  button_sensor.activate();
  state = RADIO_OFF;
  
  while(1) {
    show_state(state);
    run_state(state);

    PROCESS_WAIT_EVENT();
    if(ev == sensors_event && data == &button_sensor) {
      state = next_state(state);
    }
    
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
