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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * \addtogroup zoul-examples
 * @{
 * \defgroup zoul-aac-sensor-test Test AAC sensor
 *
 * Demonstrates the operation of the current AAC analog sensor
 * @{
 *
 * \file
 *  Example demonstrating the Zoul module on the RE-Mote & AAC sensor 0-5V 50Amps AC
 *
 * \author
 *         Javier Sánchez <asanchez@zolertia.com>
 */
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "dev/adc-sensors.h"
#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define ADC_PIN             2
#define LOOP_PERIOD         2
#define LOOP_INTERVAL       (CLOCK_SECOND * LOOP_PERIOD)
#define LEDS_PERIODIC       LEDS_GREEN
/*---------------------------------------------------------------------------*/
static struct etimer et;

static uint16_t counter;
/*---------------------------------------------------------------------------*/
PROCESS(test_aac_sensor_process, "Test AAC sensor process");
AUTOSTART_PROCESSES(&test_aac_sensor_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_aac_sensor_process, ev, data)
{
  
  PROCESS_BEGIN();

  counter = 0;

  /* Configure the ADC ports */
  /* Use pin number not mask, for example if using the PA5 pin then use 5 */
  adc_sensors.configure(ANALOG_AAC_SENSOR, ADC_PIN);

  printf("AAC test application\n");
  leds_on(LEDS_PERIODIC);
  etimer_set(&et, LOOP_INTERVAL);

  while(1) {

    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      leds_toggle(LEDS_PERIODIC);

      printf("-----------------------------------------\n"
             "Counter = 0x%08x\n", counter);
      
      printf("AC Amps = %d mA\n", adc_sensors.value(ANALOG_AAC_SENSOR));
     
      etimer_set(&et, LOOP_INTERVAL);
      counter++;
    }   
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
