/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "contiki-esb.h"

PROCESS(fader_process, "LED fader");
AUTOSTART_PROCESSES(&fader_process);

#define ON  1
#define OFF 0

static unsigned char onoroff;
static struct etimer etimer;
static struct pt fade_pt, fade_in_pt, fade_out_pt;
/*---------------------------------------------------------------------------*/
static
PT_THREAD(fade_in(struct pt *pt))
{
  static int delay;
  
  PT_BEGIN(pt);
  
  for(delay = 3980; delay > 20; delay -= 20) {
    leds_on(LEDS_ALL);
    clock_delay(4000 - delay);
    leds_off(LEDS_ALL);
    clock_delay(delay);
    PT_YIELD(pt);
  }
  
  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(fade_out(struct pt *pt))
{
  static int delay;
  
  PT_BEGIN(pt);

  for(delay = 20; delay < 3980; delay += 20) {
    leds_on(LEDS_ALL);
    clock_delay(4000 - delay);
    leds_off(LEDS_ALL);
    clock_delay(delay);
    PT_YIELD(pt);
  }
    
  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(fade(struct pt *pt))
{
  PT_BEGIN(pt);

  PT_SPAWN(pt, &fade_in_pt, fade_in(&fade_in_pt));
  PT_SPAWN(pt, &fade_out_pt, fade_out(&fade_out_pt));

  etimer_set(&etimer, CLOCK_SECOND * 10);
  PT_WAIT_UNTIL(pt, etimer_expired(&etimer));
  
  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(fader_process, ev, data)
{
  PROCESS_BEGIN();
  
  PT_INIT(&fade_pt);
  PT_INIT(&fade_in_pt);
  PT_INIT(&fade_out_pt);
  onoroff = ON;
  etimer_set(&etimer, CLOCK_SECOND * 32);

  while(1) {
    PROCESS_WAIT_EVENT();
  
    if(ev == PROCESS_EVENT_TIMER) {
      etimer_reset(&etimer);
      PT_INIT(&fade_pt);
      process_poll(&fader_process);
    }
    
    if(onoroff == ON &&
       PT_SCHEDULE(fade(&fade_pt))) {
      process_poll(&fader_process);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
fader_on(void)
{
  onoroff = ON;
  process_poll(&fader_process);
}
/*---------------------------------------------------------------------------*/
void
fader_off(void)
{
  onoroff = OFF;
}
/*---------------------------------------------------------------------------*/
