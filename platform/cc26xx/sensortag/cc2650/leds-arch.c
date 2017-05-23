/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26XX LEDs
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/leds.h"

#include "ti-lib.h"
/*---------------------------------------------------------------------------*/
static unsigned char c;
static int inited = 0;
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  if(inited) {
    return;
  }
  inited = 1;

  ti_lib_rom_ioc_pin_type_gpio_output(BOARD_IOID_LED_1);
  ti_lib_rom_ioc_pin_type_gpio_output(BOARD_IOID_LED_2);

  ti_lib_gpio_clear_multi_dio(BOARD_LED_ALL);
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return c;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  c = leds;
  ti_lib_gpio_clear_multi_dio(BOARD_LED_ALL);

  if((leds & LEDS_RED) == LEDS_RED) {
    ti_lib_gpio_set_dio(BOARD_IOID_LED_1);
  }
  if((leds & LEDS_YELLOW) == LEDS_YELLOW) {
    ti_lib_gpio_set_dio(BOARD_IOID_LED_2);
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
