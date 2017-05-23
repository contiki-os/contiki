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
 * \addtogroup common-cc26xx-peripherals
 * @{
 *
 * \defgroup sensortag-cc26xx-ext-flash SensorTag/LaunchPad External Flash
 * @{
 *
 * \file
 * Header file for the Sensortag/LaunchPad External Flash Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef EXT_FLASH_H_
#define EXT_FLASH_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize storage driver.
 * \return True when successful.
 */
bool ext_flash_open(void);

/**
 * \brief Close the storage driver
 *
 * This call will put the device in its lower power mode (power down).
 */
void ext_flash_close(void);

/**
 * \brief Read storage content
 * \param offset Address to read from
 * \param length Number of bytes to read
 * \param buf Buffer where to store the read bytes
 * \return True when successful.
 *
 * buf must be allocated by the caller
 */
bool ext_flash_read(size_t offset, size_t length, uint8_t *buf);

/**
 * \brief Erase storage sectors corresponding to the range.
 * \param offset Address to start erasing
 * \param length Number of bytes to erase
 * \return True when successful.
 *
 * The erase operation will be sector-wise, therefore a call to this function
 * will generally start the erase procedure at an address lower than offset
 */
bool ext_flash_erase(size_t offset, size_t length);

/**
 * \brief Write to storage sectors.
 * \param offset Address to write to
 * \param length Number of bytes to write
 * \param buf Buffer holding the bytes to be written
 *
 * \return True when successful.
 */
bool ext_flash_write(size_t offset, size_t length, const uint8_t *buf);

/**
 * \brief Test the flash (power on self-test)
 * \return True when successful.
 */
bool ext_flash_test(void);

/**
 * \brief Initialise the external flash
 *
 * This function will explicitly put the part in its lowest power mode
 * (power-down).
 *
 * In order to perform any operation, the caller must first wake the device
 * up by calling ext_flash_open()
 */
void ext_flash_init(void);
/*---------------------------------------------------------------------------*/
#endif /* EXT_FLASH_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
