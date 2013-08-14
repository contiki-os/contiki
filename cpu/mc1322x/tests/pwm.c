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

#include "config.h"
#include "pwm.h"
#include "rtc.h"

int main(void) 
{
	int x = 32768;
	
	trim_xtal();
	uart_init(UART1, 115200);
	rtc_init();
	
	printf("pwm test\r\n");
	pwm_init_stopped(TMR0, 12000000, x);
	pwm_init_stopped(TMR1, 12000000, x);
	TMR0->ENBL |= TMR_ENABLE_BIT(TMR0) | TMR_ENABLE_BIT(TMR1);

	for(;;) {
		printf("duty %d = %d%%\r\n", x, ((x * 100 + 32768) / 65536));
		switch(uart1_getc()) {
		case '[': x -= 1; break;
		case ']': x += 1; break;
		case '-': x -= 32; break;
		case '=': x += 32; break;
		case '_': x -= 512; break;
		case '+': x += 512; break;

		case '`': x = 65535 * 0/10; break;
		case '1': x = 65535 * 1/10; break;
		case '2': x = 65535 * 2/10; break;
		case '3': x = 65535 * 3/10; break;
		case '4': x = 65535 * 4/10; break;
		case '5': x = 65535 * 5/10; break;
		case '6': x = 65535 * 6/10; break;
		case '7': x = 65535 * 7/10; break;
		case '8': x = 65535 * 8/10; break;
		case '9': x = 65535 * 9/10; break;
		case '0': x = 65535 * 10/10; break;

		}
		x &= 65535;
		pwm_duty(TMR0, x);
	}
}


