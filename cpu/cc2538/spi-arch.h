/*
 * Copyright (c) 2013, University of Michigan.
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
 * 3. Neither the name of the University nor the names of its contributors
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
 */
/**
 * \addtogroup cc2538-spi
 * @{
 *
 * \file
 * Header file for the cc2538 SPI driver, including macros for the
 * implementation of the low-level SPI primitives such as waiting for the TX
 * FIFO to be ready, inserting into the TX FIFO, etc.
 */
#ifndef SPI_ARCH_H_
#define SPI_ARCH_H_

#include "dev/ssi.h"

#define SPI_WAITFORTxREADY() do { \
  while(!(REG(SSI0_BASE + SSI_SR) & SSI_SR_TNF)); \
} while(0)

#define SPI_TXBUF REG(SSI0_BASE + SSI_DR)

#define SPI_RXBUF REG(SSI0_BASE + SSI_DR)

#define SPI_WAITFOREOTx() do { \
  while(REG(SSI0_BASE + SSI_SR) & SSI_SR_BSY); \
} while(0)

#define SPI_WAITFOREORx() do { \
  while(!(REG(SSI0_BASE + SSI_SR) & SSI_SR_RNE)); \
} while(0)

#ifdef SPI_FLUSH
#error "You must include spi-arch.h before spi.h for the CC2538."
#endif
#define SPI_FLUSH() do { \
  SPI_WAITFOREORx(); \
  while (REG(SSI0_BASE + SSI_SR) & SSI_SR_RNE) { \
    SPI_RXBUF; \
  } \
} while(0)

#define SPI_CS_CLR(port, pin) do { \
  GPIO_CLR_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin)); \
} while(0)

#define SPI_CS_SET(port, pin) do { \
  GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin)); \
} while(0)
/*---------------------------------------------------------------------------*/
/** \name Arch-specific SPI functions
 * @{
 */

/**
 * \brief Configure a GPIO to be the chip select pin
 */
void spi_cs_init(uint8_t port, uint8_t pin);

/** \brief Enables the SPI peripheral
 */
void spi_enable(void);

/** \brief Disables the SPI peripheral
 * \note Call this function to save power when the SPI is unused.
 */
void spi_disable(void);

/**
 * \brief Configure the SPI data and clock polarity and the data size.
 *
 * This function configures the SSI peripheral to use a particular SPI
 * configuration that a slave device requires. It should always be called
 * before using the SPI bus as another driver could have changed the settings.
 *
 * See section 19.4.4 in the CC2538 user guide for more information.
 *
 * \param frame_format   Set the SSI frame format. Use SSI_CR0_FRF_MOTOROLA,
 *                       SSI_CR0_FRF_TI, or SSI_CR0_FRF_MICROWIRE.
 * \param clock_polarity In Motorola mode, set whether the clock is high or low
 *                       when idle. Use SSI_CR0_SPO or 0.
 * \param clock_phase    In Motorola mode, select whether data is valid on the
 *                       first or second edge of the clock. Use SSI_CR0_SPH or 0.
 * \param data_size      The number of bits in each "byte" of data. Must be
 *                       between 4 and 16, inclusive.
 */
void spi_set_mode(uint32_t frame_format, uint32_t clock_polarity,
                  uint32_t clock_phase, uint32_t data_size);

/** @} */

#endif /* SPI_ARCH_H_ */

/**
 * @}
 */
