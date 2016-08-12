/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
 * Author: Theo van Daele <theo.van.daele@nxp.com>
 *
 */
#include "contiki.h"
#include "sys/etimer.h"
#include "lib/sensors.h"
#include "light-sensor.h"
#include <AppHardwareApi.h>
#include <AlsDriver.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/
/* LOCAL DEFINITIONS                                                         */
/*---------------------------------------------------------------------------*/
/* #define DEBUG */
#ifdef DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

typedef enum {
  LIGHT_SENSOR_STATUS_NOT_INIT = 0,
  LIGHT_SENSOR_STATUS_INIT,
  LIGHT_SENSOR_STATUS_NOT_ACTIVE = LIGHT_SENSOR_STATUS_INIT,
  LIGHT_SENSOR_STATUS_ACTIVE
} light_sensor_status_t;

/* Absolute delta in light level needed to generate event */
#define DELTA_LIGHT_SENSOR_VALUE  1

/*---------------------------------------------------------------------------*/
/* LOCAL DATA DEFINITIONS                                                    */
/*---------------------------------------------------------------------------*/
const struct sensors_sensor light_sensor;
volatile static light_sensor_status_t light_sensor_status = LIGHT_SENSOR_STATUS_NOT_INIT;
static int prev_light_event_val = 0;
static int light_sensor_value = 0;

/*---------------------------------------------------------------------------*/
/* LOCAL FUNCTION PROTOTYPES                                                 */
/*---------------------------------------------------------------------------*/
static int adjust(int input1, int input2);
PROCESS(LightSensorSampling, "Light sensor");

/*---------------------------------------------------------------------------*/
/* PUBLIC FUNCTIONS                                                          */
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type == SENSORS_HW_INIT) {
    PRINTF("SENSORS_HW_INIT\n");
    light_sensor_status = LIGHT_SENSOR_STATUS_INIT;
    process_start(&LightSensorSampling, NULL);
    return 1;
  } else if(type == SENSORS_ACTIVE) {
    if(light_sensor_status != LIGHT_SENSOR_STATUS_NOT_INIT) {
      if(value) {
        /* ACTIVATE SENSOR */
        vALSreset();
        prev_light_event_val = 0;
        /* Activate light sensor. Use channel 0. (Channel 1 = IR). Start sampling */
        PRINTF("LIGHT SENSOR ACTIVATED\n");
        light_sensor_status = LIGHT_SENSOR_STATUS_ACTIVE;
        process_post(&LightSensorSampling, PROCESS_EVENT_MSG, (void *)&light_sensor_status);
      } else {
        /* DE-ACTIVATE SENSOR */
        vALSpowerDown();
        PRINTF("LIGHT SENSOR DE-ACTIVATED\n");
        light_sensor_status = LIGHT_SENSOR_STATUS_NOT_ACTIVE;
        process_post(&LightSensorSampling, PROCESS_EVENT_MSG, (void *)&light_sensor_status);
      }
      return 1;
    } else {
      /* Light sensor must be intialised before being (de)-activated */
      PRINTF("ERROR: NO HW_INIT LIGHT SENSOR\n");
      return 0;
    }
  } else {
    /* Non valid type */
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  if(type == SENSORS_ACTIVE) {
    return light_sensor_status == LIGHT_SENSOR_STATUS_ACTIVE;
  } else if(type == SENSORS_READY) {
    return light_sensor_status != LIGHT_SENSOR_STATUS_NOT_INIT;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* type: Not defined for the light sensor interface */
  return light_sensor_value;
}
/*---------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS                                                           */
/*---------------------------------------------------------------------------*/
/* Process to get light sensor value.
   Light sensor is sampled. Sampling stopped when sensor is de-activated.
   Event is generated if light value changed at least the value DELTA_LIGHT_SENSOR_VALUE
   since last event. */
PROCESS_THREAD(LightSensorSampling, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;
  int channel0_value, channel1_value;

  etimer_set(&et, CLOCK_SECOND / 10);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_TIMER) || (ev == PROCESS_EVENT_MSG));
    if(ev == PROCESS_EVENT_TIMER) {
      /* Handle sensor reading.  */
      PRINTF("Light sensor sample\n");
      vALSstartReadChannel(0);
      channel0_value = u16ALSreadChannelResult();
      PRINTF("Channel 0 = %d\n", channel0_value);
      vALSstartReadChannel(1);
      channel1_value = u16ALSreadChannelResult();
      PRINTF("Channel 1 = %d\n", channel1_value);
      light_sensor_value = adjust(channel0_value, channel1_value);
      PRINTF("Light output = %d\n", light_sensor_value);
      if(abs(light_sensor_value - prev_light_event_val) > DELTA_LIGHT_SENSOR_VALUE) {
        prev_light_event_val = light_sensor_value;
        sensors_changed(&light_sensor);
      }
      etimer_reset(&et);
    } else {
      /* ev == PROCESS_EVENT_MSG */
      if(*(int *)data == LIGHT_SENSOR_STATUS_NOT_ACTIVE) {
        /* Stop sampling */
        etimer_stop(&et);
      } else if((*(int *)data == LIGHT_SENSOR_STATUS_ACTIVE)) {
        /* restart sampling */
        etimer_restart(&et);
      }
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Sensor defintion for sensor module */
SENSORS_SENSOR(light_sensor, LIGHT_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* adjust() converts the 2 measured light level into 1 ambient light level.  */
/* See manual JN-RM-2003.pdf                                                 */
/* Approximation is used: output[Lux] = 0.39*(ch0-ch1)                       */
/*---------------------------------------------------------------------------*/
static int
adjust(int ch0, int ch1)
{
  if(ch0 > ch1) {
    return (39 * (ch0 - ch1)) / 100;
  } else {
    return 0;
  }
}
