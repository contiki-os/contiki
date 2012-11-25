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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki toggle the LEDs application
 * \author
 *         Who Knows who@kth.se
 */

#include "contiki.h"
#include <stdio.h> /* For printf() */
#include "dev/leds.h"

volatile uint32_t temp;
void
_delay(uint32_t del)
{
  uint32_t i;
  for (i = 0; i < del; i++)
    temp = i;
}
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
//AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN()
  ;
  static struct etimer et; // Define the timer

  leds_init();
  etimer_set(&et, CLOCK_SECOND/2);

  while (1)
    {
      PROCESS_WAIT_EVENT();
      if(etimer_expired(&et)){
          printf("Toggling LED2\n");
          leds_toggle(LEDS_GREEN);
          etimer_reset(&et);
      }
    }

PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process2, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process, &hello_world_process2);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process2, ev, data)
{
  PROCESS_BEGIN()
  ;
  static struct etimer et; // Define the timer

  leds_init();
  etimer_set(&et, CLOCK_SECOND);

  while (1)
    {
      PROCESS_WAIT_EVENT();
      if(etimer_expired(&et)){
          printf("Toggling LED3\n");
          leds_toggle(LEDS_YELLOW);
          etimer_reset(&et);
      }
    }

PROCESS_END();
}
/*---------------------------------------------------------------------------*/

