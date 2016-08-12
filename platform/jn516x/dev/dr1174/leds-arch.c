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
#include <AppHardwareApi.h>
#ifdef SENSOR_BOARD_DR1199
#include "dr1199/leds-arch-1199.h"
#endif
#ifdef SENSOR_BOARD_DR1175
#include "leds-extension.h"
#include "dr1175/leds-arch-1175.h"
#endif

#define LED_D3  (1 << 3)
#define LED_D6  (1 << 2)

static volatile unsigned char leds;

/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  vAHI_DioSetDirection(0, LED_D3 | LED_D6);
  vAHI_DioSetOutput(LED_D3 | LED_D6, 0);     /* Default off */
#ifdef SENSOR_BOARD_DR1199
  leds_arch_init_1199();
#endif
#ifdef SENSOR_BOARD_DR1175
  leds_arch_init_1175();
#endif
  leds = 0;
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return leds;
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char c)
{
  uint32 on_mask = 0;
  uint32 off_mask = 0;

  /* LOW level on pins switches ON LED for DR1174 */
  if(c & LEDS_GP0) {
    on_mask |= LED_D3;
  } else {
    off_mask |= LED_D3;
  } if(c & LEDS_GP1) {
    on_mask |= LED_D6;
  } else {
    off_mask |= LED_D6;
  } vAHI_DioSetOutput(off_mask, on_mask);
#ifdef SENSOR_BOARD_DR1199
  /* DR1174 with DR1199 */
  leds_arch_set_1199(c);
  if(c == LEDS_ALL) {
    leds = LEDS_GP0 | LEDS_GP1 | LEDS_RED | LEDS_BLUE | LEDS_GREEN;
  } else {
    leds = (c & (LEDS_GP0 | LEDS_GP1 | LEDS_RED | LEDS_BLUE | LEDS_GREEN));
  }
#elif SENSOR_BOARD_DR1175
  /* DR1174 with DR1175 */
  leds_arch_set_1175(c);
  if(c == LEDS_ALL) {
    leds = LEDS_GP0 | LEDS_GP1 | LEDS_RED | LEDS_BLUE | LEDS_GREEN | LEDS_WHITE;
  } else {
    leds = (c & (LEDS_GP0 | LEDS_GP1 | LEDS_RED | LEDS_BLUE | LEDS_GREEN | LEDS_WHITE));
/*  printf("++++++++++++++++++++ leds_arch_set: leds: 0x%x\n", leds); */
  }
#else
  /* DR1174-only */
  if(c == LEDS_ALL) {
    leds = LEDS_GP0 | LEDS_GP1;
  } else {
    leds = c;
  }
#endif
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set_level(unsigned char level, unsigned char c)
{
#ifdef SENSOR_BOARD_DR1175
  leds_arch_set_level_1175(level, c, leds);
/*  printf("++++++++++++++++++++ leds_arch_set_level: leds: 0x%x\n", leds); */
#endif
}
