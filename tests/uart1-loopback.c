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

void main(void) {
	/* Restore UART regs. to default */
	/* in case there is still bootloader state leftover */

	*(volatile uint32_t *)UART1_CON = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */

	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	#define INC 767
	#define MOD 9999
	*(volatile uint32_t *)UART1_BR = INC<<16 | MOD; 

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	*(volatile uint32_t *)UART1_CON = 0x00000003; /* enable receive and transmit */
	*(volatile uint32_t *)GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/

	uint8_t c;
	while(1) {
		if(*(volatile uint32_t*)UR1CON > 0) {
			/* Receive buffer isn't empty */
			/* read a byte and write it to the transmit buffer */
			c = *(volatile uint32_t *)UART1_DATA;
			*(volatile uint32_t *)UART1_DATA = c;
		}
	};
}
