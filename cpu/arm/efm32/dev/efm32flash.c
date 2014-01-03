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
 *         EFM32 flash manipulation routines
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "irq.h"
#include "efm32flash.h"
#include "dmactrl.h"

#include "em_cmu.h"
#include "em_msc.h"
#include "em_dma.h"
#include "em_int.h"
#include "em_emu.h"

// KLK-MCH : DO NOT Work .... still need debugging
//#define FLASH_DMA
#ifdef FLASH_DMA

#define DMA_CHANNEL_WRITEFLASH 0

/* DMA callback structure */
DMA_CB_TypeDef cb;

/* Transfer Flag */
volatile bool transferActive;

static void transferComplete(unsigned int channel, bool primary, void *user);
#endif


/*---------------------------------------------------------------------------*/
void efm32_flash_init(void)
{
  MSC_Init();

#ifdef FLASH_DMA
  DMA_Init_TypeDef        dmaInit;
  DMA_CfgChannel_TypeDef  chnlCfg;
  DMA_CfgDescr_TypeDef    descrCfg;

//  CMU_ClockEnable(cmuClock_DMA, true);
  /* Initializing the DMA */
  /*
  dmaInit.hprot        = 0;
  dmaInit.controlBlock = dmaControlBlock;
  DMA_Init(&dmaInit);
*/
  /* Setting call-back function */
  cb.cbFunc  = transferComplete;
  cb.userPtr = NULL;

  /* Setting up channel */
  chnlCfg.highPri   = false;
  chnlCfg.enableInt = true;
  chnlCfg.select    = DMAREQ_MSC_WDATA;
  chnlCfg.cb        = &(cb);
  DMA_CfgChannel(DMA_CHANNEL_WRITEFLASH, &chnlCfg);

  /* Setting up channel descriptor */
  descrCfg.dstInc  = dmaDataIncNone;
  descrCfg.srcInc  = dmaDataInc4;
  descrCfg.size    = dmaDataSize4;
  descrCfg.arbRate = dmaArbitrate1;
  descrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_WRITEFLASH, true, &descrCfg);
#endif
}

/*---------------------------------------------------------------------------*/
int flash_read(uint32_t address, void * data, uint32_t length)
{
  uint32_t * pdata = (uint32_t *)address;

  memcpy(data, pdata, length);

  return length;
}

#ifdef FLASH_DMA
/**************************************************************************//**
* @brief  Call-back called when flash transfer is complete
*****************************************************************************/
static void transferComplete(unsigned int channel, bool primary, void *user)
{
  (void) channel;
  (void) primary;
  (void) user;

  /* Clearing flag to indicate that transfer is complete */
  transferActive = false;
}

// Length must be %4
int flash_write_pages(uint32_t address, const char * data, uint32_t length)
{
  DMA_DESCRIPTOR_TypeDef *descr;
  int ret = 0;
  /* Enable writing to the MSC */
  MSC->WRITECTRL |= MSC_WRITECTRL_WREN;

  /* Load address */
  MSC->ADDRB    = (uint32_t) (address);
  MSC->WRITECMD = MSC_WRITECMD_LADDRIM;

  if (!(MSC->STATUS & (MSC_STATUS_INVADDR | MSC_STATUS_LOCKED)))
  {
    // Wait for transfer ends
    //while(DMA_ChannelEnabled(DMA_CHANNEL_WRITEFLASH) | transferActive);
    while(DMA_ChannelEnabled(DMA_CHANNEL_WRITEFLASH));

    // set transfer flag
    transferActive = true;

    // Load first word into DMA
    MSC->WDATA = *((uint32_t *)(data));

    DMA_ActivateBasic(DMA_CHANNEL_WRITEFLASH,
    true,
    false,
    (void *)(&(MSC->WDATA)),
    (void *)data,
    (length/4)-2);

    // Trigger transfer
    MSC->WRITECMD = MSC_WRITECMD_WRITETRIG;


    /* Enter EM1 while DMA transfer is active to save power. Note that
     * interrupts are disabled to prevent the ISR from being triggered
     * after checking the transferActive flag, but before entering
     * sleep. If this were to happen, there would be no interrupt to wake
     * the core again and the MCU would be stuck in EM1. While the
     * core is in sleep, pending interrupts will still wake up the
     * core and the ISR will be triggered after interrupts are enabled
     * again.
     */
#if 0
    while(1)
    {
      INT_Disable();
      if ( transferActive )
      {
        EMU_EnterEM1();
      }
      INT_Enable();

      /* Exit the loop if transfer has completed */
      if ( !transferActive )
      {
        break;
      }
    }
#endif

    ret = length;
  }

  /* Disable writing to the MSC */
  //MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;

//  DMA_Reset();

  return ret;
}
#else

