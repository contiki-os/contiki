/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * This file is part of the Contiki operating system.
 *
 * @(#)$Id: leds.c,v 1.4 2007/03/19 00:33:25 adamdunkels Exp $
 */

#include "dev/leds.h"
#include "sys/clock.h"
#include "lib/energest.h"

static unsigned char leds, invert;
/*---------------------------------------------------------------------------*/
static void
show_leds(void)
{
  leds_arch_set(leds ^ invert);
}
/*---------------------------------------------------------------------------*/
void
leds_init(void)
{
  leds_arch_init();
  leds = invert = 0;
}
/*---------------------------------------------------------------------------*/
void
leds_blink(void)
{
  /* Blink all leds. */
  leds_arch_set(LEDS_ALL);

  clock_delay(400);

  show_leds();
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_get(void) {
  return leds_arch_get();
}
/*---------------------------------------------------------------------------*/
void
leds_on(unsigned char l)
{
  if((l & LEDS_GREEN) && !(leds & LEDS_GREEN)) {
    ENERGEST_ON(ENERGEST_TYPE_LED_GREEN);
  }
  if((l & LEDS_YELLOW) && !(leds & LEDS_YELLOW)) {
    ENERGEST_ON(ENERGEST_TYPE_LED_YELLOW);
  }
  if((l & LEDS_RED) && !(leds & LEDS_RED)) {
    ENERGEST_ON(ENERGEST_TYPE_LED_RED);
  }

  leds |= l;
  show_leds();
}
/*---------------------------------------------------------------------------*/
void
leds_off(unsigned char l)
{
  
  if((l & LEDS_GREEN) && (leds & LEDS_GREEN)) {
    ENERGEST_OFF(ENERGEST_TYPE_LED_GREEN);
  }
  if((l & LEDS_YELLOW) && (leds & LEDS_YELLOW)) {
    ENERGEST_OFF(ENERGEST_TYPE_LED_YELLOW);
  }
  if((l & LEDS_RED) && (leds & LEDS_RED)) {
    ENERGEST_OFF(ENERGEST_TYPE_LED_RED);
  }
  leds &= ~l;
  show_leds();
}
/*---------------------------------------------------------------------------*/
void
leds_toggle(unsigned char leds)
{
  leds_invert(leds);
}

/*   invert the ínvert register using the leds parameter */
void leds_invert(unsigned char l) {
  invert = invert ^ l;
  show_leds();
}

/*---------------------------------------------------------------------------*/
void leds_green(int o) { o?leds_on(LEDS_GREEN):leds_off(LEDS_GREEN); }
void leds_yellow(int o) { o?leds_on(LEDS_YELLOW):leds_off(LEDS_YELLOW); }
void leds_red(int o) { o?leds_on(LEDS_RED):leds_off(LEDS_RED); }
