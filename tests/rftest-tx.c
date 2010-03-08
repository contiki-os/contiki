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
#define DELAY 400000

void fill_packet(volatile packet_t *p) {
	static volatile uint8_t count=0;
	volatile uint8_t i;
	p->length = PAYLOAD_LEN;
	p->offset = 0;
	p->data[0] = 0xff;
	for(i=1; i<PAYLOAD_LEN; i++) {
		p->data[i] = count++;
	}		
}

void main(void) {
	volatile uint32_t i;
	volatile packet_t *p;

	/* trim the reference osc. to 24MHz */
	pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);

	uart_init(INC,MOD);

	vreg_init();

	maca_init();

	set_channel(0); /* channel 11 */
//	set_power(0x0f); /* 0xf = -1dbm, see 3-22 */
//	set_power(0x11); /* 0x11 = 3dbm, see 3-22 */
	set_power(0x12); /* 0x12 is the highest, not documented */

	*GPIO_DATA0 = 0x00000000;
	*GPIO_PAD_DIR0 = ( 1 << LED );
        /* read from the data register instead of the pad */
	/* this is needed because the led clamps the voltage low */
	*GPIO_DATA_SEL0 = ( 1 << LED ); 

        /* sets up tx_on, should be a board specific item */
        *GPIO_FUNC_SEL2 = (0x01 << ((44-16*2)*2));
        *GPIO_PAD_DIR0 = *GPIO_PAD_DIR0 | (1<<(44-32));

	print_welcome("rftest-tx");

	while(1) {		
	    		
		p = get_free_packet();
		if(p) {
			fill_packet(p);
			
			toggle_gpio0(LED);
			printf("rftest-tx --- ");
			print_packet(p);
			
			tx_packet(p);
			
			for(i=0; i<DELAY; i++) { continue; }
		}
		
	}

}
