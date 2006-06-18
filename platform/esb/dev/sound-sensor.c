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
 * @(#)$Id: sound-sensor.c,v 1.1 2006/06/18 07:49:33 adamdunkels Exp $
 */
#include <stdlib.h>
#include "contiki-esb.h"
#include "dev/irq.h"

#define MIC_MIN_SENS 150

const struct sensors_sensor sound_sensor;

static unsigned int sound, micdiff, micmax, avgmax;
char sound_pause;

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  /* Initialization of ADC12 done by irq */
}
/*---------------------------------------------------------------------------*/
static int
irq(void)
{

  if (!sound_pause) {
    micdiff = micdiff + abs(ADC12MEM4 - sound) - (micdiff >> 3);


/*   if (micdiff > MIC_MIN_SENS) { */
/*     sensors_changed(&sound_sensor); */
/*   } */

/*     if (micdiff > (avgmax >> 2)) { */
/*       if (micdiff % 10 == 0) beep_beep(10); */
/*       // Subtract a little... */
/*       micdiff = micdiff - (micdiff >> 4); */
/*     } */

/*     if (micmax < micdiff) { */
/*       micmax = micdiff; */
/*     } */
  }


/*   if (micdiff > 2000) { */
/*     leds_on(LEDS_GREEN); */
/*   } */
/*   if (micdiff > 3000) { */
/*     leds_on(LEDS_YELLOW); */
/*   } */
/*   if (micdiff > 4000) { */
/*     leds_on(LEDS_RED); */
/*   } */

  sound = ADC12MEM4;


  return 0;
}
/*---------------------------------------------------------------------------*/
static void
activate(void)
{
  sound = micdiff = micmax = 0;
  sound_pause = 0;
  avgmax = 5000;
  irq_adc12_activate(&sound_sensor, 4, INCH_0 + SREF_0);
}
/*---------------------------------------------------------------------------*/
static void
deactivate(void)
{
  irq_adc12_deactivate(&sound_sensor, 4);
}
/*---------------------------------------------------------------------------*/
static int
active(void)
{
  return irq_adc12_active(4);
}
/*---------------------------------------------------------------------------*/
static unsigned int
value(int type)
{
  /* try returning the max to see what values we get... */
/*   int mictmp = micmax; */
/*   avgmax = avgmax + micmax - (avgmax >> 3); */
/*   micmax = micdiff; */
/*   return mictmp; */
  return micdiff;
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
SENSORS_SENSOR(sound_sensor, SOUND_SENSOR,
	       init, irq, activate, deactivate, active,
	       value, configure, status);
