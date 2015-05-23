/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Device drivers header file for tmp102 temperature sensor in Zolertia Z1 WSN Platform.
 * \author
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 */

#ifndef TMP102_H_
#define TMP102_H_
#include <stdio.h>
#include "i2cmaster.h"

/* -------------------------------------------------------------------------- */
/* Init the temperature sensor: ports, pins, I2C, interrupts (XXX none so far),
*/
void  tmp102_init(void);

/* Write to a register.
    args:
      reg       register to write to
      val       value to write
*/
void    tmp102_write_reg(uint8_t reg, uint16_t val);

/* Read one register.
    args:
      reg       what register to read
    returns the value of the read register
*/
uint16_t tmp102_read_reg(uint8_t reg);

/* Read temperature in raw format
    no args needed
*/
uint16_t tmp102_read_temp_raw();

/* Read only integer part of the temperature in 1deg. precision.
    no args needed
*/
int8_t tmp102_read_temp_simple();

/* Read only integer part of the temperature in 1deg. precision.
    no args needed
*/
int16_t tmp102_read_temp_x100();

/* -------------------------------------------------------------------------- */
/* Reference definitions */
/* TMP102 slave address */
#define TMP102_ADDR           0x48

/* TMP102 registers */
#define TMP102_TEMP           0x00    // read only
#define TMP102_CONF           0x01    
#define TMP102_TLOW           0x02    
#define TMP102_THIGH          0x03   

/* TMP102 Ports */
/* Accelerometer hardware ports, pins and registers on the msp430 µC */
#define TMP102_PWR_DIR        P5DIR
#define TMP102_PWR_SEL        P5SEL
#define TMP102_PWR_SEL2       P5SEL2
#define TMP102_PWR_REN        P5REN
#define TMP102_PWR_OUT        P5OUT
#define TMP102_PWR_PIN        (1<<0)          // P5.0
//#define TMP102_INT_PIN      (1<<7)          // P1.7


/* -------------------------------------------------------------------------- */
#endif /* ifndef TMP102_H_ */



