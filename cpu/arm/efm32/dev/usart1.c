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
 *         EFM32 USART1 driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include <stdlib.h>
#include <stdio.h>
#include <dev/spi.h>

#include "irq.h"
#include "em_cmu.h"
#include "em_dma.h"

#include "dmactrl.h"
#include "mutex.h"
#include "usart1.h"
#include "gpio.h"

//#define WITH_DMA
#ifdef WITH_DMA

#define DMA_CHANNEL_TX   0
#define DMA_CHANNEL_RX   1
#define DMA_CHANNELS     2
#endif


#ifdef WITH_SPI1
//#if 1
static unsigned char _u8_spi1_initialized = 0;
unsigned char spi1_busy = 0;

// Default SPI to SPI1
/*
void spi_init(void)                        __attribute__ ((alias ("spi1_init")));
uint8_t spi_readwrite(uint8_t data)        __attribute__ ((alias ("spi1_readwrite")));
int spi_readbuf(uint8_t *buf, int readlen) __attribute__ ((alias ("spi1_readbuf")));
int spi_writebuf(uint8_t *buf, int len)    __attribute__ ((alias ("spi1_writebuf")));
*/
struct mutex spi1_mutex;
unsigned long irq_flags;

#ifdef WITH_DMA
/* DMA Callback structure */
DMA_CB_TypeDef dmaCallbackSpi;
/* Transfer Flags */
volatile uint8_t rxActive;
volatile uint8_t txActive;
static volatile bool autoTx = false;

/* DMA RX buffer management */
static volatile int bytesLeft;
static volatile int rxlen = 0;
/* RX buffer pointer used by DMA */
static uint8_t *rxBufferG;

#endif

void spi1_claim_bus(void)
{
	//irq_flags = irq_save();
	//irq_disable();
	mutex_lock(&spi1_mutex);
}

void spi1_release_bus(void)
{
	mutex_unlock(&spi1_mutex);
	//irq_restore(irq_flags);
	//irq_enable();
}



#ifdef WITH_DMA

/**************************************************************************//**
 * @brief USART1 RX IRQ Handler
 * Used to read out last 3 bytes of RX registers when using AUTOTX
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
  int index = rxlen-bytesLeft;

  rxBufferG[index] = USART1->RXDATA;
  bytesLeft--;

  /* Set active flag low and disable interrupt when all bytes have been
  transferred */
  if (bytesLeft == 0)
  {
    txActive = false;
    rxActive = false;

    USART1->IEN &= ~USART_IEN_RXDATAV;
  }
}

/**************************************************************************//**
 * @brief USART1 TX IRQ Handler
 * Used to get notified when the transfer is complete when using
 * AUTOTX. When this handler is called, N-3 bytes have been transferred by DMA
 * and the USART has stopped transmitting since the RX buffers are full.
*****************************************************************************/
void USART1_TX_IRQHandler(void)
{
  /* Disable and clear the TXC interrupt */
  USART1->IEN &= USART_IEN_TXC;
  USART1->IFC = USART_IFC_TXC;

  /* Disable AUTOTX to stop requesting more data */
  USART1->CTRL &= ~USART_CTRL_AUTOTX;

  /* Enable RXDATAV interrupt to fetch the last three bytes */
  bytesLeft = 3;
  USART1->IEN |= USART_IEN_RXDATAV;
}

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
    if (autoTx)
    {
      /* Enable TXC interrupt to get notified when the last 3 bytes have
       * been requested. These will be collected by the RXDATAV interrupt. */
      USART1->IEN |= USART_IEN_TXC;
    }
    else
    {
      rxActive = false;
    }
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
  /* Use AUTOTX if MOSI data is irrelevant (reading from slave) */
  autoTx = (txBuffer == NULL);

  /* Only activate RX DMA if a receive buffer is specified */
  if (rxBuffer != NULL)
  {
    rxlen = bytes;
    /* Setting flag to indicate that RX is in progress
     * will be cleared by call-back function */
    rxActive = true;

    /* Clear RX registers */
    USART1->CMD = USART_CMD_CLEARRX;

    /* Activate RX channel */
    DMA_ActivateBasic(DMA_CHANNEL_RX,
                      true,
                      false,
                      rxBuffer,
                      (void *)&(USART1->RXDATA),
                      bytes - (autoTx ? 4 : 1)); /* Skip last 3 bytes if AUTOTX is used */
  }

  /* Clear TX registers */
  USART1->CMD = USART_CMD_CLEARTX;

  /* Setting flag to indicate that TX is in progress
   * will be cleared by callback function or USART RX interrupt (if using AUTOTX) */
  txActive = true;

  /* Activate AUTOTX when only reading from slave. If using TX data from a
   * buffer use a TX DMA channel */
  if (autoTx)
  {
    rxBufferG = (uint8_t *) rxBuffer; /* Copy buffer pointer to global variable */


    /* Setting AUTOTX will start TX as long as there is room in RX registers */
    USART1->CTRL |= USART_CTRL_AUTOTX;
  }
  else
  {
    /* Activate TX channel */
    DMA_ActivateBasic(DMA_CHANNEL_TX,
                      true,
                      false,
                      (void *)&(USART1->TXDATA),
                      txBuffer,
                      bytes - 1);
  }
}

