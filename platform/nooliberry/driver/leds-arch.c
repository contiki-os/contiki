/*
 * Copyright (c) 2013, NooliTIC
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
 */

/**
 * \file
 *         Leds arch specific file for the NooliBerry platform
 * \author
 *         Ludovic Wiart <ludovic.wiart@noolitic.biz>
 *         Olivier Debreu <olivier.debreu@noolitic.biz>
 */

#include "contiki.h"
#include "dev/leds.h"

#define LED_RED_BIT 1
#define DIRECTION_REG DDRG
#define ACTIVE_GPIO_REG PORTG

/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  DIRECTION_REG |= LED_RED_BIT;
  ACTIVE_GPIO_REG &= ~LED_RED_BIT;
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return ((ACTIVE_GPIO_REG & LED_RED_BIT) ? LEDS_RED : 0);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  if(leds & LEDS_RED) {
    ACTIVE_GPIO_REG |= LED_RED_BIT;
  } else {
    ACTIVE_GPIO_REG &= ~LED_RED_BIT;
  }
}
/*---------------------------------------------------------------------------*/
