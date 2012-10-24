/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details.
 *
 *
 */

#include <mc1322x.h>
#include <stdint.h>

volatile char u1_tx_buf[UART1_TX_BUFFERSIZE];
volatile uint32_t u1_tx_head, u1_tx_tail;

#if UART1_RX_BUFFERSIZE > 32
volatile char u1_rx_buf[UART1_RX_BUFFERSIZE-32];
volatile uint32_t u1_rx_head, u1_rx_tail;
#endif

void uart1_isr(void) {

#if UART1_RX_BUFFERSIZE > 32
	if (*UART1_USTAT & ( 1 << 6)) {   //receive interrupt
		while( *UART1_URXCON != 0 ) {   //flush the hardware fifo into the software buffer
			uint32_t u1_rx_tail_next;
			u1_rx_tail_next = u1_rx_tail+1;
			if (u1_rx_tail_next >= sizeof(u1_rx_buf))
				u1_rx_tail_next = 0;
			if (u1_rx_head != u1_rx_tail_next) {
				u1_rx_buf[u1_rx_tail]= *UART1_UDATA;
				u1_rx_tail =  u1_rx_tail_next;
			} else { //buffer is full, flush the fifo
				while (*UART1_URXCON !=0) { if (*UART1_UDATA) { } }
			}
		}
		return;
	}
#endif

	while( *UART1_UTXCON != 0 ) {
		if (u1_tx_head == u1_tx_tail) {
#if UART1_RX_BUFFERSIZE > 32
            *UART1_UCON |= (1 << 13); /*disable tx interrupt */
#else
			disable_irq(UART1);
#endif
			return;
		}

		*UART1_UDATA = u1_tx_buf[u1_tx_tail];
		u1_tx_tail++;
		if (u1_tx_tail >= sizeof(u1_tx_buf))
			u1_tx_tail = 0;
	}
}

void uart1_putc(char c) {
	/* disable UART1 since */
	/* UART1 isr modifies u1_tx_head and u1_tx_tail */
#if UART1_RX_BUFFERSIZE > 32
            *UART1_UCON |= (1 << 13); /*disable tx interrupt */
#else
			disable_irq(UART1);
#endif

	if( (u1_tx_head == u1_tx_tail) &&
	    (*UART1_UTXCON != 0)) {
		*UART1_UDATA = c;
	} else {
		u1_tx_buf[u1_tx_head] = c;
		u1_tx_head += 1;
		if (u1_tx_head >= sizeof(u1_tx_buf))
			u1_tx_head = 0;
		if (u1_tx_head == u1_tx_tail) { /* drop chars when no room */
#if UART1_DROP_CHARS
			if (u1_tx_head) { u1_tx_head -=1; } else { u1_tx_head = sizeof(u1_tx_buf); }
#else
			{
				uint32_t  u1_tx_tail_save=u1_tx_tail;
                                /* Back up head to show buffer not empty, and enable tx interrupt */
				u1_tx_head--;
#if UART1_RX_BUFFERSIZE > 32
				*UART1_UCON &= ~(1 << 13); /*enable tx interrupt */
#else
				enable_irq(UART1);
#endif
                                /* Tail will change after one character goes out */
				while (u1_tx_tail_save == u1_tx_tail) ;
                                /* Restore head to character we just stuffed */
				u1_tx_head++;
				return;
			}
#endif /* UART1_DROP_CHARS */
		}

#if UART1_RX_BUFFERSIZE > 32
		*UART1_UCON &= ~(1 << 13); /*enable tx interrupt */
#else
		enable_irq(UART1);
#endif

	}
}

uint8_t uart1_getc(void) {
#if UART1_RX_BUFFERSIZE > 32
/* First pull from the ram buffer */
uint8_t c=0;
  if (u1_rx_head != u1_rx_tail) {
    c = u1_rx_buf[u1_rx_head++];
    if (u1_rx_head >= sizeof(u1_rx_buf))
        u1_rx_head=0;
    return c;
  }
#endif
/* Then pull from the hardware fifo */
	while(uart1_can_get() == 0) { continue; }
	return *UART1_UDATA;
}
