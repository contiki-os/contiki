/*
 * Contiki PIC32 Port project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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

/**
 * \addtogroup SeedEye Contiki SEEDEYE Platform
 *
 * @{
 */

/**
 * \file   platform/seedeye/dev/leds-arch.c
 * \brief  LEDs Specific Arch Conf
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#include "contiki-conf.h"

#include "dev/leds.h"

#include "p32xxxx.h"

/*
 * IPM2 has 2 red LEDs, LED1 is mapped to the Contiki
 * LEDS_RED and LED2 is mapped to LEDS_GREEN.
 */

#define LEDS_CONF_GREEN                 0x04
#define LEDS_CONF_RED                   0x08

#define SEEDEYE_BOARD_V2_CAT3(x,y,z)    x ## y ## z

/* NOTE: Led0 works in a negated logic. */
#define led_0_on                        (LATCCLR = LEDS_CONF_RED)
#define led_0_off                       (LATCSET = LEDS_CONF_RED)
#define led_0_toggle                    (LATCINV = LEDS_CONF_RED)
#define led_1_on                        (LATCSET = LEDS_CONF_GREEN)
#define led_1_off                       (LATCCLR = LEDS_CONF_GREEN)
#define led_1_toggle                    (LATCINV = LEDS_CONF_GREEN)
#define led_on(n)                       SEEDEYE_BOARD_V2_CAT3(led_,n,_on)
#define led_off(n)                      SEEDEYE_BOARD_V2_CAT3(led_,n,_off)
#define led_toggle(n)                   SEEDEYE_BOARD_V2_CAT3(led_,n,_toggle)

/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
  TRISCCLR = 0x0C;

  led_0_off;
  led_1_off;
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return (PORTC & LEDS_CONF_RED ? 0 : LEDS_RED) | (~PORTC & LEDS_CONF_GREEN ? 0 : LEDS_GREEN);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  if(leds & LEDS_RED) {
    led_0_on;
  } else {
    led_0_off;
  }

  if(leds & LEDS_GREEN) {
    led_1_on;
  } else {
    led_1_off;
  }
}
/*---------------------------------------------------------------------------*/

/** @} */
