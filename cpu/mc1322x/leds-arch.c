/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
 *
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
 * This file is part of the Contiki OS.
 *
 *
 */

#include "contiki-conf.h"
#include "dev/leds.h"
#include "mc1322x.h"
#include "board.h"

#define LED_ARCH_RED   GPIO_LED_RED
#define LED_ARCH_GREEN GPIO_LED_GREEN
#define LED_ARCH_BLUE  GPIO_LED_BLUE

void leds_arch_init(void)
{
	/* set gpio func_sel to gpio (3) */
	GPIO->FUNC_SEL.LED_ARCH_RED = 3;
	GPIO->FUNC_SEL.LED_ARCH_GREEN = 3;
	GPIO->FUNC_SEL.LED_ARCH_BLUE = 3;

	/* set led gpios to output */
	GPIO->PAD_DIR.LED_ARCH_RED = 1;
	GPIO->PAD_DIR.LED_ARCH_GREEN = 1;
	GPIO->PAD_DIR.LED_ARCH_BLUE = 1;
}

unsigned char leds_arch_get(void)
{

	return ((GPIO->DATA.LED_ARCH_RED) ? 0 : LEDS_RED)
		| ((GPIO->DATA.LED_ARCH_GREEN) ? 0 : LEDS_GREEN)
		| ((GPIO->DATA.LED_ARCH_BLUE) ? 0 : LEDS_BLUE);

}

void leds_arch_set(unsigned char leds)
{
	if(leds & LEDS_RED)   { GPIO->DATA.LED_ARCH_RED = 1;   } else { gpio_reset(LED_ARCH_RED);   } 
	if(leds & LEDS_GREEN) { gpio_set(LED_ARCH_GREEN); } else { gpio_reset(LED_ARCH_GREEN); } 
	if(leds & LEDS_BLUE)  { gpio_set(LED_ARCH_BLUE);  } else { gpio_reset(LED_ARCH_BLUE);  }  
}

