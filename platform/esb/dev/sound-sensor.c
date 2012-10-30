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
 */
#include "contiki.h"
#include "dev/sound-sensor.h"
#include "dev/irq.h"
#include <stdlib.h>

#define MIC_MIN_SENS 150
#define SAMPLE 1

const struct sensors_sensor sound_sensor;

static unsigned int sound, micdiff, micmax, avgmax;
static int8_t mode;
static int8_t sample_div;
static int8_t ctr;
static int16_t *sample_buffer;
static int buffer_size;
static int buf_pos;

/*---------------------------------------------------------------------------*/
static int
irq(void)
{
  micdiff = micdiff + abs(ADC12MEM4 - sound) - (micdiff >> 3);
  sound = ADC12MEM4;

  if(mode == SAMPLE) {
    ctr++;
    if(ctr >= sample_div) {
      ctr = 0;
      sample_buffer[buf_pos++] = sound;
      if(buf_pos >= buffer_size) {
        mode = 0;
        sensors_changed(&sound_sensor);
        return 1;
      }
    }
  }

/*   if(micdiff > MIC_MIN_SENS) { */
/*     sensors_changed(&sound_sensor); */
/*   } */

/*     if(micdiff > (avgmax >> 2)) { */
/*       if(micdiff % 10 == 0) beep_beep(10); */
/*       // Subtract a little... */
/*       micdiff = micdiff - (micdiff >> 4); */
/*     } */

/*     if(micmax < micdiff) { */
/*       micmax = micdiff; */
/*     } */

/*   if(micdiff > 2000) { */
/*     leds_on(LEDS_GREEN); */
/*   } */
/*   if(micdiff > 3000) { */
/*     leds_on(LEDS_YELLOW); */
/*   } */
/*   if(micdiff > 4000) { */
/*     leds_on(LEDS_RED); */
/*   } */

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
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
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    /* Initialization of ADC12 done by irq */
    mode = 0;
    buffer_size = 0;
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!irq_adc12_active(4)) {
        sound = micdiff = micmax = 0;
        mode = 0;
        ctr = 0;
        sample_div = 0;
        buf_pos = 0;
        avgmax = 5000;
        irq_adc12_activate(4, (INCH_0 + SREF_0), irq);
      }
    } else {
      irq_adc12_deactivate(4);
    }
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
    return irq_adc12_active(4);
  case SENSORS_READY:
    return (mode != SAMPLE) && irq_adc12_active(4);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
sound_sensor_start_sample(void)
{
  if(buffer_size > 0) {
    buf_pos = 0;
    ctr = 0;
    mode = SAMPLE;
  }
}
/*---------------------------------------------------------------------------*/
void
sound_sensor_set_buffer(int16_t *buffer, int buf_size, int divider)
{
  sample_buffer = buffer;
  buffer_size = buf_size;
  sample_div = divider;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sound_sensor, SOUND_SENSOR,
               value, configure, status);
