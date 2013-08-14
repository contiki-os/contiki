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

void default_vreg_init(void) {
	volatile uint32_t i;
	*CRM_SYS_CNTL = 0x00000018; /* set default state */
	*CRM_VREG_CNTL = 0x00000f04; /* bypass the buck */
	for(i=0; i<0x161a8; i++) { continue; } /* wait for the bypass to take */
//	while((((*(volatile uint32_t *)(0x80003018))>>17) & 1) !=1) { continue; } /* wait for the bypass to take */
	*CRM_VREG_CNTL = 0x00000ff8; /* start the regulators */
}

void uart1_init(volatile uint16_t inc, volatile uint16_t mod, volatile uint8_t samp) {

        /* UART must be disabled to set the baudrate */
	UART1->CON = 0;
	
	UART1->BR = ( inc << 16 ) | mod;

	/* TX and CTS as outputs */
	GPIO->PAD_DIR_SET.GPIO_14 = 1;
	GPIO->PAD_DIR_SET.GPIO_16 = 1;
		
	/* RX and RTS as inputs */
	GPIO->PAD_DIR_RESET.GPIO_15 = 1;
	GPIO->PAD_DIR_RESET.GPIO_17 = 1;

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */

#if UART1_RX_BUFFERSIZE > 32
	*UART1_UCON = (1 << 0) | (1 << 1) ;	/* enable receive, transmit, and both interrupts */
	*UART1_URXCON = 30;					/* interrupt when fifo is nearly full */
	u1_rx_head = 0; u1_rx_tail = 0;
#elif UART1_RX_BUFFERSIZE < 32			/* enable receive, transmit, flow control, disable rx interrupt */
	*UART1_UCON = (1 << 0) | (1 << 1) | (1 << 12) | (1 << 14); 
	*UART1_UCTS = UART1_RX_BUFFERSIZE;  /* drop cts when tx buffer at trigger level */
	*GPIO_FUNC_SEL1 = ( (0x01 << (0*2)) | (0x01 << (1*2)) ); /* set GPIO17-16 to UART1 CTS and RTS */
#else 
	*UART1_UCON = (1 << 0) | (1 << 1) | (1 << 14); /* enable receive, transmit, disable rx interrupt */
#endif

	if(samp == UCON_SAMP_16X) 
		set_bit(*UART1_UCON,UCON_SAMP);

	/* set GPIO15-14 to UART (UART1 TX and RX)*/
	GPIO->FUNC_SEL.GPIO_14 = 1;
	GPIO->FUNC_SEL.GPIO_15 = 1;
       
	/* interrupt when there are this number or more bytes free in the TX buffer*/
	UART1->TXCON = 16;
	u1_tx_head = 0; u1_tx_tail = 0;

	/* enable UART1 interrupts in the interrupt controller */
	enable_irq(UART1);
}

void uart2_init(volatile uint16_t inc, volatile uint16_t mod, volatile uint8_t samp) {

        /* UART must be disabled to set the baudrate */
	UART2->CON = 0;
	
	UART2->BR = ( inc << 16 ) | mod;

	/* TX and CTS as outputs */
	GPIO->PAD_DIR_SET.GPIO_18 = 1;
	GPIO->PAD_DIR_SET.GPIO_20 = 1;
		
	/* RX and RTS as inputs */
	GPIO->PAD_DIR_RESET.GPIO_19 = 1;
	GPIO->PAD_DIR_RESET.GPIO_21 = 1;

	/* see Section 11.5.1.2 Alternate Modes */
	/* you must enable the peripheral first BEFORE setting the function in GPIO_FUNC_SEL */
	/* From the datasheet: "The peripheral function will control operation of the pad IF */
	/* THE PERIPHERAL IS ENABLED. */

#if UART2_RX_BUFFERSIZE > 32
	*UART2_UCON = (1 << 0) | (1 << 1) ;	/* enable receive, transmit, and both interrupts */
	*UART2_URXCON = 30;					/* interrupt when fifo is nearly full */
	u2_rx_head = 0; u2_rx_tail = 0;
#elif UART2_RX_BUFFERSIZE < 32			/* enable receive, transmit, disable flow control, disable rx interrupt */
	*UART2_UCON = (1 << 0) | (1 << 1) | (0 << 12) | (1 << 14);
	*UART2_UCTS = UART2_RX_BUFFERSIZE;  /* drop cts when tx buffer at trigger level */
	*GPIO_FUNC_SEL1 = ( (0x01 << (0*2)) | (0x01 << (1*2)) ); /* set GPIO17-16 to UART2 CTS and RTS */
#else 
	*UART2_UCON = (1 << 0) | (1 << 1) | (1 << 14); /* enable receive, transmit, disable rx interrupt */
#endif

	if(samp == UCON_SAMP_16X) 
		set_bit(*UART2_UCON,UCON_SAMP);

	/* set GPIO15-14 to UART (UART2 TX and RX)*/
	GPIO->FUNC_SEL.GPIO_18 = 1;
	GPIO->FUNC_SEL.GPIO_19 = 1;

	/* interrupt when there are this number or more bytes free in the TX buffer*/
	UART2->TXCON = 16;
	u2_tx_head = 0; u2_tx_tail = 0;

	/* enable UART2 interrupts in the interrupt controller */
	enable_irq(UART2);
}
