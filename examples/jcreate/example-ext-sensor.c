/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * \file
 *         A very simple Contiki application showing how to use ext-sensor
 *         In this case a i-Snail current sensor for 10A is used to measure
 *         current.
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "contiki.h"
#include "dev/leds.h"
#include "dev/ext-sensor.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(ext_sensor_process, "ext-sensor process");
AUTOSTART_PROCESSES(&ext_sensor_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(ext_sensor_process, ev, data)
{
  static struct etimer et;
  static long maValue;
  static long maSum;
  static int time;
  PROCESS_BEGIN();

  SENSORS_ACTIVATE(ext_sensor);

  leds_invert(LEDS_ALL);

  etimer_set(&et, CLOCK_SECOND);
  while(1) {

    leds_invert(LEDS_ALL);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    leds_invert(LEDS_ALL);

    /* the max value of the i-Snail sensor is 10 Ampere (=> ~5V) */
    /* and we are using the 2.5 V reference voltage which */
    /* means that 5 Ampere will make the sensor max-out. */
    /* => 5 Ampere will be 4095 in the sensor ADC register */
    maValue = (ext_sensor.value(ADC0) * 5000L) / 4095;
    maSum += maValue;
    time++;
    printf("Measured ext1 %d  %ld mA %ld W  %ld Wh %d (s)\n", 
	   ext_sensor.value(ADC0), maValue, (maValue * 230L) / 1000,
	   (maSum * 230L) / (1000L * 60 * 60), time);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
