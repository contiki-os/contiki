#include <mc1322x.h>
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

#define DEBUG_MACA 1

void main(void) {
	volatile packet_t *p;

	uart_init(INC,MOD);

	print_welcome("rftest-rx");

	reset_maca();
	radio_init();
	flyback_init();
	vreg_init();
	init_phy();
	free_all_packets();

	/* trim the reference osc. to 24MHz */
	pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

	enable_irq(MACA);

	/* initial radio command */
        /* nop, promiscuous, no cca */
	*MACA_CONTROL = (1 << PRM) | (NO_CCA << MODE); 

	while(1) {		
		if((p = rx_packet())) {
			/* print and free the packet */
			printf("rftest-rx --- ");
			print_packet(p);
			free_packet(p);
		}
	}
}
