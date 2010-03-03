#include <uart1.h>

void uart1_putc(char c) {
	while(*UT1CON == 31); /* wait for there to be room in the buffer */
	*UART1_DATA = c;
}
