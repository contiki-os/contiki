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

volatile char u2_tx_buf[UART2_TX_BUFFERSIZE];
volatile uint32_t u2_tx_head, u2_tx_tail;

#if UART2_RX_BUFFERSIZE > 32
volatile char u2_rx_buf[UART2_RX_BUFFERSIZE-32];
volatile uint32_t u2_rx_head, u2_rx_tail;
#endif

void uart2_isr(void) {

#if UART2_RX_BUFFERSIZE > 32
	if (*UART2_USTAT & ( 1 << 6)) {   //receive interrupt
		while( *UART2_URXCON != 0 ) {   //flush the hardware fifo into the software buffer
			uint32_t u2_rx_tail_next;
			u2_rx_tail_next = u2_rx_tail+1;
			if (u2_rx_tail_next >= sizeof(u2_rx_buf))
				u2_rx_tail_next = 0;
			if (u2_rx_head != u2_rx_tail_next) {
				u2_rx_buf[u2_rx_tail]= *UART2_UDATA;
				u2_rx_tail =  u2_rx_tail_next;
			} else {  //buffer is full, flush the fifo
				while (*UART2_URXCON !=0) if (*UART2_UDATA);
			}
		}
		return;
	}
#endif

	while( *UART2_UTXCON != 0 ) {
		if (u2_tx_head == u2_tx_tail) {
#if UART2_RX_BUFFERSIZE > 32
            *UART2_UCON |= (1 << 13); /*disable tx interrupt */
#else
			disable_irq(UART2);
#endif
			return;
		}

		*UART2_UDATA = u2_tx_buf[u2_tx_tail];
		u2_tx_tail++;
		if (u2_tx_tail >= sizeof(u2_tx_buf))
			u2_tx_tail = 0;
	}
}

void uart2_putc(char c) {
	/* disable UART2 since */
	/* UART2 isr modifies u2_tx_head and u2_tx_tail */
#if UART2_RX_BUFFERSIZE > 32
            *UART2_UCON |= (1 << 13); /*disable tx interrupt */
#else
			disable_irq(UART2);
#endif

	if( (u2_tx_head == u2_tx_tail) &&
	    (*UART2_UTXCON != 0)) {
		*UART2_UDATA = c;
	} else {
		u2_tx_buf[u2_tx_head] = c;
		u2_tx_head += 1;
		if (u2_tx_head >= sizeof(u2_tx_buf))
			u2_tx_head = 0;
		if (u2_tx_head == u2_tx_tail) { /* drop chars when no room */
#if UART2_DROP_CHARS
			if (u2_tx_head) { u2_tx_head -=1; } else { u2_tx_head = sizeof(u2_tx_buf); }
#else
			{
				uint32_t  u2_tx_tail_save=u2_tx_tail;
                                /* Back up head to show buffer not empty, and enable tx interrupt */
				u2_tx_head--;
#if UART2_RX_BUFFERSIZE > 32
				*UART2_UCON &= ~(1 << 13); /*enable tx interrupt */
#else
				enable_irq(UART2);
#endif
                                /* Tail will change after one character goes out */
				while (u2_tx_tail_save == u2_tx_tail) ;
                                /* Restore head to character we just stuffed */
				u2_tx_head++;
				return;
			}
#endif /* UART2_DROP_CHARS */
		}

#if UART2_RX_BUFFERSIZE > 32
		*UART2_UCON &= ~(1 << 13); /*enable tx interrupt */
#else
		enable_irq(UART2);
#endif

	}
}

uint8_t uart2_getc(void) {
#if UART2_RX_BUFFERSIZE > 32
/* First pull from the ram buffer */
uint8_t c=0;
  if (u2_rx_head != u2_rx_tail) {
    c = u2_rx_buf[u2_rx_head++];
    if (u2_rx_head >= sizeof(u2_rx_buf))
        u2_rx_head=0;
    return c;
  }
#endif
/* Then pull from the hardware fifo */
	while(uart2_can_get() == 0) { continue; }
	return *UART2_UDATA;
}
