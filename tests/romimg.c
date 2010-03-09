#include <mc1322x.h>
#include <board.h>

#include "tests.h"
#include "config.h"

void main(void) {
	volatile uint8_t *data;

	uart_init(INC, MOD, SAMP);

	for(data = DUMP_BASE; data < ((uint8_t *)(DUMP_BASE+DUMP_LEN)); data++) {
		uart1_putc(*data);
	}

	while(1);

}
