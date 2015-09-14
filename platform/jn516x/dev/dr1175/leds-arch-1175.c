/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
 * Author: Theo van Daele <theo.van.daele@nxp.com>
 *
 */
#include "contiki.h"
#include "dev/leds.h"
#include <LightingBoard.h>

static uint8_t white_level;
static uint8_t red_level;
static uint8_t green_level;
static uint8_t blue_level;

/*---------------------------------------------------------------------------*/
void
leds_arch_init_1175(void)
{
  /* White LED initialisation */
  white_level = 0;
  bWhite_LED_Enable();
  bWhite_LED_SetLevel(0);
  bWhite_LED_On();
  /* Coloured LED initialisation */
  red_level = 0;
  green_level = 0;
  blue_level = 0;
  bRGB_LED_Enable();
  bRGB_LED_SetGroupLevel(255);
  bRGB_LED_SetLevel(0, 0, 0);
  bRGB_LED_On();
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set_1175(unsigned char c)
{
  bWhite_LED_SetLevel(c & LEDS_WHITE ? white_level : 0);
  bRGB_LED_SetLevel(c & LEDS_RED ? red_level : 0,
                    c & LEDS_GREEN ? green_level : 0,
                    c & LEDS_BLUE ? blue_level : 0);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set_level_1175(unsigned char level, unsigned char c, unsigned char leds)
{
  if(c & LEDS_WHITE) {
    white_level = level;
  }
  if(c & LEDS_RED) {
    red_level = level;
  }
  if(c & LEDS_GREEN) {
    green_level = level;
  }
  if(c & LEDS_BLUE) {
    blue_level = level;
    /* Activate level if LED is on */
  }
  bRGB_LED_SetLevel(leds & LEDS_RED ? red_level : 0,
                    leds & LEDS_GREEN ? green_level : 0,
                    leds & LEDS_BLUE ? blue_level : 0);
  bWhite_LED_SetLevel(leds & LEDS_WHITE ? white_level : 0);
}