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
 *         EFM32 USART2 driver
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
#include "usart2.h"
#include "gpio.h"
#include "dmactrl.h"

#include "irq.h"
#include "em_emu.h"

#ifdef WITH_SPI2
static unsigned char _u8_spi2_initialized = 0;
unsigned char spi2_busy = 0;

struct mutex spi2_mutex;
unsigned long irq_flags;

/* DMA Callback structure */
DMA_CB_TypeDef cb;
/* Transfer Flags */
volatile uint8_t rxActive;
volatile uint8_t txActive;

/*---------------------------------------------------------------------------*/
void spi2_claim_bus(void)
{
	//irq_flags = irq_save();
	//irq_disable();
	mutex_lock(&spi2_mutex);
}

/*---------------------------------------------------------------------------*/
void spi2_release_bus(void)
{
	mutex_unlock(&spi2_mutex);
	//irq_restore(irq_flags);
	//irq_enable();
}

//#define  WITH_DMA
#ifdef WITH_DMA

#define DMA_CHANNEL_TX   0
#define DMA_CHANNEL_RX   1
#define DMA_CHANNELS     2

/*---------------------------------------------------------------------------*/

void transferComplete(unsigned int channel, bool primary, void *user)
{
  (void) primary;
  (void) user;

  /* Clear flag to indicate complete transfer */
  if (channel == DMA_CHANNEL_TX)
  {
    txActive = false;
  }
  else if (channel == DMA_CHANNEL_RX)
  {
    rxActive = false;
  }
  else
  {
    printf("%s: ERROR\r\n", __func__);
    while(1); /* ERROR, No action defined for this channel */
  }
}
/*---------------------------------------------------------------------------*/
/**
 * @brief  SPI DMA Transfer
 * NULL can be input as txBuffer if tx data to transmit dummy data
 * If only sending data, set rxBuffer as NULL to skip DMA activation on RX
 */
void spiDmaTransfer(void *txBuffer, void *rxBuffer,  int bytes)
{
  /* Only activate RX DMA if a receive buffer is specified */
  if (rxBuffer != NULL)
  {
    /* Setting flag to indicate that RX is in progress
     * will be cleared by call-back function */
    rxActive = true;

    /* Clear RX regiters */
    USART2->CMD = USART_CMD_CLEARRX;

    /* Activate RX channel */
    DMA_ActivateBasic(DMA_CHANNEL_RX,
                      true,
                      false,
                      rxBuffer,
                      (void *)&(USART2->RXDATA),
                      bytes - 1);
  }
  /* Setting flag to indicate that TX is in progress
   * will be cleared by call-back function */
  txActive = true;

  /* Clear TX regsiters */
  USART2->CMD = USART_CMD_CLEARTX;

  /* Activate TX channel */
  DMA_ActivateBasic(DMA_CHANNEL_TX,
                    true,
                    false,
                    (void *)&(USART2->TXDATA),
                    txBuffer,
                    bytes - 1);
}

void usart2_dma_init(void)
{
  /* Initialization structs */
  DMA_CfgChannel_TypeDef  rxChnlCfg;
  DMA_CfgDescr_TypeDef    rxDescrCfg;
  DMA_CfgChannel_TypeDef  txChnlCfg;
  DMA_CfgDescr_TypeDef    txDescrCfg;

  /* Setup call-back function */
  cb.cbFunc  = &transferComplete;
  cb.userPtr = NULL;

  /*** Setting up RX DMA ***/

  /* Setting up channel */
  rxChnlCfg.highPri   = false;
  rxChnlCfg.enableInt = true;
  rxChnlCfg.select    = DMAREQ_USART2_RXDATAV;
  rxChnlCfg.cb        = &cb;
  DMA_CfgChannel(DMA_CHANNEL_RX, &rxChnlCfg);

  /* Setting up channel descriptor */
  rxDescrCfg.dstInc  = dmaDataInc1;
  rxDescrCfg.srcInc  = dmaDataIncNone;
  rxDescrCfg.size    = dmaDataSize1;
  rxDescrCfg.arbRate = dmaArbitrate1;
  rxDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_RX, true, &rxDescrCfg);

  /*** Setting up TX DMA ***/

  /* Setting up channel */
  txChnlCfg.highPri   = false;
  txChnlCfg.enableInt = true;
  txChnlCfg.select    = DMAREQ_USART2_TXBL;
  txChnlCfg.cb        = &cb;
  DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

  /* Setting up channel descriptor */
  txDescrCfg.dstInc  = dmaDataIncNone;
  txDescrCfg.srcInc  = dmaDataIncNone;
  txDescrCfg.size    = dmaDataSize1;
  txDescrCfg.arbRate = dmaArbitrate1;
  txDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);
}

