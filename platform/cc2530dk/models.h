/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 */

/**
 * \file
 *         Header file use to configure differences between cc2530dk builds for
 *         the SmartRF/cc2530 and the cc2531 USB stick.
 *
 *         These configuration directives are hardware-specific and you
 *         normally won't have to modify them.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef __MODELS_H__
#define __MODELS_H__

/*---------------------------------------------------------------------------*/
/* LEDs */
/*---------------------------------------------------------------------------*/
/* Some files include leds.h before us */
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_RED
#define LEDS_YELLOW 4

/*
 * Smart RF LEDs
 *  1: P1_0 (Green)
 *  2: P1_1 (Red)
 *  3: P1_4 (Yellow)
 *  4: P0_1 (LED4 shares port/pin with B1 and is currently unused)
 *
 * USB Dongle LEDs
 *  1: P0_0 (Red)
 *  2: P1_1 (Green - active: low)
 */
#if MODELS_CONF_CC2531_USB_STICK
#define MODEL_STRING "TI cc2531 USB Dongle\n"
#define USB_CONF_ENABLE 1

/* Enabled the ISR for Interrupt Num 6 */
#define PORT_2_ISR_ENABLED 1

#ifndef USB_CONF_CLASS
#define DMA_CONF_ON	1
#define DMA_USB_CHANNEL	0
#define CTRL_EP_SIZE	8
#define USB_EP1_SIZE	32
#define USB_EP2_SIZE	64
#define USB_EP3_SIZE	64
#define USB_CONF_CLASS	1 /* CDC-ACM */
#endif

#define CDC_ACM_CONF_VID 0x0451 /* Vendor: TI */
#define CDC_ACM_CONF_PID 0x16A8 /* cc2531 CDC */

#undef LEDS_CONF_ALL
#define LEDS_CONF_ALL 3
#define LEDS_RED      1
#define LEDS_GREEN    2

/* H/W Connections */
#define LED2_PIN   P0_0
#define LED1_PIN   P1_1

/* P0DIR and P0SEL masks */
#define LED2_MASK  0x01
#define LED1_MASK  0x02
#else
#define MODEL_STRING "TI SmartRF05 EB\n"
#define LEDS_GREEN    1
#define LEDS_RED      2

/* H/W Connections */
#define LED1_PIN   P1_0
#define LED2_PIN   P1_1
#define LED3_PIN   P1_4

/* P0DIR and P0SEL masks */
#define LED1_MASK  0x01
#define LED2_MASK  0x02
#define LED3_MASK  0x10
#define LED4_MASK  0x02
#endif
/*---------------------------------------------------------------------------*/
/* Buttons */
/*---------------------------------------------------------------------------*/

#endif /* __MODELS_H__ */
