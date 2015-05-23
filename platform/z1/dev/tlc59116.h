/*
 * Copyright (c) 2013, Jelmer Tiete.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * This file is part of the Contiki operating system.
 * 
 */

/**
 * \file
 *         Device drivers header file for TLC59116 i2c led driver on Zolertia Z1.
 *         See http://www.ti.com/product/tlc59116 for datasheet.
 * \author
 *         Jelmer Tiete, VUB <jelmer@tiete.be>
 */

#ifndef TLC59116_H_
#define TLC59116_H_
#include <stdio.h>
#include "dev/i2cmaster.h"

#if 0
#include <stdio.h>
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif


/* -------------------------------------------------------------------------- */
/* Init the led driver: ports, pins, registers, I2C*/
void tlc59116_init(void);

/* Write to a register.
 *  args:
 *    reg       register to write to
 *    val       value to write
 */
void tlc59116_write_reg(uint8_t reg, uint8_t val);

/* Write several registers from a stream.
 *  args:
 *    len       number of bytes to read
 *    data      pointer to where the data is read from
 * First byte in stream must be the register address to begin writing to.
 * The data is then written from the second byte and increasing. The address byte
 * is not included in length len.
 */
void tlc59116_write_stream(uint8_t len, uint8_t * data);

/* Read one register.
 *  args:
 *    reg       what register to read
 *  returns the value of the read register
 */
uint8_t tlc59116_read_reg(uint8_t reg);

/* Read several registers in a stream.
 *  args:
 *    reg       what register to start reading from
 *    len       number of bytes to read
 *    whereto   pointer to where the data is saved
 */
void tlc59116_read_stream(uint8_t reg, uint8_t len, uint8_t * whereto);

/* Set pwm value for individual led
 *  args:
 *    led       led output -> 0 till 15
 *    pwm       led pwm value
 */
void tlc59116_led(uint8_t led, uint8_t pwm);


/* -------------------------------------------------------------------------- */
/* Application definitions, change if required by application. */

/* Suggested defaults according to the data sheet etc */
#define TLC59116_MODE1_DEFAULT   0x00   // Default (no sub or all call) + OSC on
#define TLC59116_MODE2_DEFAULT   0x00   // Default (output change on stop)

#define TLC59116_LEDOUT_PWM      0xAA   // LDRx = 01 -> PWM; 4 leds per reg: 01010101b -> 0xAA

/* -------------------------------------------------------------------------- */
/* Reference definitions, should not be changed */
/* TLC59116 slave address */
#define TLC59116_ADDR            0x60   //7bit adress, 8bit write adress: 0xC0
                                        //address with all address pins pulled to ground
/* TLC59116 registers */
#define TLC59116_MODE1           0x00
#define TLC59116_MODE2           0x01
#define TLC59116_PWM0_AUTOINCR   0xA2   //auto increment address for first pwm register
#define TLC59116_PWM0            0x02
#define TLC59116_PWM1            0x03
#define TLC59116_PWM2            0x04
#define TLC59116_PWM3            0x05
#define TLC59116_PWM4            0x06
#define TLC59116_PWM5            0x07
#define TLC59116_PWM6            0x08
#define TLC59116_PWM7            0x09
#define TLC59116_PWM8            0x0A
#define TLC59116_PWM9            0x0B
#define TLC59116_PWM10           0x0C
#define TLC59116_PWM11           0x0D
#define TLC59116_PWM12           0x0E
#define TLC59116_PWM13           0x0F
#define TLC59116_PWM14           0x10
#define TLC59116_PWM15           0x11
#define TLC59116_GRPPWM          0x12
#define TLC59116_GRPFREQ         0x13
#define TLC59116_LEDOUT0         0x14
#define TLC59116_LEDOUT1         0x15
#define TLC59116_LEDOUT2         0x16
#define TLC59116_LEDOUT3         0x17

/* More registers follow, but not used in this implementation */

/* -------------------------------------------------------------------------- */
#endif /* ifndef TLC59116_H_ */
