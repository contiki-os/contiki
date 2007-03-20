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
 * $Id: test-trickle.c,v 1.1 2007/03/20 12:21:19 adamdunkels Exp $
 */

/**
 * \file
 *         Testing the trickle code in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/trickle.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(test_trickle_process, "TRICKLE test");
AUTOSTART_PROCESSES(&test_trickle_process);
/*---------------------------------------------------------------------------*/
static void
trickle_recv(struct trickle_conn *c)
{
  printf("trickle message received '%s'\n", (char *)rimebuf_dataptr());
}
const static struct trickle_callbacks trickle_call = {trickle_recv};
static struct trickle_conn trickle;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_trickle_process, ev, data)
{
  PROCESS_BEGIN();

  trickle_open(&trickle, 128, &trickle_call);
  button_sensor.activate();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			     data == &button_sensor);

    rimebuf_copyfrom("Hej", 4);
    trickle_send(&trickle);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
