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
 * \addtogroup mmc-arch
 * @{
 *
 * \file
 * Implementation of the SD/MMC device driver RE-Mote-specific definitions.
 */
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "spi-arch.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/spi.h"
#include "mmc-arch.h"

#define USD_SEL_PORT_BASE       GPIO_PORT_TO_BASE(USD_SEL_PORT)
#define USD_SEL_PIN_MASK        GPIO_PIN_MASK(USD_SEL_PIN)

/*----------------------------------------------------------------------------*/
static void
mmc_arch_init(void)
{
  static uint8_t init_done;

  if(init_done) {
    return;
  }

  GPIO_SET_INPUT(USD_SEL_PORT_BASE, USD_SEL_PIN_MASK);
  GPIO_SOFTWARE_CONTROL(USD_SEL_PORT_BASE, USD_SEL_PIN_MASK);
  ioc_set_over(USD_SEL_PORT, USD_SEL_PIN, IOC_OVERRIDE_DIS);

  spix_cs_init(USD_CSN_PORT, USD_CSN_PIN);
  spix_init(USD_SPI_INSTANCE);
  spix_set_mode(USD_SPI_INSTANCE, SSI_CR0_FRF_MOTOROLA, 0, 0, 8);

  init_done = 1;
}
/*----------------------------------------------------------------------------*/
bool
mmc_arch_get_cd(uint8_t dev)
{
  mmc_arch_init();

  if(GPIO_IS_OUTPUT(USD_SEL_PORT_BASE, USD_SEL_PIN_MASK)) {
    /* Card previously detected and powered */
    return true;
  } else if(GPIO_READ_PIN(USD_SEL_PORT_BASE, USD_SEL_PIN_MASK)) {
    /* Card inserted -> power it */
    GPIO_SET_OUTPUT(USD_SEL_PORT_BASE, USD_SEL_PIN_MASK);
    GPIO_CLR_PIN(USD_SEL_PORT_BASE, USD_SEL_PIN_MASK);
    return true;
  } else {
    /* No card detected */
    return false;
  }
}
/*----------------------------------------------------------------------------*/
bool
mmc_arch_get_wp(uint8_t dev)
{
  return false;
}
/*----------------------------------------------------------------------------*/
void
mmc_arch_spi_select(uint8_t dev, bool sel)
{
  if(sel) {
    SPIX_CS_CLR(USD_CSN_PORT, USD_CSN_PIN);
  } else {
    SPIX_CS_SET(USD_CSN_PORT, USD_CSN_PIN);
  }
}
/*----------------------------------------------------------------------------*/
void
mmc_arch_spi_set_clock_freq(uint8_t dev, uint32_t freq)
{
  spix_set_clock_freq(USD_SPI_INSTANCE, freq);
}
/*----------------------------------------------------------------------------*/
void
mmc_arch_spi_xfer(uint8_t dev, const void *tx_buf, size_t tx_cnt,
                  void *rx_buf, size_t rx_cnt)
{
  const uint8_t *tx_buf_u8 = tx_buf;
  uint8_t *rx_buf_u8 = rx_buf;

  while(tx_cnt || rx_cnt) {
    SPIX_WAITFORTxREADY(USD_SPI_INSTANCE);
    if(tx_cnt) {
      SPIX_BUF(USD_SPI_INSTANCE) = *tx_buf_u8++;
      tx_cnt--;
    } else {
      SPIX_BUF(USD_SPI_INSTANCE) = 0;
    }
    SPIX_WAITFOREOTx(USD_SPI_INSTANCE);
    SPIX_WAITFOREORx(USD_SPI_INSTANCE);
    if(rx_cnt) {
      *rx_buf_u8++ = SPIX_BUF(USD_SPI_INSTANCE);
      rx_cnt--;
    } else {
      SPIX_BUF(USD_SPI_INSTANCE);
    }
  }
}
/*----------------------------------------------------------------------------*/

/** @} */
