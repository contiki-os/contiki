/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * Copyright (c) 2015, University of Bristol - http://www.bristol.ac.uk
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
 * \addtogroup remote
 * @{
 *
 * \defgroup remote-leds Re-Mote LED driver
 *
 * LED driver implementation for the Re-Mote platform
 * @{
 *
 * \file
 * LED driver implementation for the Re-Mote platform
 */
#include "contiki.h"
#include "reg.h"
#include "dev/leds.h"
#include "dev/gpio.h"
/*---------------------------------------------------------------------------*/
#define LED_GREEN_PORT GPIO_D_BASE
#define LED_GREEN_PIN  (1 << 5)

#define LED_BLUE_PORT  GPIO_C_BASE
#define LED_BLUE_PIN   (1 << 3)

#define LED_RED_PORT   GPIO_D_BASE
#define LED_RED_PIN    (1 << 2)

#define PORT_D_LEDS (LED_RED_PIN | LED_GREEN_PIN)
#define PORT_C_LEDS LED_BLUE_PIN
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  /* Initialize LED1 (Red) and LED3 (Green) */
  GPIO_SET_OUTPUT(GPIO_D_BASE, PORT_D_LEDS);
  GPIO_SET_PIN(GPIO_D_BASE, PORT_D_LEDS);

  /* Initialize LED2 - Blue */
  GPIO_SET_OUTPUT(GPIO_C_BASE, PORT_C_LEDS);
  GPIO_SET_PIN(GPIO_C_BASE, PORT_C_LEDS);
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  uint8_t mask_leds;

  mask_leds = GPIO_READ_PIN(LED_GREEN_PORT, LED_GREEN_PIN) == 0? LEDS_GREEN : 0;
  mask_leds |= GPIO_READ_PIN(LED_BLUE_PORT, LED_BLUE_PIN) == 0? LEDS_BLUE : 0;
  mask_leds |= GPIO_READ_PIN(LED_RED_PORT, LED_RED_PIN) == 0? LEDS_RED : 0;

  return mask_leds;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  if(leds & LEDS_GREEN) {
    GPIO_CLR_PIN(LED_GREEN_PORT, LED_GREEN_PIN);
  } else {
    GPIO_SET_PIN(LED_GREEN_PORT, LED_GREEN_PIN);
  }

  if(leds & LEDS_BLUE) {
    GPIO_CLR_PIN(LED_BLUE_PORT, LED_BLUE_PIN);
  } else {
    GPIO_SET_PIN(LED_BLUE_PORT, LED_BLUE_PIN);
  }

  if(leds & LEDS_RED) {
    GPIO_CLR_PIN(LED_RED_PORT, LED_RED_PIN);
  } else {
    GPIO_SET_PIN(LED_RED_PORT, LED_RED_PIN);
  }
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
