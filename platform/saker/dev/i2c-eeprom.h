/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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

/**
 * \addtogroup saker
 *
 * @{
 *
 * \defgroup saker-eeprom EEPROM driver
 *
 * This driver provides access to the external EEPROM
 * (Microchip 24AA02E48) via I2C. This EEPROM allows to store configuration
 * parameters (128 bytes).
 * In addition it provides an unique (read-only) EUI-48 address used as
 * the MAC address for the ethernet interface.
 *
 * @{
 *
 * \file
 *      Header file for the platform's I2C-EEPROM driver
 */

#ifndef I2C_EEPROM_H_
#define I2C_EEPROM_H_

#include "contiki.h"

/*---------------------------------------------------------------------------*/
/** EEPROM's I2C address */
#define EEPROM_ADDR                     (0xA0 >> 1)
/** Start address */
#define EEPROM_START_ADDR               0x00
/** Start address of write protected block */
#define EEPROM_START_ADDR_WP_BLOCK      0x80
/** Last valid word address */
#define EEPROM_END_ADDR                 0xFF
/** The address of the EUI-48 */
#define EEPROM_EUI48_START_ADDR         0xFA
/*---------------------------------------------------------------------------*/
/**
 * \brief Write to external I2C-EEPROM
 *
 * The EEPROM can be written between address EEPROM_START_ADDR and
 * EEPROM_START_ADDR_WP_BLOCK - 1.
 *
 * \param data Buffer containing data to write
 * \param addr Where to write (start address)
 * \param data_len Number of bytes to write
 * \return 0 in case of errors, else number of bytes written
 */
int
i2c_eeprom_write_block(const uint8_t *data, uint16_t addr, uint16_t data_len);
/*---------------------------------------------------------------------------*/
/**
 * \brief Read from external I2C-EEPROM
 *
 * The EEPROM can be read between EEPROM_START_ADDR to EEPROM_END_ADDR. EUI-48
 * is placed at EEPROM_START_ADDR_WP_BLOCK.
 *
 * \param buf Destination buffer for the data read from the EEPROM
 * \param addr Where to read from (start address)
 * \param buf_len Number of bytes to read
 * \return 0 in case of errors, else number of bytes read
 */
int
i2c_eeprom_read_block(uint8_t *buf, uint16_t addr, uint16_t buf_len);
/*---------------------------------------------------------------------------*/

#endif /* I2C_EEPROM_H_ */

/**
 * @}
 * @}
 */
