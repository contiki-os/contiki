/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 */

/**
 * \file
 *         Platform-specific led driver for the TI SmartRF05 Eval. Board.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "contiki-conf.h"
#include "dev/leds.h"
#include "dev/leds-arch.h"
#include "cc253x.h"
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
#if MODEL_CC2531
  P1SEL &= ~LED1_MASK;
  P1DIR |= LED1_MASK;
  P0SEL &= ~LED2_MASK;
  P0DIR |= LED2_MASK;
#else
  P1SEL &= ~(LED1_MASK | LED2_MASK | LED3_MASK);
  P1DIR |= (LED1_MASK | LED2_MASK | LED3_MASK);
#endif
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
#if MODEL_CC2531
  return (unsigned char)(LED1_PIN | ((LED2_PIN ^ 0x01) << 1));
#else
  return (unsigned char)(LED1_PIN | (LED2_PIN << 1) | (LED3_PIN << 2));
#endif
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  LED1_PIN = leds & 0x01;
#if MODEL_CC2531
  LED2_PIN = ((leds & 0x02) >> 1) ^ 0x01;
#else
  LED2_PIN = (leds & 0x02) >> 1;
  LED3_PIN = (leds & 0x04) >> 2;
#endif
}
/*---------------------------------------------------------------------------*/
