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
 * Demonstrates the use of the HP206C barometer altimeter sensor
 * @{
 *
 * \file
 *         A quick program for testing the HP206C temperature and humidity sensor
 * \author
 *         Javi Sanchez <jsanchez@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/hp206c.h"
#include "dev/leds.h"
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
PROCESS(remote_hp206c_process, "HP206C test");
AUTOSTART_PROCESSES(&remote_hp206c_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_hp206c_process, ev, data)
{
  int16_t altitude, temperature;
  int32_t pressure;
  PROCESS_BEGIN();

  hp206c.configure(HP206C_SOFT_RST, 1);

  /* Let it spin and read sensor data */

  while(1) {
    etimer_set(&et, (CLOCK_SECOND * 2));
    leds_toggle(LEDS_RED);
    hp206c.configure(HP206C_CONVERT, HP206C_CVT_4096_PT);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    altitude = hp206c.value(HP206C_READ_A);
    printf("Altitude %d.%d m\n", (int8_t)(altitude / 100), (uint8_t)(altitude % 100));
    pressure = hp206c.value(HP206C_READ_P);
    printf("Pressure %lu.%lu hPa \n", (pressure / 100), (pressure % 100));
    temperature = hp206c.value(HP206C_READ_T);
    printf("Temperature %d.%d ºC \n\n", (temperature / 100), (temperature % 100));
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
