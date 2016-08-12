/*
 * Copyright (c) 2015, Zolertia
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
 *
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
 * \addtogroup zoul
 * @{
 *
 * \defgroup zoul-cc1200 Zoul CC1200 arch
 *
 * CC1200 Zoul arch specifics
 * @{
 *
 * \file
 * CC1200 Zoul arch specifics
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "reg.h"
#include "spi-arch.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/spi.h"
#include "dev/ssi.h"
#include "dev/gpio.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define CC1200_SPI_CLK_PORT_BASE   GPIO_PORT_TO_BASE(SPI0_CLK_PORT)
#define CC1200_SPI_CLK_PIN_MASK    GPIO_PIN_MASK(SPI0_CLK_PIN)
#define CC1200_SPI_MOSI_PORT_BASE  GPIO_PORT_TO_BASE(SPI0_TX_PORT)
#define CC1200_SPI_MOSI_PIN_MASK   GPIO_PIN_MASK(SPI0_TX_PIN)
#define CC1200_SPI_MISO_PORT_BASE  GPIO_PORT_TO_BASE(SPI0_RX_PORT)
#define CC1200_SPI_MISO_PIN_MASK   GPIO_PIN_MASK(SPI0_RX_PIN)
#define CC1200_SPI_CSN_PORT_BASE   GPIO_PORT_TO_BASE(CC1200_SPI_CSN_PORT)
#define CC1200_SPI_CSN_PIN_MASK    GPIO_PIN_MASK(CC1200_SPI_CSN_PIN)
#define CC1200_GDO0_PORT_BASE      GPIO_PORT_TO_BASE(CC1200_GDO0_PORT)
#define CC1200_GDO0_PIN_MASK       GPIO_PIN_MASK(CC1200_GDO0_PIN)
#define CC1200_GDO2_PORT_BASE      GPIO_PORT_TO_BASE(CC1200_GDO2_PORT)
#define CC1200_GDO2_PIN_MASK       GPIO_PIN_MASK(CC1200_GDO2_PIN)
#define CC1200_RESET_PORT_BASE     GPIO_PORT_TO_BASE(CC1200_RESET_PORT)
#define CC1200_RESET_PIN_MASK      GPIO_PIN_MASK(CC1200_RESET_PIN)
/*---------------------------------------------------------------------------*/
#ifndef DEBUG_CC1200_ARCH
#define DEBUG_CC1200_ARCH 0
#endif
/*---------------------------------------------------------------------------*/
#if DEBUG_CC1200_ARCH > 0
#define PRINTF(...) printf(__VA_ARGS__)
#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) {} \
    if(!(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)))) { \
      printf("ARCH: Timeout exceeded in line %d!\n", __LINE__); \
    } \
  } while(0)
