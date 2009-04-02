#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */

#define BASE_UART1      0x80005000
#define UART1_CON       0x80005000
#define UART1_STAT      0x80005004
#define UART1_DATA      0x80005008
#define UR1CON          0x8000500c
#define UT1CON          0x80005010
#define UART1_CTS       0x80005014
#define UART1_BR        0x80005018

#include "maca.h"
#include "embedded_types.h"

#define reg(x) (*(volatile uint32_t *)(x))

#define DELAY 400000
#define DATA  0x00401000;

void putc(uint8_t c);
void puts(uint8_t *s);
void put_hex(uint8_t x);
void put_hex16(uint16_t x);
void put_hex32(uint32_t x);

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

__attribute__ ((section ("startup")))
void main(void) {
	uint8_t c;
	volatile uint32_t i;
	volatile uint32_t *data;
	uint32_t tmp;

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

	reg(MACA_RESET) = 0x3; /* reset, turn on the clock */
	for(i=0; i<DELAY; i++) { continue; }
	reg(MACA_RESET) = 0x2; /* unreset, turn on the clock */
	for(i=0; i<DELAY; i++) { continue; }

	init_phy();

        reg(MACA_CONTROL) = SMAC_MACA_CNTL_INIT_STATE;
	for(i=0; i<DELAY; i++) { continue; }

	data = (void *)DATA;
	data[0] = 0xabc0ffee;
	maca_txlen = 1;
	reg(MACA_DMATX) = DATA; /* get data from somewhere */
	reg(MACA_PREAMBLE) = 0xface0fff;
	

#define NL "\033[K\r\n"

	puts("\033[H\033[2J");
	while(1) {		
		puts("\033[H");
		puts("rftest-tx --- " NL);
/*
		puts("base +0       +4       +8       +c       +10      +14      +18      +1c      " NL);
		for (i = 0; i < 96; i ++) { 
			if ((i & 7) == 0) {
				put_hex16(4 * i);
			}
			putc(' ');
			put_hex32(reg(MACA_BASE+(4*i)));
			if ((i & 7) == 7)
				puts(NL);
		}
		puts(NL);
*/

		/* start a sequence */
		reg(MACA_CONTROL) = 0x00031A03;
		/* wait for it to finish */
		while (((tmp = reg(MACA_STATUS)) & 15) == 14)
			continue;
		puts("completed status is ");
		put_hex32(tmp);
		puts(NL);		
		for(i=0; i<DELAY; i++) { continue; }
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
