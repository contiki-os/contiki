#include <mc1322x.h>
#include <board.h>
#include <uart1.h>

/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
#define INC 767
#define MOD 9999

void main(void) {

        /* mask interrupts, 16 bit sample --- helps explain the baud rate */
	*UART1_CON = 0x0000c800; 
	
	
	*UART1_BR = INC<<16 | MOD; 
	
	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	*UART1_CON = 0x00000003; /* enable receive and transmit */
	*GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/
	
	uint8_t c;
	while(1) {
		if(*UART1_CON > 0) {
			/* Receive buffer isn't empty */
			/* read a byte and write it to the transmit buffer */
			c = *UART1_DATA;
			*UART1_DATA = c;
		}
	}
	
}
