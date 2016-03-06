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
 * \addtogroup srf06-common-peripherals
 * @{
 *
 * \file
 * Driver for the SmartRF06EB LEDs when a CC13xx/CC26xx EM is mounted on it
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

  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_LED_1);
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_LED_2);
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_LED_3);
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_LED_4);

  ti_lib_gpio_pin_write(BOARD_LED_ALL, 0);
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

  /* Clear everything */
  ti_lib_gpio_pin_write(BOARD_LED_ALL, 0);

  if((leds & LEDS_RED) == LEDS_RED) {
    ti_lib_gpio_pin_write(BOARD_LED_1, 1);
  }
  if((leds & LEDS_YELLOW) == LEDS_YELLOW) {
    ti_lib_gpio_pin_write(BOARD_LED_2, 1);
  }
  if((leds & LEDS_GREEN) == LEDS_GREEN) {
    ti_lib_gpio_pin_write(BOARD_LED_3, 1);
  }
  if((leds & LEDS_ORANGE) == LEDS_ORANGE) {
    ti_lib_gpio_pin_write(BOARD_LED_4, 1);
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
