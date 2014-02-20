/**
 * \file
 *
 * \brief Serial Peripheral Interface (SPI) driver for SAM.
 *
 * Copyright (c) 2011-2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
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
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "spi.h"
#include "sysclk.h"

/**
 * \defgroup sam_drivers_spi_group Serial Peripheral Interface (SPI)
 *
 * See \ref sam_spi_quickstart.
 *
 * The SPI circuit is a synchronous serial data link that provides communication
 * with external devices in Master or Slave mode. Connection to Peripheral DMA
 * Controller channel capabilities optimizes data transfers.
 *
 * @{
 */

#define SPI_WPMR_WPKEY_VALUE SPI_WPMR_WPKEY((uint32_t) 0x535049)

/**
 * \brief Enable SPI clock.
 *
 * \param p_spi Pointer to an SPI instance.
 */
void spi_enable_clock(Spi *p_spi)
{
#if (SAM4S || SAM3S || SAM3N || SAM3U)
	sysclk_enable_peripheral_clock(ID_SPI);
#elif (SAM3XA)
	if (p_spi == SPI0) {
		sysclk_enable_peripheral_clock(ID_SPI0);
	}
	#ifdef SPI1
	else if (p_spi == SPI1) {
		sysclk_enable_peripheral_clock(ID_SPI1);
	}
	#endif
#elif SAM4L
	sysclk_enable_peripheral_clock(p_spi);
#endif
}

/**
 * \brief Disable SPI clock.
 *
 * \param p_spi Pointer to an SPI instance.
 */
void spi_disable_clock(Spi *p_spi)
{
#if (SAM4S || SAM3S || SAM3N || SAM3U)
	sysclk_disable_peripheral_clock(ID_SPI);
#elif (SAM3XA)
	if (p_spi == SPI0) {
		sysclk_disable_peripheral_clock(ID_SPI0);
	}
	#ifdef SPI1
	else if (p_spi == SPI1) {
		sysclk_disable_peripheral_clock(ID_SPI1);
	}
	#endif
#elif SAM4L
	sysclk_disable_peripheral_clock(p_spi);
#endif
}

/**
 * \brief Set Peripheral Chip Select (PCS) value.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_value Peripheral Chip Select value.
 */
void spi_set_peripheral_chip_select_value(Spi *p_spi, uint32_t ul_value)
{
	p_spi->SPI_MR &= (~SPI_MR_PCS_Msk);
	p_spi->SPI_MR |= SPI_MR_PCS(ul_value);
}

/**
 * \brief Set delay between chip selects (in number of MCK clocks).
 *  If DLYBCS <= 6, 6 MCK clocks will be inserted by default.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_delay Delay between chip selects (in number of MCK clocks).
 */
void spi_set_delay_between_chip_select(Spi *p_spi, uint32_t ul_delay)
{
	p_spi->SPI_MR &= (~SPI_MR_DLYBCS_Msk);
	p_spi->SPI_MR |= SPI_MR_DLYBCS(ul_delay);
}

/**
 * \brief Read the received data and it's peripheral chip select value.
 * While SPI works in fixed peripheral select mode, the peripheral chip select
 * value is meaningless.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param data Pointer to the location where to store the received data word.
 * \param p_pcs Pointer to fill Peripheral Chip Select Value.
 *
 * \retval SPI_OK on Success.
 * \retval SPI_ERROR_TIMEOUT on Time-out.
 */
spi_status_t spi_read(Spi *p_spi, uint16_t *us_data, uint8_t *p_pcs)
{
	uint32_t timeout = SPI_TIMEOUT;
	static uint32_t reg_value;

	while (!(p_spi->SPI_SR & SPI_SR_RDRF)) {
		if (!timeout--) {
			return SPI_ERROR_TIMEOUT;
		}
	}

	reg_value = p_spi->SPI_RDR;
	if (spi_get_peripheral_select_mode(p_spi)) {
		*p_pcs = (uint8_t) ((reg_value & SPI_RDR_PCS_Msk) >> SPI_RDR_PCS_Pos);
	}
	*us_data = (uint16_t) (reg_value & SPI_RDR_RD_Msk);

	return SPI_OK;
}

/**
 * \brief Write the transmitted data with specified peripheral chip select value.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param us_data The data to transmit.
 * \param uc_pcs Peripheral Chip Select Value while SPI works in peripheral select
 * mode, otherwise it's meaningless.
 * \param uc_last Indicate whether this data is the last one while SPI is working
 * in variable peripheral select mode.
 *
 * \retval SPI_OK on Success.
 * \retval SPI_ERROR_TIMEOUT on Time-out.
 */
spi_status_t spi_write(Spi *p_spi, uint16_t us_data,
		uint8_t uc_pcs, uint8_t uc_last)
{
	uint32_t timeout = SPI_TIMEOUT;
	uint32_t value;

	while (!(p_spi->SPI_SR & SPI_SR_TDRE)) {
		if (!timeout--) {
			return SPI_ERROR_TIMEOUT;
		}
	}

	if (spi_get_peripheral_select_mode(p_spi)) {
		value = SPI_TDR_TD(us_data) | SPI_TDR_PCS(uc_pcs);
		if (uc_last) {
			value |= SPI_TDR_LASTXFER;
		}
	} else {
		value = SPI_TDR_TD(us_data);
	}

	p_spi->SPI_TDR = value;

	return SPI_OK;
}

