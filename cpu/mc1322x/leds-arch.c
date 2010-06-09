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
 * $Id: leds-arch.c,v 1.1 2010/06/09 14:43:22 maralvira Exp $
 */

#include "contiki-conf.h"
#include "dev/leds.h"
#include "mc1322x.h"
#include "board.h"

#define LED_ARCH_RED   (1ULL << LED_RED)
#define LED_ARCH_GREEN (1ULL << LED_GREEN)
#define LED_ARCH_BLUE  (1ULL << LED_BLUE)

#define LED_ARCH_YELLOW (LED_ARCH_RED  | LED_ARCH_GREEN           )
#define LED_ARCH_PURPLE (LED_ARCH_RED  |             LED_ARCH_BLUE)
#define LED_ARCH_CYAN   (           LED_ARCH_GREEN | LED_ARCH_BLUE)
#define LED_ARCH_WHITE  (LED_ARCH_RED  | LED_ARCH_GREEN | LED_ARCH_BLUE)

/*FIXME: this is broken --- don't set all of the GPIO direction */
void leds_arch_init(void)
{
	gpio_pad_dir(LED_ARCH_WHITE);
}

unsigned char leds_arch_get(void)
{
	uint64_t led = (((uint64_t)*GPIO_DATA1) << 32) | *GPIO_DATA0;

	return ((led & LED_ARCH_RED) ? 0 : LEDS_RED)
		| ((led & LED_ARCH_GREEN) ? 0 : LEDS_GREEN)
		| ((led & LED_ARCH_BLUE) ? 0 : LEDS_BLUE)
		| ((led & LED_ARCH_YELLOW) ? 0 : LEDS_YELLOW);

}

/*FIXME: this is broken --- it hits the entire GPIO data register and breaks the button sensor */
void leds_arch_set(unsigned char leds)
{
	uint64_t led;

	led = (led & ~(LED_ARCH_RED|LED_ARCH_GREEN|LED_ARCH_YELLOW|LED_ARCH_BLUE))
		| ((leds & LEDS_RED) ? LED_ARCH_RED : 0)
		| ((leds & LEDS_GREEN) ? LED_ARCH_GREEN : 0)
		| ((leds & LEDS_BLUE) ? LED_ARCH_BLUE : 0)
		| ((leds & LEDS_YELLOW) ? LED_ARCH_YELLOW : 0);

	gpio_data(led);
}