#endif
/*---------------------------------------------------------------------------*/
uint8_t spi2_readwrite(uint8_t data)
{
  return USART_SpiTransfer(USART2, data);
}

const uint8_t dummy_byte = 0x51;
//volatile uint8_t dummy_byte[512];

/*---------------------------------------------------------------------------*/
int spi2_readbuf(uint8_t *buf, int readlen)
{
#ifdef WITH_DMA
  //memset(dummy_byte, 0, sizeof(dummy_byte));
  spiDmaTransfer(&dummy_byte,buf,readlen);

  irq_disable();
  while(rxActive)
  {
    EMU_EnterEM1();
    irq_enable();
    irq_disable();
  }
  irq_enable();
#else
	int i = 0;

	for(i=0;i<readlen;i++)
	{
		buf[i] = spi2_readwrite(0x00);
	}
#endif
	return readlen;
}

/*---------------------------------------------------------------------------*/
int spi2_writebuf(uint8_t *buf, int len)
{
#ifdef WITH_DMA

  spiDmaTransfer(buf,NULL,len);
  while(txActive)
  {
    clock_delay(1);
  };
#else
	int i=0;

	for(i=0; i<len; i++)
	{
	  spi2_readwrite(buf[i]);
	}
#endif
	return len;
}

/*
void DMA_IRQHandler(void)
{
  printf("%s\r\n",__func__);
}
*/

/*---------------------------------------------------------------------------*/
void spi2_init(void)
{
  USART_InitSync_TypeDef bcinit = USART_INITSYNC_DEFAULT;

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART2, true);

  if(_u8_spi2_initialized != 0)
  {
    return;
  }

  // MSB first
  bcinit.msbf = true;

  bcinit.baudrate = 7000000;

  USART_InitSync(USART2, &bcinit);
  // Enable AutoCS
  //USART2->CTRL |= USART_CTRL_AUTOCS;

  USART2->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN |
                  USART_ROUTE_CLKPEN |
                  (USART2_LOCATION << _USART_ROUTE_LOCATION_SHIFT);
#ifdef WITH_PM
  power_register(&spi2_pm_ops);
#endif

#ifdef WITH_DMA
  usart2_dma_init();
#endif

  mutex_init(&spi2_mutex);
  _u8_spi2_initialized = 1;
}
#else /* Use Usart as UART */

static int (*usart2_input_handler)(unsigned char c);

/*---------------------------------------------------------------------------*/
//USART2_TX_IRQHandler
void USART2_RX_IRQHandler(void)
{
  unsigned char c;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(USART2->STATUS & USART_STATUS_RXDATAV)
  {
    c = USART_Rx(USART2);
    if(usart2_input_handler != NULL) usart2_input_handler(c);
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
usart2_active(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
void
usart2_set_input(int (*input)(unsigned char c))
{
  usart2_input_handler = input;
}
/*---------------------------------------------------------------------------*/
int *usart2_get_input(void)
{
  return (int*) usart2_input_handler;
}
/*---------------------------------------------------------------------------*/
void
usart2_writeb(unsigned char c)
{
  watchdog_periodic();
#ifdef USART2_LF_TO_CRLF
  if(c == '\n')
  {
    USART_Tx(USART2, '\r');
  }
#endif

  USART_Tx(USART2, c);

#ifdef USART2_LF_TO_CRLF
  if(c == '\r')
  {
    USART_Tx(USART2, '\n');
  }
#endif
}
/*---------------------------------------------------------------------------*/
unsigned int
usart2_send_bytes(const unsigned char *seq, unsigned int len)
{
  /* TODO : Use DMA Here ... */

  int i=0;
  for(i=0;i<len;i++)
  {
    usart2_writeb(seq[i]);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
void
usart2_drain(void)
{
  while (!(USART2->STATUS & USART_STATUS_TXBL));
}
/*---------------------------------------------------------------------------*/
/**
 * Initialize the UART.
 *
 */
void
usart2_init(unsigned long baudrate)
{
  USART_InitAsync_TypeDef init =   USART_INITASYNC_DEFAULT;
  /* Configure controller */

  // Enable clocks
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART2, true);

  init.enable = usartDisable;
  init.baudrate = baudrate;
  USART_InitAsync(USART2, &init);

  /* Enable pins at USART2 location */
  USART2->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN |
                  (USART2_LOCATION << _USART_ROUTE_LOCATION_SHIFT);

  /* Clear previous RX interrupts */
  USART_IntClear(USART2, USART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(USART2_RX_IRQn);

  /* Enable RX interrupts */
  USART_IntEnable(USART2, USART_IF_RXDATAV);
  NVIC_EnableIRQ(USART2_RX_IRQn);

  /* Finally enable it */
  USART_Enable(USART2, usartEnable);
}
/*---------------------------------------------------------------------------*/

#endif /* WITH_SPI2 */

/** @} */