/**
 * \brief Set clock default state.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param ul_polarity Default clock state is logical one(high)/zero(low).
 */
void spi_set_clock_polarity(Spi *p_spi, uint32_t ul_pcs_ch,
		uint32_t ul_polarity)
{
	if (ul_polarity) {
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_CPOL;
	} else {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CPOL);
	}
}

/**
 * \brief Set Data Capture Phase.
 *
 * \param p_spi Pointer to an SPI instance.
 *  \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 *  \param ul_phase Data capture on the rising/falling edge of clock.
 */
void spi_set_clock_phase(Spi *p_spi, uint32_t ul_pcs_ch, uint32_t ul_phase)
{
	if (ul_phase) {
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_NCPHA;
	} else {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_NCPHA);
	}
}

/**
 * \brief Configure CS behavior for SPI transfer (\ref spi_cs_behavior_t).
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param ul_cs_behavior Behavior of the Chip Select after transfer.
 */
void spi_configure_cs_behavior(Spi *p_spi, uint32_t ul_pcs_ch,
		uint32_t ul_cs_behavior)
{
	if (ul_cs_behavior == SPI_CS_RISE_FORCED) {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CSAAT);
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_CSNAAT;
	} else if (ul_cs_behavior == SPI_CS_RISE_NO_TX) {
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CSAAT);
		p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_CSNAAT);
	} else if (ul_cs_behavior == SPI_CS_KEEP_LOW) {
		p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_CSAAT;
	}
}

/**
 * \brief Set number of bits per transfer.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param ul_bits Number of bits (8~16), use the pattern defined 
 *        in the device header file.
 */
void spi_set_bits_per_transfer(Spi *p_spi, uint32_t ul_pcs_ch,
		uint32_t ul_bits)
{
	p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_BITS_Msk);
	p_spi->SPI_CSR[ul_pcs_ch] |= ul_bits;
}

/**
 * \brief Calculate the baudrate divider.
 *
 * \param baudrate Baudrate value.
 * \param mck      SPI module input clock frequency (MCK clock, Hz).
 *
 * \return Divider or error code.
 *   \retval >=0  Success.
 *   \retval  <0  Error.
 */
int16_t spi_calc_baudrate_div(const uint32_t baudrate, uint32_t mck)
{
	int baud_div = div_ceil(mck, baudrate);

	/* The value of baud_div is from 1 to 255 in the SCBR field. */
	if (baud_div <= 0 || baud_div > 255) {
		return -1;
	}

	return baud_div;
}

/**
 * \brief Set Serial Clock Baud Rate divider value (SCBR).
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param uc_baudrate_divider Baudrate divider from MCK.
 */
void spi_set_baudrate_div(Spi *p_spi, uint32_t ul_pcs_ch,
		uint8_t uc_baudrate_divider)
{
	p_spi->SPI_CSR[ul_pcs_ch] &= (~SPI_CSR_SCBR_Msk);
	p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_SCBR(uc_baudrate_divider);
}

/**
 * \brief Configure timing for SPI transfer.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_pcs_ch Peripheral Chip Select channel (0~3).
 * \param uc_dlybs Delay before SPCK (in number of MCK clocks).
 * \param uc_dlybct Delay between consecutive transfers (in number of MCK clocks).
 */
void spi_set_transfer_delay(Spi *p_spi, uint32_t ul_pcs_ch,
		uint8_t uc_dlybs, uint8_t uc_dlybct)
{
	p_spi->SPI_CSR[ul_pcs_ch] &= ~(SPI_CSR_DLYBS_Msk | SPI_CSR_DLYBCT_Msk);
	p_spi->SPI_CSR[ul_pcs_ch] |= SPI_CSR_DLYBS(uc_dlybs)
			| SPI_CSR_DLYBCT(uc_dlybct);
}


/**
 * \brief Enable or disable write protection of SPI registers.
 *
 * \param p_spi Pointer to an SPI instance.
 * \param ul_enable 1 to enable, 0 to disable.
 */
void spi_set_writeprotect(Spi *p_spi, uint32_t ul_enable)
{
#if SAM4L
	if (ul_enable) {
		p_spi->SPI_WPCR = SPI_WPCR_SPIWPKEY_VALUE | SPI_WPCR_SPIWPEN;
	} else {
		p_spi->SPI_WPCR = SPI_WPCR_SPIWPKEY_VALUE;
	}
#else
	if (ul_enable) {
		p_spi->SPI_WPMR = SPI_WPMR_WPKEY_VALUE | SPI_WPMR_WPEN;
	} else {
		p_spi->SPI_WPMR = SPI_WPMR_WPKEY_VALUE;
	}
#endif
}

/**
 * \brief Indicate write protect status.
 *
 * \param p_spi Pointer to an SPI instance.
 *
 * \return SPI_WPSR value.
 */
uint32_t spi_get_writeprotect_status(Spi *p_spi)
{
	return p_spi->SPI_WPSR;
}

/**
 * @}
 */
