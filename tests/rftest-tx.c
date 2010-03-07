#include <mc1322x.h>
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

#define DELAY 100000
#define DATA  0x00401000
#define DEBUG_MACA 1

uint32_t ackBox[10];

#define command_xcvr_rx() \
	do { \
		maca_txlen = (uint32_t)1<<16;	\
		maca_dmatx = (uint32_t)&ackBox; \
		maca_dmarx = DATA;		     \
		maca_tmren = (maca_cpl_clk | maca_soft_clk);	      \
		maca_control = (control_prm | control_asap | control_seq_rx); \
	}while(0)

#define PAYLOAD_LEN 16 /* not including the extra 4 bytes for len+fcs+somethingelse */
/* maca dmatx needs extra 4 bytes for checksum */
/* needs + 4 bytes for len(1 byte) + fcs(2 bytes) + somethingelse */
#define command_xcvr_tx() \
	do { \
        maca_txlen = (uint32_t)(PAYLOAD_LEN+4); \
        maca_dmatx = (uint32_t)DATA;					\
        maca_dmarx = (uint32_t)&ackBox; \
        maca_control = (control_prm | control_mode_no_cca | \
                        control_asap | control_seq_tx); \
	}while(0)


volatile uint8_t *data;
uint8_t count=0;
void fill_data(void) {
	uint8_t i;
	for(i=0; i<PAYLOAD_LEN; i++) {
		data[i] = count++;
	}		
}

void main(void) {
	volatile uint32_t i;
	uint16_t status;

	*GPIO_PAD_DIR0 = LED;
	led_on();
	
	uart_init(INC,MOD);

	reset_maca();
	radio_init();
	vreg_init();
	flyback_init();
	init_phy();

	/* trim the reference osc. to 24MHz */
	pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

	*MACA_CONTROL = (1 << PRM) | (NO_CCA << MODE);
	for(i=0; i<DELAY; i++) { continue; }

	data = (void *)DATA;
	*MACA_DMARX = DATA; /* put data somewhere */
	*MACA_PREAMBLE = 0;

	/* sets up tx_on, should be a board specific item */
	*GPIO_FUNC_SEL2 = (0x01 << ((44-16*2)*2));
	*GPIO_PAD_DIR0 = *GPIO_PAD_DIR0 | (1<<(44-32));

	fill_data();
	command_xcvr_tx();

	while(1) {		

		if(_is_action_complete_interrupt(maca_irq)) {
			maca_clrirq = maca_irq;
			
			status = *MACA_STATUS & 0x0000ffff;
			switch(status)
			{
			case(cc_aborted):
			{
				printf("aborted\n\r");
				ResumeMACASync();				
				break;
				
			}
			case(cc_not_completed):
			{
				printf("not completed\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_timeout):
			{
				printf("timeout\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_no_ack):
			{
				printf("no ack\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_ext_timeout):
			{
				printf("ext timeout\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_ext_pnd_timeout):
			{
				printf("ext pnd timeout\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_success):
			{
//				printf("success\n\r");
				
				printf("rftest-tx ---  payload len+crc: 0x%02x timestamp: 0x%08x\n\r", PAYLOAD_LEN+4, maca_timestamp);
				printf(" data: ");
				for(i=0; i<PAYLOAD_LEN; i++) {
					printf("%02x ",data[i]);
				}
				printf("\n\r");

				toggle_led();

				fill_data();
				command_xcvr_tx();
				
				break;
				
			}
			default:
			{
				printf("status: 0x%04x",status);
				ResumeMACASync();
				
			}
			}
		} else if (_is_filter_failed_interrupt(maca_irq)) {
			printf("filter failed\n\r");
			ResumeMACASync();
		}
		
		for(i=0; i<DELAY; i++) { continue; }

	};
}
