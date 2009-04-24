#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */

#define BASE_UART1      0x80005000
#define UART1_CON       0x80005000
#define UART1_STAT      0x80005004
#define UART1_DATA      0x80005008
#define UR1CON          0x8000500c
#define UT1CON          0x80005010
#define UART1_CTS       0x80005014
#define UART1_BR        0x80005018

#include "embedded_types.h"

#define reg(x) (*(volatile uint32_t *)(x))

#define DELAY 400000

void putc(uint8_t c);
void puts(uint8_t *s);
void put_hex(uint8_t x);
void put_hex16(uint16_t x);
void put_hex32(uint32_t x);

const uint8_t hex[16]={'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

#define DUMP_BASE 0x00000000
#define DUMP_LEN  0x00014000
//#define DUMP_LEN  16

#include "isr.h"

__attribute__ ((section ("startup")))
void main(void) {
	volatile uint32_t i;
//	volatile uint8_t *data;
	volatile uint8_t *data;

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

	for(data=DUMP_BASE; data<(DUMP_BASE+DUMP_LEN); data++) {
		putc(*data);
	}

	while(1);

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
