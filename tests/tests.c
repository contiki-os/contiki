#include <mc1322x.h>
#include "put.h"

void uart1_init(uint16_t inc, uint16_t mod) {
	/* Restore UART regs. to default */
	/* in case there is still bootloader state leftover */
	
	*UART1_CON = 0x0000c800; /* mask interrupts, 16 bit sample --- helps explain the baud rate */
	
	/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
	*UART1_BR = (inc << 16) | mod; 
	
	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */
	*UART1_CON = 0x00000003; /* enable receive and transmit */
	*GPIO_FUNC_SEL0 = ( (0x01 << (14*2)) | (0x01 << (15*2)) ); /* set GPIO15-14 to UART (UART1 TX and RX)*/
	
}

void print_welcome(char* testname) {
	puts("mc1322x-test: ");
	puts(testname);
	puts("\n\r");
	puts("board: ");
#if (BOARD == redbee-dev)
	puts("redbee-dev");
#elif (BOARD == redbee-r1)
	puts("redbee-dev");
#endif
	puts("\n\r");
}
