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
 *
 * \defgroup zoul-grove-loudness-sensor-test Grove's loudness sensor
 *
 * Demonstrates the operation of the Grove's analog loudness sensor
 * @{
 *
 * \file
 *  Grove's loudness sensor example using the ADC sensors wrapper
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "dev/adc-sensors.h"
/*---------------------------------------------------------------------------*/
#define ADC_PIN              5
#define SENSOR_READ_INTERVAL (CLOCK_SECOND / 8)
/*---------------------------------------------------------------------------*/
PROCESS(remote_grove_loudness_process, "Grove loudness test process");
AUTOSTART_PROCESSES(&remote_grove_loudness_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_grove_loudness_process, ev, data)
{
  PROCESS_BEGIN();

  uint16_t loudness;

  /* Use pin number not mask, for example if using the PA5 pin then use 5 */
  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN);

  /* And periodically poll the sensor */

  while(1) {
    etimer_set(&et, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);

    if(loudness != ADC_WRAPPER_ERROR) {
      printf("%u\n", loudness);
    } else {
      printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
      PROCESS_EXIT();
    }

    if(loudness < 100) {
      leds_off(LEDS_ALL);
    }

    if((loudness >= 100) && (loudness < 500)) {
      leds_on(LEDS_BLUE);
      leds_off(LEDS_GREEN | LEDS_RED);
    }

    if((loudness >= 500) && (loudness < 1000)) {
      leds_on(LEDS_GREEN);
      leds_off(LEDS_BLUE | LEDS_RED);
    }

    if(loudness >= 1000) {
      leds_on(LEDS_RED);
      leds_off(LEDS_BLUE | LEDS_GREEN);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

