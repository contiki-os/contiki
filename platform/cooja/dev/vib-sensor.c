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
 * $Id: vib-sensor.c,v 1.2 2010/01/14 19:12:31 nifi Exp $
 */

#include "dev/vib-sensor.h"
#include "lib/simEnvChange.h"

const struct simInterface vib_interface;
const struct sensors_sensor vib_sensor;

// COOJA variables
char simVibChanged;
char simVibIsActive;
char simVibValue = 0;

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return simVibValue;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
  case SENSORS_HW_INIT:
    simVibIsActive = 0;
    return 1;
  case SENSORS_ACTIVE:
    simVibIsActive = c;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return simVibIsActive;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
  // Check if Vib value has changed
  if (simVibIsActive && simVibChanged) {
    simVibValue++;

    sensors_changed(&vib_sensor);
    simVibChanged = 0;
  }
}
/*---------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*---------------------------------------------------------------------------*/

SIM_INTERFACE(vib_interface,
	doInterfaceActionsBeforeTick,
	doInterfaceActionsAfterTick);

SENSORS_SENSOR(vib_sensor, VIB_SENSOR,
               value, configure, status);
