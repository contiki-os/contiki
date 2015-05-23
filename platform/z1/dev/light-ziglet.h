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
 *         Device drivers header file for light ziglet sensor in Zolertia Z1 WSN Platform.
 * \author
 *         Antonio Lignan, Zolertia <alinan@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 */

#ifndef LIGHT_ZIGLET_H_
#define LIGHT_ZIGLET_H_
#include <stdio.h>
#include "i2cmaster.h"

/* Init the light ziglet sensor: ports, pins, I2C, interrupts */
void  light_ziglet_init(void);

/* Write to a register.
    args:
      reg       register to write to
      val       value to write
 */
void   tsl2563_write_reg(uint8_t reg, uint16_t val);

/* Read one register.
    args:
      reg       what register to read
    returns the value of the read register
 */
uint16_t tsl2563_read_reg(uint8_t reg);

/* Takes a single light reading
   args: none
   returns a lux value
 */
uint16_t light_ziglet_read();

/* Calculates the lux values from the calibration table
   args: raw values from sensor
   returns a lux value
 */
uint16_t calculateLux(uint16_t *readRaw);

/* Turns the light ziglet ON and polls the sensor for a light reading */
uint16_t light_ziglet_on(void);

/* -------------------------------------------------------------------------- */
/* Reference definitions */

/* TSL2563 slave address */
#define TSL2563_ADDR 0x39

/* Registers */
#define TSL2563_READ 0xAC
#define TSL2563_PWRN 0x03

/* Calibration settings */
#define K1T 0X0040
#define B1T 0x01f2
#define M1T 0x01b2

#define K2T 0x0080
#define B2T 0x0214
#define M2T 0x02d1

#define K3T 0x00c0
#define B3T 0x023f
#define M3T 0x037b

#define K4T 0x0100
#define B4T 0x0270
#define M4T 0x03fe

#define K5T 0x0138
#define B5T 0x016f
#define M5T 0x01fc

#define K6T 0x019a
#define B6T 0x00d2
#define M6T 0x00fb

#define K7T 0x029a
#define B7T 0x0018
#define M7T 0x0012

#define K8T 0x029a
#define B8T 0x0000
#define M8T 0x0000

/* -------------------------------------------------------------------------- */
#endif /* ifndef LIGHT_ZIGLET_H_ */

