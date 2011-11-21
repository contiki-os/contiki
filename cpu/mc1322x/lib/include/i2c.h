/*
 * Copyright (c) 2011, Hedde Bosman <heddebosman@incas3.eu>
 *
 * I2C communication device drivers for mc1322x
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
 * $Id$
 */

#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "isr.h"
#include "gpio.h"

#define I2C_NON_BLOCKING 1

/* The I2C Interrupt Service Routine */
void i2c_isr (void);

/* Enable the I2C module */
void i2c_enable(void);

/* Disable the I2C module */
void i2c_disable(void);



/* Returns 1 if the I2C bus is active (we or some other device are transferring data) */
uint8_t i2c_busy(void);

/* Returns 1 if our data is sent or received */
uint8_t i2c_transferred(void); // indicates success of data last transfer (send or receive)



/* start receiving data from 'slave_address' of length 'byte_ctr' and store it in 'rx_buf'
 * @parameter slave_addr	7bits (and will be padded with the 'receive bit')
 * @parameter byte_ctr 		the number of bytes expected to be received
 * @parameter rx_buf		the buffer in which the received bytes will be stored.
 * 
 * IFDEF I2C_NON_BLOCKING THEN THIS FUNCTION RETURNS IMMEDIATLY BUT THE TRANSFER WILL ONLY 
 * BE COMPLETE WHEN i2c_transferred IS 1
 */
void i2c_receiveinit( uint8_t slave_address, uint8_t byte_ctr, uint8_t *rx_buf);

/* start sending data to 'slave_address' of length 'byte_ctr' found  in the buffer 'rx_buf'
 * @parameter slave_addr	7bits (and will be padded with the 'send bit')
 * @parameter byte_ctr 		the number of bytes to be sent
 * @parameter tx_buf		the buffer in which the bytes to be sent are stored
 * 
 * IFDEF I2C_NON_BLOCKING THEN THIS FUNCTION RETURNS IMMEDIATLY BUT THE TRANSFER WILL ONLY 
 * BE COMPLETE WHEN i2c_transferred IS 1
 */
void i2c_transmitinit(uint8_t slave_address, uint8_t byte_ctr, uint8_t *tx_buf);


#ifndef I2C_NON_BLOCKING
#define I2C_NON_BLOCKING 1
#endif

#if I2C_NON_BLOCKING
uint8_t i2c_receive(void);
void    i2c_transmit(void);
#endif

// TODO: see if this is better fit in platform definition
#define I2C_SDA       13    //SDA == P5.1 // GPIO 13
#define I2C_SCL       12    //SCL == P5.2 // GPIO 12

// TODO: this could use a nice struct with bit members...

#define I2C_BASE (0x80006000)

#define I2CADR		((volatile uint8_t *) ( I2C_BASE + 0x00	))
#define I2CFDR		((volatile uint8_t *) ( I2C_BASE + 0x04	))
#define I2CCR		((volatile uint8_t *) ( I2C_BASE + 0x08	))
#define I2CSR		((volatile uint8_t *) ( I2C_BASE + 0x0C	))
#define I2CDR		((volatile uint8_t *) ( I2C_BASE + 0x10	))
#define I2CDFSRR	((volatile uint8_t *) ( I2C_BASE + 0x14	))
#define I2CCKER		((volatile uint8_t *) ( I2C_BASE + 0x18	))

// TODO: fix nice structs

// i2c CR
#define I2C_MEN		0x80
#define I2C_MIEN	0x40
#define I2C_MSTA	0x20
#define I2C_MTX		0x10
#define I2C_TXAK	0x08
#define I2C_RSTA	0x04
#define I2C_BCST	0x01

// i2c SR
#define I2C_MCF		0x80
#define I2C_MAAS	0x40
#define I2C_MBB		0x20
#define I2C_MAL		0x10
#define I2C_BCSTM	0x08
#define I2C_SRW		0x04
#define I2C_MIF		0x02
#define I2C_RXAK	0x01

// i2c digital filter sample rate register
#define I2C_DFSR 	0x3f // default = 0x10

// i2c CKER
#define I2C_CKEN	0x01


#endif
