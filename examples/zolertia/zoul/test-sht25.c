/*
 * Copyright (c) 2015, Zolertia <http://www.zolertia.com>
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
 * \defgroup zoul-sht25-test SHT25 temperature and humidity sensor test
 *
 * Demonstrates the use of the SHT25 digital temperature and humidity sensor
 * @{
 *
 * \file
 *         A quick program for testing the SHT25 temperature and humidity sensor
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/sht25.h"
/*---------------------------------------------------------------------------*/
PROCESS(remote_sht25_process, "SHT25 test");
AUTOSTART_PROCESSES(&remote_sht25_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_sht25_process, ev, data)
{
  int16_t temperature, humidity;

  PROCESS_BEGIN();
  SENSORS_ACTIVATE(sht25);

  /* Check if the sensor voltage operation is over 2.25V */
  if(sht25.value(SHT25_VOLTAGE_ALARM)) {
    printf("Voltage is lower than recommended for the sensor operation\n");
    PROCESS_EXIT();
  }

  /* Configure the sensor for maximum resolution (14-bit temperature, 12-bit
   * relative humidity), this will require up to 85ms for the temperature
   * integration, and 29ms for the relative humidity (this is the default
   * setting at power on).  To achieve a faster integration time at the cost
   * of a lower resolution, change the value below accordingly, see sht25.h.
   */
  sht25.configure(SHT25_RESOLUTION, SHT2X_RES_14T_12RH);

  /* Let it spin and read sensor data */

  while(1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    temperature = sht25.value(SHT25_VAL_TEMP);
    printf("Temperature %02d.%02d ºC, ", temperature / 100, temperature % 100);
    humidity = sht25.value(SHT25_VAL_HUM);
    printf("Humidity %02d.%02d RH\n", humidity / 100, humidity % 100);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

