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
 * \file
 *         Reed sensor driver file
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */

#include "contiki.h"
#include "lib/sensors.h"
#include "dev/reed-sensor.h"
#include "sys/process.h"
#include "sys/ctimer.h"
/*---------------------------------------------------------------------------*/
#ifndef REED_CHECK_PERIOD
#define REED_CHECK_PERIOD    CLOCK_SECOND
#endif
/*---------------------------------------------------------------------------*/
static int current_status = -1;
static struct ctimer change_timer;
process_event_t reed_sensor_event_changed;
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return ~(REED_PORT_DIR & REED_READ_PIN);
  }
  return REED_SENSOR_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if((!status(SENSORS_ACTIVE)) || (type != REED_SENSOR_VAL)) {
    return REED_SENSOR_ERROR;
  }
  return (REED_PORT_READ & REED_READ_PIN) ? REED_CLOSED : REED_OPEN;
}
/*---------------------------------------------------------------------------*/
static void
check_callback(void *data)
{
  static int new_status;
  if(current_status == -1) {
    ctimer_stop(&change_timer);
    return;
  }

  new_status = value(REED_SENSOR_VAL);
  if(new_status != current_status) {
    current_status = new_status;
    process_post(PROCESS_BROADCAST, reed_sensor_event_changed, &current_status);
  }
  ctimer_reset(&change_timer);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
  case SENSORS_ACTIVE:
    if(c) {
      if(!status(SENSORS_ACTIVE)) {
        REED_PORT_SEL |= REED_READ_PIN;
        REED_PORT_DIR &= ~REED_READ_PIN;
        REED_PORT_REN |= REED_READ_PIN;
        REED_PORT_PRES |= REED_READ_PIN;
      }
    } else {
      REED_PORT_DIR |= REED_READ_PIN;
      REED_PORT_REN &= ~REED_READ_PIN;
    }
    return REED_SENSOR_SUCCESS;
  case REED_SENSOR_MODE:
    if(c == REED_SENSOR_EVENT_MODE) {
      current_status = value(REED_SENSOR_VAL);
      ctimer_set(&change_timer, REED_CHECK_PERIOD, check_callback, NULL);
    } else if(c == REED_SENSOR_EVENT_POLL) {
      current_status = -1;
      ctimer_stop(&change_timer);
    } else {
      return REED_SENSOR_ERROR;
    }
    return REED_SENSOR_SUCCESS;
  }
  return REED_SENSOR_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(reed_sensor, REED_SENSOR, value, configure, status);
