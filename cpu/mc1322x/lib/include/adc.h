/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details.
 *
 *
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "utils.h"

/* the Vbatt measurment reads about 200mV low --- trim by ADC_VBATT_TRIM */
/* correction tracks well --- within 50mV over 2.1V to 3.6V */
/* offset from correct for tags running from 3.29 vreg */
/* trim = 146 */
/* tag 1: -90mV */
/* tag 2: -30mV */
/* tag 3: -30mV */
/* tag 4: -40mV */
/* tag 5: +10mV */
/* tag 6: -40mV */
/* new trim 183 */

/* without per unit calibration, vbatt is probably +/- 75mV */
#define ADC_VBATT_TRIM 183

/* ADC registers are all 16-bit wide with 16-bit access only */
#define ADC_BASE        (0x8000D000)

/* Structure-based register definitions */

struct ADC_struct {
	union {
		uint16_t COMP[8];
		struct {
			uint16_t COMP_0;
			uint16_t COMP_1;
			uint16_t COMP_2;
			uint16_t COMP_3;
			uint16_t COMP_4;
			uint16_t COMP_5;
			uint16_t COMP_6;
			uint16_t COMP_7;
		};
	};
	uint16_t BAT_COMP_OVER;
	uint16_t BAT_COMP_UNDER;
	union {
		uint16_t SEQ_1;
		struct ADC_SEQ_1 {
			uint16_t CH0:1;
			uint16_t CH1:1;
			uint16_t CH2:1;
			uint16_t CH3:1;
			uint16_t CH4:1;
			uint16_t CH5:1;
			uint16_t CH6:1;
			uint16_t CH7:1;
			uint16_t BATT:1;
			uint16_t :6;
			uint16_t SEQ_MODE:1;
		} SEQ_1bits;
	};
	union {
		uint16_t SEQ_2;
		struct ADC_SEQ_2 {
			uint16_t CH0:1;
			uint16_t CH1:1;
			uint16_t CH2:1;
			uint16_t CH3:1;
			uint16_t CH4:1;
			uint16_t CH5:1;
			uint16_t CH6:1;
			uint16_t CH7:1;
			uint16_t :7;
			uint16_t SEQ_MODE:1;
		} SEQ_2bits;
	};
	union {
		uint16_t CONTROL;
		struct ADC_CONTROL {
			uint16_t ON:1;
			uint16_t TIMER1_ON:1;
			uint16_t TIMER2_ON:1;
			uint16_t SOFT_RESET:1;
			uint16_t AD1_VREFHL_EN:1;
			uint16_t AD2_VREFHL_EN:1;
			uint16_t :6;
			uint16_t COMPARE_IRQ_MASK:1;
			uint16_t SEQ1_IRQ_MASK:1;
			uint16_t SEQ2_IRQ_MASK:1;
			uint16_t FIFO_IRQ_MASK:1;
		} CONTROLbits;
	};
	uint16_t TRIGGERS;
	uint16_t PRESCALE;
	uint16_t reserved1;
	uint16_t FIFO_READ;
	uint16_t FIFO_CONTROL;
	union {
		uint16_t FIFO_STATUS;
		struct ADC_FIFO_STATUS {
			uint16_t LEVEL:4;
			uint16_t FULL:1;
			uint16_t EMPTY:1;
			uint16_t :10;
		} FIFO_STATUSbits;
	};
	uint16_t reserved2[5];
	uint16_t SR_1_HIGH;
	uint16_t SR_1_LOW;
	uint16_t SR_2_HIGH;
	uint16_t SR_2_LOW;
	uint16_t ON_TIME;
	uint16_t CONVERT_TIME;
	uint16_t CLOCK_DIVIDER;
	uint16_t reserved3;
	union {
		uint16_t OVERRIDE;
		struct ADC_OVERRIDE {
			uint16_t MUX1:4;
			uint16_t MUX2:4;
			uint16_t AD1_ON:1;
			uint16_t AD2_ON:1;
			uint16_t :6;
		} OVERRIDEbits;
	};
	uint16_t IRQ;
	uint16_t MODE;
	uint16_t RESULT_1;
	uint16_t RESULT_2;
};

static volatile struct ADC_struct * const ADC = (void *) (ADC_BASE);

#define NUM_ADC_CHAN 9

#define adc_enable()  (ADC->CONTROLbits.ON = 1)
#define adc_disable() (ADC->CONTROLbits.ON = 0)
#define adc_select_channels(chans) (ADC->SEQ_1 = (ADC->SEQ_1 & 0xFE00) | chans)
void adc_setup_chan(uint8_t channel);

extern uint16_t adc_reading[NUM_ADC_CHAN];
/* use the internal reference to return adc_readings in mV */
#define adc_voltage(x) (adc_reading[x] * 1200/adc_reading[8]) 
/* return vbatt voltage in mV */
#define adc_vbatt 4095 * 1200/adc_reading[8] + ADC_VBATT_TRIM

void ADC_flush(void);
uint16_t ADC_READ(void);
void read_scanners(void);
void adc_init(void);
void adc_service(void);

#endif
