/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 * \file
 *         Device drivers header file for MTS300 sensor board.
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#ifndef __MTS300_H__
#define __MTS300_H__

#include <avr/io.h>
#include "contiki-conf.h"

#define SOUNDER_PORT PORTC
#define SOUNDER_MASK _BV(2)
#define SOUNDER_DDR DDRC

/* MTS300CA and MTS310CA, the light sensor power is controlled 
 * by setting signal INT1(PORTE pin 5).
 * Both light and thermistor use the same ADC channel.
 */
#define LIGHT_PORT_DDR DDRE
#define LIGHT_PORT PORTE
#define LIGHT_PIN_MASK _BV(5)
#define LIGHT_ADC_CHANNEL 1

/* MTS300CA and MTS310CA, the thermistor power is controlled 
 * by setting signal INT2(PORTE pin 6).
 * Both light and thermistor use the same ADC channel.
 */
#define TEMP_PORT_DDR DDRE
#define TEMP_PORT PORTE
#define TEMP_PIN_MASK _BV(6)
#define TEMP_ADC_CHANNEL 1

/* Power is controlled to the accelerometer by setting signal
 * PW4(PORTC pin 4), and the analog data is sampled on ADC3 and ADC4.
 */
#define ACCEL_PORT_DDR DDRC
#define ACCEL_PORT PORTC
#define ACCEL_PIN_MASK _BV(4)
#define ACCELX_ADC_CHANNEL 3
#define ACCELY_ADC_CHANNEL 4

/* Power is controlled to the magnetometer by setting signal
 * PW5(PORTC pin 5), and the analog data is sampled on ADC5 and ADC6.
 */
#define MAGNET_PORT_DDR DDRC
#define MAGNET_PORT PORTC
#define MAGNET_PIN_MASK _BV(5)
#define MAGNETX_ADC_CHANNEL 5
#define MAGNETY_ADC_CHANNEL 6


#define MIC_PORT_DDR DDRC
#define MIC_PORT PORTC
#define MIC_PIN_MASK _BV(3)
#define MIC_ADC_CHANNEL 2

void sounder_on();
void sounder_off();

uint16_t get_light();
uint16_t get_temp();

uint16_t get_accx();
uint16_t get_accy();

uint16_t get_magx();
uint16_t get_magy();

uint16_t get_mic();

void mts300_init();

#endif /* __MTS300_H__ */



