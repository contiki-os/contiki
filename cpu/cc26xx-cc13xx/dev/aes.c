/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * All rights reserved.
  *
 * Port to cc26xx:
 * Copyright (c) 2017, Alexander Lityagin   alexraynepe196@gmail.com
 *                                          alexraynepe196@hotbox.ru
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
/**
 * \addtogroup cc26xx-aes
 * @{
 *
 * \file
 * Implementation of the cc26xx AES driver
 */
#include <stdbool.h>
#include <stdint.h>
#include "contiki.h"
#include "dev/aes.h"
#include "driverlib/crypto.h"

//*****************************************************************************
// Current AES operation initialized to None
//*****************************************************************************
extern
volatile uint32_t g_ui32CurrentAesOp;
/*---------------------------------------------------------------------------*/
AESResult
aes_load_keys(const void *keys, uint8_t key_size, uint8_t count,
              AESKeyArea start_area)
{
  int i;
  uint32_t* kb = (uint32_t*)keys;
  const unsigned key128_size = 128/32;
  AESResult res = CRYPTO_SUCCESS;

  /* Change count to the number of 128-bit key areas */
  ASSERT(key_size != AES_KEY_STORE_SIZE_KEY_SIZE_128);
  ASSERT(count <= AES_KEY_AREAS);

  crypto_cleanup();
  for(i = start_area; count > 0; i++, count--, kb += key128_size) {
      res = CRYPTOAesLoadKey(kb, i);
      if (res != AES_SUCCESS){
          return res;
      }
  }

  return CRYPTO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static
void aes_load_iv(const AESNonce *iv){
    if(iv != NULL) {
      /* Write initialization vector */
        HWREG(CRYPTO_BASE + CRYPTO_O_AESIV0) = iv->w[0];
        HWREG(CRYPTO_BASE + CRYPTO_O_AESIV1) = iv->w[1];
        HWREG(CRYPTO_BASE + CRYPTO_O_AESIV2) = iv->w[2];
        HWREG(CRYPTO_BASE + CRYPTO_O_AESIV3) = iv->w[3];
    }
}

// this routine imports from driverlib/crypto to provide flexibility with
//  use ciphed output
AESResult aes_auth_crypt_start(uint32_t ctrl
         , AESKeyArea key_area, const AESNonce *iv
         , const void *adata, AESSize adata_len
         , const void *data_in, void *data_out, AESSize data_len
         , struct process *notify_proc)
{

    // Disable global interrupt, enable local interrupt and clear any pending
    // interrupts.
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);

    //HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 0;
    //HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH1CTL, CRYPTO_DMACH0CTL_EN_BITN) = 0;
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = 0;
    HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1LEN) = 0;

    // Configure master control module for AES operation.
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = CRYPTO_ALGSEL_AES;

    crypto_cleanup();
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    // Enable internal interrupts.
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQTYPE) = CRYPTO_IRQTYPE_LEVEL;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_DMA_IN_DONE |
                                          CRYPTO_IRQEN_RESULT_AVAIL;

    // Enable keys to read (e.g. Key 0).
    HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) = key_area;

    // Wait until key is loaded to the AES module.
    do
    {
        CPUdelay(1);
    }
    while((HWREG(CRYPTO_BASE + CRYPTO_O_KEYREADAREA) & CRYPTO_KEYREADAREA_BUSY));

    // Check for Key store Read error.
    if((HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT)& CRYPTO_KEY_ST_RD_ERR))
    {
        return (AES_KEYSTORE_READ_ERROR);
    }

    aes_load_iv(iv);

    // Write the configuration for 128 bit AES-CCM.
    if ( (HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) & CRYPTO_AESCTL_SAVED_CONTEXT_RDY) == 0)
        ctrl &= ~CRYPTO_AESCTL_SAVED_CONTEXT_RDY;
    HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) = ctrl;

    // Write the length of the crypto block (plain text).
    // Low and high part (high part is assumed to be always 0).
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN0) = data_len;
    HWREG(CRYPTO_BASE + CRYPTO_O_AESDATALEN1)  =  0;

    //
    // Write the length of the header field.
    // Also called AAD - Additional Authentication Data.
    //
    HWREG(CRYPTO_BASE + CRYPTO_O_AESAUTHLEN) = adata_len;

    // Check if any header information (AAD).
    // If so configure the DMA controller to fetch the header.
    if(adata_len != 0)
    {
        // Enable DMA channel 0.
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

        // Register the base address of the header (AAD).
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) = (uint32_t)adata;

        // Header length in bytes (may be non-block size aligned).
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = adata_len;

        // Wait for completion of the header data transfer, DMA_IN_DONE.
        do
        {
            CPUdelay(1);
        }
        while(!(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & CRYPTO_IRQSTAT_DMA_IN_DONE));

        // Check for DMA errors.
        if(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & CRYPTO_DMA_BUS_ERR)
        {
            return AES_DMA_BUS_ERROR;
        }
    }

    // Clear interrupt status.
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    // Wait for interrupt lines from module to be cleared
    while(HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT) & (CRYPTO_IRQSTAT_DMA_IN_DONE | CRYPTO_IRQSTAT_RESULT_AVAIL));

    // Disable CRYPTO_IRQEN_DMA_IN_DONE interrupt as we only
    // want interrupt to trigger once RESULT_AVAIL occurs.
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) &= ~CRYPTO_IRQEN_DMA_IN_DONE;


    // Is using interrupts enable globally.
    if (notify_proc != NULL){
        crypto_register_process_notification(notify_proc);
        IntPendClear(INT_CRYPTO_RESULT_AVAIL_IRQ);
        IntEnable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    }

    // Enable interrupts locally.
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQEN) = CRYPTO_IRQEN_RESULT_AVAIL;

    // Perform encryption if requested.
    {
        // Enable DMA channel 0
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH0CTL, CRYPTO_DMACH0CTL_EN_BITN) = 1;

        // base address of the payload data in ext. memory.
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0EXTADDR) = (uint32_t)data_in;

        // Enable DMA channel 1
        HWREGBITW(CRYPTO_BASE + CRYPTO_O_DMACH1CTL, CRYPTO_DMACH1CTL_EN_BITN) = 1;

        // Base address of the output data buffer.
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1EXTADDR) = (uint32_t)data_out;

        // Payload data length in bytes, equal to the plaintext length.
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH0LEN) = data_len;
        HWREG(CRYPTO_BASE + CRYPTO_O_DMACH1LEN) = data_len;
    }

  return CRYPTO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
