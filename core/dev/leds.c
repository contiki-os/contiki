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
 * @(#)$Id: leds.c,v 1.7 2009/02/24 21:30:20 adamdunkels Exp $
 */

#include "dev/leds.h"
#include "sys/clock.h"
#include "sys/energest.h"

static unsigned char leds, invert;
/*---------------------------------------------------------------------------*/
static void
show_leds(unsigned char changed)
{
  if(changed & LEDS_GREEN) {
    /* Green did change */
    if((invert ^ leds) & LEDS_GREEN) {
      ENERGEST_ON(ENERGEST_TYPE_LED_GREEN);
    } else {
      ENERGEST_OFF(ENERGEST_TYPE_LED_GREEN);
    }
  }
  if(changed & LEDS_YELLOW) {
    if((invert ^ leds) & LEDS_YELLOW) {
      ENERGEST_ON(ENERGEST_TYPE_LED_YELLOW);
    } else {
      ENERGEST_OFF(ENERGEST_TYPE_LED_YELLOW);
    }
  }
  if(changed & LEDS_RED) {
    if((invert ^ leds) & LEDS_RED) {
      ENERGEST_ON(ENERGEST_TYPE_LED_RED);
    } else {
      ENERGEST_OFF(ENERGEST_TYPE_LED_RED);
    }
  }
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
  unsigned char inv;
  inv = ~(leds ^ invert);
  leds_invert(inv);

  clock_delay(400);

  leds_invert(inv);
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_get(void) {
  return leds_arch_get();
}
/*---------------------------------------------------------------------------*/
void
leds_on(unsigned char ledv)
{
  unsigned char changed;
  changed = (~leds) & ledv;
  leds |= ledv;
  show_leds(changed);
}
/*---------------------------------------------------------------------------*/
void
leds_off(unsigned char ledv)
{
  unsigned char changed;
  changed = leds & ledv;
  leds &= ~ledv;
  show_leds(changed);
}
/*---------------------------------------------------------------------------*/
void
leds_toggle(unsigned char ledv)
{
  leds_invert(ledv);
}
/*---------------------------------------------------------------------------*/
/*   invert the invert register using the leds parameter */
void
leds_invert(unsigned char ledv) {
  invert = invert ^ ledv;
  show_leds(ledv);
}
/*---------------------------------------------------------------------------*/