void usart1_dma_init(void)
{
  /* Initialization structs */
  //DMA_Init_TypeDef        dmaInit;
  DMA_CfgChannel_TypeDef  rxChnlCfg;
  DMA_CfgDescr_TypeDef    rxDescrCfg;
  DMA_CfgChannel_TypeDef  txChnlCfg;
  DMA_CfgDescr_TypeDef    txDescrCfg;

  /* Initializing the DMA */
  /*
  dmaInit.hprot        = 0;
  dmaInit.controlBlock = dmaControlBlock;
  DMA_Init(&dmaInit);
  */

  /* Setup call-back function */
  dmaCallbackSpi.cbFunc  = transferComplete;
  dmaCallbackSpi.userPtr = NULL;

  /*** Setting up RX DMA ***/

  /* Setting up channel */
  rxChnlCfg.highPri   = false;
  rxChnlCfg.enableInt = true;
  rxChnlCfg.select    = DMAREQ_USART1_RXDATAV;
  rxChnlCfg.cb        = &dmaCallbackSpi;
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
  txChnlCfg.select    = DMAREQ_USART1_TXBL;
  txChnlCfg.cb        = &dmaCallbackSpi;
  DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

  /* Setting up channel descriptor */
  txDescrCfg.dstInc  = dmaDataIncNone;
  txDescrCfg.srcInc  = dmaDataInc1;
  txDescrCfg.size    = dmaDataSize1;
  txDescrCfg.arbRate = dmaArbitrate1;
  txDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);


  /* Configure interrupt for TX/RX, but do not enable them */
  /* Interrupts will be enabled only for reading last 3 bytes
   * when using AUTOTX */
  NVIC_ClearPendingIRQ(USART1_RX_IRQn);
  NVIC_EnableIRQ(USART1_RX_IRQn);
  NVIC_ClearPendingIRQ(USART1_TX_IRQn);
  NVIC_EnableIRQ(USART1_TX_IRQn);

}

/**************************************************************************//**
 * @brief  Returns true if an SPI transfer is active
 *****************************************************************************/
bool spiIsActive(void)
{
  bool temp;

  irq_disable();
  temp = rxActive;
  temp = temp || txActive;
  irq_enable();
  return temp;
}

#endif

uint8_t spi1_readwrite(uint8_t data)
{
  /*
	USART_Tx(USART1, data);
	return USART_Rx(USART1);
	*/
  USART1->TXDATA = data;
  while (!(USART1->STATUS & USART_STATUS_TXC));
  return (uint8_t)(USART1->RXDATA);
}

