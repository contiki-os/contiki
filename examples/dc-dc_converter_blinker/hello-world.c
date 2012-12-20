/*
 * Copyright (c) 2012, KTH, Royal Institute of Technology(Stockholm, Sweden)
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 * This is work by the CSD master project. Fall 2012. Microgrid team.
 * Author: Saulius Alisauskas   <saulius@kth.se>
 *         Javier Lara Peinado  <javierlp@kth.se>
 *
 */

#include "contiki.h"
#include "dev/leds.h"

/* Process for blinking GREEN led */
PROCESS(green_led_blink_process, "Process for blinking GREEN led");
PROCESS_THREAD(green_led_blink_process, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND/2);

  while (1)
    {
      PROCESS_WAIT_EVENT();
      if(etimer_expired(&et)){
          leds_toggle(LEDS_GREEN);
          etimer_reset(&et);
      }
    }

PROCESS_END();
}

/* Process for blinking RED led */
PROCESS(red_led_blink_process, "Process for blinking RED led");
PROCESS_THREAD(red_led_blink_process, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;

  leds_init();
  etimer_set(&et, CLOCK_SECOND);

  while (1)
    {
      PROCESS_WAIT_EVENT();
      if(etimer_expired(&et)){
          leds_toggle(LEDS_RED);
          etimer_reset(&et);
      }
    }

PROCESS_END();
}

AUTOSTART_PROCESSES(&red_led_blink_process, &green_led_blink_process);
