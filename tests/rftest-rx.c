#include <mc1322x.h>
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

#define LED LED_GREEN

void maca_rx_callback(volatile packet_t *p) {
	(void)p;
	gpio_data_set(1ULL<< LED);
	gpio_data_reset(1ULL<< LED);
}

void main(void) {
	volatile packet_t *p;

	gpio_data(0);
	
	gpio_pad_dir_set( 1ULL << LED );
        /* read from the data register instead of the pad */
	/* this is needed because the led clamps the voltage low */
	gpio_data_sel( 1ULL << LED);

	/* trim the reference osc. to 24MHz */
	trim_xtal();

	uart_init(INC, MOD, SAMP);

	vreg_init();

	maca_init();

        /* sets up tx_on, should be a board specific item */
	//       *GPIO_FUNC_SEL2 = (0x01 << ((44-16*2)*2));
	gpio_pad_dir_set( 1ULL << 44 );

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

	print_welcome("rftest-rx");
	while(1) {		

		/* call check_maca() periodically --- this works around */
		/* a few lockup conditions */
		check_maca();

		if((p = rx_packet())) {
			/* print and free the packet */
			printf("rftest-rx --- ");
			print_packet(p);
			free_packet(p);
		}
	}
}
