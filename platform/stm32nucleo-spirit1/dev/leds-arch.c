/*
 * Copyright (c) 2012, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup stm32nucleo-spirit1-peripherals
 * @{
 *
 * \file
 * Driver for the stm32nucleo-spirit1 LEDs
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "dev/leds.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
#ifndef X_NUCLEO_IKS01A1
/* The Red LED (on SPIRIT1 exp board) is exposed only if the
 * X-NUCLEO-IKS01A1 sensor board is NOT used, becasue of a pin conflict.
 */
extern st_lib_gpio_typedef *st_lib_a_led_gpio_port[];
extern const uint16_t st_lib_a_led_gpio_pin[];
#endif /*X_NUCLEO_IKS01A1*/

extern st_lib_gpio_typedef *st_lib_gpio_port[];
extern const uint16_t st_lib_gpio_pin[];
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  /* We have at least one led, the one on the Nucleo (GREEN)*/
  st_lib_bsp_led_init(LED2);
  st_lib_bsp_led_off(LED2);

#ifndef X_NUCLEO_IKS01A1
/* The Red LED (on SPIRIT1 exp board) is exposed only if the
 * X-NUCLEO-IKS01A1 sensor board is NOT used, becasue of a pin conflict.
 */
  st_lib_radio_shield_led_init(RADIO_SHIELD_LED);
  st_lib_radio_shield_led_off(RADIO_SHIELD_LED);
#endif /*X_NUCLEO_IKS01A1*/
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  unsigned char ret = 0;
  if(st_lib_hal_gpio_read_pin(st_lib_gpio_port[LED2], st_lib_gpio_pin[LED2])) {
    ret |= LEDS_GREEN;
  }

#ifndef X_NUCLEO_IKS01A1
/* The Red LED (on SPIRIT1 exp board) is exposed only if the
 * X-NUCLEO-IKS01A1 sensor board is NOT used, becasue of a pin conflict.
 */
  if(st_lib_hal_gpio_read_pin(st_lib_a_led_gpio_port[RADIO_SHIELD_LED],
                              st_lib_a_led_gpio_pin[RADIO_SHIELD_LED])) {
    ret |= LEDS_RED;
  }
#endif /*X_NUCLEO_IKS01A1*/

  return ret;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  if(leds & LEDS_GREEN) {
    st_lib_bsp_led_on(LED2);
  } else {
    st_lib_bsp_led_off(LED2);
  }

#ifndef X_NUCLEO_IKS01A1
/* The Red LED (on SPIRIT1 exp board) is exposed only if the
 * X-NUCLEO-IKS01A1 sensor board is NOT used, becasue of a pin conflict.
 */
  if(leds & LEDS_RED) {
    st_lib_radio_shield_led_on(RADIO_SHIELD_LED);
  } else {
    st_lib_radio_shield_led_off(RADIO_SHIELD_LED);
  }
#endif /*X_NUCLEO_IKS01A1*/
}
/*---------------------------------------------------------------------------*/
/** @} */
