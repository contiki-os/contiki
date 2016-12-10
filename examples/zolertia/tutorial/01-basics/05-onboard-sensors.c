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
 * \file
 *         An example showing how to use the on-board sensors
 * \author
 *         Antonio Lignan <alinan@zolertia.com> <antonio.lignan@gmail.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include <stdio.h>
#include "dev/leds.h"
#include "sys/etimer.h"

/* These library allows to use the on-board sensors */
#if CONTIKI_TARGET_ZOUL
#include "dev/adc-zoul.h"
#include "dev/zoul-sensors.h"
#else /* Assumes the Z1 mote */
#include "dev/tmp102.h"
#include "dev/adxl345.h"
#include "dev/battery-sensor.h"
#endif
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(test_onboard_sensors_process, "Test on-board sensors");
AUTOSTART_PROCESSES(&test_onboard_sensors_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_onboard_sensors_process, ev, data)
{
  PROCESS_BEGIN();

  static uint16_t temp;
  static uint32_t batt;

#if CONTIKI_TARGET_ZOUL
  /*  The sensors are already started at boot */
#else
  static int8_t x_axis;
  static int8_t y_axis;
  static int8_t z_axis;

  /* Initialize the sensors, the SENSORS_ACTIVATE(...) macro invokes the
   * configure(...) method of Contiki's sensor API
   */
  SENSORS_ACTIVATE(adxl345);
  SENSORS_ACTIVATE(tmp102);
  SENSORS_ACTIVATE(battery_sensor);
#endif

  /* Spin the timer */
  etimer_set(&et, CLOCK_SECOND);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* Read the sensors */
#if CONTIKI_TARGET_ZOUL
    batt = vdd3_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED);
    printf("VDD = %u mV\n", (uint16_t)batt);

    temp = cc2538_temp_sensor.value(CC2538_SENSORS_VALUE_TYPE_CONVERTED);
    printf("Core temperature = %d.%u C\n", temp / 1000, temp % 1000);

#else /* Assumes Z1 mote */
    x_axis = adxl345.value(X_AXIS);
    y_axis = adxl345.value(Y_AXIS);
    z_axis = adxl345.value(Z_AXIS);
    temp   = tmp102.value(TMP102_READ);
    batt   = battery_sensor.value(1);

    /* Print the readings */
    printf("Acceleration: X %d Y %d Z %d\n", x_axis, y_axis, z_axis);
    printf("Temperature: %d.%u C\n", temp / 100, temp % 100);

    /* Convert the ADC readings to mV */
    batt *= 5000;
    batt /= 4095;
    printf("Battery: %u\n\n", (uint16_t)batt);
#endif

    etimer_reset(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
