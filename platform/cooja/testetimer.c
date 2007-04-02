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
 * $Id: testetimer.c,v 1.2 2007/04/02 16:31:28 fros4943 Exp $
 */


#include <stdio.h>
#include "contiki.h"
#include "sys/loader.h"

#include "lib/list.h"
#include "lib/random.h"

#include "net/uip.h"

#include "sys/etimer.h"
#include "sys/clock.h"

#include "lib/sensors.h"
#include "sys/log.h"


PROCESS(etimer_test_process, "ETimer test process");

PROCESS_THREAD(etimer_test_process, ev, data)
{
  static struct etimer mytimer;

  static int custom_counter = 0;
  static char logMess[100];

  PROCESS_BEGIN();

  etimer_set(&mytimer, 1111);

  sprintf(logMess, "Starting event timer test process (counter=%i)\n", custom_counter);
  log_message(logMess, "");

  while(1) {
    PROCESS_WAIT_EVENT();

    if (etimer_expired(&mytimer)) {
      custom_counter++;
      sprintf(logMess, "Timed out (counter=%i)\n", custom_counter);
      log_message(logMess, "");

      etimer_restart(&mytimer);
    }
  }

  PROCESS_END();
}
