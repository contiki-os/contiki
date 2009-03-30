#define GPIO_FUNC_SEL0  0x80000018 /* GPIO 15 - 0;  2 bit blocks */
#define GPIO_FUNC_SEL1  0x8000001c /* GPIO 16 - 31; 2 bit blocks*/

#define BASE_UART1      0x80005000
#define UART1_CON       0x80005000
#define UART1_STAT      0x80005004
#define UART1_DATA      0x80005008
#define UR1CON          0x8000500c
#define UT1CON          0x80005010
#define UART1_CTS       0x80005014
#define UART1_BR        0x80005018

#include "embedded_types.h"

#define DELAY 1000

void main(void) {

	volatile uint32_t i;
	
	*(volatile uint32_t *)GPIO_FUNC_SEL0 = (0x01 << (14*2)); /* set GPIO14 to UART (UART1 TX)*/
//	*(volatile uint32_t *)GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) || (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	*(volatile uint32_t *)UART1_BR = 0x004C03E8; /* Baud rate: (INC<<16 || MOD) */ /* is 115200 @ 24 MHz --- unexplained */

	*(volatile uint32_t *)UART1_CON = 0x00000003; /* enable receive and transmit */


	while(1) {
		for(i=0; i<DELAY; i++) { continue; }
		*(volatile uint32_t *)UART1_DATA = 'H';
/*		for(i=0; i<DELAY; i++) { continue; }
		*(volatile uint32_t *)UART1_DATA = 'e';
		for(i=0; i<DELAY; i++) { continue; }
		*(volatile uint32_t *)UART1_DATA = 'l';
		for(i=0; i<DELAY; i++) { continue; }
		*(volatile uint32_t *)UART1_DATA = 'l';
		for(i=0; i<DELAY; i++) { continue; }
		*(volatile uint32_t *)UART1_DATA = 'o';
		for(i=0; i<DELAY; i++) { continue; }
		*(volatile uint32_t *)UART1_DATA = ' ';
		*(volatile uint32_t *)UART1_DATA = 'W';
		*(volatile uint32_t *)UART1_DATA = 'o';
		*(volatile uint32_t *)UART1_DATA = 'r';
		*(volatile uint32_t *)UART1_DATA = 'l';
		*(volatile uint32_t *)UART1_DATA = 'd';
		*(volatile uint32_t *)UART1_DATA = '\n';
		*(volatile uint32_t *)UART1_DATA = '\r';*/
	}

/* 	while(1) { */
/* 		if(*(volatile uint32_t*)UR1CON > 0) { */
/* 			/\* Receive buffer isn't empty *\/ */
/* 			/\* read a byte and write it to the transmit buffer *\/ */
/* 		} */
/* 	}; */
}
