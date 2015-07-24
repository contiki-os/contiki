/**
******************************************************************************
* @file    contiki-spirit1-main.c
* @author  System LAB
* @version V1.0.0
* @date    17-June-2015
* @brief   Contiki main file for SPIRIT1 platform
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/
/**
 * \file
 *         A very simple Contiki application showing sensor values for ST Nucleo
 */

#include "contiki.h"

#include <stdio.h> /* For printf() */

#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"

#ifdef COMPILE_SENSORS
#include "dev/temperature-sensor.h"
#include "dev/humidity-sensor.h"
#include "dev/pressure-sensor.h"
#include "dev/magneto-sensor.h"
#include "dev/acceleration-sensor.h"
#include "dev/gyroscope-sensor.h"
#endif /*COMPILE_SENSORS*/

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define PRINT_INTERVAL 5*CLOCK_SECOND

/*---------------------------------------------------------------------------*/
PROCESS(sensor_demo_process, "Sensor demo process");
AUTOSTART_PROCESSES(&sensor_demo_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_demo_process, ev, data)
{
  static struct etimer etimer;
  static unsigned long _button_pressed;

  PROCESS_BEGIN();
  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

#ifdef COMPILE_SENSORS
  SENSORS_ACTIVATE(temperature_sensor);
  SENSORS_ACTIVATE(humidity_sensor);
  SENSORS_ACTIVATE(pressure_sensor);
  SENSORS_ACTIVATE(magneto_sensor);
  SENSORS_ACTIVATE(acceleration_sensor);
  SENSORS_ACTIVATE(gyroscope_sensor);
#endif

  while(1)
  {
    etimer_set(&etimer, PRINT_INTERVAL);
    
    PROCESS_WAIT_EVENT();
    if (ev == sensors_event && data == &button_sensor)
    {
      printf("Sensor event detected: Button Pressed.\n\n");
      printf("Toggling Leds\n");
      _button_pressed++;
      leds_toggle(LEDS_ALL);
    }


    printf("Button state:\t%s (pressed %lu times)\n", button_sensor.value(0)?"Released":"Pressed",
                                                      _button_pressed);

#ifdef COMPILE_SENSORS
    printf("LEDs status:\tRED:n/a GREEN:%s\n", leds_get()&LEDS_GREEN?"on":"off");
#else
    printf("LEDs status:\tRED:%s GREEN:%s\n", leds_get()&LEDS_RED?"on":"off", 
                                              leds_get()&LEDS_GREEN?"on":"off");
#endif /*COMPILE_SENSORS*/

#ifdef COMPILE_SENSORS
    printf("Temperature:\t%d.%d C\n", temperature_sensor.value(0)/10, temperature_sensor.value(0)%10);

    printf("Humidity:\t%d.%d rH\n", humidity_sensor.value(0)/10, humidity_sensor.value(0)%10);

    printf("Pressure:\t%d.%d mbar\n", pressure_sensor.value(0)/10, pressure_sensor.value(0)%10);

    printf("Magneto:\t%d/%d/%d (X/Y/Z) mgauss\n", magneto_sensor.value(X_AXIS),
                                                  magneto_sensor.value(Y_AXIS),
                                                  magneto_sensor.value(Z_AXIS));

    printf("Acceleration:\t%d/%d/%d (X/Y/Z) mg\n", acceleration_sensor.value(X_AXIS),
                                                   acceleration_sensor.value(Y_AXIS),
                                                   acceleration_sensor.value(Z_AXIS));

    printf("Gyroscope:\t%d/%d/%d (X/Y/Z) mdps\n", gyroscope_sensor.value(X_AXIS),
                                                  gyroscope_sensor.value(Y_AXIS),
                                                  gyroscope_sensor.value(Z_AXIS));
#endif

    printf ("\n");
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
