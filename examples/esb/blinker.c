/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2006-08-28
 * Updated : $Date: 2010/01/14 18:18:51 $
 *           $Revision: 1.3 $
 */

#include "contiki-esb.h"

PROCESS(blinker_process, "Blinker");

AUTOSTART_PROCESSES(&blinker_process);

PROCESS_THREAD(blinker_process, ev, data)
{
  static struct etimer etimer, pir_timer, vib_timer;
  static int on = 0;
  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  etimer_stop(&pir_timer);
  etimer_stop(&vib_timer);
  etimer_set(&etimer, CLOCK_SECOND * 4);
  button_sensor.configure(SENSORS_ACTIVE, 1);
  vib_sensor.configure(SENSORS_ACTIVE, 1);
  pir_sensor.configure(SENSORS_ACTIVE, 1);

  while(1) {

    PROCESS_WAIT_EVENT();

    if(ev == sensors_event) {
      if(data == &vib_sensor) {
	if(etimer_expired(&vib_timer)) {
	  leds_on(LEDS_RED);
	  etimer_set(&vib_timer, CLOCK_SECOND);
	} else {
	  leds_off(LEDS_RED);
	  etimer_stop(&vib_timer);
	}
	beep();

      } else if(data == &pir_sensor) {
	leds_on(LEDS_YELLOW);
	etimer_set(&pir_timer, CLOCK_SECOND);

      } else if(data == &button_sensor) {
	beep_beep(100);
      }

    } else if(ev == PROCESS_EVENT_TIMER) {
      if(data == &etimer) {
	if(on) {
	  etimer_set(&etimer, CLOCK_SECOND * 4);
	  leds_off(LEDS_GREEN);
	} else {
	  etimer_set(&etimer, CLOCK_SECOND / 2);
	  leds_on(LEDS_GREEN);
	}
	on = !on;
      } else if(data == &pir_timer) {
	leds_off(LEDS_YELLOW);
      } else if(data == &vib_timer) {
	leds_off(LEDS_RED);
      }
    }
  }

 exit:
  leds_off(LEDS_ALL);
  PROCESS_END();
}
