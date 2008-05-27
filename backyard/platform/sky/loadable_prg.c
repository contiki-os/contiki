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
 * @(#)$Id: loadable_prg.c,v 1.1 2008/05/27 13:16:34 adamdunkels Exp $
 */

#include <stdio.h>

#include "contiki.h"

#include "dev/leds.h"

PROCESS(test_process, "Test process");

PROCESS_THREAD(test_process, ev, data)
{
  static struct etimer etimer;

  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("test_process starting\n");

  while(1) {
    leds_on(LEDS_RED);
    etimer_set(&etimer, CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
    leds_off(LEDS_RED);
    etimer_set(&etimer, CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&etimer));
  }

 exit:
  printf("test_process exiting\n");
  leds_off(LEDS_RED);
  PROCESS_END();
}

/*
 * Initialize this kernel module.
 */
void
_init(void)
{
  process_start(&test_process, NULL);
}

/*
 * When this module is unloaded we must clean up!
 */
void
_fini(void)
{
  process_exit(&test_process);
}
