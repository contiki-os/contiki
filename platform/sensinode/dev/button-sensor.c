/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: button-sensor.c,v 1.1 2009/09/08 20:06:28 zdshelby Exp $
 */

#include "lib/sensors.h"
/*#include "dev/hwconf.h"*/
#include "dev/button-sensor.h"
#include "dev/leds.h"

const struct sensors_sensor button_sensor;

static struct timer debouncetimer;

/*
HWCONF_PIN(BUTTON, 2, 7);
HWCONF_IRQ(BUTTON, 2, 7);
*/

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  timer_set(&debouncetimer, 0);
  BUTTON_IRQ_EDGE_SELECTD();

  BUTTON_SELECT();
  BUTTON_MAKE_INPUT();
}
/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  if(BUTTON_CHECK_IRQ()) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 4);
      sensors_changed(&button_sensor);
      return 1;
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  sensors_add_irq(&button_sensor, BUTTON_IRQ_PORT());
  BUTTON_ENABLE_IRQ();
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  BUTTON_DISABLE_IRQ();
  sensors_remove_irq(&button_sensor, BUTTON_IRQ_PORT());
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return BUTTON_IRQ_ENABLED();
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  return BUTTON_READ() || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, void *c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void *
status(int type)
{
  return NULL;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
