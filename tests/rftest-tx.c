#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

#define DELAY 100000
#define DATA  0x00401000;

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


volatile uint8_t led;
#define LED LED_RED
#define led_on() do  { led = 1; *GPIO_DATA0 = LED; } while(0);
#define led_off() do { led = 0; *GPIO_DATA0 = 0x00000000; } while(0);

void toggle_led(void) {
	if(0 == led) {
		led_on();
		led = 1;

	} else {
		led_off();
	}
}

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

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

        *MACA_CONTROL = SMAC_MACA_CNTL_INIT_STATE;
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
				puts("aborted\n\r");
				ResumeMACASync();				
				break;
				
			}
			case(cc_not_completed):
			{
				puts("not completed\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_timeout):
			{
				puts("timeout\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_no_ack):
			{
				puts("no ack\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_ext_timeout):
			{
				puts("ext timeout\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_ext_pnd_timeout):
			{
				puts("ext pnd timeout\n\r");
				ResumeMACASync();
				break;
				
			}
			case(cc_success):
			{
//				puts("success\n\r");
				
				puts("rftest-tx --- " );
				puts(" payload len+crc: 0x");
				put_hex(PAYLOAD_LEN+4);
				puts(" timestamp: 0x");
				put_hex32(maca_timestamp);
				puts("\n\r");
				puts(" data: ");
				for(i=0; i<PAYLOAD_LEN; i++) {
					put_hex(data[i]);
					putc(' ');
				}
				puts("\n\r");

				toggle_led();

				fill_data();
				command_xcvr_tx();
				
				break;
				
			}
			default:
			{
				puts("status: ");
				put_hex16(status);
				ResumeMACASync();
				
			}
			}
		} else if (_is_filter_failed_interrupt(maca_irq)) {
			puts("filter failed\n\r");
			ResumeMACASync();
		}
		
		for(i=0; i<DELAY; i++) { continue; }

	};
}
