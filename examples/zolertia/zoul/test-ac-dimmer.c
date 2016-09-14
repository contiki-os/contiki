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
 * \defgroup zoul-ac-dimmer-test Krida Electronics AC light dimmer test example
 *
 * Demonstrates the use of an AC dimmer with zero-crossing, connected to the
 * ADC1 and ADC2 pins (PA5 and PA4 respectively), powered over the D+5.1 pin
 *
 * @{
 *
 * \file
 *         A quick program to test an AC dimmer
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/ac-dimmer.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
PROCESS(remote_ac_dimmer_process, "AC light dimmer test");
AUTOSTART_PROCESSES(&remote_ac_dimmer_process);
/*---------------------------------------------------------------------------*/
static uint8_t dimming;
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_ac_dimmer_process, ev, data)
{
  PROCESS_BEGIN();

  dimming = 0;
  SENSORS_ACTIVATE(ac_dimmer);

  printf("AC dimmer: min %u%% max %u%%\n", DIMMER_DEFAULT_MIN_DIMM_VALUE,
                                           DIMMER_DEFAULT_MAX_DIMM_VALUE);

  /* Set the lamp to 10% and wait a few seconds */
  ac_dimmer.value(DIMMER_DEFAULT_MIN_DIMM_VALUE);
  etimer_set(&et, CLOCK_SECOND * 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  /* Upon testing for duty cycles lower than 10% there was noise (probably from
   * the triac), causing the driver to skip a beat, and from time to time made
   * the test lamp blink.  This is easily reproducible by setting the dimmer to
   * 5% and using a logic analyzer on the SYNC and GATE pins.  The noise was
   * picked-up also by the non-connected test probes of the logic analyser.
   * Nevertheless the difference between 10% and 2% bright-wise is almost
   * negligible
   */
  while(1) {

    dimming += DIMMER_DEFAULT_MIN_DIMM_VALUE;
    if(dimming > DIMMER_DEFAULT_MAX_DIMM_VALUE) {
      dimming = DIMMER_DEFAULT_MIN_DIMM_VALUE;
    }

    ac_dimmer.value(dimming);
    printf("AC dimmer: light is now --> %u\n", ac_dimmer.status(SENSORS_ACTIVE));

    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

