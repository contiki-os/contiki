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
 */
/* -------------------------------------------------------------------------- */
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-led-strip LED strip compatible with Zoul-based platforms
 *
 * Driver to control a bright LED strip powered at 3VDC, drawing power directly
 * from the battery power supply.  An example on how to adapt 12VDC LED strips
 * to 3VDC is provided at http://www.hackster.io/zolertia
 * @{
 *
 * \file
 * Header file for a bright LED strip driver
 */
/* -------------------------------------------------------------------------- */
#ifndef LED_STRIP_H_
#define LED_STRIP_H_
/* -------------------------------------------------------------------------- */
#include <stdint.h>
/* -------------------------------------------------------------------------- */
#define LED_STRIP_OFF   0xFF
#define LED_STRIP_ON    0x00

#define LED_STRIP_ERROR   -1
/* -------------------------------------------------------------------------- */
/**
 * \brief Init function for the bright LED strip driver
 *
 * The LED strip driver allows to lighten up any application using up to 4
 * LEDs 3VDC-powered per strip
 * The function is set to power OFF the LEDs as default,
 * it should be called from the contiki-main initialization process.
 *
 * \return ignored
 */
void led_strip_config(void);

/**
 * \brief Function to turn ON/OFF the LED strip
 *
 * \param val Set ON/OFF (LED_STRIP_ON or LED_STRIP_OFF)
 * \return the selected antenna position, or LED_STRIP_ERROR if not
 *         previously configured
 */
int led_strip_switch(uint8_t val);

/**
 * \brief Function to get the LED strip current state
 *
 * \return Current LED strip state or LED_STRIP_ERROR if not
 *         previously configured
 */
int led_strip_get(void);
/* -------------------------------------------------------------------------- */
#endif /* ifndef LED_STRIP_H_ */
/* -------------------------------------------------------------------------- */
/**
 * @}
 * @}
 */