int spi1_readbuf(uint8_t *buf, int readlen)
{
#ifdef WITH_DMA
  spiDmaTransfer(NULL, buf, readlen);
  while(spiIsActive());
#else
  int i = 0;

  irq_disable();
  //USART1->CTRL |= USART_CTRL_AUTOTX;
  for(i=0;i<readlen;i++)
  {
      buf[i] = spi1_readwrite(0xFF);
  }
  //USART1->CTRL &= ~USART_CTRL_AUTOTX;
  irq_enable();
#endif
  return readlen;
}

int spi1_writebuf(uint8_t *buf, int len)
{
#ifdef WITH_DMA
  spiDmaTransfer(buf, NULL, len);

  while(spiIsActive());

  while (!(USART1->STATUS & USART_STATUS_TXC));

#else
	int i=0;

	irq_disable();

	for(i=0; i<len; i++)
	{
		spi1_readwrite(buf[i]);
	}

	irq_enable();
#endif
	return len;
}

/*---------------------------------------------------------------------------*/
void spi1_init(void)
{
  USART_InitSync_TypeDef bcinit = USART_INITSYNC_DEFAULT;

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART1, true);

  if(_u8_spi1_initialized != 0)
  {
    return;
  }
  // MSB first
  bcinit.msbf = true;

  bcinit.baudrate = 7000000;

  USART_InitSync(USART1, &bcinit);

  USART1->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN |
                  (USART1_LOCATION << _USART_ROUTE_LOCATION_SHIFT);
#ifdef WITH_PM
  power_register(&spi1_pm_ops);
#endif

#ifdef WITH_DMA
  usart1_dma_init();
#endif

  mutex_init(&spi1_mutex);
  _u8_spi1_initialized = 1;
}

#else /* Use Usart as UART */

static uart_input_t usart1_input_handler;

/*---------------------------------------------------------------------------*/
//USART1_TX_IRQHandler
void USART1_RX_IRQHandler(void)
{
  unsigned char c;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(USART1->STATUS & USART_STATUS_RXDATAV)
  {
    c = USART_Rx(USART1);
    if(usart1_input_handler != NULL) usart1_input_handler(c);
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
usart1_active(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
void
usart1_set_input(uart_input_t input)
{
  usart1_input_handler = input;
}
/*---------------------------------------------------------------------------*/
uart_input_t usart1_get_input(void)
{
  return usart1_input_handler;
}
/*---------------------------------------------------------------------------*/
void
usart1_writeb(unsigned char c)
{
  watchdog_periodic();
  USART_Tx(USART1, c);
}
/*---------------------------------------------------------------------------*/
unsigned int
usart1_send_bytes(const unsigned char *seq, unsigned int len)
{
  /* TODO : Use DMA Here ... */

  int i=0;
  for(i=0;i<len;i++)
  {
    usart1_writeb(seq[i]);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
void
usart1_drain(void)
{
  while (!(USART1->STATUS & USART_STATUS_TXBL));
}
/*---------------------------------------------------------------------------*/
/**
 * Initialize the UART.
 *
 */
void
usart1_init(unsigned long baudrate)
{
  USART_InitAsync_TypeDef init =   USART_INITASYNC_DEFAULT;
  /* Configure controller */

  // Enable clocks
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART1, true);

  init.enable = usartDisable;
  init.baudrate = baudrate;
  if(baudrate > 115200) init.oversampling = usartOVS4;
  USART_InitAsync(USART1, &init);

  /* Enable pins at USART1 location */
  USART1->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN |
                  (USART1_LOCATION << _USART_ROUTE_LOCATION_SHIFT);

  /* Clear previous RX interrupts */
  USART_IntClear(USART1, USART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(USART1_RX_IRQn);

  /* Enable RX interrupts */
  USART_IntEnable(USART1, USART_IF_RXDATAV);
  NVIC_EnableIRQ(USART1_RX_IRQn);

  /* Finally enable it */
  USART_Enable(USART1, usartEnable);
}
/*---------------------------------------------------------------------------*/

#endif /* WITH_SPI1 */

/** @} */
