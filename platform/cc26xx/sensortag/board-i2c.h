/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \defgroup sensortag-cc26xx-i2c SensorTag 2.0 I2C functions
 * @{
 *
 * \file
 * Header file for the Sensortag I2C Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_I2C_H_
#define BOARD_I2C_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define BOARD_I2C_INTERFACE_0     0
#define BOARD_I2C_INTERFACE_1     1
/*---------------------------------------------------------------------------*/
/**
 * \brief Put the I2C controller in a known state
 *
 * In this state, pins SDA and SCL will be under i2c control and pins SDA HP
 * and SCL HP will be configured as gpio inputs. This is equal to selecting
 * BOARD_I2C_INTERFACE_0, but without selecting a slave device address
 */
#define board_i2c_deselect() board_i2c_select(BOARD_I2C_INTERFACE_0, 0)
/*---------------------------------------------------------------------------*/
/**
 * \brief Select an I2C slave
 * \param interface The I2C interface to be used (BOARD_I2C_INTERFACE_0 or _1)
 * \param slave_addr The slave's address
 *
 * The various sensors on the sensortag are connected either on interface 0 or
 * 1. All sensors are connected to interface 0, with the exception of the MPU
 * that is connected to 1.
 */
void board_i2c_select(uint8_t interface, uint8_t slave_addr);

/**
 * \brief Burst read from an I2C device
 * \param buf Pointer to a buffer where the read data will be stored
 * \param len Number of bytes to read
 * \return True on success
 */
bool board_i2c_read(uint8_t *buf, uint8_t len);

/**
 * \brief Burst write to an I2C device
 * \param buf Pointer to the buffer to be written
 * \param len Number of bytes to write
 * \return True on success
 */
bool board_i2c_write(uint8_t *buf, uint8_t len);

/**
 * \brief Single write to an I2C device
 * \param data The byte to write
 * \return True on success
 */
bool board_i2c_write_single(uint8_t data);

/**
 * \brief Write and read in one operation
 * \param wdata Pointer to the buffer to be written
 * \param wlen Number of bytes to write
 * \param rdata Pointer to a buffer where the read data will be stored
 * \param rlen Number of bytes to read
 * \return True on success
 */
bool board_i2c_write_read(uint8_t *wdata, uint8_t wlen, uint8_t *rdata,
                          uint8_t rlen);

/**
 * \brief Enables the I2C peripheral with defaults
 *
 * This function is called to wakeup and initialise the I2C.
 *
 * This function can be called explicitly, but it will also be called
 * automatically by board_i2c_select() when required. One of those two
 * functions MUST be called before any other I2C operation after a chip
 * sleep / wakeup cycle or after a call to board_i2c_shutdown(). Failing to do
 * so will lead to a bus fault.
 */
void board_i2c_wakeup(void);

/**
 * \brief Stops the I2C peripheral and restores pins to s/w control
 *
 * This function is called automatically by the board's LPM logic, but it
 * can also be called explicitly.
 */
void board_i2c_shutdown(void);
/*---------------------------------------------------------------------------*/
#endif /* BOARD_I2C_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
