#include <mc1322x.h>
#include <types.h>

volatile char u1_tx_buf[1024];
volatile uint32_t u1_head, u1_tail;

void uart1_isr(void) {
 	while( *UART1_UTXCON != 0 ) {
		if (u1_head == u1_tail) {
			disable_irq(UART1);
			return;
		}
		*UART1_UDATA = u1_tx_buf[u1_tail];
		u1_tail++;		
		if (u1_tail >= sizeof(u1_tx_buf))
			u1_tail = 0;
	}
	enable_irq(UART1);
}

void uart1_putc(char c) {
	uint32_t h = u1_head;
	h = u1_head + 1;
	if (h >= sizeof(u1_tx_buf))
		h = 0;
	if (h == u1_tail) /* drop chars when no room */
		return;
	u1_tx_buf[u1_head] = c;
	u1_head = h;

	uart1_isr();
}

uint8_t uart1_getc(void) {
	while(uart1_can_get() == 0) { continue; }
	return *UART1_UDATA;
}