int flash_write_pages(uint32_t address, const char * data, uint32_t length)
{
  msc_Return_TypeDef retval = mscReturnOk;

  // Disable interrupts.
  INT_Disable();

  //retval = MscLoadData((const uint32_t *) data,length/4);
  retval = MSC_WriteWord((uint32_t *) address, (const void*) data, length);

  INT_Enable();

  if(retval != mscReturnOk)
  {
    return retval;
  }
  else
  {
    return length;
  }
}
#endif

/*---------------------------------------------------------------------------*/
// Erase <number> pages from start_address
int flash_erase(uint32_t start_address, uint32_t length)
{
  unsigned int i = 0;
  uint32_t number = (length / FLASH_PAGE_SIZE);

  if(length%FLASH_PAGE_SIZE != 0) number++;

  INT_Disable();

  for(i=0; i<number; i++)
  {
    MSC_ErasePage((uint32_t *) start_address + i * FLASH_PAGE_SIZE);
  }
  // Enable interrupts
  INT_Enable();

  return length;
}

#if 0
// Allocates a buffer of FLASH_PAGE_SIZE bytes statically (rather than on the stack).
#ifndef STATIC_FLASH_BUFFER
#define STATIC_FLASH_BUFFER 0
#endif

int flash_write_partial(uint32_t address, const char * data, uint32_t length)
{
  const uint32_t end = address + length;
  uint32_t i;
  uint32_t next_page, curr_page;
  uint16_t offset;

#if STATIC_FLASH_BUFFER
  static unsigned char buf[FLASH_PAGE_SIZE];
#else
  unsigned char buf[FLASH_PAGE_SIZE];
#endif


  for(i = address; i < end;)
  {
	  next_page = (i | (FLASH_PAGE_SIZE-1)) + 1;
	  curr_page = i & ~(FLASH_PAGE_SIZE-1);
	  offset = i-curr_page;
	  if(next_page > end) {
		next_page = end;
	  }

	  // Read a page from flash and put it into a mirror buffer.
	  flash_read(curr_page,buf,FLASH_PAGE_SIZE);

	  // Update flash mirror data with new data.
	  memcpy(buf + offset, data, next_page - i);
	  // Erase flash page.
	  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
	  flash_erase(curr_page,1);
	  // Write modified data form mirror buffer into the flash.
	  flash_write_pages(curr_page, buf, FLASH_PAGE_SIZE);
	  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);

	  data = (uint8_t *)data + next_page - i;
	  i = next_page;
  }

  return length;
}
#endif

/*---------------------------------------------------------------------------*/
int flash_write(uint32_t address, const char * data, uint32_t length)
{
  uint8_t *tmppage;
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  /*
  if(length%FLASH_PAGE_SIZE)
  {
    tmppage = calloc(1,FLASH_PAGE_SIZE);
    memcpy(tmppage,data, length);
    flash_write_pages(address, tmppage, FLASH_PAGE_SIZE);
    free(tmppage);
  }
  else
  {
    flash_write_pages(address, data, length);
  }
  */
  flash_write_pages(address, data, length);
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);

  return length;
}


/*---------------------------------------------------------------------------*/
int flash_compread(uint32_t address, void * data, uint32_t length)
{
  uint32_t i = 0;

  for(i=0; i<length; i++)
  {
    ((uint8_t *)data)[i] = ~(*(uint8_t *)(address+i));
  }

  return length;
}

/*---------------------------------------------------------------------------*/
// TODO: Provide an MSC API with complemented data at WRDATA register write time
int flash_compwrite(uint32_t address, const char * data, uint32_t length)
{
  msc_Return_TypeDef retval = mscReturnOk;
  uint32_t i = 0;
  uint32_t tmp_buf;

  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  // Disable interrupts.
  irq_disable();

  for(i=0; i<length; i=i+4)
  {
    memcpy(&tmp_buf, data+i, 4);
    tmp_buf = ~tmp_buf;
    retval = MSC_WriteWord((uint32_t *) (address+i), (const void*) &tmp_buf, 4);
  }

  irq_enable();
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);

  if(retval != mscReturnOk)
  {
    return retval;
  }
  else
  {
    return length;
  }
}

/** @} */
