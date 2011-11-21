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
 * $Id: ext-sensor.c,v 1.2 2010/08/25 19:34:42 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne, Marcus Lundén,
 *           Jesper Karlsson
 * Created : 2005-11-01
 * Updated : $Date: 2010/08/25 19:34:42 $
 *           $Revision: 1.2 $
 */


#include "contiki.h"
#include "dev/ext-sensor.h"
#include "dev/sky-sensors.h"

/* SREF_0 is AVCC */
/* SREF_1 is Vref+ */
/* ADC0 == P6.0/A0 == port "under" logo */
/* ADC1 == P6.1/A1 == port "over" logo */
/* ADC2 == P6.2/A2, bottom expansion port */
/* ADC3 == P6.1/A3, bottom expansion port, End Of (ADC-)Sequence */

#define INPUT_CHANNEL      ((1 << INCH_0) | (1 << INCH_1) | \
                            (1 << INCH_2) | (1 << INCH_3))
#define INPUT_REFERENCE     SREF_0
#define EXT_MEM0            ADC12MEM0
#define EXT_MEM1            ADC12MEM1
#define EXT_MEM2            ADC12MEM2
#define EXT_MEM3            ADC12MEM3

const struct sensors_sensor ext_sensor;
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  /* ADC0 corresponds to the port under the logo, ADC1 to the port
     over the logo, ADC2 and ADC3 corresponds to port on the JCreate
     bottom expansion port) */
  switch(type) {
  case ADC0:
    return EXT_MEM0;
  case ADC1:
    return EXT_MEM1;
  case ADC2:
    return EXT_MEM2;
  case ADC3:
    return EXT_MEM3;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return sky_sensors_status(INPUT_CHANNEL, type);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  return sky_sensors_configure(INPUT_CHANNEL, INPUT_REFERENCE, type, c);
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ext_sensor, "Ext", value, configure, status);
