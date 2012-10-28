/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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

#include "lib/sensors.h"
#include "dev/button-sensor.h"
#include "lib/simEnvChange.h"

const struct simInterface button_interface;
const struct sensors_sensor button_sensor;
static struct timer debouncetimer;

// COOJA variables
char simButtonChanged;
char simButtonIsDown;
char simButtonIsActive;
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return simButtonIsDown || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  if(type == SENSORS_ACTIVE) {
    simButtonIsActive = c;
    return 1;
  } else if(type == SENSORS_HW_INIT) {
    simButtonIsActive = 1;
    timer_set(&debouncetimer, 0);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return (type == SENSORS_ACTIVE) ? simButtonIsActive : 0;
}
/*---------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
  // Check if button value has changed
  if(simButtonChanged && simButtonIsActive && simButtonIsDown) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 10);
      sensors_changed(&button_sensor);
    }
  }
  simButtonChanged = 0;
}
/*---------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*---------------------------------------------------------------------------*/

SIM_INTERFACE(button_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);

SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);
