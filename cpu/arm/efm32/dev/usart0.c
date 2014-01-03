/*
 * Copyright (c) 2013, Kerlink
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
 * This file is part of the Contiki operating system.
 */

/**
 * \addtogroup efm32-devices
 * @{
 */

/**
 * \file
 *         EFM32 USART0 driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include <stdlib.h>
#include <stdio.h>

#include "em_cmu.h"
#include "em_usart.h"
#include "em_dma.h"
#include "mutex.h"
#include "usart0.h"
#include "gpio.h"
#include "dmactrl.h"

#include "irq.h"
#include "em_emu.h"

#ifdef WITH_SPI0
static unsigned char _u8_spi0_initialized = 0;
unsigned char spi0_busy = 0;

struct mutex spi0_mutex;
unsigned long irq_flags;

void spi0_claim_bus(void)
{
    //irq_flags = irq_save();
    //irq_disable();
    mutex_lock(&spi0_mutex);
}

void spi0_release_bus(void)
{
    mutex_unlock(&spi0_mutex);
    //irq_restore(irq_flags);
    //irq_enable();
}


uint8_t spi0_readwrite(uint8_t data)
{
  /*
    USART_Tx(USART0, data);
    return USART_Rx(USART0);
    */
  USART0->TXDATA = data;
  while (!(USART0->STATUS & USART_STATUS_TXC));
  return (uint8_t)(USART0->RXDATA);
}

int spi0_readbuf(uint8_t *buf, int readlen)
{
#ifdef WITH_DMA

#else
  int i = 0;

  irq_disable();
  //USART1->CTRL |= USART_CTRL_AUTOTX;
  for(i=0;i<readlen;i++)
  {
      buf[i] = spi0_readwrite(0xFF);
  }
  //USART1->CTRL &= ~USART_CTRL_AUTOTX;
  irq_enable();
#endif
  return readlen;
}

int spi0_writebuf(uint8_t *buf, int len)
{
#ifdef WITH_DMA

#else
	int i=0;

	irq_disable();

	for(i=0; i<len; i++)
	{
		spi0_readwrite(buf[i]);
	}

	irq_enable();
#endif
	return len;
}

/*---------------------------------------------------------------------------*/
void spi0_init(void)
{
  USART_InitSync_TypeDef bcinit = USART_INITSYNC_DEFAULT;

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART0, true);

  if(_u8_spi0_initialized != 0)
  {
    return;
  }
  // MSB first
  bcinit.msbf = true;

  bcinit.baudrate = 7000000;

  USART_InitSync(USART0, &bcinit);

  USART0->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN |
                  (USART0_LOCATION << _USART_ROUTE_LOCATION_SHIFT);
#ifdef WITH_PM
  power_register(&spi0_pm_ops);
#endif

  mutex_init(&spi0_mutex);
  _u8_spi0_initialized = 1;
}

#else /* Use Usart as UART */

static int (*usart0_input_handler)(unsigned char c);

/*---------------------------------------------------------------------------*/
//USART0_TX_IRQHandler
void USART0_RX_IRQHandler(void)
{
  unsigned char c;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(USART0->STATUS & USART_STATUS_RXDATAV)
  {
    c = USART_Rx(USART0);
    if(usart0_input_handler != NULL) usart0_input_handler(c);
  }
  /*
  else
  {
	  PRINTF("ERROR: control reg = 0x%lX", (SI32_USART_0->CONTROL.U32 & 0x7));
	  // Disable all errors
	  SI32_USART_0->CONTROL_CLR = 0x07;
  }
*/
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
uint8_t
usart0_active(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
void
usart0_set_input(int (*input)(unsigned char c))
{
  usart0_input_handler = input;
}
/*---------------------------------------------------------------------------*/
int *usart0_get_input(void)
{
  return (int*) usart0_input_handler;
}
/*---------------------------------------------------------------------------*/
void
usart0_writeb(unsigned char c)
{
  watchdog_periodic();
#ifdef USART0_LF_TO_CRLF
  if(c == '\n')
  {
    USART_Tx(USART0, '\r');
  }
#endif

  USART_Tx(USART0, c);

#ifdef USART0_LF_TO_CRLF
  if(c == '\r')
  {
    USART_Tx(USART0, '\n');
  }
#endif
}
/*---------------------------------------------------------------------------*/
unsigned int
usart0_send_bytes(const unsigned char *seq, unsigned int len)
{
  /* TODO : Use DMA Here ... */

  int i=0;
  for(i=0;i<len;i++)
  {
    usart0_writeb(seq[i]);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
void
usart0_drain(void)
{
  while (!(USART0->STATUS & USART_STATUS_TXBL));
}
/*---------------------------------------------------------------------------*/
/**
 * Initialize the UART.
 *
 */
void
usart0_init(unsigned long baudrate)
{
  USART_InitAsync_TypeDef init =   USART_INITASYNC_DEFAULT;
  /* Configure controller */

  // Enable clocks
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART0, true);

  init.enable = usartDisable;
  init.baudrate = baudrate;

  if(baudrate > 115200) init.oversampling = usartOVS4;

  USART_InitAsync(USART0, &init);

  /* Enable pins at USART0 location #2 */
  USART0->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN |
                  (USART0_LOCATION << _USART_ROUTE_LOCATION_SHIFT);

  /* Clear previous RX interrupts */
  USART_IntClear(USART0, USART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);

  /* Enable RX interrupts */
  USART_IntEnable(USART0, USART_IF_RXDATAV);
  NVIC_EnableIRQ(USART0_RX_IRQn);

  /* Finally enable it */
  USART_Enable(USART0, usartEnable);
}
/*---------------------------------------------------------------------------*/
#endif /* WITH_SPI0 */

/** @} */
