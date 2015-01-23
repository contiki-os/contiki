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
 * Implementation of the cc2538 SPI peripheral
 *
 * \authors
 *        Brad Campbell
 *        Vasilis Michopoulos <basilismicho@gmail.com>
 */
#include "contiki.h"
#include "reg.h"
#include "spi-arch.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"

#define SPI_CLK_PORT_BASE        GPIO_PORT_TO_BASE(SPI_CLK_PORT)
#define SPI_CLK_PIN_MASK         GPIO_PIN_MASK(SPI_CLK_PIN)
#define SPI_MOSI_PORT_BASE       GPIO_PORT_TO_BASE(SPI_MOSI_PORT)
#define SPI_MOSI_PIN_MASK        GPIO_PIN_MASK(SPI_MOSI_PIN)
#define SPI_MISO_PORT_BASE       GPIO_PORT_TO_BASE(SPI_MISO_PORT)
#define SPI_MISO_PIN_MASK        GPIO_PIN_MASK(SPI_MISO_PIN)

#ifndef SPI_CONF_DATA_SIZE
#define SPI_CONF_DATA_SIZE       8
#endif
#if SPI_CONF_DATA_SIZE < 4 || SPI_CONF_DATA_SIZE > 16
#error SPI_CONF_DATA_SIZE must be set between 4 and 16 inclusive.
#endif

#if SSI_MODE_SLAVE
static int (*input_handler)(void);
/*---------------------------------------------------------------------------*/
/*
 * Set up input function for interrupt
 * This function should take the input from the SPI buffer
 */
void
ssi_set_input(int (*input)(void))
{
  input_handler = input;
}
#endif
/*---------------------------------------------------------------------------*/

/**
 * \brief Initialize the SPI bus.
 *
 * This SPI init() function uses the following #defines to set the pins:
 *    SPI_CLK_PORT               SPI_CLK_PIN
 *    SPI_MOSI_PORT              SPI_MOSI_PIN
 *    SPI_MISO_PORT              SPI_MISO_PIN
 *
 * This sets the mode to Motorola SPI with the following format options:
 *    Clock phase:               1; data captured on second (rising) edge
 *    Clock polarity:            1; clock is high when idle
 *    Data size:                 8 bits
 */
