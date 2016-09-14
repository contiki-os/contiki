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
 * \addtogroup zoul
 * @{
 *
 * \defgroup remote-revb-leds RE-Mote revision B arch LED
 *
 * LED driver implementation for the RE-Mote revision B
 * @{
 *
 * \file
 * LED driver implementation for the RE-Mote revision B
 */
#include "contiki.h"
#include "reg.h"
#include "dev/leds.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
/*---------------------------------------------------------------------------*/
#define LEDS_PORTB_PIN_MASK  (LEDS_GREEN_PIN_MASK | LEDS_BLUE_PIN_MASK)
/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  /* Initialize LED2 (Green) and LED3 (Blue) */
  GPIO_SOFTWARE_CONTROL(GPIO_B_BASE, LEDS_PORTB_PIN_MASK);
  GPIO_SET_OUTPUT(GPIO_B_BASE, LEDS_PORTB_PIN_MASK);
  GPIO_CLR_PIN(GPIO_B_BASE, LEDS_PORTB_PIN_MASK);

  /* Initialize LED1 (Red) */
  GPIO_SOFTWARE_CONTROL(LEDS_RED_PORT_BASE, LEDS_RED_PIN_MASK);
  GPIO_SET_OUTPUT(LEDS_RED_PORT_BASE, LEDS_RED_PIN_MASK);
  GPIO_CLR_PIN(LEDS_RED_PORT_BASE, LEDS_RED_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  uint8_t mask_leds;

  mask_leds = GPIO_READ_PIN(LEDS_GREEN_PORT_BASE, LEDS_GREEN_PIN_MASK) == 0 ? 0: LEDS_GREEN;
  mask_leds |= GPIO_READ_PIN(LEDS_BLUE_PORT_BASE, LEDS_BLUE_PIN_MASK) == 0 ? 0 : LEDS_BLUE;
  mask_leds |= GPIO_READ_PIN(LEDS_RED_PORT_BASE, LEDS_RED_PIN_MASK) == 0 ? 0 : LEDS_RED;

  return mask_leds;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  if(leds & LEDS_GREEN) {
    GPIO_SET_PIN(LEDS_GREEN_PORT_BASE, LEDS_GREEN_PIN_MASK);
  } else {
    GPIO_CLR_PIN(LEDS_GREEN_PORT_BASE, LEDS_GREEN_PIN_MASK);
  }

  if(leds & LEDS_BLUE) {
    GPIO_SET_PIN(LEDS_BLUE_PORT_BASE, LEDS_BLUE_PIN_MASK);
  } else {
    GPIO_CLR_PIN(LEDS_BLUE_PORT_BASE, LEDS_BLUE_PIN_MASK);
  }

  if(leds & LEDS_RED) {
    GPIO_SET_PIN(LEDS_RED_PORT_BASE, LEDS_RED_PIN_MASK);
  } else {
    GPIO_CLR_PIN(LEDS_RED_PORT_BASE, LEDS_RED_PIN_MASK);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
