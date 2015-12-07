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
 * -----------------------------------------------------------------
 *
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne, Marcus Lundén,
 *           Jesper Karlsson
 * Created : 2005-11-01
 */

#include "contiki.h"
#include "dev/ext-sensor.h"
#include "dev/sky-sensors.h"

const struct sensors_sensor ext_sensor;
static uint8_t active;
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  #if 0
  /* ADC0 corresponds to the port under the logo,
   * ADC1 to the port over the logo,
   * ADC2 and ADC3 corresponds to port on the JCreate bottom expansion port) */
  switch(type) {
    case ADC0:
      return ADC12MEM6;
    case ADC1:
      return ADC12MEM7;
    case ADC2:
      return ADC12MEM8;
    case ADC3:
      return ADC12MEM9;
  }
  #endif /* 0 */
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return active;
  default:
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  switch(type) {
    case SENSORS_ACTIVE:
      if(c) {
        if(!status(SENSORS_ACTIVE)) {
          #if 0
          /* SREF_1 is Vref+ */
          /* MemReg6 == P6.0/A0 == port "under" logo */
          ADC12MCTL6 = (INCH_0 + SREF_0);
          /* MemReg7 == P6.1/A1 == port "over" logo */
          ADC12MCTL7 = (INCH_1 + SREF_0);
          /* MemReg8 == P6.2/A2, bottom expansion port */
          ADC12MCTL8 = (INCH_2 + SREF_0);
          /* MemReg9 == P6.1/A3, bottom expansion port, End Of (ADC-)Sequence */
          ADC12MCTL9 = (INCH_3 + SREF_0);
          #endif /* 0 */
          sky_sensors_activate(0x0F);
          active = 1;
        }
      } else {
        sky_sensors_deactivate(0x0F);
        active = 0;
      }
      return 1;
  default:
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ext_sensor, "Ext", value, configure, status);