void
spi_init(void)
{
  spi_enable();

  /* Start by disabling the peripheral before configuring it */
  REG(SSI0_BASE + SSI_CR1) = 0;

  /* Set the IO clock as the SSI clock */
  REG(SSI0_BASE + SSI_CC) = 1;

  /* Set the mux correctly to connect the SSI pins to the correct GPIO pins
   *\note CS pin will be configured by spi_cs_init and toggled manually when SPI is set to master*/
#if SSI_MODE_SLAVE
  REG(IOC_CLK_SSIIN_SSI0) = (SPI_CLK_PORT << 3) + SPI_CLK_PIN;
  REG(IOC_SSIRXD_SSI0) = (SPI_MOSI_PORT << 3) + SPI_MOSI_PIN;
  ioc_set_sel(SPI_MISO_PORT, SPI_MISO_PIN, IOC_PXX_SEL_SSI0_TXD);
#else
  ioc_set_sel(SPI_CLK_PORT, SPI_CLK_PIN, IOC_PXX_SEL_SSI0_CLKOUT);
  ioc_set_sel(SPI_MOSI_PORT, SPI_MOSI_PIN, IOC_PXX_SEL_SSI0_TXD);
  REG(IOC_SSIRXD_SSI0) = (SPI_MISO_PORT << 3) + SPI_MISO_PIN;
#endif
  /* Put all the SSI gpios into peripheral mode */
  GPIO_PERIPHERAL_CONTROL(SPI_CLK_PORT_BASE, SPI_CLK_PIN_MASK);
  GPIO_PERIPHERAL_CONTROL(SPI_MOSI_PORT_BASE, SPI_MOSI_PIN_MASK);
  GPIO_PERIPHERAL_CONTROL(SPI_MISO_PORT_BASE, SPI_MISO_PIN_MASK);

  /* Disable any pull ups or the like */
  ioc_set_over(SPI_CLK_PORT, SPI_CLK_PIN, IOC_OVERRIDE_DIS);
  ioc_set_over(SPI_MOSI_PORT, SPI_MOSI_PIN, IOC_OVERRIDE_DIS);
  ioc_set_over(SPI_MISO_PORT, SPI_MISO_PIN, IOC_OVERRIDE_DIS);

  /*System clock in CC2538 port is set to 16.5MHz. Therefore...
   *
   * NOTE: keep slave seperate from master. Since master can be configured with much higher bit-rate if required.
   * Even though, If master operates in higher bit-rate than slave, received by slave data will be wrong*/
#if SSI_MODE_SLAVE
  /*
   * In Slave mode, maxbitrate must be <= sysclk/6 or sysclk/12 following:
   * SSIClk = SysClk / (CPSDVSR × (1 + SCR))
   *
   *  2 * (7+1) = 16
   *
   *  Therefore bitrate is set to approximately 1mbit/sec
   * */

  REG(SSI0_BASE | SSI_CPSR) = 0x00000002;
  REG(SSI0_BASE | SSI_CR0) = (0x7 << SSI_CR0_SCR_S);

#else
  /*
   * In Master mode, maxbitrate must be <= sysclk/2 following:
   * SSIClk = SysClk / (CPSDVSR × (1 + SCR))
   *
   *  2 * (7+1) = 16
   *
   *  Therefore bitrate is set to approximately 1mbit/sec
   * */
  REG(SSI0_BASE | SSI_CPSR) = 0x00000002;
  REG(SSI0_BASE | SSI_CR0) = (0x7 << SSI_CR0_SCR_S);
#endif
  /* Configure the default SPI options.
   *   mode:  Motorola frame format
   *   clock: High when idle
   *   data:  Valid on rising edges of the clock
   *   bits:  8 byte data
   */
  REG(SSI0_BASE + SSI_CR0) |= SSI_CR0_SPH | SSI_CR0_SPO | (SPI_CONF_DATA_SIZE - 1);

#if SSI_MODE_SLAVE
  /* Set SSI to SLAVE mode */
  REG(SSI0_BASE | SSI_CR1) |= SSI_CR1_MS;
  /*
   * SSI Interrupt Masks:
   *  RX half empty or less
   *  Receive timeout interrupt flag
   */
  REG(SSI0_BASE | SSI_IM) = SSI_IM_RXIM;

  /* Enable SSI Interrupts */
  nvic_interrupt_enable(NVIC_INT_SSI0);
#endif

  /* Enable the SSI */
  REG(SSI0_BASE + SSI_CR1) |= SSI_CR1_SSE;
}
/*---------------------------------------------------------------------------*/
void
spi_cs_init(uint8_t port, uint8_t pin, uint8_t soft_control)
{
#if SSI_MODE_SLAVE
  REG(IOC_SSIFSSIN_SSI0) = (port << 3) + pin;
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
#else
  if(soft_control) {
    GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
    GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  } else {
    ioc_set_sel(port, pin, IOC_PXX_SEL_SSI0_FSSOUT);
    GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  }
#endif
  ioc_set_over(port, pin, IOC_OVERRIDE_DIS);
}
/*---------------------------------------------------------------------------*/
void
spi_enable(void)
{
  /* Enable the clock for the SSI peripheral */
  REG(SYS_CTRL_RCGCSSI) |= 1;
}
/*---------------------------------------------------------------------------*/
void
spi_disable(void)
{
  /* Gate the clock for the SSI peripheral */
  REG(SYS_CTRL_RCGCSSI) &= ~1;
}
/*---------------------------------------------------------------------------*/
void
spi_set_mode(uint32_t frame_format, uint32_t clock_polarity, uint32_t clock_phase, uint32_t data_size)
{
  /* Disable the SSI peripheral to configure it */
  REG(SSI0_BASE + SSI_CR1) = 0;

  if(data_size < 4 || data_size > 16) {
    /*thats an error ... add error message*/
  } else {
    /* Configure the SSI options */
    REG(SSI0_BASE + SSI_CR0) = (0x7 << SSI_CR0_SCR_S) | clock_phase | clock_polarity | frame_format | (data_size - 1);
  }
  /* Re-enable the SSI */
  REG(SSI0_BASE + SSI_CR1) |= SSI_CR1_SSE;
}
#if SSI_MODE_SLAVE
/*---------------------------------------------------------------------------*/
void
ssi_isr(void)
{
  uint16_t mis;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* Store the current MIS*/
  mis = REG(SSI0_BASE | SSI_MIS) & 0x000000FF;
  /*Clear the flag
   * */
  REG(SSI0_BASE | SSI_ICR) = 0x00000003;

  if(mis & (SSI_MIS_RXMIS)) {
    if(input_handler != NULL) {
      /*SSI has an 8 byte long FIFO. The interrupt triggers when
       * FIFO is half full. Therefore, receive all of the data in one
       * function call. If not, Transmission data will have to be
       * multiples of 4 which is not convenient
       *
       * For transmissions up to 8 bytes reception can be asynchronous and
       * thus interrupts are obsolete*/
      input_handler();
    } else {
      /* To prevent an Overrun Error, we need to flush the FIFO even if we
       * don't have an input_handler. Use mis as a data trash can
       * SSI FIFO is 8 bytes long so repeat process till empty*/
      while(REG(SSI0_BASE + SSI_SR) & SSI_SR_RNE) {
        mis = REG(SSI0_BASE | SSI_DR);
      }
    }
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
#endif
/** @} */
