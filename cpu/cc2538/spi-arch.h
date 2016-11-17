/*
 * Copyright (c) 2013, University of Michigan.
 *
 * Copyright (c) 2015, Weptech elektronik GmbH
 * Author: Ulf Knoblich, ulf.knoblich@weptech.de
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
 *
 * It supports the usage of SSI_NUM_INSTANCES instances by providing new
 * functions calls like
 *
 * - spix_init(uint8_t instance)
 * - spix_enable(uint8_t instance)
 * - spix_disable(uint8_t instance)
 * - spix_set_mode(unit8_t instance, ...)
 *
 * and new macros like
 *
 * - SPIX_WAITFORTxREADY(x)
 * - SPIX_WAITFOREOTx(x)
 * - SPIX_WAITFOREORx(x)
 * - SPIX_FLUSH(x)
 *
 * Some of the old functions and macros are still supported.
 * When using these deprecated functions, the SSI module to use 
 * has to be be selected by means of the macro SPI_CONF_DEFAULT_INSTANCE.
 *
 * This SPI driver depends on the following defines:
 *
 * For the SSI0 module:
 *
 * - SPI0_CKL_PORT
 * - SPI0_CLK_PIN
 * - SPI0_TX_PORT
 * - SPI0_TX_PIN
 * - SPI0_RX_PORT
 * - SPI0_RX_PIN
 *
 * For the SSI1 module:
 *
 * - SPI1_CKL_PORT
 * - SPI1_CLK_PIN
 * - SPI1_TX_PORT
 * - SPI1_TX_PIN
 * - SPI1_RX_PORT
 * - SPI1_RX_PIN
 */
#ifndef SPI_ARCH_H_
#define SPI_ARCH_H_

#include "contiki.h"

#include "dev/ssi.h"
/*---------------------------------------------------------------------------*/
/* The SPI instance to use when using the deprecated SPI API. */
#ifdef SPI_CONF_DEFAULT_INSTANCE
#if SPI_CONF_DEFAULT_INSTANCE > (SSI_INSTANCE_COUNT - 1)
#error Invalid SPI_CONF_DEFAULT_INSTANCE: valid values are 0 and 1
#else
#define SPI_DEFAULT_INSTANCE            SPI_CONF_DEFAULT_INSTANCE
#endif
#endif
/*---------------------------------------------------------------------------*/
/* Default values for the clock rate divider */
#ifdef SPI0_CONF_CPRS_CPSDVSR 
#define SPI0_CPRS_CPSDVSR               SPI0_CONF_CPRS_CPSDVSR 
#else
#define SPI0_CPRS_CPSDVSR               2
#endif

#ifdef SPI1_CONF_CPRS_CPSDVSR 
#define SPI1_CPRS_CPSDVSR               SPI1_CONF_CPRS_CPSDVSR 
#else
#define SPI1_CPRS_CPSDVSR               2
#endif
/*---------------------------------------------------------------------------*/
/* New API macros */
#define SPIX_WAITFORTxREADY(spi) do { \
    while(!(REG(SSI_BASE(spi) + SSI_SR) & SSI_SR_TNF)) ; \
} while(0)
#define SPIX_BUF(spi)                   REG(SSI_BASE(spi) + SSI_DR)
#define SPIX_WAITFOREOTx(spi) do { \
    while(REG(SSI_BASE(spi) + SSI_SR) & SSI_SR_BSY) ; \
} while(0)
#define SPIX_WAITFOREORx(spi) do { \
    while(!(REG(SSI_BASE(spi) + SSI_SR) & SSI_SR_RNE)) ; \
} while(0)
#define SPIX_FLUSH(spi) do { \
    while(REG(SSI_BASE(spi) + SSI_SR) & SSI_SR_RNE) { \
        SPIX_BUF(spi);                                           \
    } \
} while(0)
#define SPIX_CS_CLR(port, pin) do { \
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin)); \
} while(0)
#define SPIX_CS_SET(port, pin) do { \
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin)); \
} while(0)
/*---------------------------------------------------------------------------*/
/* Deprecated macros provided for compatibility reasons */
#ifdef SPI_DEFAULT_INSTANCE
#define SPI_WAITFORTxREADY()            SPIX_WAITFORTxREADY(SPI_DEFAULT_INSTANCE)
#define SPI_TXBUF                       SPIX_BUF(SPI_DEFAULT_INSTANCE)
#define SPI_RXBUF                       SPI_TXBUF
#define SPI_WAITFOREOTx()               SPIX_WAITFOREOTx(SPI_DEFAULT_INSTANCE)
#define SPI_WAITFOREORx()               SPIX_WAITFOREORx(SPI_DEFAULT_INSTANCE)
#ifdef SPI_FLUSH
#error You must include spi-arch.h before spi.h for the CC2538
#else
#define SPI_FLUSH()                     SPIX_FLUSH(SPI_DEFAULT_INSTANCE)
#endif
#define SPI_CS_CLR(port, pin)           SPIX_CS_CLR(port, pin)
#define SPI_CS_SET(port, pin)           SPIX_CS_SET(port, pin)
#endif  /* #ifdef SPI_DEFAULT_INSTANCE */
/*---------------------------------------------------------------------------*/
/** \name Arch-specific SPI functions
 * @{
 */

/**
 * \brief Initialize the SPI bus for the instance given
 *
 * This sets the mode to Motorola SPI with the following format options:
 *    Clock phase:               1; data captured on second (rising) edge
 *    Clock polarity:            1; clock is high when idle
 *    Data size:                 8 bits
 *
 * Use spix_set_mode() to change the spi mode.
 */
void spix_init(uint8_t spi);

/**
 * \brief Enables the SPI peripheral for the instance given
 */
void spix_enable(uint8_t spi);

/**
 * \brief Disables the SPI peripheral for the instance given
 * \note Call this function to save power when the SPI is unused.
 */
void spix_disable(uint8_t spi);

/**
 * \brief Configure the SPI data and clock polarity and the data size for the
 * instance given
 *
 * This function configures the SSI peripheral to use a particular SPI
 * configuration that a slave device requires. It should always be called
 * before using the SPI bus as another driver could have changed the settings.
 *
 * See section 19.4.4 in the CC2538 user guide for more information.
 *
 * \param spi            The SSI instance to use.       
 * \param frame_format   Set the SSI frame format. Use SSI_CR0_FRF_MOTOROLA,
 *                       SSI_CR0_FRF_TI, or SSI_CR0_FRF_MICROWIRE.
 * \param clock_polarity In Motorola mode, set whether the clock is high or low
 *                       when idle. Use SSI_CR0_SPO or 0.
 * \param clock_phase    In Motorola mode, select whether data is valid on the
 *                       first or second edge of the clock. Use SSI_CR0_SPH or 0.
 * \param data_size      The number of bits in each "byte" of data. Must be
 *                       between 4 and 16, inclusive.
 */
void spix_set_mode(uint8_t spi, uint32_t frame_format,
                   uint32_t clock_polarity, uint32_t clock_phase,
                   uint32_t data_size);

/**
 * \brief Sets the SPI clock frequency of the given SSI instance.
 *
 * \param spi SSI instance
 * \param freq Frequency (Hz)
 */
void spix_set_clock_freq(uint8_t spi, uint32_t freq);

/**
 * \brief Configure a GPIO to be the chip select pin.
 *
 * Even if this function does not depend on the SPI instance used, we rename
 * it to reflect the new naming convention.
 */
void spix_cs_init(uint8_t port, uint8_t pin);

/** @} */

#endif /* SPI_ARCH_H_ */

/**
 * @}
 */
