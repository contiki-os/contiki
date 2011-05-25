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

#ifndef __I2CMASTER_H__
#define __I2CMASTER_H__

#include <stdio.h>
#include "contiki.h"
#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif
#include <dev/spi.h>
#include <dev/leds.h>



void i2c_enable(void);

void i2c_receiveinit(u8_t slave_address);
u8_t i2c_receive_n(u8_t byte_ctr, u8_t *rx_buf);

void i2c_transmitinit(u8_t slave_address);
void i2c_transmit_n(u8_t byte_ctr, u8_t *tx_buf);

u8_t i2c_busy(void);

//XXX Temporary place for defines that are lacking in mspgcc4's gpio.h
#ifdef __GNUC__
#ifndef P5SEL2_
  #define P5SEL2_             0x0045  /* Port 5 Selection 2*/
  sfrb(P5SEL2, P5SEL2_);
#endif
#endif
#ifdef __IAR_SYSTEMS_ICC__
#define P5SEL2_              (0x0045u)  /* Port 5 Selection 2*/
DEFC(   P5SEL2             , P5SEL2_)
#endif

//XXX Should these defines be in the contiki-conf.h to make it more platform-independent?
#define I2C_PxDIR   P5DIR
#define I2C_PxIN    P5IN
#define I2C_PxOUT   P5OUT
#define I2C_PxSEL   P5SEL
#define I2C_PxSEL2  P5SEL2
#define I2C_PxREN   P5REN


#define I2C_SDA       (1 << 1)    //SDA == P5.1 
#define I2C_SCL       (1 << 2)    //SCL == P5.2
#define I2C_PRESC_1KHZ_LSB      0x00 
#define I2C_PRESC_1KHZ_MSB      0x20
#define I2C_PRESC_100KHZ_LSB    0x50 
#define I2C_PRESC_100KHZ_MSB    0x00
#define I2C_PRESC_400KHZ_LSB    0x14 
#define I2C_PRESC_400KHZ_MSB    0x00

// I2C configuration with RX interrupts
#ifdef I2C_CONF_RX_WITH_INTERRUPT
#define I2C_RX_WITH_INTERRUPT I2C_CONF_RX_WITH_INTERRUPT // XXX Move I2C_CONF_RX_WITH_INTERRUPT to contiki-conf.h or platform-conf.h
#else /* I2C_CONF_RX_WITH_INTERRUPT */
#define I2C_RX_WITH_INTERRUPT 1
#endif /* I2C_CONF_RX_WITH_INTERRUPT */


#if 0
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif

#endif    /* #ifdef __I2CMASTER_H__ */
