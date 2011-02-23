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
}

void uart1_putc(char c) {
	/* disable UART1 since */
	/* UART1 isr modifies u1_head and u1_tail */ 
	disable_irq(UART1);

	if( (u1_head == u1_tail) &&
	    (*UART1_UTXCON != 0)) {
		*UART1_UDATA = c;
	} else {
		u1_tx_buf[u1_head] = c;
		u1_head += 1;
		if (u1_head >= sizeof(u1_tx_buf))
			u1_head = 0;
		if (u1_head == u1_tail) { /* drop chars when no room */
			if (u1_head) { u1_head -=1; } else { u1_head = sizeof(u1_tx_buf); }
		}
		enable_irq(UART1);
	}
}

uint8_t uart1_getc(void) {
	while(uart1_can_get() == 0) { continue; }
	return *UART1_UDATA;
}
