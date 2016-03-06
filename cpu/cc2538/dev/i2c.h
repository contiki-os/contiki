/*
 * Copyright (c) 2015, Mehdi Migault
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-i2c cc2538 I2C Control
 *
 * cc2538 I2C Control Module
 * @{
 *
 * \file
 * Header file with declarations for the I2C Control module
 *
 * \author
 * Mehdi Migault
 */
#ifndef I2C_H_
#define I2C_H_

#include "reg.h"
#include "sys-ctrl.h"
#include "gpio.h"
#include "ioc.h"
#include <stdio.h>    /* For debug */
#include "clock.h"    /* For temporisation */
/*---------------------------------------------------------------------------*/
/** \name I2C Master commands
 * @{
 */
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
/** @} */
/*---------------------------------------------------------------------------*/
/** \name I2C Master status flags
 * @{
 */
#define I2C_MASTER_ERR_NONE          0
#define I2CM_STAT_BUSY      0x00000001
#define I2CM_STAT_ERROR     0x00000002
#define I2CM_STAT_ADRACK    0x00000004
#define I2CM_STAT_DATACK    0x00000008
#define I2CM_STAT_ARBLST    0x00000010
#define I2CM_STAT_IDLE      0x00000020
#define I2CM_STAT_BUSBSY    0x00000040
#define I2CM_STAT_INVALID   0x00000080
/** @} */
/*---------------------------------------------------------------------------*/
/** \name I2C registers
 * @{
 */
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
#define I2C_SEND                    0x00  /* Master send */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name I2C Functions
 * @{
 */

/**
 * \brief Initialize the I2C peripheral and pins
 * \param port_sda  The GPIO number of the pin used fort SDA
 * \param pin_sda   The pin number used for SDA
 * \param port_scl  The GPIO number of the pin used fort SCL
 * \param pin_scl   The pin number used for SCL
 * \param bus_speed The clock frequency used by I2C module
 *
 * \e bus_speed can take the following values:
 *
 * - I2C_SCL_NORMAL_BUS_SPEED : 100KHz
 * - I2C_SCL_FAST_BUS_SPEED : 400KHz
 */
void i2c_init(uint8_t port_sda, uint8_t pin_sda, uint8_t port_scl,
              uint8_t pin_scl, uint32_t bus_speed);

/** \brief Enable master I2C module */
void i2c_master_enable(void);

/** \brief Disable master I2C module */
void i2c_master_disable(void);

/**
 * \brief Initialize I2C peripheral clock with given frequency
 * \param freq The desired frequency
 *
 * \e freq can take the following values:
 *
 * - I2C_SCL_NORMAL_BUS_SPEED : 100KHz
 * - I2C_SCL_FAST_BUS_SPEED : 400KHz
 */
void i2c_set_frequency(uint32_t freq);

/**
 * \brief Set the address of slave and access mode for the next I2C communication
 * \param slave_addr The receiver slave address on 7 bits
 * \param access_mode The I2C access mode (send/receive)
 *
 * \e access_mode can take the following values:
 *
 * - I2C_RECEIVE : 1
 * - I2C_SEND : 0
 */
void i2c_master_set_slave_address(uint8_t slave_addr, uint8_t access_mode);

/**
 * \brief Prepare data to be transmitted
 * \param data The byte of data to be transmitted from the I2C master
 */
void i2c_master_data_put(uint8_t data);

/**
 * \brief Return received data from I2C
 * \return The byte received by I2C after à receive command
 */
uint8_t i2c_master_data_get(void);

/**
 * \brief Control the state of the master module for send and receive operations
 * \param cmd The operation to perform
 *
 * \e cmd can take the following values:
 *
 * - I2C_MASTER_CMD_SINGLE_SEND
 * - I2C_MASTER_CMD_SINGLE_RECEIVE
 * - I2C_MASTER_CMD_BURST_SEND_START
 * - I2C_MASTER_CMD_BURST_SEND_CONT
 * - I2C_MASTER_CMD_BURST_SEND_FINISH
 * - I2C_MASTER_CMD_BURST_SEND_ERROR_STOP
 * - I2C_MASTER_CMD_BURST_RECEIVE_START
 * - I2C_MASTER_CMD_BURST_RECEIVE_CONT
 * - I2C_MASTER_CMD_BURST_RECEIVE_FINISH
 * - I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP
 */
void i2c_master_command(uint8_t cmd);

/**
 * \brief Return the busy state of I2C module
 * \retval 0 The I2C module is not busy
 * \retval 1 The I2C module is busy
 */
uint8_t i2c_master_busy(void);

/**
 * \brief Return the status register if error occurred during last communication
 * \retval I2C_MASTER_ERR_NONE Return 0 if no error occurred
 *
 * If an error occurred, return the status register of the I2C module.
 * Use the result with the I2CM_STAT_* flags to custom your processing
 */
uint8_t i2c_master_error(void);
/**
 * \brief      Perform all operations to send a byte to a slave
 * \param slave_addr The adress of the slave to which data are sent
 * \param data The data to send to the slave
 * \return     Return the value of i2c_master_error() after the I2C operation
 */
uint8_t i2c_single_send(uint8_t slave_addr, uint8_t data);

/**
 * \brief      Perform all operations to receive a byte from a slave
 * \param slave_addr The address of the slave from which data are received
 * \param data A pointer to store the received data
 * \return     Return the value of i2c_master_error() after the I2C operation
 */
uint8_t i2c_single_receive(uint8_t slave_addr, uint8_t *data);
/**
 * \brief      Perform all operations to send multiple bytes to a slave
 * \param slave_addr The address of the slave to which data are sent
 * \param data A pointer to the data to send to the slave
 * \param len  Number of bytes to send
 * \return     Return the value of i2c_master_error() after the I2C operation
 */
uint8_t i2c_burst_send(uint8_t slave_addr, uint8_t *data, uint8_t len);

/**
 * \brief      Perform all operations to receive multiple bytes from a slave
 * \param slave_addr The address of the slave from which data are received
 * \param data A pointer to store the received data
 * \param len  Number of bytes to receive
 * \return     Return the value of i2c_master_error() after the I2C operation
 */
uint8_t i2c_burst_receive(uint8_t slave_addr, uint8_t *data, uint8_t len);
/** @} */

#endif /* I2C_H_ */

/**
 * @}
 * @}
 */
