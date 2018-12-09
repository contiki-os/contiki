/*
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
 *
 * This file ports cc2538/crypto.h API for cc26xx
 *
 * Author: Alexander Lityagin <alexraynepe196@gmail.com>
 */

/**
 * \addtogroup cc26xx-crypto
 * @{
 *
 * \file
 * Implementation of the cc26xx AES cryptoprocessor driver
 */
#include <stdbool.h>

#include "contiki.h"
#include "sys/energest.h"

#include "ti-lib.h"
#include "prcm.h"
#include "lpm.h"
#include "driverlib/crypto.h"

#include "dev/crypto.h"
#include "dev/aes.h"

/*---------------------------------------------------------------------------*/
volatile
struct process *on_crypto_notify_process = NULL;

#define CRYPTO_IRQ_FLAGS    (CRYPTO_RESULT_RDY|CRYPTO_DMA_IN_DONE)

//*****************************************************************************
// Current AES operation initialized to None
//*****************************************************************************
extern
volatile uint32_t g_ui32CurrentAesOp;

/*---------------------------------------------------------------------------*/
/** \brief The AES/SHA cryptoprocessor ISR
 *
 *        This is the interrupt service routine for the AES/SHA
 *        cryptoprocessor.
 *
 *        This ISR is called at worst from PM0, so lpm_exit() does not need
 *        to be called.
 */
void CryptoIntHandler(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  CRYPTOIntClear(CRYPTO_IRQ_FLAGS);

  if(on_crypto_notify_process != NULL) {
    process_poll((struct process *)on_crypto_notify_process);
    on_crypto_notify_process = NULL;
  }
  else
      process_post(PROCESS_BROADCAST, PROCESS_EVENT_CRYPTO
                   , (void*)(uintptr_t) CRYPTOIntStatus(true));

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
crypto_register_process_notification(struct process *p)
{
    on_crypto_notify_process = p;
}

/*---------------------------------------------------------------------------*/
static
uint8_t lpm_request(void)
{
    if(!crypto_is_enabled() || (g_ui32CurrentAesOp == CRYPTO_AES_NONE)) {
        return LPM_MODE_MAX_SUPPORTED;
    }

    //* have some data buffered, wait until crypto completes
    return LPM_MODE_SLEEP;
}

static
void lpm_drop_handler(uint8_t mode)
{
    /*
     * if this is not a shutdown notification and we have an
     * input handler, we do nothing
     */
    if((mode != LPM_MODE_SHUTDOWN)) {
      return;
    }
  /*
   * if some crypto operation pended
   */
  if(g_ui32CurrentAesOp != CRYPTO_AES_NONE) {
      //TODO - should we some handle before shutdown crypto core?
      ;
  }

  /*
   * If we reach here, we either don't care about staying awake or we have
   * received a shutdown notification
   */
  if(crypto_is_enabled() == true) {
    /* Disable the module */
    crypto_disable();
  }

  /*
   * Always stop the clock in run mode. Also stop in Sleep and Deep Sleep if
   * this is a request for full shutdown
   */
  ti_lib_prcm_peripheral_run_disable(PRCM_PERIPH_CRYPTO);
  if(mode == LPM_MODE_SHUTDOWN) {
    ti_lib_prcm_peripheral_sleep_disable(PRCM_PERIPH_CRYPTO);
  }
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());
}
/*---------------------------------------------------------------------------*/
/* Declare a data structure to register with LPM. */
LPM_MODULE(crypto_module, lpm_request, lpm_drop_handler, NULL, LPM_DOMAIN_NONE);

/*---------------------------------------------------------------------------*/
bool crypto_is_active_dma(void){
    uint32_t stat = HWREG(CRYPTO_BASE + CRYPTO_O_DMASTAT);
    stat &= (CRYPTO_DMASTAT_CH0_ACTIVE | CRYPTO_DMASTAT_CH1_ACTIVE);
    return  (stat != 0);
}
/*---------------------------------------------------------------------------*/
void
crypto_init(void)
{
  lpm_register_module(&crypto_module);

  // always stop in Deep Sleep
  ti_lib_prcm_peripheral_deep_sleep_disable(PRCM_PERIPH_CRYPTO);

  {
      crypto_enable();
      //сброшу модуль на всякий
      HWREG(CRYPTO_BASE + CRYPTO_O_DMASWRESET) = CRYPTO_DMASWRESET_RESET;
      while (crypto_is_active_dma());
      HWREG(CRYPTO_BASE + CRYPTO_O_SWRESET)    = CRYPTO_SWRESET_RESET;
      while ((HWREG(CRYPTO_BASE + CRYPTO_O_SWRESET) & CRYPTO_SWRESET_RESET) != 0);
  }
  crypto_disable();
}
/*---------------------------------------------------------------------------*/
void
crypto_enable(void)
{
    /* Enable clock in active mode */
    ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_CRYPTO);
    ti_lib_prcm_peripheral_sleep_enable(PRCM_PERIPH_CRYPTO);
    ti_lib_prcm_load_set();
    while(!ti_lib_prcm_load_get());
    if (on_crypto_notify_process != NULL)
        CRYPTOIntEnable(CRYPTO_RESULT_RDY);
}
/*---------------------------------------------------------------------------*/
void
crypto_disable(void)
{
    CRYPTOIntDisable(CRYPTO_IRQ_FLAGS);
  /* Gate the clock for the AES/SHA cryptoprocessor */
    ti_lib_prcm_peripheral_run_disable(PRCM_PERIPH_CRYPTO);
    ti_lib_prcm_peripheral_sleep_disable(PRCM_PERIPH_CRYPTO);
    ti_lib_prcm_load_set();
    while(!ti_lib_prcm_load_get());
}
/*---------------------------------------------------------------------------*/
bool crypto_is_enabled(void){
    uint32_t crypto_stat = HWREG(PRCM_BASE + PRCM_O_SECDMACLKGR);
    crypto_stat &= PRCM_SECDMACLKGS_CRYPTO_CLK_EN;
    return (crypto_stat != 0);
}

void crypto_cleanup()
{
    unsigned clr = CRYPTO_IRQCLR_DMA_BUS_ERR
                | CRYPTO_IRQCLR_KEY_ST_WR_ERR
                | CRYPTO_IRQCLR_KEY_ST_RD_ERR
                ;
    HWREG(CRYPTO_BASE + CRYPTO_O_IRQCLR) = clr;
}


/** @} */
