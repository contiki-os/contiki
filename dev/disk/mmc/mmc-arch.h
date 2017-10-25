/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit@wsystem.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup mmc
 * @{
 *
 * \defgroup mmc-arch SD/MMC architecture-specific definitions
 *
 * SD/MMC device driver architecture-specific definitions.
 * @{
 *
 * \file
 * Header file for the SD/MMC device driver architecture-specific definitions.
 */
#ifndef MMC_ARCH_H_
#define MMC_ARCH_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/** \brief Callback of the SD/MMC driver to call when the card-detection signal
 * changes.
 * \param dev Device
 * \param cd Whether a card is detected
 * \note Using this function is not mandatory. This only allows to detect a card
 * replacement between two successive calls to the SD/MMC driver API.
 */
void mmc_arch_cd_changed_callback(uint8_t dev, bool cd);

/** \brief Gets the state of the card-detection signal.
 * \param dev Device
 * \return Whether a card is detected
 */
bool mmc_arch_get_cd(uint8_t dev);

/** \brief Gets the state of the write-protection signal.
 * \param dev Device
 * \return Whether the card is write-protected
 */
bool mmc_arch_get_wp(uint8_t dev);

/** \brief Sets the SPI /CS signal as indicated.
 * \param dev Device
 * \param sel Whether to assert /CS
 */
void mmc_arch_spi_select(uint8_t dev, bool sel);

/** \brief Sets the SPI clock frequency.
 * \param dev Device
 * \param freq Frequency (Hz)
 */
void mmc_arch_spi_set_clock_freq(uint8_t dev, uint32_t freq);

/** \brief Performs an SPI transfer.
 * \param dev Device
 * \param tx_buf Pointer to the transmission buffer, or \c NULL
 * \param tx_cnt Number of bytes to transmit, or \c 0
 * \param rx_buf Pointer to the reception buffer, or \c NULL
 * \param rx_cnt Number of bytes to receive, or \c 0
 */
void mmc_arch_spi_xfer(uint8_t dev, const void *tx_buf, size_t tx_cnt,
                       void *rx_buf, size_t rx_cnt);

#endif /* MMC_ARCH_H_ */

/**
 * @}
 * @}
 */
