#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */

#define BASE_UART1      0x80005000
#define UART1_CON       0x80005000
#define UART1_STAT      0x80005004
#define UART1_DATA      0x80005008
#define UR1CON          0x8000500c
#define UT1CON          0x80005010
#define UART1_CTS       0x80005014
#define UART1_BR        0x80005018

#define GPIO_PAD_DIR0   0x80000000
#define GPIO_DATA0      0x80000008

#include "maca.h"
#include "embedded_types.h"
#include "isr.h"

#define reg(x) (*(volatile uint32_t *)(x))

#define DELAY 400000
#define DATA  0x00401000;

#define NL "\033[K\r\n"

void putc(uint8_t c);
void puts(uint8_t *s);
void put_hex(uint8_t x);
void put_hex16(uint16_t x);
void put_hex32(uint32_t x);

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

uint32_t ackBox[10];

#define MAX_PAYLOAD 128
volatile uint8_t data[MAX_PAYLOAD];
/* maca_rxlen is very important */
#define command_xcvr_rx() \
	do { \
	maca_txlen = ((0xff)<<16); \
		maca_dmatx = (uint32_t)&ackBox; \
		maca_dmarx = data; \
		maca_tmren = (maca_cpl_clk | maca_soft_clk);	      \
		maca_control = (control_prm | control_asap | control_seq_rx); \
	}while(FALSE)


void dump_regs(uint32_t base, uint32_t len) {
	volatile uint32_t i;
    
	puts("base +0       +4       +8       +c       +10      +14      +18      +1c      \n\r");                                                     
	for (i = 0; i < len; i ++) {                                                                                                                   
		if ((i & 7) == 0) {                                                                                                                   
			put_hex16(4 * i);                                                                                                             
		}                                                                                                                                     
		puts(" ");                                                                                                                            
		put_hex32(reg(base+(4*i)));                                                                                                      
		if ((i & 7) == 7)                                                                                                                     
			puts(NL);                                                                                                                     
	}                                                                                                                                             
	puts(NL); 
}

volatile uint8_t led;

#include "led.h"
#define LED LED_GREEN

#define led_on() do  { led = 1; reg(GPIO_DATA0) = LED; } while(0);
#define led_off() do { led = 0; reg(GPIO_DATA0) = 0x00000000; } while(0);

void toggle_led(void) {
	if(0 == led) {
		led_on();
		led = 1;

	} else {
		led_off();
	}
}

__attribute__ ((section ("startup")))
void main(void) {
	uint8_t c;
	volatile uint32_t i;
	uint32_t tmp;
	uint16_t status;

	*(volatile uint32_t *)GPIO_PAD_DIR0 = LED;
	led_on();
	
	/* Restore UART regs. to default */
	/* in case there is still bootloader state leftover */

	reg(UART1_CON) = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */

	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	#define INC 767
	#define MOD 9999
	reg(UART1_BR) = INC<<16 | MOD; 

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	reg(UART1_CON) = 0x00000003; /* enable receive and transmit */
	reg(GPIO_FUNC_SEL0) = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	reset_maca();
	radio_init();
	flyback_init();
	vreg_init();
	init_phy();

	set_power(0x0f); /* 0dbm */
	set_channel(0); /* channel 11 */

        reg(MACA_CONTROL) = SMAC_MACA_CNTL_INIT_STATE;
	for(i=0; i<DELAY; i++) { continue; }

	reg(MACA_DMARX) = data; /* put data somewhere */
	reg(MACA_PREAMBLE) = 0;

/* 	puts("maca_base\n\r"); */
/* 	dump_regs(MACA_BASE, 96); */
/* 	puts("modem write base\n\r"); */
/* 	dump_regs(0x80009000, 96); */
/* 	puts("modem read base\n\r"); */
/* 	dump_regs(0x800091c0, 96); */
/* 	puts("CRM\n\r"); */
/* 	dump_regs(0x80003000, 96); */
/* 	puts("reserved modem_base\n\r"); */
/* 	dump_regs(0x80009200, 192); */

	command_xcvr_rx();

//	puts("\033[H\033[2J");
	while(1) {		

		if(_is_action_complete_interrupt(maca_irq)) {
			maca_clrirq = maca_irq;
			
			status = reg(MACA_STATUS) & 0x0000ffff;
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
				
				puts("rftest-rx --- " );
				puts(" maca_getrxlvl: 0x");
				put_hex(reg(MACA_GETRXLVL));
				puts(" timestamp: 0x");
				put_hex32(maca_timestamp);
				puts("\n\r");
				puts(" data: 0x");
				put_hex32(data);
				putc(' ');
				for(i=0; i<=(reg(MACA_GETRXLVL)-4); i++) { /* fcs+somethingelse is not transferred by DMA */
					put_hex(data[i]);
					putc(' ');
				}				

				puts("\n\r");

				toggle_led();

				ResumeMACASync();

				command_xcvr_rx();
				
				break;
				
			}
			default:
			{
				puts("status: ");
				put_hex16(status);
				ResumeMACASync();
				command_xcvr_rx();
				
			}
			}
		} else if (_is_filter_failed_interrupt(maca_irq)) {
			puts("filter failed\n\r");
			ResumeMACASync();
			command_xcvr_rx();
		} else if (_is_checksum_failed_interrupt(maca_irq)) {
			puts("crc failed\n\r");
			ResumeMACASync();
			command_xcvr_rx();
		}

		
	};
}

void putc(uint8_t c) {
	while(reg(UT1CON)==31); /* wait for there to be room in the buffer */
	reg(UART1_DATA) = c;
}
	
void puts(uint8_t *s) {
	while(s && *s!=0) {
		putc(*s++);
	}
}

void put_hex(uint8_t x)
{
        putc(hex[x >> 4]);
        putc(hex[x & 15]);
}

void put_hex16(uint16_t x)
{
        put_hex((x >> 8) & 0xFF);
        put_hex((x) & 0xFF);
}

void put_hex32(uint32_t x)
{
        put_hex((x >> 24) & 0xFF);
        put_hex((x >> 16) & 0xFF);
        put_hex((x >> 8) & 0xFF);
        put_hex((x) & 0xFF);
}
