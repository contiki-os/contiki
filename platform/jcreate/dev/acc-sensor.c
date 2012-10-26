/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * Author  : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : 2005-11-01
 * Updated : $Date: 2010/08/25 19:34:42 $
 *           $Revision: 1.2 $
 */

#include "contiki.h"
#include "dev/acc-sensor.h"
#include "dev/sky-sensors.h"

/* Configure ADC12_2 to sample channel 4, 5, 6 and use */
/* the Vref+ as reference (SREF_1) since it is a stable reference */
#define INPUT_CHANNEL      ((1 << INCH_4) | (1 << INCH_5) | (1 << INCH_6))
#define INPUT_REFERENCE     SREF_1
#define ACC_MEM_X            ADC12MEM4  /* Xout */
#define ACC_MEM_Y            ADC12MEM5  /* Yout */
#define ACC_MEM_Z            ADC12MEM6  /* Zout */

const struct sensors_sensor acc_sensor;

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  switch(type) {
  case ACC_SENSOR_X:
    return ACC_MEM_X;
  case ACC_SENSOR_Y:
    return ACC_MEM_Y;
  case ACC_SENSOR_Z:
    return ACC_MEM_Z;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  if(type == SENSORS_ACTIVE) {
    /* Sleep Mode P2.3 */
    if(c) {
      P2OUT |= 0x08;
      P2DIR |= 0x08;
    } else {
      /* Sensor deactivated. Changed to sleep mode. */
      P2OUT &= ~0x08;
    }
  } else if(type == ACC_SENSOR_SENSITIVITY) {
    /* g-Select1 P2.0, g-Select2 P2.1 */
    P2DIR |= 0x03;
    P2OUT &= ~0x03;
    P2OUT |= c & 0x03;
  }
  return sky_sensors_configure(INPUT_CHANNEL, INPUT_REFERENCE, type, c);
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  if(type == ACC_SENSOR_SENSITIVITY) {
    return (P2OUT & P2DIR) & 0x03;
  }
  return sky_sensors_status(INPUT_CHANNEL, type);
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(acc_sensor, ACC_SENSOR, value, configure, status);