#else
#define PRINTF(...)
#define BUSYWAIT_UNTIL(cond, max_time) while(!cond)
#endif
/*---------------------------------------------------------------------------*/
extern int cc1200_rx_interrupt(void);
/*---------------------------------------------------------------------------*/
void
cc1200_int_handler(uint8_t port, uint8_t pin)
{
  /* To keep the gpio_register_callback happy */
  cc1200_rx_interrupt();
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_spi_select(void)
{
  /* Set CSn to low (0) */
  GPIO_CLR_PIN(CC1200_SPI_CSN_PORT_BASE, CC1200_SPI_CSN_PIN_MASK);
  /* The MISO pin should go low before chip is fully enabled. */
  BUSYWAIT_UNTIL(
    GPIO_READ_PIN(CC1200_SPI_MISO_PORT_BASE, CC1200_SPI_MISO_PIN_MASK) == 0,
    RTIMER_SECOND / 100);
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_spi_deselect(void)
{
  /* Set CSn to high (1) */
  GPIO_SET_PIN(CC1200_SPI_CSN_PORT_BASE, CC1200_SPI_CSN_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
int
cc1200_arch_spi_rw_byte(uint8_t c)
{
  SPI_WAITFORTx_BEFORE();
  SPIX_BUF(CC1200_SPI_INSTANCE) = c;
  SPIX_WAITFOREOTx(CC1200_SPI_INSTANCE);
  SPIX_WAITFOREORx(CC1200_SPI_INSTANCE);
  c = SPIX_BUF(CC1200_SPI_INSTANCE);

  return c;
}
/*---------------------------------------------------------------------------*/
int
cc1200_arch_spi_rw(uint8_t *inbuf, const uint8_t *write_buf, uint16_t len)
{
  int i;
  uint8_t c;

  if((inbuf == NULL && write_buf == NULL) || len <= 0) {
    return 1;
  } else if(inbuf == NULL) {
    for(i = 0; i < len; i++) {
      SPI_WAITFORTx_BEFORE();
      SPIX_BUF(CC1200_SPI_INSTANCE) = write_buf[i];
      SPIX_WAITFOREOTx(CC1200_SPI_INSTANCE);
      SPIX_WAITFOREORx(CC1200_SPI_INSTANCE);
      c = SPIX_BUF(CC1200_SPI_INSTANCE);
      /* read and discard to avoid "variable set but not used" warning */
      (void)c;
    }
  } else if(write_buf == NULL) {
    for(i = 0; i < len; i++) {
      SPI_WAITFORTx_BEFORE();
      SPIX_BUF(CC1200_SPI_INSTANCE) = 0;
      SPIX_WAITFOREOTx(CC1200_SPI_INSTANCE);
      SPIX_WAITFOREORx(CC1200_SPI_INSTANCE);
      inbuf[i] = SPIX_BUF(CC1200_SPI_INSTANCE);
    }
  } else {
    for(i = 0; i < len; i++) {
      SPI_WAITFORTx_BEFORE();
      SPIX_BUF(CC1200_SPI_INSTANCE) = write_buf[i];
      SPIX_WAITFOREOTx(CC1200_SPI_INSTANCE);
      SPIX_WAITFOREORx(CC1200_SPI_INSTANCE);
      inbuf[i] = SPIX_BUF(CC1200_SPI_INSTANCE);
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_gpio0_setup_irq(int rising)
{

  GPIO_SOFTWARE_CONTROL(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  GPIO_SET_INPUT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  GPIO_DETECT_EDGE(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  GPIO_TRIGGER_SINGLE_EDGE(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);

  if(rising) {
    GPIO_DETECT_RISING(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  } else {
    GPIO_DETECT_FALLING(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  }

  GPIO_ENABLE_INTERRUPT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  ioc_set_over(CC1200_GDO0_PORT, CC1200_GDO0_PIN, IOC_OVERRIDE_PUE);
  nvic_interrupt_enable(CC1200_GPIOx_VECTOR);
  gpio_register_callback(cc1200_int_handler, CC1200_GDO0_PORT,
                         CC1200_GDO0_PIN);
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_gpio2_setup_irq(int rising)
{

  GPIO_SOFTWARE_CONTROL(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  GPIO_SET_INPUT(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  GPIO_DETECT_EDGE(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  GPIO_TRIGGER_SINGLE_EDGE(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);

  if(rising) {
    GPIO_DETECT_RISING(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  } else {
    GPIO_DETECT_FALLING(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  }

  GPIO_ENABLE_INTERRUPT(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  ioc_set_over(CC1200_GDO2_PORT, CC1200_GDO2_PIN, IOC_OVERRIDE_PUE);
  nvic_interrupt_enable(CC1200_GPIOx_VECTOR);
  gpio_register_callback(cc1200_int_handler, CC1200_GDO2_PORT,
                         CC1200_GDO2_PIN);
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_gpio0_enable_irq(void)
{
  GPIO_ENABLE_INTERRUPT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  ioc_set_over(CC1200_GDO0_PORT, CC1200_GDO0_PIN, IOC_OVERRIDE_PUE);
  nvic_interrupt_enable(CC1200_GPIOx_VECTOR);
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_gpio0_disable_irq(void)
{
  GPIO_DISABLE_INTERRUPT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_gpio2_enable_irq(void)
{
  GPIO_ENABLE_INTERRUPT(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  ioc_set_over(CC1200_GDO2_PORT, CC1200_GDO2_PIN, IOC_OVERRIDE_PUE);
  nvic_interrupt_enable(CC1200_GPIOx_VECTOR);
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_gpio2_disable_irq(void)
{
  GPIO_DISABLE_INTERRUPT(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
int
cc1200_arch_gpio0_read_pin(void)
{
  return GPIO_READ_PIN(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
int
cc1200_arch_gpio2_read_pin(void)
{
  return GPIO_READ_PIN(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
int
cc1200_arch_gpio3_read_pin(void)
{
  return 0x00;
}
/*---------------------------------------------------------------------------*/
void
cc1200_arch_init(void)
{
  /* First leave RESET high */
  GPIO_SOFTWARE_CONTROL(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);
  GPIO_SET_OUTPUT(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);
  ioc_set_over(CC1200_RESET_PORT, CC1200_RESET_PIN, IOC_OVERRIDE_OE);
  GPIO_SET_PIN(CC1200_RESET_PORT_BASE, CC1200_RESET_PIN_MASK);

  /* Initialize CSn, enable CSn and then wait for MISO to go low*/
  spix_cs_init(CC1200_SPI_CSN_PORT, CC1200_SPI_CSN_PIN);

  /* Initialize SPI */
  spix_init(CC1200_SPI_INSTANCE);

  /* Configure GPIOx */
  GPIO_SOFTWARE_CONTROL(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  GPIO_SET_INPUT(CC1200_GDO0_PORT_BASE, CC1200_GDO0_PIN_MASK);
  GPIO_SOFTWARE_CONTROL(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);
  GPIO_SET_INPUT(CC1200_GDO2_PORT_BASE, CC1200_GDO2_PIN_MASK);

  /* Leave CSn as default */
  cc1200_arch_spi_deselect();

  /* Ensure MISO is high */
  BUSYWAIT_UNTIL(
    GPIO_READ_PIN(CC1200_SPI_MISO_PORT_BASE, CC1200_SPI_MISO_PIN_MASK),
    RTIMER_SECOND / 10);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

