#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

#define LED LED_YELLOW

void main(void) {

	/* pin direction */
	gpio_pad_dir_set(LED);

	/* timer setup */
	/* CTRL */
#define COUNT_MODE 1      /* use rising edge of primary source */
#define PRIME_SRC  0xf    /* Perip. clock with 128 prescale (for 24Mhz = 187500Hz)*/
#define SEC_SRC    0      /* don't need this */
#define ONCE       0      /* keep counting */
#define LEN        1      /* count until compare then reload with value in LOAD */
#define DIR        0      /* count up */
#define CO_INIT    0      /* other counters cannot force a re-initialization of this counter */
#define OUT_MODE   0      /* OFLAG is asserted while counter is active */

	*TMR_ENBL = 0;                     /* tmrs reset to enabled */
	*TMR0_SCTRL = 0;
	*TMR0_LOAD = 0;                    /* reload to zero */
	*TMR0_COMP_UP = 18750;             /* trigger a reload at the end */
	*TMR0_CMPLD1 = 18750;              /* compare 1 triggered reload level, 10HZ maybe? */
	*TMR0_CNTR = 0;                    /* reset count register */
	*TMR0_CTRL = (COUNT_MODE<<13) | (PRIME_SRC<<9) | (SEC_SRC<<7) | (ONCE<<6) | (LEN<<5) | (DIR<<4) | (CO_INIT<<3) | (OUT_MODE);
	*TMR_ENBL = 0xf;                   /* enable all the timers --- why not? */

	while(1) {

		/* blink on */
		gpio_data_set(LED);
		
		while((*TMR0_SCTRL >> 15) == 0) { continue; }
		*TMR0_SCTRL = 0; /*clear bit 15, and all the others --- should be ok, but clearly not "the right thing to do" */

		/* blink off */
		gpio_data_set(0);

		while((*TMR0_SCTRL >> 15) == 0) { continue; }
		*TMR0_SCTRL = 0; /*clear bit 15, and all the others --- should be ok, but clearly not "the right thing to do" */

	};
}
