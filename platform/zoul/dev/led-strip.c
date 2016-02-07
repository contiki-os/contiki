/*
 * Copyright (c) 2015, Zolertia
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-led-strip
 * @{
 *
 * Driver to control a bright LED strip powered at 3VDC, drawing power directly
 * from the battery power supply.  An example on how to adapt 12VDC LED strips
 * to 3VDC is provided at http://www.hackster.io/zolertia
 * @{
 *
 * \file
 * Driver for a bright LED strip
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/gpio.h"
#include "led-strip.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#ifndef LED_STRIP_PORT
#define LED_STRIP_PORT      GPIO_A_NUM
#endif
#ifndef LED_STRIP_PIN
#define LED_STRIP_PIN       6
#endif
#define LED_STRIP_PORT_BASE GPIO_PORT_TO_BASE(LED_STRIP_PORT)
#define LED_STRIP_PIN_MASK  GPIO_PIN_MASK(LED_STRIP_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t initialized = 0;
/*---------------------------------------------------------------------------*/
void
led_strip_config(void)
{
  /* Software controlled */
  GPIO_SOFTWARE_CONTROL(LED_STRIP_PORT_BASE, LED_STRIP_PIN_MASK);
  /* Set pin to output */
  GPIO_SET_OUTPUT(LED_STRIP_PORT_BASE, LED_STRIP_PIN_MASK);
  /* Set the pin to a default position */
  GPIO_SET_PIN(LED_STRIP_PORT_BASE, LED_STRIP_PIN_MASK);

  initialized = 1;
}
/*---------------------------------------------------------------------------*/
int
led_strip_switch(uint8_t val)
{
  if(!initialized) {
    return LED_STRIP_ERROR;
  }

  if(val != LED_STRIP_ON && val != LED_STRIP_OFF) {
    return LED_STRIP_ERROR;
  }

  /* Set the LED to ON or OFF */
  GPIO_WRITE_PIN(LED_STRIP_PORT_BASE, LED_STRIP_PIN_MASK, val);

  return val;
}
/*---------------------------------------------------------------------------*/
int
led_strip_get(void)
{
  if(!initialized) {
    return LED_STRIP_ERROR;
  }

  /* Inverse logic, return ON if the pin is low */
  if(GPIO_READ_PIN(LED_STRIP_PORT_BASE, LED_STRIP_PIN_MASK)) {
    return LED_STRIP_OFF;
  }
  return LED_STRIP_ON;
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */

