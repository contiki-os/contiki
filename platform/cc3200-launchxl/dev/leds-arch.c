/*
 * Copyright (c) 2015, 3B Scientific GmbH - http://www.3bscientific.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc3200-Launchpad
 * @{
 *
 * \defgroup cc3200-leds LAUNCHPAD LED driver
 *
 * LED driver implementation for the TI LAUNCPAD + cc3200
 * @{
 *
 * \file
 * LED driver implementation for the TI LAUNCPAD + cc3200
 *
 * \author
 *         Dominik Grauert <dominik.grauert@3bscientific.com>
 */

#include "contiki.h"
#include "dev/leds.h"

#include "hw_types.h"
#include "hw_memmap.h"

#include "rom.h"
#include "rom_map.h"

#include "gpio.h"

#define LEDS_GPIO_PIN_MASK   (LEDS_ALL << 1)
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{

  // IOs were still initialized in PinMuxConfig(void)
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return (char)(MAP_GPIOPinRead(GPIOA1_BASE, LEDS_GPIO_PIN_MASK) >> 1);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  MAP_GPIOPinWrite(GPIOA1_BASE, LEDS_GPIO_PIN_MASK, leds << 1);
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
