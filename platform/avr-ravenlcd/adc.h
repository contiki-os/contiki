/*
 *  Copyright (c) 2008  Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *
 * \brief
 *      Functions to control the ADC of the MCU. This is used to read the
 *      joystick.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define adc_conversion_ongoing (ADCSRA |= (1<<ADSC))
#define adc_conversion_done (!(ADCSRA |= (1<<ADSC)))

/** \brief Lists the different ways in which the ADC can be triggered. */
typedef enum {
    ADC_TRIG_FREE_RUN   =   ((0<<ADTS2)|(0<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_ANACOMP    =   ((0<<ADTS2)|(0<<ADTS1)|(1<<ADTS0)),
    ADC_TRIG_EXTINT0    =   ((0<<ADTS2)|(1<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_TIM0_COMPA =   ((0<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)),
    ADC_TRIG_TIM0_OVF   =   ((1<<ADTS2)|(0<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_TIM1_COMPB =   ((1<<ADTS2)|(0<<ADTS1)|(1<<ADTS0)),
    ADC_TRIG_TIM1_OVF   =   ((1<<ADTS2)|(1<<ADTS1)|(0<<ADTS0)),
    ADC_TRIG_TIM1_CAPT  =   ((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0))
} adc_trig_t;

/** \brief Lists a variety of prescalers used with the ADC. */
typedef enum {
    ADC_PS_2   = ((0<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)),
    ADC_PS_4   = ((0<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)),
    ADC_PS_8   = ((0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)),
    ADC_PS_16  = ((1<<ADPS2)|(0<<ADPS1)|(0<<ADPS0)),
    ADC_PS_32  = ((1<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)),
    ADC_PS_64  = ((1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)),
    ADC_PS_128 = ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))
} adc_ps_t;

/**
 *  \brief Lists the ways in which the voltage reference can be configured
 *   for use with the ADC.
*/
typedef enum {
    ADC_REF_AREF = ((0<<REFS1)|(0<<REFS0)),
    ADC_REF_AVCC = ((0<<REFS1)|(1<<REFS0)),
    ADC_REF_INT  = ((1<<REFS1)|(1<<REFS0))
} adc_ref_t;

/** \brief Lists each channel's mask value for the ADC MUX. */
typedef enum {
    ADC_CHAN_ADC0 = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC1 = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0)),
    ADC_CHAN_ADC2 = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC3 = ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(1<<MUX0)),
    ADC_CHAN_ADC4 = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC5 = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0)),
    ADC_CHAN_ADC6 = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0)),
    ADC_CHAN_ADC7 = ((0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))
} adc_chan_t;

/** \brief Lists the two ADC adjustment values. */
typedef enum {
    ADC_ADJ_RIGHT = 0,
    ADC_ADJ_LEFT = 1
} adc_adj_t;

int adc_init(adc_chan_t chan, adc_trig_t trig, adc_ref_t ref, adc_ps_t prescale);
void adc_deinit(void);
int adc_conversion_start(void);
int16_t adc_result_get(adc_adj_t adjust);

#endif /* ADC_H_ */

