/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
/**
 * \author Atis Elsts <atis.elsts@sics.se>
 * \file
 *         Sensor test for DR1175 evaluation board (light + temp/humidity).
 */

#include "contiki.h"

#include "light-sensor.h"
#include "ht-sensor.h"

/*---------------------------------------------------------------------------*/
PROCESS(test_process, "Sensor test process");
AUTOSTART_PROCESSES(&test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_process, ev, data)
{
  static struct etimer et;
  static uint8_t led_status;
  uint8_t r, g, b;
  int val;

  PROCESS_BEGIN();

  puts("initializing sensors...");

  /* White LED initialisation */
  bWhite_LED_Enable();
  bWhite_LED_SetLevel(0);
  bWhite_LED_On();

  /* Coloured LED initialisation */
  bRGB_LED_Enable();
  bRGB_LED_SetGroupLevel(255);
  bRGB_LED_SetLevel(0, 0, 0);
  bRGB_LED_On();

  /* Make sensor active for measuring */
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(ht_sensor);

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 1);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    puts("reading sensors...");

    val = ht_sensor.value(HT_SENSOR_HUM);
    printf("humidity: %d\n", val);

    val = ht_sensor.value(HT_SENSOR_TEMP);
    printf("temperature: %d\n", val);

    led_status++;
    bWhite_LED_SetLevel((led_status & 0x1) ? 255 : 0);
    bWhite_LED_On();

    r = (led_status & 0x7) & 0x1;
    g = (led_status & 0x7) & 0x2;
    b = (led_status & 0x7) & 0x4;
    bRGB_LED_SetLevel(r ? 255 : 0, g ? 255 : 0, b ? 255 : 0);
    bRGB_LED_On();

    puts("");
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
