/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         ADC interface function implementation for K60 CPU.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "adc.h"
#include "K60.h"

static inline ADC_Type *
adc_num_to_ptr(adc_number_t adc_num)
{
  switch(adc_num) {
  case 0:
    return ADC0;
    break;
  case 1:
    return ADC1;
    break;
  default:
    return 0;
    break;
  }
}

adc_error_t
adc_calibrate(adc_number_t adc_num)
{
  uint16_t cal;
  ADC_Type *ADC_ptr;

  ADC_ptr = adc_num_to_ptr(adc_num);
  if(!ADC_ptr) {
    return ADC_INVALID_PARAM;
  }

  ADC_ptr->SC3 |= ADC_SC3_CAL_MASK;
  while(ADC_ptr->SC3 & ADC_SC3_CAL_MASK); /* wait for calibration to finish */
  while(!(ADC_ptr->SC1[0] & ADC_SC1_COCO_MASK));
  if(ADC_ptr->SC3 & ADC_SC3_CALF_MASK) {
    /* calibration failed for some reason, possibly SC2[ADTRG] is 1 ? */
    return ADC_CAL_FAILED;
  }

  /*
   * Following the steps in the reference manual:
   */
  /* 1. Initialize or clear a 16-bit variable in RAM. */
  /* 2. Add the plus-side calibration results CLP0, CLP1, CLP2, CLP3, CLP4, and
   * CLPS to the variable. */
  cal = ADC_ptr->CLP0 + ADC_ptr->CLP1 + ADC_ptr->CLP2 + ADC_ptr->CLP3 +
    ADC_ptr->CLP4 + ADC_ptr->CLPS;
  /* 3. Divide the variable by two. */
  cal /= 2;
  /* 4. Set the MSB of the variable. */
  cal |= (1 << 15);
  /* 5. The previous two steps can be achieved by setting the carry bit,
   * rotating to the right through the carry bit on the high byte and again on
   * the low byte.
   * Don't care about the above optimization, we only do this once on startup
   * anyway... */
  /* 6. Store the value in the plus-side gain calibration register PG. */
  ADC_ptr->PG = cal;

  /* 7. Repeat the procedure for the minus-side gain calibration value. */
  cal = ADC_ptr->CLM0 + ADC_ptr->CLM1 + ADC_ptr->CLM2 + ADC_ptr->CLM3 +
    ADC_ptr->CLM4 + ADC_ptr->CLMS;
  cal /= 2;
  cal |= (1 << 15);
  ADC_ptr->MG = cal;

  return ADC_SUCCESS;
}
uint16_t
adc_read_raw(adc_number_t adc_num, adc_channel_t adc_channel)
{
  ADC_Type *ADC_ptr;

  ADC_ptr = adc_num_to_ptr(adc_num);
  if(!ADC_ptr) {
    return ADC_INVALID_PARAM;
  }
  ADC_ptr->SC1[0] = ADC_SC1_ADCH((uint8_t)adc_channel); /* Select the correct channel and initiate a conversion */

  /* Wait for the conversion to finish */
  while(!((ADC_ptr->SC1[0]) & ADC_SC1_COCO_MASK));

  return ADC_ptr->R[0];
}
