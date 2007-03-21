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
 * $Id: fader.c,v 1.1 2007/03/21 23:17:28 adamdunkels Exp $
 */

#include "contiki.h"
#include "dev/leds.h"

PROCESS(fader_process, "LED fader");
AUTOSTART_PROCESSES(&fader_process);

#define ON  1
#define OFF 0

struct fader {
  struct pt fade_pt, fade_in_pt, fade_out_pt;
  struct etimer etimer;
  int led;
  int delay;
};

static unsigned char onoroff;

/*---------------------------------------------------------------------------*/
static
PT_THREAD(fade_in(struct fader *f))
{
  PT_BEGIN(&f->fade_in_pt);
  
  for(f->delay = 3980; f->delay > 20; f->delay -= 20) {
    leds_on(f->led);
    clock_delay(4000 - f->delay);
    leds_off(f->led);
    clock_delay(f->delay);
    PT_YIELD(&f->fade_in_pt);
  }
  
  PT_END(&f->fade_in_pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(fade_out(struct fader *f))
{  
  PT_BEGIN(&f->fade_out_pt);

  for(f->delay = 20; f->delay < 3980; f->delay += 20) {
    leds_on(f->led);
    clock_delay(4000 - f->delay);
    leds_off(f->led);
    clock_delay(f->delay);
    PT_YIELD(&f->fade_out_pt);
  }
    
  PT_END(&f->fade_out_pt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(fade(struct fader *f))
{
  PT_BEGIN(&f->fade_pt);

  while(1) {
    
    PT_SPAWN(&f->fade_pt, &f->fade_in_pt, fade_in(f));
    PT_SPAWN(&f->fade_pt, &f->fade_out_pt, fade_out(f));
    
    etimer_set(&f->etimer, CLOCK_SECOND * 4);
    PT_WAIT_UNTIL(&f->fade_pt, etimer_expired(&f->etimer));
  }
    
  PT_END(&f->fade_pt);
}
/*---------------------------------------------------------------------------*/
static void
init_fader(struct fader *f, int led)
{
  PT_INIT(&f->fade_pt);
  PT_INIT(&f->fade_in_pt);
  PT_INIT(&f->fade_out_pt);
  f->led = led;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(fader_process, ev, data)
{
  static struct fader red, green, yellow;
  static struct timer timer;
  static struct etimer etimer;
  
  PROCESS_BEGIN();

  init_fader(&red, LEDS_RED);
  init_fader(&green, LEDS_GREEN);
  init_fader(&yellow, LEDS_YELLOW);
  
  timer_set(&timer, CLOCK_SECOND/4);
  while(!timer_expired(&timer)) {
    PT_SCHEDULE(fade(&red));
  }
  timer_set(&timer, CLOCK_SECOND/4);
  while(!timer_expired(&timer)) {
    PT_SCHEDULE(fade(&red));
    PT_SCHEDULE(fade(&green));
  }
  
  timer_set(&timer, CLOCK_SECOND/4);
  while(!timer_expired(&timer)) {
    PT_SCHEDULE(fade(&green));
    PT_SCHEDULE(fade(&yellow));
  }

  etimer_set(&etimer, CLOCK_SECOND * 4);
  fader_on();
  
  while(1) {
    PROCESS_WAIT_EVENT();
  
    if(ev == PROCESS_EVENT_TIMER) {
      etimer_set(&etimer, CLOCK_SECOND * 4);
      process_poll(&fader_process);
    }
    
    if(onoroff == ON &&
       PT_SCHEDULE(fade(&red)) &&
       PT_SCHEDULE(fade(&yellow)) &&
       PT_SCHEDULE(fade(&green))) {
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
