/*
 * Copyright (c) 2016, University of Bristol - http://www.bristol.ac.uk
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-adc-sensor
 * @{
 *
 * \file
 * Driver for the CC13xx/CC26xx ADC
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/sensors.h"
#include "dev/adc-sensor.h"
#include "gpio-interrupt.h"
#include "sys/timer.h"
#include "lpm.h"

#include "ti-lib.h"
#include "driverlib/aux_adc.h"
#include "aux-ctrl.h"

#include <stdio.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
static uint8_t channel = ADC_COMPB_IN_AUXIO0;
static bool is_active = false;

static aux_consumer_module_t adc_aux = {
  .clocks = AUX_WUC_ADI_CLOCK | AUX_WUC_ANAIF_CLOCK | AUX_WUC_SMPH_CLOCK
};
/*---------------------------------------------------------------------------*/
static int
config(int type, int c)
{
  switch(type) {
  case SENSORS_ACTIVE:
    is_active = c;

    if(is_active) {
      /* Request AUX access, with ADI and SMPH clocks */
      aux_ctrl_register_consumer(&adc_aux);

      ti_lib_aux_adc_select_input(channel);
    } else {
      aux_ctrl_unregister_consumer(&adc_aux);
    }
    break;

  case ADC_SENSOR_SET_CHANNEL:
    channel = c;
    if(is_active) {
      ti_lib_aux_adc_select_input(channel);
    }
    break;

  default:
    break;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
    if(is_active) {
      return 1;
    }
    break;
  default:
    break;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if(type == ADC_SENSOR_VALUE) {
    int val;

    if(!is_active) {
      puts("ADC not active");
      return 0;
    }

    ti_lib_aux_adc_enable_sync(AUXADC_REF_FIXED, AUXADC_SAMPLE_TIME_2P7_US,
                               AUXADC_TRIGGER_MANUAL);

    ti_lib_aux_adc_gen_manual_trigger();
    val = ti_lib_aux_adc_read_fifo();

    ti_lib_aux_adc_disable();

    return val;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(adc_sensor, ADC_SENSOR, value, config, status);
/*---------------------------------------------------------------------------*/
/** @} */
