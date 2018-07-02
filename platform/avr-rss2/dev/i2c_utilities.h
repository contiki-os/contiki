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
 * \file
 *         includes for i2c core functions
 * \author
 *         Robert Olsson <robert@radio-sensors.com>
 */

#include "contiki.h"

/* Here we define the i2c address for dev we support */
#define I2C_AT24MAC_ADDR  0xB0 /* EUI64 ADDR */
#define I2C_SHT25_ADDR    (0x40 << 1) /* SHT2X ADDR */
#define I2C_DS1307_ADDR    0xD0 /* DS1307 rtc */

/* Here we define a enumration for devices */
#define I2C_AT24MAC       (1<<0)
#define I2C_SHT25         (1<<1)

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 16000000L
#endif

/* I2C clock in Hz */
#define F_SCL  100000L

#define REG(x)         (*((volatile unsigned long *)(x)))


#define I2C_MASTER_CMD_SINGLE_SEND              0x00000007
#define I2C_MASTER_CMD_SINGLE_RECEIVE           0x00000007
#define I2C_MASTER_CMD_BURST_SEND_START         0x00000003
#define I2C_MASTER_CMD_BURST_SEND_CONT          0x00000001
#define I2C_MASTER_CMD_BURST_SEND_FINISH        0x00000005
#define I2C_MASTER_CMD_BURST_SEND_ERROR_STOP    0x00000004
#define I2C_MASTER_CMD_BURST_RECEIVE_START      0x0000000b
#define I2C_MASTER_CMD_BURST_RECEIVE_CONT       0x00000009
#define I2C_MASTER_CMD_BURST_RECEIVE_FINISH     0x00000005
#define I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP 0x00000004


#define I2C_MASTER_ERR_NONE          0
#define I2CM_STAT_BUSY      0x00000001
#define I2CM_STAT_ERROR     0x00000002
#define I2CM_STAT_ADRACK    0x00000004
#define I2CM_STAT_DATACK    0x00000008
#define I2CM_STAT_ARBLST    0x00000010
#define I2CM_STAT_IDLE      0x00000020
#define I2CM_STAT_BUSBSY    0x00000040
#define I2CM_STAT_INVALID   0x00000080



#define I2CM_CR   0x40020020  /* I2C master config */
#define I2CM_TPR  0x4002000C  /* I2C master timer period */
#define I2CM_SA   0x40020000  /* I2C master slave address */
#define I2CM_DR   0x40020008  /* I2C master data */
#define I2CM_CTRL 0x40020004  /* Master control in write */
#define I2CM_STAT I2CM_CTRL   /* Master status in read */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name I2C Miscellaneous
 * @{
 */
#define I2C_SCL_NORMAL_BUS_SPEED  100000  /* 100KHz I2C */
#define I2C_SCL_FAST_BUS_SPEED    400000  /* 400KHz I2C */
#define I2C_RECEIVE                 0x01  /* Master receive */
#define I2C_SEND                    0x00  /* Master s */
uint8_t i2c_master_data_get(void);
uint8_t i2c_single_receive(uint8_t slave_addr, uint8_t *data);
uint8_t i2c_burst_receive(uint8_t slave_addr, uint8_t *data, uint8_t len);
uint8_t i2c_master_error(void);
uint8_t i2c_master_busy(void);
void i2c_master_command(uint8_t cmd);
void i2c_master_data_put(uint8_t data);
void i2c_master_set_slave_address(uint8_t slave_addr, uint8_t access_mode);
uint8_t i2c_single_send(uint8_t slave_addr, uint8_t data);
uint8_t i2c_burst_send(uint8_t slave_addr, uint8_t *data, uint8_t len);
uint8_t i2c_burst_send(uint8_t slave_addr, uint8_t *data, uint8_t len);
void i2c_master_enable(void);
void i2c_write(uint8_t u8data);
void
i2c_read_mem(uint8_t addr, uint8_t reg, uint8_t buf[], uint8_t bytes);
uint8_t i2c_readAck(void);
uint8_t i2c_getstatus(void);
void i2c_read_mem(uint8_t addr, uint8_t reg, uint8_t buf[], uint8_t bytes);


extern uint16_t i2c_probed; /* i2c devices we have probed */
