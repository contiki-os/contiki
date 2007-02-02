/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * @(#)$Id: qleds.c,v 1.1 2007/02/02 14:07:34 bg- Exp $
 */

#include "contiki.h"

#include "dev/leds.h"

#include <avr/io.h>

void
leds_init(void)
{
#ifdef CB_GATEWAY
  DDRE  |= LEDS_CONF_GREEN | LEDS_CONF_YELLOW;
  PORTE |= LEDS_CONF_GREEN | LEDS_CONF_YELLOW; /* LEDS off */
#else
  DDRB  |= LEDS_CONF_ORANGE | LEDS_CONF_GREEN;
  DDRE  |= LEDS_CONF_RED | LEDS_CONF_YELLOW;

  PORTB &= ~(LEDS_CONF_ORANGE | LEDS_CONF_GREEN);
  PORTE &= ~(LEDS_CONF_RED | LEDS_CONF_YELLOW);
#endif
}

void
leds_on(unsigned char leds)
{
#ifdef CB_GATEWAY
  if (leds & LEDS_GREEN)
    PORTE &= ~LEDS_CONF_GREEN;
  if (leds & LEDS_YELLOW)
    PORTE &= ~LEDS_CONF_YELLOW;
#else
  if (leds & LEDS_ORANGE)
    PORTB |= LEDS_CONF_ORANGE;
  if (leds & LEDS_GREEN)
    PORTB |= LEDS_CONF_GREEN;

  if (leds & LEDS_RED)
    PORTE |= LEDS_CONF_RED;
  if (leds & LEDS_YELLOW)
    PORTE |= LEDS_CONF_YELLOW;
#endif
}

void
leds_off(unsigned char leds)
{
#ifdef CB_GATEWAY
  if (leds & LEDS_GREEN)
    PORTE |= LEDS_CONF_GREEN;
  if (leds & LEDS_YELLOW)
    PORTE |= LEDS_CONF_YELLOW;
#else
  if (leds & LEDS_ORANGE)
    PORTB &= ~LEDS_CONF_ORANGE;
  if (leds & LEDS_GREEN)
    PORTB &= ~LEDS_CONF_GREEN;

  if (leds & LEDS_RED)
    PORTE &= ~LEDS_CONF_RED;
  if (leds & LEDS_YELLOW)
    PORTE &= ~LEDS_CONF_YELLOW;
#endif
}

void
leds_toggle(unsigned char leds)
{
  /*
   * Synonym: void leds_invert(unsigned char leds);
   */
  asm(".global leds_invert\nleds_invert:\n");

#ifdef CB_GATEWAY
  if (leds & LEDS_GREEN)
    PORTE ^= ~LEDS_CONF_GREEN;
  if (leds & LEDS_YELLOW)
    PORTE ^= ~LEDS_CONF_YELLOW;
#else
  if (leds & LEDS_ORANGE)
    PORTB ^= LEDS_CONF_ORANGE;
  if (leds & LEDS_GREEN)
    PORTB ^= LEDS_CONF_GREEN;

  if (leds & LEDS_RED)
    PORTE ^= LEDS_CONF_RED;
  if (leds & LEDS_YELLOW)
    PORTE ^= LEDS_CONF_YELLOW;
#endif
}
