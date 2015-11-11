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
#include "pot-sensor.h"
#include <stdlib.h>
#include <GenericBoard.h>

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
  POT_STATUS_NOT_INIT = 0,
  POT_STATUS_INIT,
  POT_STATUS_NOT_ACTIVE = POT_STATUS_INIT,
  POT_STATUS_ACTIVE
} pot_status_t;

/* Absolute delta in pot level needed to generate event  */
#define DELTA_POT_VALUE  1

/*---------------------------------------------------------------------------*/
/* LOCAL DATA DEFINITIONS                                                    */
/*---------------------------------------------------------------------------*/
const struct sensors_sensor pot_sensor;
volatile static pot_status_t pot_status = POT_STATUS_NOT_INIT;
static int prev_pot_event_val = 0;
static int pot_value = 0;

/*---------------------------------------------------------------------------*/
/* LOCAL FUNCTION PROTOTYPES                                                 */
/*---------------------------------------------------------------------------*/
PROCESS(POTSampling, "POT");

/*---------------------------------------------------------------------------*/
/* PUBLIC FUNCTIONS                                                          */
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type == SENSORS_HW_INIT) {
    pot_status = POT_STATUS_INIT;
    bPotEnable();
    process_start(&POTSampling, NULL);
    return 1;
  } else if(type == SENSORS_ACTIVE) {
    if(pot_status != POT_STATUS_NOT_INIT) {
      if(value) {
        /* ACTIVATE SENSOR  */
        bPotEnable();
        prev_pot_event_val = 0;
        /* Activate POT. */
        PRINTF("POT ACTIVATED\n");
        pot_status = POT_STATUS_ACTIVE;
        process_post(&POTSampling, PROCESS_EVENT_MSG, (void *)&pot_status);
      } else {
        /* DE-ACTIVATE SENSOR */
        bPotDisable();
        PRINTF("POT DE-ACTIVATED\n");
        pot_status = POT_STATUS_NOT_ACTIVE;
        process_post(&POTSampling, PROCESS_EVENT_MSG, (void *)&pot_status);
      }
      return 1;
    } else {
      /*
         POT must be intialised before being (de)-activated */
      PRINTF("ERROR: NO HW_INIT POT\n");
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
    return pot_status == POT_STATUS_ACTIVE;
  } else if(type == SENSORS_READY) {
    return pot_status != POT_STATUS_NOT_INIT;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* type: Not defined for the pot interface */
  return pot_value;
}
/*---------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS                                                           */
/*---------------------------------------------------------------------------*/
/* Process to get POT_SENSOR value.
   POT is sampled. Sampling stopped when POT is de-activated.
   Event is generated if pot value changed at least the value DELTA_POT_VALUE
   since last event. */
PROCESS_THREAD(POTSampling, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer et;

  etimer_set(&et, CLOCK_SECOND / 10);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_TIMER) || (ev == PROCESS_EVENT_MSG));
    if(ev == PROCESS_EVENT_TIMER) {
      /* Handle sensor reading.   */
      PRINTF("POT sample\n");
      pot_value = u16ReadPotValue();
      PRINTF("POT = %d\n", pot_value);
      if(abs(pot_value - prev_pot_event_val) > DELTA_POT_VALUE) {
        prev_pot_event_val = pot_value;
        sensors_changed(&pot_sensor);
      }
      etimer_reset(&et);
    } else {
      /* ev == PROCESS_EVENT_MSG */
      if(*(int *)data == POT_STATUS_NOT_ACTIVE) {
        /* Stop sampling */
        etimer_stop(&et);
      } else if((*(int *)data == POT_STATUS_ACTIVE)) {
        /* restart sampling */
        etimer_restart(&et);
      }
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Sensor defintion for sensor module */
SENSORS_SENSOR(pot_sensor, POT_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/

