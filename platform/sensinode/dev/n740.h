/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 *         Header File for the module which controls the Sensinode N740
 *         8-bit serial-in/serial or parallel-out shift register.
 *
 *         This is where the Accelerometer, Leds, Light and Battery Sensors
 *         are connected.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef N740SERPAR_H_
#define N740SERPAR_H_

#include "8051def.h"

#define N740_SER_PAR_ACC_GSEL    0x01 /* Acceleration Sensor g-Select */
#define N740_SER_PAR_CHIP_SEL    0x02 /* Flash Chip Select */
#define N740_SER_PAR_LIGHT       0x04 /* Light Sensor */
#define N740_SER_PAR_ACC         0x08 /* Acceleration Sensor */
#define N740_SER_PAR_RF_IN_GAIN  0x10 /* Receiver Amplifier, best not set */
#define N740_SER_PAR_U5_ENABLE   0x20 /* U5 analog switch enable */
#define N740_SER_PAR_LED_GREEN   0x40 /* Led 1 */
#define N740_SER_PAR_LED_RED     0x80 /* Led 2 */

#define N740_ANALOG_SWITCH_USB      0
#define N740_ANALOG_SWITCH_SERIAL   1

#define N740_PINS 0xE0
#define N740_PINS_GPIO()   {P1SEL &= ~N740_PINS;}
#define N740_PINS_PER_IO() {P1SEL |= N740_PINS;}

/* Serial/Parallel Shift Register (74HC595D) Functions */
void n740_ser_par_init(void);
void n740_ser_par_set(uint8_t data) ;
uint8_t n740_ser_par_get(void);

/* Analog Switch (U5 - 74HC4053D) Functions */
void n740_analog_switch(uint8_t state);
void n740_analog_activate();
void n740_analog_deactivate();

#endif /* N740SERPAR_H_ */
