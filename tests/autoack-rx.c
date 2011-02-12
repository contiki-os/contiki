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
#include <board.h>
#include <stdio.h>

#include "tests.h"
#include "config.h"

#define LED LED_GREEN

void maca_rx_callback(volatile packet_t *p) {
	(void)p;
	gpio_data_set(1ULL<< LED);
	gpio_data_reset(1ULL<< LED);
}

void main(void) {
	volatile packet_t *p;
	volatile uint8_t t=20;
	uint8_t chan;
	char c;

	gpio_data(0);
	
	gpio_pad_dir_set( 1ULL << LED );
        /* read from the data register instead of the pad */
	/* this is needed because the led clamps the voltage low */
	gpio_data_sel( 1ULL << LED);

	/* trim the reference osc. to 24MHz */
	trim_xtal();

	uart_init(INC, MOD, SAMP);

	vreg_init();

	maca_init();

        /* sets up tx_on, should be a board specific item */
        *GPIO_FUNC_SEL2 = (0x01 << ((44-16*2)*2));
	gpio_pad_dir_set( 1ULL << 44 );

	set_power(0x0f); /* 0dbm */
	chan = 0;
	set_channel(chan); /* channel 11 */

	*MACA_MACPANID = 0xaaaa;
	*MACA_MAC16ADDR = 0x1111;
	*MACA_TXACKDELAY = 68; /* 68 puts the tx ack at about the correct spot */
	set_prm_mode(AUTOACK);

	print_welcome("rftest-rx");
	while(1) {		

		/* call check_maca() periodically --- this works around */
		/* a few lockup conditions */
		check_maca();

		if((p = rx_packet())) {
			/* print and free the packet */
			printf("autoack-rx --- ");
			print_packet(p);
			free_packet(p);
		}

		if(uart1_can_get()) {
			c = uart1_getc();
			if(c == 'z') t++;
			if(c == 'x') t--;
			*MACA_TXACKDELAY = t;
			printf("tx ack delay: %d\n\r", t);
		}

	}
}
