#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

void main(void) {

	uart1_init(INC,MOD);
	
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
