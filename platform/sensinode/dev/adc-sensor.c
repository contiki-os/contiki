/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         ADC sensor module for sensinode devices.
 *
 *         This file respects configuration in contiki-conf.h. It also turns
 *         off features which are not present in the model that we are
 *         building for.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "dev/sensinode-sensors.h"

#if ADC_SENSOR_ON
SENSORS_SENSOR(adc_sensor, ADC_SENSOR, value, configure, status);

static uint8_t ready;

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint16_t reading;
  /*
   * For single-shot AD conversions, we may only write to ADCCON3[3:0] once
   * (This write triggers the conversion). We thus use the variable 'command'
   * to store intermediate steps (reference, decimation rate, input channel)
   */
  uint8_t command;

  ADCCFG = 0; /* Enables/Disables Input Channel */

  /* 1.25V ref, max decimation rate */
  command = ADEDIV1 | ADEDIV0;

  /* Clear the Interrupt Flag */
  TCON_ADCIF = 0;

  /* Depending on the desired reading, append the input bits to 'command' and
   * enable the corresponding input channel in ADCCFG if necessary */
  switch(type) {
#if TEMP_SENSOR_ON
  case ADC_SENSOR_TYPE_TEMP:
    command |= ADECH3 | ADECH2 | ADECH1;
    break;
#endif
#if ACC_SENSOR_ON
  case ADC_SENSOR_TYPE_ACC_X:
    ADCCFG = ADC5EN;
    command |= ADECH2 | ADECH0;
    break;
  case ADC_SENSOR_TYPE_ACC_Y:
    ADCCFG = ADC6EN;
    command |= ADECH2 | ADECH1;
    break;
  case ADC_SENSOR_TYPE_ACC_Z:
    ADCCFG = ADC7EN;
    command |= ADECH2 | ADECH1 | ADECH0;
    break;
#endif
#if VDD_SENSOR_ON
  case ADC_SENSOR_TYPE_VDD:
    command |= ADECH3 | ADECH2 | ADECH1 | ADECH0;
    break;
#endif
#if LIGHT_SENSOR_ON
  case ADC_SENSOR_TYPE_LIGHT:
    ADCCFG = ADC0EN;
    break;
#endif
#if BATTERY_SENSOR_ON
  case ADC_SENSOR_TYPE_BATTERY:
    ADCCFG = ADC1EN;
    command |= ADECH0 | ADEREF1; /* AVDD_SOC reference */
    break;
#endif
  default:
    /* If the sensor is not present or disabled in conf, return -1 */
    return -1;
  }

  /* Writing in bits 3:0 of ADCCON3 will trigger a single conversion */
  ADCCON3 = command;

  /*
   * When the conversion is complete, the ADC interrupt flag is set. We don't
   * use an ISR here, we just wait on the flag and clear it afterwards.
   */
  while(!TCON_ADCIF);

  /* Clear the Interrupt Flag */
  TCON_ADCIF = 0;

  reading = 0;
  reading = ADCL;
  reading |= (((uint8_t) ADCH) << 8);
  /* 12-bit decimation rate: 4 LS bits are noise */
  reading >>= 4;

  return reading;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
    return ready;
  }
/*---------------------------------------------------------------------------*/
/*
 * On N740 we can control Ill and Acc individually:
 * ADC_VAL_OTHERS     0x01
 * ADC_VAL_LIGHT_ON   0x04
 * ADC_VAL_ACC_ON     0x08
 * ADC_VAL_ACC_GSEL   0x10
 *
 * Return Value is always light | acc | acc_gsel
 *
 * SENSORS_ACTIVE:
 *   - 1: Activate everything, use default setting for ACC G-select
 *   - 0: Turn everything off
 *   - xyz: Mask with the defines above and act accordingly.
 *
 * SENSORS_READY:
 *   - Return Status (0: all off or a value based on the defines above)
 */
static int
configure(int type, int value)
{
#ifdef MODEL_N740
  /*
   * Read current state of the ser-par, ignoring current sensor settings
   * Those will be set all over depending on VALUE
   */
  uint8_t ser_par_val = n740_ser_par_get() & 0xF2;
#endif /* MODEL_N740 */

  /* 'Others' are either compiled in or not. Can't be turned on/off */
  ready = ADC_VAL_ALL;

  switch(type) {
  case SENSORS_HW_INIT:
  case SENSORS_ACTIVE:
#ifdef MODEL_N740
    if(value == ADC_VAL_ALL) {
      value = ADC_VAL_ACC_ON | ADC_VAL_LIGHT_ON;
#if ACC_SENSOR_GSEL
      value |= ADC_VAL_ACC_GSEL;
#endif /* ACC_SENSOR_GSEL */
    }
#endif /* MODEL_N740 */

    /* OK, Now value definitely specifies our bits, start masking
     * We will refuse to turn things on if they are specified OFF in conf. */
#ifdef MODEL_N740
#if ACC_SENSOR_ON
    if(value & ADC_VAL_ACC_ON) {
      P0SEL |= 0x80 | 0x40 | 0x20;
      ser_par_val |= N740_SER_PAR_ACC;
      ready |= ADC_VAL_ACC_ON;
#if ACC_SENSOR_GSEL
      if(value & ADC_VAL_ACC_GSEL) {
        ser_par_val |= N740_SER_PAR_ACC_GSEL;
        ready |= ADC_VAL_ACC_GSEL;
      }
#endif /*ACC_SENSOR_GSEL */
    }
#endif /* ACC_SENSOR_ON */

#if LIGHT_SENSOR_ON
    if(value & ADC_VAL_LIGHT_ON) {
      ser_par_val |= N740_SER_PAR_LIGHT;
      ready |= ADC_VAL_LIGHT_ON;
    }
#endif /* LIGHT_SENSOR_ON */
      n740_ser_par_set(ser_par_val);
#endif /* MODEL_N740 */
    }
  return ready;
}

#endif /* ADC_SENSOR_ON */
