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
 * \addtogroup sensortag-cc26xx-spi
 * @{
 *
 * \file
 * Board-specific SPI driver common to the Sensortag and LaunchPad
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "ti-lib.h"
#include "board-spi.h"
#include "board.h"
#include <lpm.h>

#include <stdbool.h>

#ifndef BOARD_SPI_LPM
#define BOARD_SPI_LPM BOARD_SPI_LPM_NONE
#endif

/*---------------------------------------------------------------------------*/
static bool
accessible(void)
{
  /* First, check the PD */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
     != PRCM_DOMAIN_POWER_ON) {
    return false;
  }

  /* Then check the 'run mode' clock gate */
  if(!(HWREG(PRCM_BASE + PRCM_O_SSICLKGR) & PRCM_SSICLKGR_CLK_EN_SSI0)) {
    return false;
  }

  return true;
}

bool  board_spi_busy(void){
    return ti_lib_ssi_busy(SSI0_BASE) || ((ti_lib_ssi_status(SSI0_BASE) & SSI_TX_EMPTY) == 0);
}

static void board_spi_config(uint32_t bit_rate, uint32_t clk_pin);
static void board_spi_down();

#if BOARD_SPI_LPM == BOARD_SPI_LPM_BASIC

static
uint8_t lpm_request(void)
{
    return LPM_MODE_SLEEP;
}

LPM_MODULE(board_spi_module, lpm_request, NULL, NULL, LPM_DOMAIN_SERIAL | LPM_DOMAIN_PERIPH);

#elif BOARD_SPI_LPM == BOARD_SPI_LPM_DEEPSLEEP_OFF

static
uint8_t lpm_request(void)
{

  if (accessible() != false)
  if (board_spi_busy())
      return LPM_MODE_SLEEP;

  return LPM_MODE_MAX_SUPPORTED;
}

static void lpm_wake(void);
static void lpm_down(uint8_t mode);

LPM_MODULE(board_spi_module, lpm_request, lpm_down, lpm_wake, LPM_DOMAIN_NONE);
struct board_spi_lpmsave_t{
        uint32_t bit_rate;
        uint32_t clk_pin;
} board_spi_lpmsave;

static
void lpm_wake(void){
    board_spi_config(board_spi_lpmsave.bit_rate, board_spi_lpmsave.clk_pin);
}

static
void lpm_down(uint8_t mode){
    if (mode >= LPM_MODE_DEEP_SLEEP){
        board_spi_down();
    }
}

#endif

/*---------------------------------------------------------------------------*/
bool
board_spi_write(const uint8_t *buf, size_t len)
{
  if(accessible() == false) {
    return false;
  }

  while(len > 0) {
    uint32_t ul;

    ti_lib_ssi_data_put(SSI0_BASE, *buf);
    ti_lib_rom_ssi_data_get(SSI0_BASE, &ul);
    len--;
    buf++;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
bool
board_spi_read(uint8_t *buf, size_t len)
{
  if(accessible() == false) {
    return false;
  }

  while(len > 0) {
    uint32_t ul;

    if(!ti_lib_rom_ssi_data_put_non_blocking(SSI0_BASE, 0)) {
      /* Error */
      return false;
    }
    ti_lib_rom_ssi_data_get(SSI0_BASE, &ul);
    *buf = (uint8_t)ul;
    len--;
    buf++;
  }
  return true;
}
/*---------------------------------------------------------------------------*/
void
board_spi_flush()
{
  if(accessible() == false) {
    return;
  }

  uint32_t ul;
  while(ti_lib_rom_ssi_data_get_non_blocking(SSI0_BASE, &ul));
}
/*---------------------------------------------------------------------------*/
static
void
board_spi_config(uint32_t bit_rate, uint32_t clk_pin)
{
  uint32_t buf;

  /* First, make sure the SERIAL PD is on */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
         != PRCM_DOMAIN_POWER_ON));

  /* Enable clock in active mode */
  ti_lib_rom_prcm_peripheral_run_enable(PRCM_PERIPH_SSI0);
  ti_lib_rom_prcm_peripheral_sleep_enable(PRCM_PERIPH_SSI0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

#if BOARD_SPI_LPM > BOARD_SPI_LPM_NONE
  board_spi_module.domain_lock = LPM_DOMAIN_SERIAL;
#endif

  /* SPI configuration */
  ti_lib_ssi_int_disable(SSI0_BASE, SSI_RXOR | SSI_RXFF | SSI_RXTO | SSI_TXFF);
  ti_lib_ssi_int_clear(SSI0_BASE, SSI_RXOR | SSI_RXTO);
  ti_lib_rom_ssi_config_set_exp_clk(SSI0_BASE, ti_lib_sys_ctrl_clock_get(),
                                    SSI_FRF_MOTO_MODE_0,
                                    SSI_MODE_MASTER, bit_rate, 8);
  ti_lib_rom_ioc_pin_type_ssi_master(SSI0_BASE, BOARD_IOID_SPI_MISO,
                                     BOARD_IOID_SPI_MOSI, IOID_UNUSED, clk_pin);
  ti_lib_ssi_enable(SSI0_BASE);

  /* Get rid of residual data from SSI port */
  while(ti_lib_ssi_data_get_non_blocking(SSI0_BASE, &buf));
}

void
board_spi_open(uint32_t bit_rate, uint32_t clk_pin){
#if BOARD_SPI_LPM == BOARD_SPI_LPM_DEEPSLEEP_OFF
    board_spi_lpmsave.bit_rate = bit_rate;
    board_spi_lpmsave.clk_pin  = clk_pin;
#endif
    board_spi_config(bit_rate, clk_pin);
#if BOARD_SPI_LPM > BOARD_SPI_LPM_NONE
    lpm_register_module(&board_spi_module);
#endif
}


/*---------------------------------------------------------------------------*/
void
board_spi_close()
{
#if BOARD_SPI_LPM > BOARD_SPI_LPM_NONE
  lpm_unregister_module(&board_spi_module);
#endif
  board_spi_down();
}

static
void board_spi_down(){

#if BOARD_SPI_LPM > BOARD_SPI_LPM_NONE
  board_spi_module.domain_lock = LPM_DOMAIN_NONE | PRCM_DOMAIN_PERIPH;
#endif

  /* Power down SSI0 */
  ti_lib_rom_prcm_peripheral_sleep_disable(PRCM_PERIPH_SSI0);
  ti_lib_rom_prcm_peripheral_run_disable(PRCM_PERIPH_SSI0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Restore pins to a low-consumption state */
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_SPI_MISO);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_SPI_MISO, IOC_IOPULL_DOWN);

  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_SPI_MOSI);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_SPI_MOSI, IOC_IOPULL_DOWN);

  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_SPI_CLK_FLASH);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_SPI_CLK_FLASH, IOC_IOPULL_DOWN);
}
/*---------------------------------------------------------------------------*/

/** @} */
