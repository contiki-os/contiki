#include <mc1322x.h>
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

#define LED LED_RED

/* 802.15.4 PSDU is 127 MAX */
/* 2 bytes are the FCS */
/* therefore 125 is the max payload length */
#define PAYLOAD_LEN 16
#define DELAY 100000

void fill_packet(volatile packet_t *p) {
	static volatile uint8_t count=0;
	volatile uint8_t i;
	p->length = PAYLOAD_LEN;
	p->offset = 0;
	for(i=0; i<PAYLOAD_LEN; i++) {
		p->data[i] = count++;
	}

	/* acks get treated differently, even in promiscuous mode */
	/* setting the third bit makes sure that we never send an ack */
        /* or any valid 802.15.4-2006 packet */
	p->data[0] |= (1 << 3); 
}

void main(void) {
	volatile uint32_t i;
	volatile packet_t *p;

	/* trim the reference osc. to 24MHz */
	trim_xtal();

	uart_init(INC, MOD, SAMP);

	vreg_init();

	maca_init();

	set_channel(0); /* channel 11 */
//	set_power(0x0f); /* 0xf = -1dbm, see 3-22 */
//	set_power(0x11); /* 0x11 = 3dbm, see 3-22 */
	set_power(0x12); /* 0x12 is the highest, not documented */

        /* sets up tx_on, should be a board specific item */
        *GPIO_FUNC_SEL2 = (0x01 << ((44-16*2)*2));
	gpio_pad_dir_set( 1ULL << 44 );

	print_welcome("rftest-tx");

	while(1) {		
	    		
		/* call check_maca() periodically --- this works around */
		/* a few lockup conditions */
		check_maca();

		p = get_free_packet();
		if(p) {
			fill_packet(p);
			
			printf("rftest-tx --- ");
			print_packet(p);

			tx_packet(p);
			
			for(i=0; i<DELAY; i++) { continue; }
		}
		
	}

}
