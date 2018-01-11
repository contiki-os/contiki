/*
 * Copyright (c) 2012-2013, Thingsquare, http://www.thingsquare.com/.
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * \addtogroup zolertia-orion-ethernet-router
 * @{
 *
 * \defgroup zolertia-eth-arch-gpio Zolertia ENC28J60 GPIO arch
 *
 * ENC28J60 eth-gw GPIO arch specifics
 * @{
 *
 * \file
 * eth-gw GPIO arch specifics
 */
/*---------------------------------------------------------------------------*/
#include "clock.h"
#include "dev/gpio.h"
/*---------------------------------------------------------------------------*/
#define CLK_PORT    GPIO_PORT_TO_BASE(ETH_SPI_CLK_PORT)
#define CLK_BIT     GPIO_PIN_MASK(ETH_SPI_CLK_PIN)
#define MOSI_PORT   GPIO_PORT_TO_BASE(ETH_SPI_MOSI_PORT)
#define MOSI_BIT    GPIO_PIN_MASK(ETH_SPI_MOSI_PIN)
#define MISO_PORT   GPIO_PORT_TO_BASE(ETH_SPI_MISO_PORT)
#define MISO_BIT    GPIO_PIN_MASK(ETH_SPI_MISO_PIN)
#define CSN_PORT    GPIO_PORT_TO_BASE(ETH_SPI_CSN_PORT)
#define CSN_BIT     GPIO_PIN_MASK(ETH_SPI_CSN_PIN)
#define RESET_PORT  GPIO_PORT_TO_BASE(ETH_RESET_PORT)
#define RESET_BIT   GPIO_PIN_MASK(ETH_RESET_PIN)
/*---------------------------------------------------------------------------*/
/* Delay in us */
#define DELAY 10
/*---------------------------------------------------------------------------*/
static void
delay(void)
{
  clock_delay_usec(DELAY);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_select(void)
{
  GPIO_CLR_PIN(CSN_PORT, CSN_BIT);
  delay();
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_deselect(void)
{
  GPIO_SET_PIN(CSN_PORT, CSN_BIT);
}
/*---------------------------------------------------------------------------*/
void
enc28j60_arch_spi_init(void)
{
  /* Set all pins to GPIO mode */
  GPIO_SOFTWARE_CONTROL(CLK_PORT, CLK_BIT);
  GPIO_SOFTWARE_CONTROL(MOSI_PORT, MOSI_BIT);
  GPIO_SOFTWARE_CONTROL(MISO_PORT, MISO_BIT);
  GPIO_SOFTWARE_CONTROL(CSN_PORT, CSN_BIT);
  GPIO_SOFTWARE_CONTROL(RESET_PORT, RESET_BIT);

  /* CSN, MOSI, CLK and RESET are output pins */
  GPIO_SET_OUTPUT(CSN_PORT, CSN_BIT);
  GPIO_SET_OUTPUT(MOSI_PORT, MOSI_BIT);
  GPIO_SET_OUTPUT(CLK_PORT, CLK_BIT);
  GPIO_SET_OUTPUT(RESET_PORT, RESET_BIT);

  /* MISO is an input pin */
  GPIO_SET_INPUT(MISO_PORT, MISO_BIT);

  /* Enable the device */
  GPIO_SET_INPUT(RESET_PORT, RESET_BIT);

  /* The CS pin is active low, so we set it high when we haven't
     selected the chip. */
  enc28j60_arch_spi_deselect();

  /* The CLK is active low, we set it high when we aren't using it. */
  GPIO_CLR_PIN(CLK_PORT, CLK_BIT);
}
/*---------------------------------------------------------------------------*/
uint8_t
enc28j60_arch_spi_write(uint8_t output)
{
  int i;
  uint8_t input;
  input = 0;

  for(i=0; i < 8; i++) {
    /* Write data on MOSI pin */
    if(output & 0x80) {
      GPIO_SET_PIN(MOSI_PORT, MOSI_BIT);
    } else {
      GPIO_CLR_PIN(MOSI_PORT, MOSI_BIT);
    }
    output <<= 1;

    /* Set clock high  */
    GPIO_SET_PIN(CLK_PORT, CLK_BIT);
    delay();

    /* Read data from MISO pin */
    input <<= 1;
    if(GPIO_READ_PIN(MISO_PORT, MISO_BIT) != 0) {
      input |= 0x1;
    }

    /* Set clock low */
    GPIO_CLR_PIN(CLK_PORT, CLK_BIT);
    delay();
  }
  return input;
}
/*---------------------------------------------------------------------------*/
uint8_t
enc28j60_arch_spi_read(void)
{
  return enc28j60_arch_spi_write(0);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

