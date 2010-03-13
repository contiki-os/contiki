#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

void main(void) {

	uart1_init(INC,MOD,SAMP);
	
	while(1) {
		if(uart1_can_get()) {
			/* Receive buffer isn't empty */
			/* read a byte and write it to the transmit buffer */
			uart1_putc(uart1_getc());
		}
	}
	
}
