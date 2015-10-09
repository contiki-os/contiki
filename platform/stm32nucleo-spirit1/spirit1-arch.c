/*
 * Copyright (c) 2012, STMicroelectronics.
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
 * 3. Neither the name of the Institute nor the names of its contributors
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
 *
 *
 */
/*---------------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "spirit1-arch.h"
#include "spirit1.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
extern void spirit1_interrupt_callback(void);
st_lib_spirit_bool spiritdk_timer_expired = S_FALSE;
/*---------------------------------------------------------------------------*/
/* use the SPI-port to acquire the status bytes from the radio. */
#define CS_TO_SCLK_DELAY  0x0100
/*---------------------------------------------------------------------------*/
extern st_lib_spi_handle_typedef st_lib_p_spi_handle;
/*---------------------------------------------------------------------------*/
uint16_t
spirit1_arch_refresh_status(void)
{
  volatile uint16_t mcstate = 0x0000;
  uint8_t header[2];
  header[0] = 0x01;
  header[1] = MC_STATE1_BASE;
  uint32_t spi_timeout = ((uint32_t)1000);  /*<! Value of Timeout when SPI communication fails */

  IRQ_DISABLE();

  /* Puts the SPI chip select low to start the transaction */
  st_lib_radio_spi_cs_low();

  for(volatile uint16_t index = 0; index < CS_TO_SCLK_DELAY; index++) {
  }

  /* Write the aHeader bytes and read the SPIRIT1 status bytes */
  st_lib_hal_spi_transmit_receive(&st_lib_p_spi_handle, (uint8_t *)&header[0], (uint8_t *)&mcstate, 1, spi_timeout);
  mcstate = mcstate << 8;

  /* Write the aHeader bytes and read the SPIRIT1 status bytes */
  st_lib_hal_spi_transmit_receive(&st_lib_p_spi_handle, (uint8_t *)&header[1], (uint8_t *)&mcstate, 1, spi_timeout);

  /* To be sure to don't rise the Chip Select before the end of last sending */
  while(st_lib_hal_spi_get_flag(&st_lib_p_spi_handle, SPI_FLAG_TXE) == RESET) ;

  /* Puts the SPI chip select high to end the transaction */
  st_lib_radio_spi_cs_high();

  IRQ_ENABLE();

  return mcstate;
}
