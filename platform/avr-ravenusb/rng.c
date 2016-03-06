//	ADC-based strong RNG
//	Very slow, but who cares---if you need fast random numbers, use a PRNG.

#include "rng.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "contiki.h"

#ifndef RNG_CONF_USE_ADC
#define RNG_CONF_USE_ADC			(!RNG_CONF_USE_RADIO_CLOCK && defined(ADMUX) && defined(ADCSRA) && defined(ADCSRB) && defined(ADSC) && defined(ADEN))
#endif

#ifndef RNG_CONF_USE_RADIO_CLOCK
#define RNG_CONF_USE_RADIO_CLOCK	((!RNG_CONF_USE_ADC) && RF230BB)
#endif

/* delay_us uses floating point which includes (in some avr-gcc's) a 256 byte __clz_tab in the RAM through the .data section. */
/* _delay_loop_1 avoids this, it is 3 CPU cycles per loop, 375ns @ 8MHz */
//#define TEMPORAL_AGITATION()		do { static uint8_t agitator; agitator*=97; agitator+=101; _delay_us(agitator>>1); } while (0);
#define TEMPORAL_AGITATION()		do { static uint8_t agitator; agitator*=97; agitator+=101; _delay_loop_1(agitator>>1); } while (0);


// -------------------------------------------------------------------------
#if RNG_CONF_USE_ADC
/*	The hope is that there is enough noise in the LSB when pointing the
**	ADC at the internal band-gap input and using the internal 2.56v
**	AREF.
**
**	TODO: Run some randomness tests on the output of this RNG!
*/

#define BITS_TO_SHIFT		9

#define ADC_CHAN_ADC1 ((0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0))
#define ADC_CHAN_BAND_GAP ((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0))
#define ADC_REF_AREF  ((0<<REFS1)|(0<<REFS0))
#define ADC_REF_AVCC  ((0<<REFS1)|(1<<REFS0))
#define ADC_REF_INT   ((1<<REFS1)|(1<<REFS0))
#define ADC_TRIG_FREE_RUN  ((0<<ADTS2)|(0<<ADTS1)|(0<<ADTS0))
#define ADC_PS_128  ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))
#define ADC_PS_2  ((0<<ADPS2)|(0<<ADPS1)|(1<<ADPS0))

#ifndef CONTIKI_CONF_RNG_ADC_CHANNEL
#define CONTIKI_CONF_RNG_ADC_CHANNEL	ADC_CHAN_BAND_GAP
#endif

#ifndef CONTIKI_CONF_RNG_ADC_REF
#define CONTIKI_CONF_RNG_ADC_REF	ADC_REF_INT
#endif

static uint8_t
extract_random_bit_() {
	uint8_t ret = 0;

	// Store the state so that we can restore it when we are done.
	uint8_t sreg = SREG;
	uint8_t adcsra = ADCSRA;
	uint8_t admux = ADMUX;
	uint8_t adcsrb = ADCSRB;
#ifdef PRR
	uint8_t prr = PRR;
#endif

	// Disable interrupts
	cli();

#ifdef PRR
	// Enable ADC module
	PRR &= ~(1 << PRADC);
#endif

	// Wait for any ADC conversion which
	// might currently be happening to finish.
	while(ADCSRA & (1<<ADSC));

	// Configure the ADC module
	ADCSRA = (1<<ADEN)|ADC_PS_128;
	ADMUX = (uint8_t)CONTIKI_CONF_RNG_ADC_REF|(uint8_t)CONTIKI_CONF_RNG_ADC_CHANNEL;
	ADCSRB = ADC_TRIG_FREE_RUN;

	// This loop is where we try to come up with our
	// random bit. Unfortunately, the time it takes
	// for this to happen is non-deterministic, but
	// the result should be non-biased random bit.
	do {
		// Start conversion for first bit
		ADCSRA |= (1<<ADSC);
		// Wait for conversion to complete.
		while(ADCSRA & (1<<ADSC));
		ret = (ADC&1);
		ret <<= 1;

		// Start conversion for second bit
		ADCSRA |= (1<<ADSC);
		// Wait for conversion to complete.
		while(ADCSRA & (1<<ADSC));
		ret |= (ADC&1);

		// Toggling the reference voltage
		// seems to help introduce noise.
		ADMUX^=(1<<REFS1);

		// We only want to exit the loop if the first
		// and second sampled bits are different.
		// This is preliminary conditioning.
	} while((ret==0)||(ret==3));

	// Toss out the other bit, we only care about one of them.
	ret &= 1;

	ADCSRA=0;

	// Restore the state
	ADCSRB = adcsrb;
	ADMUX = admux;
	ADCSRA = adcsra;
#ifdef PRR
	PRR = prr;
#endif
	SREG = sreg;

	return ret;
}

// -------------------------------------------------------------------------
#elif RNG_CONF_USE_RADIO_CLOCK
/*	Here we are hoping to find some noise in the clock skew
**	of two different oscilating crystals. On the RZUSBstick,
**	there are two such crystals: An 8MHz crystal for the
**	microcontroller, and a 16MHz crystal and for the radio.
**	The MCLK pin of the RF230 chip is conveniently connected
**	to pin 6 of port D. First we need to have the radio
**	output the 16MHz signal (it defaults to 1MHz), and then
**	we can try to find some noise by sampling pin 6 of port D.
**
**	The suitability of this method as a real random number
**	generator has yet to be determined. It is entirely possible
**	that the perceived randomness of the output is due to
**	the temporal agitator mechanism that I have employed.
**	Use with caution!
**
**	TODO: Run some randomness tests on the output of this RNG!
*/

#define BITS_TO_SHIFT		8

#include "radio/rf230bb/hal.h"
#include "radio/rf230bb/at86rf230_registermap.h"

#ifndef TRX_CTRL_0
#define TRX_CTRL_0 0x03
#endif

static uint8_t
extract_random_bit_() {
	uint8_t ret;
	uint8_t trx_ctrl_0 = hal_register_read(TRX_CTRL_0);

	// Set radio clock output to 8MHz
	hal_register_write(TRX_CTRL_0,0x8|5);

	do {
		TEMPORAL_AGITATION(); // WARNING: This step may hide lack of entropy!

		ret = !!(PIND&(1<<6));
		ret <<= 1;
		ret |= !!(PIND&(1<<6));
	} while((ret==0)||(ret==3));

	// Toss out the other bit, we only care about one of them.
	ret &= 1;

	// Restore the clkm state
	hal_register_write(TRX_CTRL_0,trx_ctrl_0);

	return ret;
}

#endif

// -------------------------------------------------------------------------

static uint8_t
extract_random_bit() {
	uint8_t ret;

	// These next two lines attempt to sync ourselves to
	// any pattern that might happen to be present in the
	// raw random source stream. After this, we use the
	// bias removal mechanism below to filter out the first
	// sign of noise.
	while(extract_random_bit_()==1);
	while(extract_random_bit_()==0);

	do {
		ret = extract_random_bit_();
		ret <<= 1;
		ret |= extract_random_bit_();
	} while((ret==0)||(ret==3));

	// Toss out the other bit, we only care about one of them.
	ret &= 1;

	return ret;
}

uint8_t
rng_get_uint8() {
	uint8_t ret = 0, i;
	for(i=0;i<BITS_TO_SHIFT;i++) {
		// Leftshift.
		ret <<= 1;

		// Add a random bit.
		ret |= extract_random_bit();
	}
	return ret;
}

