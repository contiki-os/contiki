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

#include <stdint.h>
#include "crm.h"
#include "adc.h"
#include "gpio-util.h"

//#define ADC_CHANS_ENABLED 0x3F // channels 0-5 enabled
//#define ADC_CHANS_ENABLED 0x7E // channels 1-6 enabled
//#define ADC_CHANS_ENABLED (1 << 6) // only channel 6 enabled
#define ADC_CHANS_ENABLED 0x1FF // all channels, including battery reference voltage

//#define ADC_PRESCALE_VALUE 24 // divide by 24 for 1MHz.

#define ADC_SAMPLE_FREQ 400 // Hz (minimum of ~366.21 Hz)
#define ADC_PRESCALE_VALUE (REF_OSC / ADC_SAMPLE_FREQ)

#define ADC_USE_TIMER      0
#define ADC_USE_INTERRUPTS 0 // incomplete support

uint16_t adc_reading[NUM_ADC_CHAN];

void ADC_flush(void) {
	while(ADC->FIFO_STATUSbits.EMPTY == 0) ADC->FIFO_READ;
}

uint16_t ADC_READ(void) {
	while(ADC->FIFO_STATUSbits.EMPTY); // loop while empty
	return ADC->FIFO_READ; // upper 4 bits are channel number
}


void adc_service(void) {
	uint16_t value;
	uint8_t channel;
	while (ADC->FIFO_STATUSbits.EMPTY == 0) {
		value = ADC->FIFO_READ;
		channel = value >> 12;
		if (channel < NUM_ADC_CHAN) adc_reading[channel] = value & 0xFFF;
	}
}

void adc_init(void) {
	uint8_t n;

	ADC->CLOCK_DIVIDER = 80;       // 300 KHz
	ADC->PRESCALE      = ADC_PRESCALE_VALUE - 1;       // divide by 24 for 1MHz.

	ADC->CONTROL       = 1;

	// The ON-TIME must always be 10µs or greater - typical On-Time value = 0x000A (10dec)
	ADC->ON_TIME       = 10;

	/*
	NOTE
	The ADC analog block requires 6 ADC_Clocks per conversion, and the
	ADC_Clock must 300kHz or less. With 6 clocks/conversion and a 33.33µs
	clock period:
	* The ADC convert time must always be 20µs or greater
	* If the ADC_Clock is a frequency lower than 300kHz, the convert time
	must always be 6 ADC_Clock periods or greater
	* For override mode, extend convert time to 40µs minimum or greater
	For the convert time:
	* This delay is a function of the Prescale Clock (typically 1 MHz)
	* The register must be initialized for proper operation
	* For a 20µs convert time with 1MHz, value = 0x0014 (20dec)
	* If convert time is insufficient, inaccurate sample data will result
	*/
	ADC->CONVERT_TIME  = 1000000 / (115200 / 8) - 1;

	ADC->MODE          = 0;        // Automatic

#if ADC_USE_INTERRUPTS
	ADC->FIFO_CONTROL  = 7;
#else
	ADC->FIFO_CONTROL  = 0;
#endif

#if ADC_USE_TIMER
	ADC->SR_1_HIGH       = 0x0000;
	ADC->SR_1_LOW        = (REF_OSC / ADC_PRESCALE_VALUE) / (115200 / 8) + 1;
#endif

	ADC->SEQ_1         = 0
#if ADC_USE_TIMER
		| (1 << 15) // sequence based on Timer 1.
#else
		| (0 << 15) // sequence based on convert time.
#endif
		| ADC_CHANS_ENABLED;

	ADC->CONTROL       = 0xF001
//#if ADC_USE_TIMER
		| (1 << 1) // Timer 1 enable
//#endif
	;
	ADC->OVERRIDE     = (1 << 8);

	for (n=0; n<=8; n++) {
		if ((ADC_CHANS_ENABLED >> n) & 1) {
			gpio_select_function(30 + n, 1); // Function 1 = ADC
			gpio_set_pad_dir(30 + n, PAD_DIR_INPUT);
		}
	}
}
