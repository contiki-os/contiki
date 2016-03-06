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
 * \defgroup sensortag-cc26xx-sensor-common SensorTag 2.0 Sensors
 * @{
 *
 * \file
 * Header file for the Sensortag-CC26xx Common sensor utilities
 */
/*---------------------------------------------------------------------------*/
#ifndef SENSOR_H
#define SENSOR_H
/*---------------------------------------------------------------------------*/
#include "board-i2c.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief Reads a sensor's register over I2C
 * \param addr The address of the register to read
 * \param buf Pointer to buffer to place data
 * \param len Number of bytes to read
 * \return TRUE if the required number of bytes are received
 *
 * The sensor must be selected before this routine is called.
 */
bool sensor_common_read_reg(uint8_t addr, uint8_t *buf, uint8_t len);

/**
 * \brief Write to a sensor's register over I2C
 * \param addr The address of the register to read
 * \param buf Pointer to buffer containing data to be written
 * \param len Number of bytes to write
 * \return TRUE if successful write
 *
 * The sensor must be selected before this routine is called.
 */
bool sensor_common_write_reg(uint8_t addr, uint8_t *buf, uint8_t len);

/**
 * \brief Fill a result buffer with dummy error data
 * \param buf Pointer to the buffer where to write the data
 * \param len Number of bytes to fill
 * \return bitmask of error flags
 */
void sensor_common_set_error_data(uint8_t *buf, uint8_t len);
/*---------------------------------------------------------------------------*/
#endif /* SENSOR_H */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
