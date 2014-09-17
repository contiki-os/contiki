/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Power mode switching functions for the K60 CPU.
 *
 *         This file is based on sample code from Freescale.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "power-modes.h"
#include "config-board.h"
#include "K60.h"

#if K60_CPU_REV == 1
#define PMCTRL MC->PMCTRL
#define SET_PMCTRL(x) (PMCTRL = MC_PMCTRL_LPLLSM(x) | MC_PMCTRL_LPWUI_MASK)
#elif K60_CPU_REV == 2
#define PMCTRL SMC->PMCTRL
#define SET_PMCTRL(x) (PMCTRL = SMC_PMCTRL_STOPM(x) | SMC_PMCTRL_LPWUI_MASK)
#else
#error Unknown K60 CPU revision!
#endif

#define POWER_MODE_NORMAL (0b000)
#define POWER_MODE_VLPS   (0b010)
#define POWER_MODE_LLS    (0b011)

void
power_modes_init(void)
{
  /* Clear LLS protection */
  /* Clear VLPS, VLPW, VLPR protection */
  /* Note: This register can only be written once after each reset, so we must
   * enable all power modes that we wish to use. */
#if K60_CPU_REV == 1
  MC->PMPROT |= MC_PMPROT_ALLS_MASK | MC_PMPROT_AVLP_MASK;
#else /* K60_CPU_REV == 1 */
  SMC->PMPROT |= SMC_PMPROT_ALLS_MASK | SMC_PMPROT_AVLP_MASK;
#endif /* K60_CPU_REV == 1 */
}
static void
wait(void)
{
  /* Clear the SLEEPDEEP bit to make sure we go into WAIT (sleep) mode instead
   * of deep sleep.
   */
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

  /* WFI instruction will start entry into WAIT mode */
  asm("WFI");
}
static void
stop(void)
{
  /* Set the SLEEPDEEP bit to enable deep sleep mode (STOP) */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /* WFI instruction will start entry into STOP mode */
  asm("WFI");
}
void
power_mode_wait(void)
{
  wait();
}
void
power_mode_stop(void)
{
  uint8_t dummy;
  SET_PMCTRL(POWER_MODE_NORMAL);
  /* Wait for the write to complete before stopping core */
  dummy = PMCTRL;
  (void)dummy; /* Avoid warnings about set but not used variable [-Wunused-but-set-variable] */
  stop();
}
void
power_mode_vlps(void)
{
  uint8_t dummy;
  SET_PMCTRL(POWER_MODE_VLPS);
  /* Wait for the write to complete before stopping core */
  dummy = PMCTRL;
  (void)dummy; /* Avoid warnings about set but not used variable [-Wunused-but-set-variable] */
  stop();
}
void
power_mode_lls(void)
{
  uint8_t dummy;
  SET_PMCTRL(POWER_MODE_LLS);
  /* Wait for the write to complete before stopping core */
  dummy = PMCTRL;
  (void)dummy; /* Avoid warnings about set but not used variable [-Wunused-but-set-variable] */
  stop();
}
