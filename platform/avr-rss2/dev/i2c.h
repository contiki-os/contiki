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
 */

/**
 *
 * Some code adopted from Robert Olsson <robert@herjulf.se> and Manee
 * @Author: 
 * Nsabagwa Mary <mnsabagwa@cit.ac.ug>
 * Okello Joel <okellojoelocaye@gmail.com>,
 * Alinitwe Sandra Kamugisha <sandraalinitwe@gmail.com>
 * Byamukama Maximus <maximus.byamukama@gmail.com>
 */

#include "contiki.h"

/* Here we define the i2c address for dev we support */
#define I2C_AT24MAC_ADDR  0xB0 /* EUI64 ADDR */
#define I2C_SHT25_ADDR    (0x40 << 1) /* SHT2X ADDR */
#define I2C_DS1307_ADDR   0xD2 /* DS1307 rtc */
#define I2C_MS5611_ADDR   0xEC /* ms5611 */
#define I2C_MCP3424_ADDR  (0x68<<1)//(0x68<<1)/*ADC */

/* Here we define a enumration for devices */
#define I2C_AT24MAC       (1<<0)
#define I2C_SHT25         (1<<1)
#define I2C_DS1307        (1<<2)
#define I2C_MS5611	  (1<<3)
#define I2C_MCP3424	  (1<<4)

 #define ADDR_W      0xEC  
/* define CPU frequency in Mhz here if not defined in Makefile */

#ifndef F_CPU
#define F_CPU 16000000L
#endif

/* I2C clock in Hz */
#define F_SCL  100000L
#define I2C_READ    1
#define I2C_WRITE   0

void i2c_init(uint32_t speed);
uint8_t i2c_start(uint8_t addr);
void i2c_start_wait(uint8_t addr);
void i2c_stop(void);
uint8_t i2c_readNak(void);
void i2c_write(uint8_t u8data);
void i2c_read_mem(uint8_t addr, uint8_t reg, uint8_t buf[], uint8_t bytes);
void i2c_write_mem(uint8_t addr, uint8_t reg, uint8_t value);
void i2c_at24mac_read(char *buf, uint8_t eui64);
unsigned char i2c_write1(unsigned char data);
uint16_t i2c_probe(void);
extern uint16_t i2c_probed; /* i2c devices we have probed */
void i2c_send(char cmd);
uint8_t i2c_readAck(void);
