#include <mc1322x.h>
#include <board.h>

#include "tests.h"

/* INC = 767; MOD = 9999 works: 115200 @ 24 MHz 16 bit sample */
#define INC 767
#define MOD 9999

void main(void) {
	
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
