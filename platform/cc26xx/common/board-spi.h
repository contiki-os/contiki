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
 * \addtogroup cc26xx-srf-tag
 * @{
 *
 * \defgroup common-cc26xx-peripherals CC13xx/CC26xx peripheral driver pool
 *
 * Drivers for peripherals present on more than one CC13xx/CC26xx board. For
 * example, the same external flash driver is used for both the part found on
 * the Sensortag as well as the part on the LaunchPad.
 *
 * @{
 *
 * \defgroup sensortag-cc26xx-spi SensorTag/LaunchPad SPI functions
 * @{
 *
 * \file
 * Header file for the Sensortag/LaunchPad SPI Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_SPI_H_
#define BOARD_SPI_H_
/*---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the SPI interface
 * \param bit_rate The bit rate to use
 * \param clk_pin The IOID for the clock pin. This can be IOID_0 etc
 * \return none
 *
 * This function will make sure the peripheral is powered, clocked and
 * initialised. A chain of calls to board_spi_read(), board_spi_write() and
 * board_spi_flush() must be preceded by a call to this function. It is
 * recommended to call board_spi_close() after such chain of calls.
 */
void board_spi_open(uint32_t bit_rate, uint32_t clk_pin);

/**
 * \brief Close the SPI interface
 * \return True when successful.
 *
 * This function will stop clocks to the SSI module and will set MISO, MOSI
 * and CLK to a low leakage state. It is recommended to call this function
 * after a chain of calls to board_spi_read() and board_spi_write()
 */
void board_spi_close(void);

/**
 * \brief Clear data from the SPI interface
 * \return none
 */
void board_spi_flush(void);

/**
 * \brief Read from an SPI device
 * \param buf The buffer to store data
 * \param length The number of bytes to read
 * \return True when successful.
 *
 * Calls to this function must be preceded by a call to board_spi_open(). It is
 * recommended to call board_spi_close() at the end of an operation.
 */
bool board_spi_read(uint8_t *buf, size_t length);

/**
 * \brief Write to an SPI device
 * \param buf The buffer with the data to write
 * \param length The number of bytes to write
 * \return True when successful.
 *
 * Calls to this function must be preceded by a call to board_spi_open(). It is
 * recommended to call board_spi_close() at the end of an operation.
 */
bool board_spi_write(const uint8_t *buf, size_t length);
/*---------------------------------------------------------------------------*/
#endif /* BOARD_SPI_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
