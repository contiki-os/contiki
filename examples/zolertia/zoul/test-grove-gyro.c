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
 * \defgroup zoul-grove-gyro-test Grove's 3-axis gyroscope test application
 *
 * Demonstrates the use of the Grove's 3-axis gyroscope based on the ITG-3200
 * @{
 *
 * \file
 *  Test file for the external Grove gyroscope
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include "dev/grove-gyro.h"
/*---------------------------------------------------------------------------*/
#define SENSOR_READ_INTERVAL       (CLOCK_SECOND)
#define GROVE_GYRO_EXPECTED_ADDR   GROVE_GYRO_ADDR
/*---------------------------------------------------------------------------*/
PROCESS(remote_grove_gyro_process, "Grove gyro test process");
AUTOSTART_PROCESSES(&remote_grove_gyro_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
static void
gyro_interrupt_callback(uint8_t status)
{
  /* The interrupt indicates that new data is available, the status value
   * returns the outcome of the read operation, check to validate if the
   * data is valid to read
   */
  leds_toggle(LEDS_PURPLE);

  printf("Gyro: X_axis %u, Y_axis %u, Z_axis %u\n", gyro_values.x,
                                                    gyro_values.y,
                                                    gyro_values.z);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_grove_gyro_process, ev, data)
{
  PROCESS_BEGIN();

  uint8_t aux;

  /* Use Contiki's sensor macro to enable the sensor */
  SENSORS_ACTIVATE(grove_gyro);

  /* The sensor itself is in low-power mode, to power on just the sensor and not
   * the 3 gyroscope axis use GROVE_GYRO_SENSOR.  Alternatively the value
   * GROVE_GYRO_ALL could also be used to power everything at once
   */
  grove_gyro.configure(GROVE_GYRO_POWER_ON, GROVE_GYRO_SENSOR);  

  /* Read back the configured sensor I2C address to check if the sensor is
   * working OK, this is the only case in which the value() returns a value
   */
  aux = grove_gyro.value(GROVE_GYRO_ADDR);
  if(aux == GROVE_GYRO_EXPECTED_ADDR) {
    printf("Gyro sensor started with addr 0x%02X\n", GROVE_GYRO_EXPECTED_ADDR);
  } else {
    printf("Gyro sensor with unrecognized address 0x%02X\n", aux);
    PROCESS_EXIT();
  }

  /* Register the interrupt handler */
  GROVE_GYRO_REGISTER_INT(gyro_interrupt_callback);

  /* The gyroscope sensor should be on now but the three gyroscope axis should
   * be off, to enable a single axis or any combination of the 3 you can use as
   * argument either GROVE_GYRO_X, GROVE_GYRO_Y, GROVE_GYRO_Z.  To enable or
   * disable the three axis alternatively use GROVE_GYRO_XYZ
   */
  grove_gyro.configure(GROVE_GYRO_POWER_ON, GROVE_GYRO_XYZ);

  /* Calibrate the sensor taking 200 samples every 5ms for the zero-point offset
   * value, this has to be done manually and is up to the user
   */
  grove_gyro.configure(GROVE_GYRO_CALIBRATE_ZERO, 1);

  /* Enabling the data interrupt will feed data directly to the extern data
   * structure and notify us about it, depending on the sampling rate and
   * divisor this could generate many interrupts/second.  A zero argument would
   * disable the interrupts
   */
  grove_gyro.configure(GROVE_GYRO_DATA_INTERRUPT, 1);

  /* And periodically poll the sensor, values are in º/s */

  while(1) {
    etimer_set(&et, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* This sensor has a different operation from others using Contiki's sensor
     * API, to make data acquisition we write the readings directly to the
     * extern data structure, allowing to write more than 1 value at the same 
     * operation, and also allowing upon a data interrupt event to immediatly 
     * access the data.  The return value of the value() call is then the status 
     * result of the read operation
     */
    if(grove_gyro.value(GROVE_GYRO_XYZ) == GROVE_GYRO_SUCCESS) {

      /* Converted values with a 2-digit precision */
      printf("Gyro: X: %u.%u, Y: %u.%u, Z: %u.%u\n", gyro_values.x / 100,
                                                     gyro_values.x % 100,
                                                     gyro_values.y / 100,
                                                     gyro_values.y % 100,
                                                     gyro_values.z / 100,
                                                     gyro_values.z % 100);
    } else {
      printf("Error, enable the DEBUG flag in the grove-gyro driver for info,");
      printf(" or check if the sensor is properly connected\n");
      PROCESS_EXIT();
    }

    if(grove_gyro.value(GROVE_GYRO_TEMP) == GROVE_GYRO_SUCCESS) {
      printf("Gyro: temperature %d.%02u C\n", gyro_values.temp / 100,
                                             gyro_values.temp % 100);
    } else {
      printf("Error, enable the DEBUG flag in the grove-gyro driver for info,");
      printf(" or check if the sensor is properly connected\n");
      PROCESS_EXIT();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

