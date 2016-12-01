/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-relay-test A simple program to test a generic relay
 *
 * Demonstrates the use of a generic relay, connected by default at the ADC1
 * connector of the RE-Mote
 *
 * @{
 *
 * \file
 *         A quick program to test a generic relay
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/relay.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
PROCESS(remote_relay_process, "Generic relay test");
AUTOSTART_PROCESSES(&remote_relay_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_relay_process, ev, data)
{
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(relay);

  /* Activate the relay and wait for 5 seconds */
  relay.value(RELAY_ON);
  etimer_set(&et, CLOCK_SECOND * 5);
  printf("\nRelay: switch should be ON --> %u\n", relay.status(SENSORS_ACTIVE));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  /* Now turn off and wait 5 seconds more */
  relay.value(RELAY_OFF);
  etimer_set(&et, CLOCK_SECOND * 5);
  printf("Relay: switch should be OFF --> %u\n\n", relay.status(SENSORS_ACTIVE));
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  /* Let it spin and toggle each second */
  while(1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    relay.value(RELAY_TOGGLE);
    printf("Relay: switch is now --> %u\n", relay.status(SENSORS_ACTIVE));
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

