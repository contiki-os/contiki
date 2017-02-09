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
 * \defgroup zoul-rotation-sensor-test Phidget 1109 rotation sensor example
 *
 * Demonstrates the operation of the analog phidget 1109 rotation sensor
 * @{
 *
 * \file
 *         Phidget analog rotation sensor example using the ADC sensor wrapper
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
#define SENSOR_READ_INTERVAL (CLOCK_SECOND / 4)
/*---------------------------------------------------------------------------*/
PROCESS(remote_rotation_process, "Phidget rotation test process");
AUTOSTART_PROCESSES(&remote_rotation_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_rotation_process, ev, data)
{
  PROCESS_BEGIN();

  uint16_t rotation;

  /* Use pin number not mask, for example if using the PA5 pin then use 5 */
  adc_sensors.configure(ANALOG_PHIDGET_ROTATION_1109, 5);

  /* And periodically poll the sensor */

  while(1) {
    etimer_set(&et, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    rotation = adc_sensors.value(ANALOG_PHIDGET_ROTATION_1109);

    if(rotation != ADC_WRAPPER_ERROR) {
      printf("Rotation = %u\n", rotation);
    } else {
      printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
      PROCESS_EXIT();
    }

    if(rotation <= 45) {
      leds_off(LEDS_ALL);
    }

    if((rotation > 45) && (rotation < 150)) {
      leds_on(LEDS_GREEN);
      leds_off(LEDS_BLUE | LEDS_RED);
    }

    if((rotation > 150) && (rotation < 250)) {
      leds_on(LEDS_RED | LEDS_GREEN);
      leds_off(LEDS_BLUE);
    }

    if(rotation >= 250) {
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