AESResult
aes_auth_crypt_check_status(void)
{
    uint32_t Status;

    // Get the current DMA status.
    Status = HWREG(CRYPTO_BASE + CRYPTO_O_DMASTAT);

    // Check if DMA is still busy.
    if(Status & CRYPTO_DMA_BSY)
    {
        return (CRYPTO_PENDING);
    }

    // Check the status of the DMA operation - return error if not success.
    if(Status & CRYPTO_DMA_BUS_ERROR)
    {
        g_ui32CurrentAesOp = CRYPTO_AES_NONE;
        return (CRYPTO_DMA_BUS_ERROR);
    }

    Status = HWREG(CRYPTO_BASE + CRYPTO_O_IRQSTAT);
    if ((Status & CRYPTO_IRQSTAT_RESULT_AVAIL) == 0)
        return (CRYPTO_PENDING);

    // Operation successful - disable interrupt and return success
    IntDisable(INT_CRYPTO_RESULT_AVAIL_IRQ);
    return (CRYPTO_SUCCESS);
}
/*---------------------------------------------------------------------------*/
static
bool is_context_ready(void){
    return ((HWREG(CRYPTO_BASE + CRYPTO_O_AESCTL) &  CRYPTO_AESCTL_SAVED_CONTEXT_RDY) != 0);
}

AESResult
aes_auth_crypt_get_result(AESNonce *iv, AESBlock* tag)
{
    // Read tag - wait for the context ready bit.
    do
    {
        CPUdelay(1);
    }
    while(!is_context_ready());

    // Operation successful -  clear interrupt status.
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = (CRYPTO_IRQCLR_DMA_IN_DONE |
                                            CRYPTO_IRQCLR_RESULT_AVAIL);

    // Result has already been copied to the output buffer by DMA
    // Disable master control.
    HWREG(CRYPTO_BASE + CRYPTO_O_ALGSEL) = 0x00000000;

    ASSERT(tag != NULL);
    // Read the Tag registers.
    tag->w[0] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT0);
    tag->w[1] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT1);
    tag->w[2] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT2);
    tag->w[3] = HWREG(CRYPTO_BASE + CRYPTO_O_AESTAGOUT3);

    if(iv != NULL)
    {
      /* Read initialization vector */
        iv->w[0] = HWREG(CRYPTO_BASE + CRYPTO_O_AESIV0);
        iv->w[1] = HWREG(CRYPTO_BASE + CRYPTO_O_AESIV1);
        iv->w[2] = HWREG(CRYPTO_BASE + CRYPTO_O_AESIV2);
        iv->w[3] = HWREG(CRYPTO_BASE + CRYPTO_O_AESIV3);
    }
    else {
        (void)HWREG(CRYPTO_BASE + CRYPTO_O_AESIV0);
        (void)HWREG(CRYPTO_BASE + CRYPTO_O_AESIV1);
        (void)HWREG(CRYPTO_BASE + CRYPTO_O_AESIV2);
        (void)HWREG(CRYPTO_BASE + CRYPTO_O_AESIV3);
    }

    g_ui32CurrentAesOp = CRYPTO_AES_NONE;
    return CRYPTO_SUCCESS;
}

/** @} */
