/*
 * Copyright (c) 2016, University of Bristol - http://www.bristol.ac.uk
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
 * \addtogroup cc26xx-trng
 * @{
 *
 * \file
 *
 * Implementation of the CC13xx/CC26xx RNG driver
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lpm.h"
#include "sys/process.h"
#include "dev/soc-trng.h"
#include "ti-lib.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
/*---------------------------------------------------------------------------*/
#ifdef SOC_TRNG_CONF_CACHE_LEN
#define SOC_TRNG_CACHE_LEN SOC_TRNG_CONF_CACHE_LEN
#else
/** Size of the random number cache. Each slot holds 4 16-bit numbers */
#define SOC_TRNG_CACHE_LEN 4
#endif
/*---------------------------------------------------------------------------*/
#define MIN_REFILL_CYCLES_MAX 0x00000000
/*---------------------------------------------------------------------------*/
PROCESS(soc_trng_process, "CC13xx/CC26xx TRNG process");
/*---------------------------------------------------------------------------*/
static process_event_t rng_ready_event = PROCESS_EVENT_NONE;
/*---------------------------------------------------------------------------*/
static soc_trng_callback_t notify_cb = NULL;
/*---------------------------------------------------------------------------*/
#define soc_trng_isr TRNGIntHandler
/*---------------------------------------------------------------------------*/
static uint64_t rands_cache[SOC_TRNG_CACHE_LEN];
static bool rands_mask[SOC_TRNG_CACHE_LEN];
/*---------------------------------------------------------------------------*/
static void
disable_number_ready_interrupt(void)
{
  ti_lib_trng_int_disable(TRNG_NUMBER_READY);
  ti_lib_rom_int_disable(INT_TRNG_IRQ);
}
/*---------------------------------------------------------------------------*/
static void
enable_number_ready_interrupt(void)
{
  ti_lib_trng_int_clear(TRNG_NUMBER_READY);
  ti_lib_trng_int_enable(TRNG_NUMBER_READY);
  ti_lib_rom_int_enable(INT_TRNG_IRQ);
}
/*---------------------------------------------------------------------------*/
static bool
accessible(void)
{
  /* First, check the PD */
  if(ti_lib_rom_prcm_power_domain_status(PRCM_DOMAIN_PERIPH)
     != PRCM_DOMAIN_POWER_ON) {
    return false;
  }

  /* Then check the 'run mode' clock gate */
  if(!(HWREG(PRCM_BASE + PRCM_O_SECDMACLKGR) &
       PRCM_SECDMACLKGDS_TRNG_CLK_EN_M)) {
    return false;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
static void
power_up(void)
{
  /* First, make sure the PERIPH PD is on */
  ti_lib_rom_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);
  while((ti_lib_rom_prcm_power_domain_status(PRCM_DOMAIN_PERIPH)
         != PRCM_DOMAIN_POWER_ON));

  /* Enable clock in active mode */
  ti_lib_rom_prcm_peripheral_run_enable(PRCM_PERIPH_TRNG);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());
}
/*---------------------------------------------------------------------------*/
static void
reset_synchronous(void)
{
  ti_lib_trng_reset();
  while(HWREG(TRNG_BASE + TRNG_O_SWRESET));
}
/*---------------------------------------------------------------------------*/
static uint8_t
request(void)
{
  if(notify_cb) {
    return LPM_MODE_SLEEP;
  }
  return LPM_MODE_MAX_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
LPM_MODULE(rng_module, request, NULL, NULL, LPM_DOMAIN_NONE);
/*---------------------------------------------------------------------------*/
static uint64_t
read_number(void)
{
  uint64_t ran = (uint64_t)HWREG(TRNG_BASE + TRNG_O_OUT1) << 32;
  ran += ti_lib_rom_trng_number_get(TRNG_LOW_WORD);

  return ran;
}
/*---------------------------------------------------------------------------*/
uint64_t
soc_trng_rand_synchronous()
{
  uint64_t ran;
  bool interrupts_disabled;
  int i;

  /* If the TRNG is gathering entropy, return a cached value */
  if(notify_cb) {
    for(i = 0; i < SOC_TRNG_CACHE_LEN; i++) {
      if(rands_mask[i]) {
        rands_mask[i] = false;
        return rands_cache[i];
      }
    }
    return 0;
  }

  if(!accessible()) {
    power_up();
  }

  /*
   * If we were previously enabled, we either have a number already available,
   * or we need clock, which means we are calculating. If neither is true then
   * we need setup from scratch.
   */
  if((ti_lib_trng_status_get() & (TRNG_NEED_CLOCK | TRNG_NUMBER_READY)) == 0) {
    reset_synchronous();
    ti_lib_trng_configure(MIN_REFILL_CYCLES_MAX, SOC_TRNG_REFILL_CYCLES_MIN, 0);
    ti_lib_trng_enable();
  }

  interrupts_disabled = ti_lib_int_master_disable();

  while((ti_lib_trng_status_get() & TRNG_NUMBER_READY) == 0);

  ran = read_number();

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  return ran;
}
/*---------------------------------------------------------------------------*/
uint8_t
soc_trng_rand_asynchronous(uint32_t samples, soc_trng_callback_t cb)
{
  int i;
  bool interrupts_disabled;

  if(notify_cb != NULL) {
    return SOC_TRNG_RAND_ASYNC_REQUEST_ERROR;
  }

  if(!accessible()) {
    power_up();
  }

  /*
   * First we need to cache some random numbers for general use in case the
   * application requests them while we are calculating.
   *
   * If we were previously enabled, we either have a number already available,
   * or we need clock, which means we are calculating. If neither is true then
   * we need setup from scratch.
   */
  if((ti_lib_trng_status_get() & (TRNG_NEED_CLOCK | TRNG_NUMBER_READY)) == 0) {
    reset_synchronous();
  }

  interrupts_disabled = ti_lib_int_master_disable();

  ti_lib_trng_disable();
  ti_lib_trng_configure(MIN_REFILL_CYCLES_MAX, SOC_TRNG_REFILL_CYCLES_MIN, 0);
  ti_lib_trng_enable();

  /* Cache SOC_TRNG_CACHE_LEN min-entropy rands */
  for(i = 0; i < SOC_TRNG_CACHE_LEN; i++) {
    while((ti_lib_trng_status_get() & TRNG_NUMBER_READY) == 0);
    rands_mask[i] = true;
    rands_cache[i] = read_number();
  }

  /* Configure the RNG to the required entropy */
  ti_lib_trng_disable();
  ti_lib_trng_configure(MIN_REFILL_CYCLES_MAX, samples, 0);

  /*
   * Clear the TRNG_NUMBER_READY flag. This will trigger a new calculation
   * as soon as the module gets enabled.
   */
  ti_lib_trng_int_clear(TRNG_NUMBER_READY);

  /* Enable clock in sleep mode and register with LPM */
  ti_lib_rom_prcm_peripheral_sleep_enable(PRCM_PERIPH_TRNG);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  lpm_register_module(&rng_module);

  notify_cb = cb;

  /* Enable the number ready interrupt and fire-up the module */
  enable_number_ready_interrupt();
  ti_lib_trng_enable();

  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }

  return SOC_TRNG_RAND_ASYNC_REQUEST_OK;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(soc_trng_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == rng_ready_event);

    if(notify_cb) {
      uint64_t ran = read_number();

      notify_cb(ran);
      notify_cb = NULL;
    }

    /* Disable clock in sleep mode */
    ti_lib_rom_prcm_peripheral_sleep_disable(PRCM_PERIPH_TRNG);
    ti_lib_prcm_load_set();
    while(!ti_lib_prcm_load_get());

    lpm_unregister_module(&rng_module);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
soc_trng_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  ti_lib_trng_disable();

  disable_number_ready_interrupt();

  ti_lib_trng_configure(MIN_REFILL_CYCLES_MAX, SOC_TRNG_REFILL_CYCLES_MIN, 0);
  ti_lib_trng_enable();

  process_post(&soc_trng_process, rng_ready_event, NULL);

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
soc_trng_init()
{
  if(rng_ready_event != PROCESS_EVENT_NONE) {
    return;
  }

  /* Register the RNG ready event */
  rng_ready_event = process_alloc_event();
  process_start(&soc_trng_process, NULL);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
