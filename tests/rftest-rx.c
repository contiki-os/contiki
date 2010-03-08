#include <mc1322x.h>
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

#define LED LED_GREEN

void maca_rx_callback(volatile packet_t *p) {
	(void)p;
	toggle_gpio0(LED);
	toggle_gpio0(LED);
}

void main(void) {
	volatile packet_t *p;

	*GPIO_DATA0 = 0x00000000;
	*GPIO_PAD_DIR0 = ( 1 << LED );
        /* read from the data register instead of the pad */
	/* this is needed because the led clamps the voltage low */
	*GPIO_DATA_SEL0 = ( 1 << LED ); 

	/* trim the reference osc. to 24MHz */
	pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);

	uart_init(INC,MOD);

	vreg_init();

	maca_init();

        /* sets up tx_on, should be a board specific item */
        *GPIO_FUNC_SEL2 = (0x01 << ((44-16*2)*2));
	*GPIO_PAD_DIR_SET1 = (1 << (44-32));

        /* sets up rx_on, should be a board specific item */
        *GPIO_FUNC_SEL2 = (0x02 << ((45-16*2)*2));
	*GPIO_PAD_DIR_SET1 = (1 << (45-32));

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

	print_welcome("rftest-rx");
	while(1) {		
		if((p = rx_packet())) {
			/* print and free the packet */
			printf("rftest-rx --- ");
			print_packet(p);
			free_packet(p);
		}
	}
}
