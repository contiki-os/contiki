/*
 * Copyright (c) 2011 Zolertia(TM) is a trademark by Advancare,SL
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
 *
 * -----------------------------------------------------------------
 *
 * Author  : Enric M. Calvo (based on work by A. Dunkels, J. Eriksson, N. Finne)
 * Created : 2011-02-22
 *           $Revision: 1.0 $
 */

#include "dev/potentiometer-sensor.h"
#include "dev/sky-sensors.h"

#include "contiki.h"
#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif


/* Configure ADC12_2 to sample channel 11 (voltage) and use */
/* the Vref+ as reference (SREF_1) since it is a stable reference */
#define INPUT_CHANNEL          (1 << INCH_4)
#define INPUT_REFERENCE        SREF_0
#define POTENTIOMETER_MEM      ADC12MEM4

const struct sensors_sensor potentiometer_sensor;
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return POTENTIOMETER_MEM;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  return sky_sensors_configure(INPUT_CHANNEL, INPUT_REFERENCE, type, c);
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return sky_sensors_status(INPUT_CHANNEL, type);
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(potentiometer_sensor, POTENTIOMETER_SENSOR, value, configure, status);
