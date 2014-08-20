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
 *         ADC interface functions for K60 CPU.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */
#ifndef K60_DRIVERS_ADC_H_
#define K60_DRIVERS_ADC_H_

#include <stdint.h>

typedef uint8_t adc_number_t;

typedef enum adc_cal_result {
  ADC_SUCCESS = 0,
  ADC_INVALID_PARAM,
  ADC_CAL_FAILED,
} adc_error_t;

typedef enum adc_channel {
  ADC_CH_DAD0 = 0b00000,
  ADC_CH_DAD1 = 0b00001,
  ADC_CH_DAD2 = 0b00010,
  ADC_CH_DAD3 = 0b00011,
  ADC_CH_AD4 = 0b00100,
  ADC_CH_AD5 = 0b00101,
  ADC_CH_AD6 = 0b00110,
  ADC_CH_AD7 = 0b00111,
  ADC_CH_AD8 = 0b01000,
  ADC_CH_AD9 = 0b01001,
  ADC_CH_AD10 = 0b01010,
  ADC_CH_AD11 = 0b01011,
  ADC_CH_AD12 = 0b01100,
  ADC_CH_AD13 = 0b01101,
  ADC_CH_AD14 = 0b01110,
  ADC_CH_AD15 = 0b01111,
  ADC_CH_AD16 = 0b10000,
  ADC_CH_AD17 = 0b10001,
  ADC_CH_AD18 = 0b10010,
  ADC_CH_AD19 = 0b10011,
  ADC_CH_AD20 = 0b10100,
  ADC_CH_AD21 = 0b10101,
  ADC_CH_AD22 = 0b10110,
  ADC_CH_AD23 = 0b10111,
  ADC_CH_AD24 = 0b11000,
  ADC_CH_AD25 = 0b11001,
  ADC_CH_AD26 = 0b11010,
  ADC_CH_AD27 = 0b11011,
  ADC_CH_AD28 = 0b11100,
  ADC_CH_AD29 = 0b11101,
  ADC_CH_AD30 = 0b11110,
  ADC_CH_AD31 = 0b11111, /* Module Disabled */
} adc_channel_t;

adc_error_t adc_calibrate(adc_number_t adc_num);

uint16_t adc_read_raw(adc_number_t adc_num, adc_channel_t adc_channel);

#endif /* K60_DRIVERS_ADC_H_ */
