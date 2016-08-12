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
 *         I2C communication device driver header file for Zolertia Z1 sensor node.
 * \author
 *         Enric Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 */

#ifndef I2CMASTER_H_
#define I2CMASTER_H_

#include "contiki.h"

void i2c_enable(void);
void i2c_disable(void);

void i2c_receiveinit(uint8_t slave_address);
uint8_t i2c_receive_n(uint8_t byte_ctr, uint8_t *rx_buf);

void i2c_transmitinit(uint8_t slave_address);
void i2c_transmit_n(uint8_t byte_ctr, uint8_t *tx_buf);

uint8_t i2c_busy(void);
void i2c_setrate(uint8_t p_lsb, uint8_t p_msb);

#if 0
#include <stdio.h>
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif

#endif /* #ifdef I2CMASTER_H_ */
