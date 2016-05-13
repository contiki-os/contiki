/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-examples
 * @{
 * \defgroup zoul-grove-relay-test Test Grove Relay
 *
 * Demonstrates the operation of the Grove Relay
 * @{
 *
 * \file
 *  HLS8-T73 GROVE_RELAY example    
 *
 * \author
 *         Toni Lozano <tlozano@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "cpu.h"
#include "contiki.h"
#include "dev/leds.h"
#include "dev/sys-ctrl.h"
#include "dev/grove-relay.h"
/*---------------------------------------------------------------------------*/
#define RELAY_ACTION_INTERVAL (CLOCK_SECOND*5)
#define RELAY_ON_INTERVAL (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
PROCESS(test_grove_relay_process, "Test Grove Relay process");
AUTOSTART_PROCESSES(&test_grove_relay_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_grove_relay_process, ev, data)
{
  PROCESS_BEGIN();

  grove_relay_configure();

  /* And periodically activate the relay */

  while(1) {
    etimer_set(&et, RELAY_ACTION_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    if((grove_relay_set(GROVE_RELAY_ON)) == GROVE_RELAY_ERROR) {
      printf("Grove Relay ERROR set\n"); 
    }
    leds_on(LEDS_GREEN);
    
    etimer_set(&et, RELAY_ON_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    
    if((grove_relay_set(GROVE_RELAY_OFF)) == GROVE_RELAY_ERROR) {
      printf("Grove Relay ERROR set\n"); 
    }    
    leds_off(LEDS_GREEN);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
