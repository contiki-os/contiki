/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup remote-zonik-test Zolertia Zonik sonometer test application
 *
 *  Example of Zonik board implementation and simple operation: Infinite loop
 *  enablinkg the sensor and rading few times, acquiring the dBA of sensor
 *  The first value acquired is invalid, because it's in hw init state awaiting
 *  a valid internal reading.Once the driver is initialized, posterior readings
 *  are valid.Finally in the loop disables the board with standard call and
 *  shows the error, and loop again enabling it.
 *
 * @{
 * \file
 *         RE-Mote test application of Zolertia Zonik sound sensor
 * \author
 *         Aitor Mejias <amejias@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "zonik.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define MAX_VALID_READINGS    10L
#define MAX_INVALID_READINGS  3L
/*---------------------------------------------------------------------------*/
PROCESS(test_remote_zonik_process, "Test Zonik driver process");
AUTOSTART_PROCESSES(&test_remote_zonik_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_remote_zonik_process, ev, data)
{
  static int16_t zonik_val;
  static uint8_t i;

  PROCESS_BEGIN();

  printf("Initial status of sensor is: 0x%04X\n",
         zonik.status(SENSORS_ACTIVE));

  while(1) {
    /* Configure Zonik and activate the internal process readings */
    SENSORS_ACTIVATE(zonik);

    printf("Initialized. Sensor status: 0x%04X\n",
           zonik.status(SENSORS_ACTIVE));

    /* Read sensor value dBA multiple times */
    for(i = 0; i < MAX_VALID_READINGS; i++) {
      /* Wait a bit */
      etimer_set(&et, CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

      /* Get data from sensor */
      zonik_val = zonik.value(ZONIK_DBA_LEQ_VALUE);
      leds_toggle(LEDS_GREEN);
      printf("Value (dBA): %d\n", zonik_val);
    }

    printf("Sensor status is: 0x%04X\n",
           zonik.status(SENSORS_ACTIVE));

    /* Disable Zonik sensor */
    SENSORS_DEACTIVATE(zonik);
    printf("Process Stopped: 0x%04X\n", zonik.status(SENSORS_ACTIVE));

    for(i = 0; i < MAX_INVALID_READINGS; i++) {
      /* Wait a bit */
      etimer_set(&et, CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

      /* Get data from sensor */
      zonik_val = zonik.value(ZONIK_DBA_LEQ_VALUE);
      leds_toggle(LEDS_GREEN);
      printf("Value (dBA): %d\n", zonik_val);
    }

    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

