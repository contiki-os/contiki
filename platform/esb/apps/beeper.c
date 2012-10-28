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
 */
#include "contiki-esb.h"

PROCESS(beeper_process, "Beeper");

AUTOSTART_PROCESSES(&beeper_process);

static struct etimer etimer;

static struct pt beeper_pt;

static
PT_THREAD(beeper_thread(struct pt *pt))
{
  PT_BEGIN(pt);

  while(1) {
    PT_WAIT_UNTIL(pt, etimer_expired(&etimer));
    etimer_reset(&etimer);
    leds_invert(LEDS_RED);
    beep();
    
    PT_WAIT_UNTIL(pt, etimer_expired(&etimer));
    etimer_reset(&etimer);
    leds_invert(LEDS_RED);
  }
  
  PT_END(pt);
}

PROCESS_THREAD(beeper_process, ev, data)
{
  PROCESS_BEGIN();
  
  etimer_set(&etimer, CLOCK_SECOND / 2);
  PT_INIT(&beeper_pt);
  
  button_sensor.configure(SENSORS_ACTIVE, 1);
  
  while(1) {

    beeper_thread(&beeper_pt);
    
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_EXIT) {
      break;
    } else if(ev == sensors_event &&
	      data == &button_sensor) {
      leds_invert(LEDS_YELLOW);
    }
    
  }
  PROCESS_END();
}
