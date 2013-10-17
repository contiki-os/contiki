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
 */
#include "contiki.h"
#include "reg.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"
#include "spi-arch.h"
#include "sys/energest.h"
#include "dev/leds.h"


/*---------------------------------------------------------------------------*/
/*
 * Once we know what SSI we're on, configure correct values to be written to
 * the correct registers
 */
#if SSI_BASE==SSI1_BASE
/* Running, in sleep, in deep sleep, enable the clock for the correct SSI */
#define SYS_CTRL_RCGCSSI_SSI            SYS_CTRL_RCGCSSI_SSI1
#define SYS_CTRL_SCGCSSI_SSI            SYS_CTRL_SCGCSSI_SSI1
#define SYS_CTRL_DCGCSSI_SSI            SYS_CTRL_DCGCSSI_SSI1

#define NVIC_INT_SSI                    NVIC_INT_SSI1

#define IOC_PXX_SEL_SSI_TXD             IOC_PXX_SEL_SSI1_TXD
#define IOC_PXX_SEL_SSI_CLKOUT          IOC_PXX_SEL_SSI1_CLKOUT
#define IOC_PXX_SEL_SSI_FSSOUT          IOC_PXX_SEL_SSI1_FSSOUT
#define IOC_PXX_SEL_SSI_STXSER_EN       IOC_PXX_SEL_SSI1_STXSER_EN

#define IOC_CLK_SSI_SSI                 IOC_CLK_SSI_SSI1
#define IOC_SSIRXD_SSI                  IOC_SSIRXD_SSI1
#define IOC_SSIFSSIN_SSI                IOC_SSIFSSIN_SSI1
#define IOC_CLK_SSIIN_SSI               IOC_CLK_SSIIN_SSI1
#else /* Defaults for SSI0 */
#define SYS_CTRL_RCGCSSI_SSI            SYS_CTRL_RCGCSSI_SSI0
#define SYS_CTRL_SCGCSSI_SSI            SYS_CTRL_SCGCSSI_SSI0
#define SYS_CTRL_DCGCSSI_SSI            SYS_CTRL_DCGCSSI_SSI0

#define NVIC_INT_SSI                    NVIC_INT_SSI0

#define IOC_PXX_SEL_SSI_TXD             IOC_PXX_SEL_SSI0_TXD
#define IOC_PXX_SEL_SSI_CLKOUT          IOC_PXX_SEL_SSI0_CLKOUT
#define IOC_PXX_SEL_SSI_FSSOUT          IOC_PXX_SEL_SSI0_FSSOUT
#define IOC_PXX_SEL_SSI_STXSER_EN       IOC_PXX_SEL_SSI0_STXSER_EN


#define IOC_CLK_SSI_SSI                 IOC_CLK_SSI_SSI0
#define IOC_SSIRXD_SSI                  IOC_SSIRXD_SSI0
#define IOC_SSIFSSIN_SSI                IOC_SSIFSSIN_SSI0
#define IOC_CLK_SSIIN_SSI               IOC_CLK_SSIIN_SSI0
#endif


#if SSI_ISR_ENABLE
static int (* input_handler)( void );
static int (* reset_handler)( void );
/*---------------------------------------------------------------------------*/
/*
 * Set up input function for interrupt
 * This function should take the input from the SPI buffer
 */
void
ssi_set_input(int(*input)(void))
{
  input_handler = input;
}
/*---------------------------------------------------------------------------*/
/*
 * Set up input function for interrupt
 * This function should reset SPI TX settings when SPI is set to SLAVE
 */
void
ssi_set_reset(int (* input)(void))
{
  reset_handler = input;
}
#endif
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the SPI bus.
 *
 * This SPI init() function uses the following #defines to set the pins:
 *    CC2538_SPI_CLK_PORT_NUM    CC2538_SPI_CLK_PIN_NUM
 *    CC2538_SPI_MOSI_PORT_NUM   CC2538_SPI_MOSI_PIN_NUM
 *    CC2538_SPI_MISO_PORT_NUM   CC2538_SPI_MISO_PIN_NUM
 *    CC2538_SPI_SEL_PORT_NUM    CC2538_SPI_SEL_PIN_NUM
 *
 * This sets the SPI data width to 8 bits and the mode to Freescale mode 3.
 */
