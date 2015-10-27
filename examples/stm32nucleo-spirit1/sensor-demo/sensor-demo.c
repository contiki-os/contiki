/*
 * Copyright (c) 2012, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include <stdio.h> /* For printf() */

#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/radio-sensor.h"
#include "dev/sensor-common.h"

#include "st-lib.h"

#ifdef X_NUCLEO_IKS01A1
#include "dev/temperature-sensor.h"
#include "dev/humidity-sensor.h"
#include "dev/pressure-sensor.h"
#include "dev/magneto-sensor.h"
#include "dev/acceleration-sensor.h"
#include "dev/gyroscope-sensor.h"
#endif /*X_NUCLEO_IKS01A1*/

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define PRINT_INTERVAL 5 * CLOCK_SECOND

/*---------------------------------------------------------------------------*/
PROCESS(sensor_demo_process, "Sensor demo process");
AUTOSTART_PROCESSES(&sensor_demo_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_demo_process, ev, data)
{
  static struct etimer etimer;
  static unsigned long _button_pressed;
  static int sensor_value = 0;

  PROCESS_BEGIN();
  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  SENSORS_ACTIVATE(radio_sensor);

#ifdef X_NUCLEO_IKS01A1
  SENSORS_ACTIVATE(temperature_sensor);
  SENSORS_ACTIVATE(humidity_sensor);
  SENSORS_ACTIVATE(pressure_sensor);
  SENSORS_ACTIVATE(magneto_sensor);
  SENSORS_ACTIVATE(acceleration_sensor);
  SENSORS_ACTIVATE(gyroscope_sensor);
#endif /*X_NUCLEO_IKS01A1*/

  while(1) {
    etimer_set(&etimer, PRINT_INTERVAL);

    PROCESS_WAIT_EVENT();
    if(ev == sensors_event && data == &button_sensor) {
      printf("Sensor event detected: Button Pressed.\n\n");
      printf("Toggling Leds\n");
      _button_pressed++;
      leds_toggle(LEDS_ALL);
    }

    printf("Button state:\t%s (pressed %lu times)\n", button_sensor.value(0) ? "Released" : "Pressed",
           _button_pressed);

#ifdef X_NUCLEO_IKS01A1
    printf("LEDs status:\tRED:n/a GREEN:%s\n", leds_get() & LEDS_GREEN ? "on" : "off");
#else
    printf("LEDs status:\tRED:%s GREEN:%s\n", leds_get() & LEDS_RED ? "on" : "off",
           leds_get() & LEDS_GREEN ? "on" : "off");
#endif /*X_NUCLEO_IKS01A1*/
    sensor_value = radio_sensor.value(RADIO_SENSOR_LAST_PACKET);
    printf("Radio (RSSI):\t%d.%d dBm\n", sensor_value / 10, ABS_VALUE(sensor_value) % 10);
    printf("Radio (LQI):\t%d\n", radio_sensor.value(RADIO_SENSOR_LAST_VALUE));

#ifdef X_NUCLEO_IKS01A1
    sensor_value = temperature_sensor.value(0);
    printf("Temperature:\t%d.%d C\n", sensor_value / 10, ABS_VALUE(sensor_value) % 10);

    sensor_value = humidity_sensor.value(0);
    printf("Humidity:\t%d.%d rH\n", sensor_value / 10, ABS_VALUE(sensor_value) % 10);

    sensor_value = pressure_sensor.value(0);
    printf("Pressure:\t%d.%d mbar\n", sensor_value / 10, ABS_VALUE(sensor_value) % 10);

    /* NOTE: this demo uses the mapping of ST Nucleo sensors on Contiki sensor API.
     * For a real use case of sensors like acceleration, magneto and gyroscope,
     * it is better to directly call the ST lib to get the three value (X/Y/Z)
     * at once.
     */
    printf("Magneto:\t%d/%d/%d (X/Y/Z) mgauss\n", magneto_sensor.value(X_AXIS),
           magneto_sensor.value(Y_AXIS),
           magneto_sensor.value(Z_AXIS));

    printf("Acceleration:\t%d/%d/%d (X/Y/Z) mg\n", acceleration_sensor.value(X_AXIS),
           acceleration_sensor.value(Y_AXIS),
           acceleration_sensor.value(Z_AXIS));

    printf("Gyroscope:\t%d/%d/%d (X/Y/Z) mdps\n", gyroscope_sensor.value(X_AXIS),
           gyroscope_sensor.value(Y_AXIS),
           gyroscope_sensor.value(Z_AXIS));
#endif /*X_NUCLEO_IKS01A1*/

    printf("\n");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
