/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell.h"

#include "dev/leds.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_blink_process, "blink");
SHELL_COMMAND(blink_command,
	      "blink",
	      "blink [num]: blink LEDs ([num] times)",
	      &shell_blink_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_blink_process, ev, data)
{
  static struct etimer etimer;
  static int i, num, on = 0;
  const char *nextptr;

  PROCESS_EXITHANDLER(if(on) { leds_toggle(LEDS_ALL); } );

  PROCESS_BEGIN();

  if(data != NULL) {
    num = shell_strtolong(data, &nextptr);
    if(nextptr != data) {
      etimer_set(&etimer, CLOCK_SECOND / 2);
      for(i = 0; i < num; ++i) {
	leds_toggle(LEDS_ALL);
	on = 1;
	PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
	etimer_reset(&etimer);
	
	leds_toggle(LEDS_ALL);
	on = 0;
	PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
	etimer_reset(&etimer);
      }
      PROCESS_EXIT();
    }
  }

  while(1) {
    struct shell_input *input;
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_TIMER && data == &etimer) {
      if(on) {
	leds_toggle(LEDS_ALL);
	on = 0;
      }
    } else if(ev == shell_event_input) {
      input = data;
      if(input->len1 + input->len2 == 0) {
	if(on) {
	  leds_toggle(LEDS_ALL);
	  on = 0;
	}
	PROCESS_EXIT();
      }
      shell_output(&blink_command,
		   input->data1, input->len1,
		   input->data2, input->len2);

      if(on == 0) {
	leds_toggle(LEDS_ALL);
	on = 1;
      }
      etimer_set(&etimer, CLOCK_SECOND / 16);
    }
  }
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_blink_init(void)
{
  shell_register_command(&blink_command);
}
/*---------------------------------------------------------------------------*/
