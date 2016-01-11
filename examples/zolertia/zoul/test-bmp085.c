/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 * \defgroup zoul-bmp085-test BMP085 pressure and temperature sensor test
 *
 * Demonstrates the use of the BMP085 digital pressure and temperature sensor
 * @{
 *
 * \file
 *  Driver for the external BMP085 digital pressure and temperature sensor
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include "dev/bmp085.h"
/*---------------------------------------------------------------------------*/
#define SENSOR_READ_INTERVAL (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
PROCESS(remote_bmp085_process, "BMP085 test process");
AUTOSTART_PROCESSES(&remote_bmp085_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_bmp085_process, ev, data)
{
  PROCESS_BEGIN();
  static uint16_t pressure;
  static int16_t temperature;

  /* Use Contiki's sensor macro to enable the sensor */
  SENSORS_ACTIVATE(bmp085);

  /* And periodically poll the sensor */

  while(1) {
    etimer_set(&et, SENSOR_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    pressure = bmp085.value(BMP085_READ_PRESSURE);
    temperature = bmp085.value(BMP085_READ_TEMP);

    if((pressure != BMP085_ERROR) && (temperature != BMP085_ERROR)) {
      printf("Pressure = %u.%u(hPa), ", pressure / 10, pressure % 10);
      printf("Temperature = %d.%u(ºC)\n", temperature / 10, temperature % 10);
    } else {
      printf("Error, enable the DEBUG flag in the BMP085 driver for info, ");
      printf("or check if the sensor is properly connected\n");
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

