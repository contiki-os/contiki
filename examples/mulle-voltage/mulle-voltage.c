/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         An example for using the ADC to read battery voltage.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#define EXAMPLE_POLL_FREQUENCY 8

#include "contiki.h"
#include "K60.h"
#include "voltage.h"
#include "power-control.h"

#include <stdio.h>              /* For printf() */
#include <stdint.h>             /* int16_t et al. */

/*---------------------------------------------------------------------------*/
PROCESS(mulle_voltage_process, "Mulle voltage process");
AUTOSTART_PROCESSES(&mulle_voltage_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mulle_voltage_process, ev, data)
{
  static struct etimer et;
  uint16_t Vbat;
  uint16_t Vchr;

  PROCESS_BEGIN();

  printf("\nMulle Vbat, Vchr measurement\n");

  /* initialize the ADC hardware */
  power_control_avdd_set(1);
  printf("Initialize voltage monitoring\n");
  voltage_init();

  /* Poll at 8Hz */
  etimer_set(&et, CLOCK_SECOND / EXAMPLE_POLL_FREQUENCY);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Reset the etimer to trig again */
    etimer_reset(&et);
    Vbat = voltage_read_vbat();
    Vchr = voltage_read_vchr();

    printf("Vbat: %u\tVchr: %u\n", Vbat, Vchr);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
