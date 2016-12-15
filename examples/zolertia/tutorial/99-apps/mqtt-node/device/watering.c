/*
 * Copyright (c) 2016, Antonio Lignan - antonio.lignan@gmail.com
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
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "dev/adc-sensors.h"
#include "dev/relay.h"
#include "watering.h"
#include "mqtt-res.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#if DEBUG_APP
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
sensor_values_t watering_sensors;
/*---------------------------------------------------------------------------*/
command_values_t watering_commands;
/*---------------------------------------------------------------------------*/
process_event_t watering_sensors_data_event;
process_event_t watering_sensors_alarm_event;
/*---------------------------------------------------------------------------*/
static uint8_t electrovalve_status = 0;
/*---------------------------------------------------------------------------*/
PROCESS(watering_sensors_process, "Irrigation sensor process");
/*---------------------------------------------------------------------------*/
static int
activate_electrovalve(int arg)
{
  if(!electrovalve_status) {
    PRINTF("Irrigation: electrovalve open\n");
    electrovalve_status = 1;
  }
  process_poll(&watering_sensors_process);
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
poll_sensors(void)
{
  watering_sensors.sensor[WATERING_SENSOR_SOIL].value = adc_sensors.value(ANALOG_SOIL_MOIST_SENSOR);

  mqtt_sensor_check(&watering_sensors, watering_sensors_alarm_event,
                    watering_sensors_data_event);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(watering_sensors_process, ev, data)
{
  static struct etimer et;
  static struct etimer valve;

  /* This is where our process start */
  PROCESS_BEGIN();

  /* Load sensor defaults */
  watering_sensors.num = 0;

  mqtt_sensor_register(&watering_sensors, WATERING_SENSOR_SOIL,
                       DEFAULT_SOIL_MOIST_MAX, DEFAULT_PUBLISH_EVENT_SOIL,
                       DEFAULT_PUBLISH_ALARM_SOIL, DEFAULT_SUBSCRIBE_CFG_SOILTHR,
                       DEFAULT_SOIL_MOIST_MIN, DEFAULT_SOIL_MOIST_MAX,
                       DEFAULT_SOIL_THRESH, DEFAULT_SOIL_THRESL, 0);

  /* Sanity check */
  if(watering_sensors.num != DEFAULT_SENSORS_NUM) {
    printf("Irrigation sensors: error! number of sensors mismatch!\n");
    PROCESS_EXIT();
  }

  /* Load commands default */
  watering_commands.num = 1;
  memcpy(watering_commands.command[WATERING_COMMAND_VALVE].command_name,
         DEFAULT_COMMAND_EVENT_VALVE, strlen(DEFAULT_COMMAND_EVENT_VALVE));
  watering_commands.command[WATERING_COMMAND_VALVE].cmd = activate_electrovalve;

  /* Sanity check */
  if(watering_commands.num != DEFAULT_COMMANDS_NUM) {
    printf("Irrigation commands: error! number of commands mismatch!\n");
    PROCESS_EXIT();
  }

  /* Get an event ID for our events */
  watering_sensors_data_event = process_alloc_event();
  watering_sensors_alarm_event = process_alloc_event();

  /* Start the periodic process */
  etimer_set(&et, DEFAULT_SAMPLING_INTERVAL);

  /* Configure ADC channel for soil moisture measurements */
  adc_sensors.configure(ANALOG_SOIL_MOIST_SENSOR, 5);

  /* Configure GPIO for Relay activation */
  SENSORS_ACTIVATE(relay);
  
  while(1) {

    PROCESS_YIELD();

    if(electrovalve_status) {
      relay.value(RELAY_ON);
      etimer_set(&valve, ELECTROVALVE_ON_INTERVAL);
      electrovalve_status = 0;

    } else {  
      if(ev == PROCESS_EVENT_TIMER && data == &valve) {
        relay.value(RELAY_OFF);
        electrovalve_status = 0;
      }
    }
    
    if(ev == PROCESS_EVENT_TIMER && data == &et) {
      poll_sensors();
      etimer_reset(&et);
    } else if(ev == sensors_stop_event) {
      PRINTF("Irrigation: sensor readings paused\n");
      etimer_stop(&et);
    } else if(ev == sensors_restart_event) {
      PRINTF("Irrigation: sensor readings enabled\n");
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/** @} */
