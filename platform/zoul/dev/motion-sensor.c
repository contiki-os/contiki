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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-motion-sensor
 * @{
 *
 * \file
 *         Digital motion sensor driver
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/motion-sensor.h"
#include "lib/sensors.h"
#include "dev/sys-ctrl.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define MOTION_SENSOR_PORT_BASE  GPIO_PORT_TO_BASE(MOTION_SENSOR_PORT)
#define MOTION_SENSOR_PIN_MASK   GPIO_PIN_MASK(MOTION_SENSOR_PIN)
/*---------------------------------------------------------------------------*/
void (*presence_int_callback)(uint8_t value);
/*---------------------------------------------------------------------------*/
PROCESS(motion_int_process, "Motion interrupt process handler");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(motion_int_process, ev, data)
{
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    presence_int_callback(0);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
motion_interrupt_handler(uint8_t port, uint8_t pin)
{
  process_poll(&motion_int_process);
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return MOTION_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return GPIO_READ_PIN(MOTION_SENSOR_PORT_BASE, MOTION_SENSOR_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if(type != MOTION_ACTIVE) {
    PRINTF("Motion: invalid configuration option\n");
    return MOTION_ERROR;
  }

  if(!value) {
    presence_int_callback = NULL;
    GPIO_DISABLE_INTERRUPT(MOTION_SENSOR_PORT_BASE, MOTION_SENSOR_PIN_MASK);
    return MOTION_SUCCESS;
  }

  /* Configure interruption */
  GPIO_SOFTWARE_CONTROL(MOTION_SENSOR_PORT_BASE, MOTION_SENSOR_PIN_MASK);
  GPIO_SET_INPUT(MOTION_SENSOR_PORT_BASE, MOTION_SENSOR_PIN_MASK);
  GPIO_DETECT_RISING(MOTION_SENSOR_PORT_BASE, MOTION_SENSOR_PIN_MASK);
  GPIO_TRIGGER_SINGLE_EDGE(MOTION_SENSOR_PORT_BASE, MOTION_SENSOR_PIN_MASK);
  ioc_set_over(MOTION_SENSOR_PORT, MOTION_SENSOR_PIN, IOC_OVERRIDE_DIS);
  gpio_register_callback(motion_interrupt_handler, MOTION_SENSOR_PORT,
                         MOTION_SENSOR_PIN);

  process_start(&motion_int_process, NULL);

  GPIO_ENABLE_INTERRUPT(MOTION_SENSOR_PORT_BASE, MOTION_SENSOR_PIN_MASK);
  NVIC_EnableIRQ(MOTION_SENSOR_VECTOR);
  return MOTION_SUCCESS;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(motion_sensor, MOTION_SENSOR, value, configure, status);
/*---------------------------------------------------------------------------*/
/** @} */