void
spi_init(void)
{
  /* Enable clock for the SSI while Running, in Sleep and Deep Sleep (Enable SSI) */
  REG(SYS_CTRL_RCGCSSI) |= SYS_CTRL_RCGCSSI_SSI;
  REG(SYS_CTRL_SCGCSSI) |= SYS_CTRL_SCGCSSI_SSI;
  REG(SYS_CTRL_DCGCSSI) |= SYS_CTRL_DCGCSSI_SSI;

  /* Start by disabling the peripheral before configuring it */
  REG(SSI_BASE | SSI_CR1) = 0x00000000;

  /* Set the IO clock as the SSI clock */
  REG(SSI_BASE | SSI_CC) = 0x00000001;

  /* Set the mux correctly to connect the SSI pins to the correct GPIO pins */
#if SSI_MODE_SLAVE
  REG(IOC_CLK_SSIIN_SSI) = (CC2538_SPI_CLK_PORT_NUM << 3) + CC2538_SPI_CLK_PIN_NUM;
  REG(IOC_SSIRXD_SSI) = (CC2538_SPI_MOSI_PORT_NUM << 3) + CC2538_SPI_MOSI_PIN_NUM;
  REG(IOC_SSIFSSIN_SSI) = (CC2538_SPI_SEL_PORT_NUM << 3) + CC2538_SPI_SEL_PIN_NUM;
  ioc_set_sel(CC2538_SPI_MISO_PORT_NUM, CC2538_SPI_MISO_PIN_NUM, IOC_PXX_SEL_SSI_TXD);
#else
  ioc_set_sel(CC2538_SPI_CLK_PORT_NUM, CC2538_SPI_CLK_PIN_NUM, IOC_PXX_SEL_SSI_CLKOUT);
  ioc_set_sel(CC2538_SPI_MOSI_PORT_NUM, CC2538_SPI_MOSI_PIN_NUM, IOC_PXX_SEL_SSI_TXD);
  REG(IOC_SSIRXD_SSI) = (CC2538_SPI_MISO_PORT_NUM << 3) + CC2538_SPI_MISO_PIN_NUM;
  ioc_set_sel(CC2538_SPI_SEL_PORT_NUM, CC2538_SPI_SEL_PIN_NUM, IOC_PXX_SEL_SSI_FSSOUT);
#endif

  /* Put all the SSI gpios into peripheral mode */
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_CLK_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_CLK_PIN_NUM));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_MOSI_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_MOSI_PIN_NUM));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_MISO_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_MISO_PIN_NUM));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(CC2538_SPI_SEL_PORT_NUM), GPIO_PIN_MASK(CC2538_SPI_SEL_PIN_NUM));

  /* Disable any pull ups or the like */
  ioc_set_over(CC2538_SPI_CLK_PORT_NUM, CC2538_SPI_CLK_PIN_NUM, IOC_OVERRIDE_DIS);
  ioc_set_over(CC2538_SPI_MOSI_PORT_NUM, CC2538_SPI_MOSI_PIN_NUM, IOC_OVERRIDE_DIS);
  ioc_set_over(CC2538_SPI_SEL_PORT_NUM, CC2538_SPI_SEL_PIN_NUM, IOC_OVERRIDE_DIS);
  ioc_set_over(CC2538_SPI_MISO_PORT_NUM, CC2538_SPI_MISO_PIN_NUM, IOC_OVERRIDE_DIS);


  /* Configure the clock */
  REG(SSI_BASE | SSI_CPSR) = 0x00000002;

  /* Put the SSI in motorola SPI mode with 8 bit data */
  REG(SSI_BASE | SSI_CR0) = SSI_CR0_SPH | SSI_CR0_SPO | (0x00000007);

  /* Enable the SSI */
  REG(SSI_BASE | SSI_CR1) |= SSI_CR1_SSE;

#if SSI_MODE_SLAVE
  /* Set SSI to SLAVE mode */
  REG(SSI_BASE | SSI_CR1) |= SSI_CR1_MS;
#endif
#if SSI_ISR_ENABLE
  /*
   * SSI Interrupt Masks:
   *  RX half empty or less
   *  Receive timeout interrupt flag
   */
  REG(SSI_BASE | SSI_IM) = SSI_IM_RTIM;// | SSI_IM_RXIM;

  /* Enable SSI Interrupts */
  nvic_interrupt_enable(NVIC_INT_SSI);
#endif

  /* Clear the RX FIFO */
  /*SPI_WAITFOREORx();*/
}

#if SSI_ISR_ENABLE
/*---------------------------------------------------------------------------*/
void
ssi_isr(void)
{
/*  uint16_t mis;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  leds_toggle(LEDS_GREEN);
   Store the current MIS
  mis = REG(SSI_BASE | SSI_MIS) & 0x000000FF;*/
  /*Clear the flag
   * */
  /*REG(SSI_BASE | SSI_ICR) = 0x00000003;*/


/*  if(mis & (SSI_MIS_RXMIS)) {
       wait for buffer to fill up
       if(input_handler != NULL) {
          input_handler();
       } else {
          To prevent an Overrun Error, we need to flush the FIFO even if we
          * don't have an input_handler. Use mis as a data trash can
         mis = REG(SSI_BASE | SSI_DR);
       }
    }
  else if (mis & (SSI_MIS_RTMIS)){
       ISR triggered due to timeout
        if(reset_handler != NULL) {
          reset_handler();
      }
       Flush the RX FIFO
      mis = REG(SSI_BASE | SSI_DR);
    }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);*/
}
#endif
/** @} */
