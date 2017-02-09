/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-motion-test Digital motion sensor test
 *
 * The example application shows how to use any digital motion sensor, basically
 * driving a signal high when motion is detected.
 *
 * @{
 *
 * \file
 *         Test application for the digital motion/presence sensor
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cpu.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "dev/motion-sensor.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define LEDS_OFF_HYSTERISIS      RTIMER_SECOND
/*---------------------------------------------------------------------------*/
static struct rtimer rt;
/*---------------------------------------------------------------------------*/
PROCESS(test_presence_sensor, "Test digital motion sensor");
AUTOSTART_PROCESSES(&test_presence_sensor);
/*---------------------------------------------------------------------------*/
void
rt_callback(struct rtimer *t, void *ptr)
{
  leds_off(LEDS_RED);
}
/*---------------------------------------------------------------------------*/
static void
presence_callback(uint8_t value)
{
  printf("*** Presence detected!\n");
  leds_on(LEDS_RED);
  rtimer_set(&rt, RTIMER_NOW() + LEDS_OFF_HYSTERISIS, 1, rt_callback, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_presence_sensor, ev, data)
{
  PROCESS_BEGIN();

  /* Register the callback handler when presence is detected */
  MOTION_REGISTER_INT(presence_callback);

  /* Enable the sensor, as default it assumes the signal pin has a pull-down
   * resistor and a rising interrupt (signal goes high when motion is detected),
   * this is the case of the Grove's PIR sensor v.1.0.  If the sensor has an
   * inverse logic, change at the motion-sensor.c driver file
   * GPIO_DETECT_FALLING instead of GPIO_DETECT_RISING if using an external
   * pull-up resistor
   */
  SENSORS_ACTIVATE(motion_sensor);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */

