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
 * \addtogroup remote-antenna
 * @{
 *
 * Driver for the RE-Mote RF switch, to enable either the built-in 2.4GHz RF
 * interface of the CC2538, or the CC1200 Sub-1GHz RF interface, both routed to
 * the RP-SMA connector for an external antenna.
 * When the 2.4GHz RF interface is enabled, the CC1200 is powered down.
 * When the CC1200 is enabled, alternatively the 2.4GHz can be also used if
 * placing an 0Ohm resistor (R19), to connect either via a non-mounted chip
 * antenna, or with an external antenna connected to a non-mounted U.Fl
 * connector with a pigtail.
 *
 * RF switch state:
 * - LOW: 2.4GHz RF interface on RP-SMA connector, CC1200 powered-off.
 * - HIGH: Sub-1GHz RF interface on RP-SMA connector.
 * @{
 *
 * \file
 * Driver for the RE-Mote RF antenna switch
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/gpio.h"
#include "antenna-sw.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define ANTENNA_RF_SW_PORT_BASE  GPIO_PORT_TO_BASE(ANTENNA_RF_SW_PORT)
#define ANTENNA_RF_SW_PIN_MASK   GPIO_PIN_MASK(ANTENNA_RF_SW_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t initialized = 0;
/*---------------------------------------------------------------------------*/
void
antenna_sw_config(void)
{
  /* Software controlled */
  GPIO_SOFTWARE_CONTROL(ANTENNA_RF_SW_PORT_BASE,
                        ANTENNA_RF_SW_PIN_MASK);

  /* Set pin to output */
  GPIO_SET_OUTPUT(ANTENNA_RF_SW_PORT_BASE, ANTENNA_RF_SW_PIN_MASK);

  /* Set the antenna selector to a default position */
  GPIO_WRITE_PIN(ANTENNA_RF_SW_PORT_BASE, ANTENNA_RF_SW_PIN_MASK,
                 ANTENNA_SW_SELECT_DEFAULT);

  initialized = 1;
}
/*---------------------------------------------------------------------------*/
int
antenna_sw_get(void)
{
  if(!initialized) {
    return ANTENNA_SW_SELECT_ERROR;
  }

  /* Set the antenna selector */
  return GPIO_READ_PIN(ANTENNA_RF_SW_PORT_BASE, ANTENNA_RF_SW_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
int
antenna_sw_select(uint8_t val)
{
  if(!initialized) {
    return ANTENNA_SW_SELECT_ERROR;
  }

  if(val != ANTENNA_SW_SELECT_SUBGHZ && val != ANTENNA_SW_SELECT_2_4GHZ) {
    return ANTENNA_SW_SELECT_ERROR;
  }

  if(val & antenna_sw_get()) {
    return val;
  }

  /* Set the antenna selector */
  GPIO_WRITE_PIN(ANTENNA_RF_SW_PORT_BASE, ANTENNA_RF_SW_PIN_MASK, val);

  return val;
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
