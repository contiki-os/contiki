/*
 * Copyright (c) 2008, Technical University of Munich
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
 * This file is part of the Contiki operating system.
 *
 * @(#)$$
 */

/**
 * \addtogroup usbstick
 *
 */

#ifndef __CONTIKI_RAVEN_H__
#define __CONTIKI_RAVEN_H__

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

/* LED's for Raven USB
 * Led0 = Blue
 * Led1 = Red
 * Led2 = Green
 * Led3 = Orange
 */
#define Leds_init()                 (DDRD  |=  0xA0, DDRE  |=  0xC0)
#define Led0_on()                   (PORTD |=  0x80)
#define Led1_on()                   (PORTD &= ~0x20)
#define Led2_on()                   (PORTE &= ~0x80)
#define Led3_on()                   (PORTE &= ~0x40)
#define Led0_off()                  (PORTD &= ~0x80)
#define Led1_off()                  (PORTD |=  0x20)
#define Led2_off()                  (PORTE |=  0x80)
#define Led3_off()                  (PORTE |=  0x40)
#define Led0_toggle()               (PIND |= 0x80)
#define Led1_toggle()               (PIND |= 0x20)
#define Led2_toggle()               (PINE |= 0x80)
#define Led3_toggle()               (PINE |= 0x40)
#define Leds_on()                   (Led0_on(),Led1_on(),Led2_on(),Led3_on())
#define Leds_off()                  (Led0_off(),Led1_off(),Led2_off(),Led3_off())

#if I_HATE_BLUE_LEDS || JACKDAW_CONF_ALT_LED_SCHEME
/*	The original LED scheme of this platform
 *	used the blue LED pretty much as a power light,
 *	leaving it on continuously after USB enumeration.
 *	This was driving me CRAZY, so I changed the scheme
 *	around a bit so that the orange LED is now used
 *	for status. I think this is much better, and
 *	here is why:
 *
 *	* Blue LEDs are annoyingly blury to look at,
 *	  causing eye strain in the long term.
 *	* The Rods (the non-color sensitive cells) on your
 *	  retina are more sensitive to blue wavelengths than
 *	  any other wavelength. This is what makes blue
 *	  LEDs seem so blindingly bright in a dark room. This
 *	  is called the Purkinje effect.
 *
 *	Thus, I think it is best to reserve the blue LED
 *	for the *least* used function: the VCP activity LED.
 *
 *	My scheme also reverses the red and green LEDs, so that
 *	(R)ed is (R)eceive (from the radio's perspective).
 *
 *	References:
 *	 * <http://van.physics.illinois.edu/qa/listing.php?id=1871>
 *	 * <http://texyt.com/bright+blue+leds+annoyance+health+risks>
 *	 * <http://en.wikipedia.org/wiki/Purkinje_effect>
 *
 *	- darco (11-15-2010)
 */
#define jackdaw_led_RX_on()          Led1_on()
#define jackdaw_led_RX_off()         Led1_off()

#define jackdaw_led_TX_on()          Led2_on()
#define jackdaw_led_TX_off()         Led2_off()

#define jackdaw_led_STAT_on()        Led3_on()
#define jackdaw_led_STAT_off()       Led3_off()
#define jackdaw_led_STAT_toggle()    Led3_toggle()

#define jackdaw_led_VCP_on()         Led0_on()
#define jackdaw_led_VCP_off()        Led0_off()
#define jackdaw_led_VCP_toggle()     Led0_toggle()

#else /* !(I_HATE_BLUE_LEDS || JACKDAW_CONF_ALT_LED_SCHEME) */

#define jackdaw_led_RX_on()          Led1_on()
#define jackdaw_led_RX_off()         Led1_off()

#define jackdaw_led_TX_on()          Led2_on()
#define jackdaw_led_TX_off()         Led2_off()

#define jackdaw_led_STAT_on()        Led0_on()
#define jackdaw_led_STAT_off()       Led0_off()
#define jackdaw_led_STAT_toggle()    Led0_toggle()

#define jackdaw_led_VCP_on()         Led3_on()
#define jackdaw_led_VCP_off()        Led3_off()
#define jackdaw_led_VCP_toggle()     Led3_toggle()

#endif /* !(I_HATE_BLUE_LEDS || JACKDAW_CONF_ALT_LED_SCHEME) */

void init_lowlevel(void);
void init_net(void);


#endif /* #ifndef __CONTIKI_RAVEN_H__ */
