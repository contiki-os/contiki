#include <mc1322x.h>
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

#define DELAY 400000
#define DATA  0x00401000;

uint32_t ackBox[10];

#define MAX_PAYLOAD 128
volatile uint8_t data[MAX_PAYLOAD];
/* maca_rxlen is very important */
#define command_xcvr_rx() \
	do { \
	maca_txlen = ((0xff)<<16); \
		maca_dmatx = (uint32_t)&ackBox; \
		maca_dmarx = (uint32_t)data;			      \
		maca_tmren = (maca_cpl_clk | maca_soft_clk);	      \
		maca_control = (control_prm | control_asap | control_seq_rx); \
	}while(0)

#define LED LED_GREEN

#define led_on() do  { led = 1; *GPIO_DATA0 = LED; } while(0);
#define led_off() do { led = 0; *GPIO_DATA0 = 0x00000000; } while(0);

volatile uint8_t led;
void toggle_led(void) {
	if(0 == led) {
		led_on();
		led = 1;

	} else {
		led_off();
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
	flyback_init();
	vreg_init();
	init_phy();

	/* trim the reference osc. to 24MHz */
	pack_XTAL_CNTL(CTUNE_4PF, CTUNE, FTUNE, IBIAS);

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

        *MACA_CONTROL = SMAC_MACA_CNTL_INIT_STATE;
	for(i=0; i<DELAY; i++) { continue; }

	*MACA_DMARX = (uint32_t)data; /* put data somewhere */
	*MACA_PREAMBLE = 0;

	command_xcvr_rx();

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

				printf("rftest-rx --- maca_getrxlvl: 0x%02x timestamp: 0x%08x\n\r", maca_getrxlvl, maca_timestamp);
				printf(" data: 0x%08x ", (uint32_t)data);
				for(i=0; i<=(*MACA_GETRXLVL-4); i++) { /* fcs+somethingelse is not transferred by DMA */
					printf("%02x ", data[i]);
				}				
				printf("\n\r");

				toggle_led();

				ResumeMACASync();

				command_xcvr_rx();
				
				break;
				
			}
			default:
			{
				printf("status: %04x\n\r",status);
				ResumeMACASync();
				command_xcvr_rx();
				
			}
			}
		} else if (_is_filter_failed_interrupt(maca_irq)) {
			printf("filter failed\n\r");
			ResumeMACASync();
			command_xcvr_rx();
		} else if (_is_checksum_failed_interrupt(maca_irq)) {
			printf("crc failed\n\r");
			ResumeMACASync();
			command_xcvr_rx();
		}

		
	};
}
