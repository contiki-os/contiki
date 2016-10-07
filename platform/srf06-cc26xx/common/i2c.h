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
 * \author Andreas Urke <arurke@gmail.com>
 *
 * \defgroup cc26xx-cc13xx-i2c cc26xx/cc13xx Generic I2C functions
 * @{
 *
 * \file
 * Header file for the generic cc26xx/cc13xx I2C Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef I2C_H_
#define I2C_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define I2C_SPEED_NORMAL      false
#define I2C_SPEED_FAST        true
#define I2C_PULL_DOWN         IOC_IOPULL_DOWN
#define I2C_PULL_UP           IOC_IOPULL_UP
#define I2C_PULL_NO_PULL      IOC_NO_IOPULL
/*---------------------------------------------------------------------------*/
/**
 * \brief Deselect I2C slave
 *
 * Sets selected pins to GPIO input and internal pull to configured pull.
 * This is also called by i2c_shutdown(). Should be called by user
 * after done with I2C read/write operations.
 */
void i2c_deselect();
/*---------------------------------------------------------------------------*/
/**
 * \brief Select an I2C slave
 * \param new_pin_sda SDA pin IOID
 * \param new_pin_scl SCL pin IOID
 * \param new_slave_address The slave's address
 * \param new_speed I2C speed, I2C_SPEED_NORMAL or I2C_SPEED_FAST
 * \param new_pin_pull SCL & SDA pin internal pull to set at deselect/shutdown
 *                     , valid values I2C_PULL_DOWN/UP/NO_PULL
 *
 * Must be called before read/write commands
 */
void i2c_select(uint32_t new_pin_sda, uint32_t new_pin_scl,
                      uint8_t new_slave_address, bool new_speed,
                      uint32_t new_pin_pull);

/**
 * \brief Burst read from an I2C device
 * \param buf Pointer to a buffer where the read data will be stored
 * \param len Number of bytes to read
 * \return True on success
 */
bool i2c_read(uint8_t *buf, uint8_t len);

/**
 * \brief Burst write to an I2C device
 * \param buf Pointer to the buffer to be written
 * \param len Number of bytes to write
 * \return True on success
 */
bool i2c_write(uint8_t *buf, uint8_t len);

/**
 * \brief Single write to an I2C device
 * \param data The byte to write
 * \return True on success
 */
bool i2c_write_single(uint8_t data);

/**
 * \brief Write and read in one operation
 * \param wdata Pointer to the buffer to be written
 * \param wlen Number of bytes to write
 * \param rdata Pointer to a buffer where the read data will be stored
 * \param rlen Number of bytes to read
 * \return True on success
 */
bool i2c_write_read(uint8_t *wdata, uint8_t wlen, uint8_t *rdata,
                          uint8_t rlen);

/**
 * \brief Write one byte and read len bytes in one operation
 * \param wdata The byte to be written
 * \param buf Pointer to the buffer where the read data will be stored
 * \param len Number of bytes to read
 * \return True on success
 */
bool
i2c_write_single_read_multi(uint8_t wdata, uint8_t *buf, uint8_t len);

/**
 * \brief Enables the I2C peripheral with defaults
 *
 * This function is called to wakeup and initialise the I2C.
 *
 * This function can be called explicitly, but it will also be called
 * automatically by i2c_select() when required. One of those two
 * functions MUST be called before any other I2C operation after a chip
 * sleep / wakeup cycle or after a call to i2c_shutdown(). Failing to do
 * so will lead to a bus fault.
 */
void i2c_wakeup(void);

/**
 * \brief Stops the I2C peripheral and restores pins to s/w control
 *
 * This function is called automatically by the board's LPM logic, but it
 * can also be called explicitly.
 */
void i2c_shutdown(void);
/*---------------------------------------------------------------------------*/
#endif /* I2C_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
