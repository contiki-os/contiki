/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 */

/**
 * \addtogroup nrf52dk
 * @{
 *
 * \addtogroup nrf52dk-devices Device drivers
 * @{
 *
 * \addtogroup nrf52dk-devices-led LED driver
 * @{
 *
 * \file
 *         Architecture specific LED driver implementation for nRF52 DK.
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 */
#include "boards.h"
#include "contiki.h"
#include "dev/leds.h"

/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  LEDS_CONFIGURE(LEDS_MASK);
  LEDS_OFF(LEDS_MASK);
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return (unsigned char)(LED_IS_ON(LEDS_MASK) >> LED_START);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  unsigned int mask = (unsigned int)leds << LED_START;
  LEDS_OFF(LEDS_MASK);
  LEDS_ON(mask);
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 * @}
 */
