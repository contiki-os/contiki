/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-wdt cc2538 watchdog timer driver
 *
 * Driver for the cc2538 Watchdog Timer
 * @{
 *
 * \file
 * Implementation of the cc2538 watchdog driver. The peripheral runs in
 * watchdog mode.
 */
#include "contiki.h"
#include "reg.h"
#include "cpu.h"
#include "dev/smwdthrosc.h"
/*---------------------------------------------------------------------------*/
/* Enabled by default */
#ifndef WATCHDOG_CONF_ENABLE
#define WATCHDOG_CONF_ENABLE 1
#endif

#if WATCHDOG_CONF_ENABLE
#define WATCHDOG_ENABLE SMWDTHROSC_WDCTL_EN
#else
#define WATCHDOG_ENABLE 0
#endif
/*---------------------------------------------------------------------------*/
/** \brief Initialisation function for the WDT. Currently simply explicitly
 * sets the WDT interval to max interval */
void
watchdog_init(void)
{
  /* Max interval, don't enable yet */
  REG(SMWDTHROSC_WDCTL) = 0;
}
/*---------------------------------------------------------------------------*/
/** \brief Starts the WDT in watchdog mode if enabled by user configuration,
 * maximum interval */
void
watchdog_start(void)
{
  /* Max interval (32768), watchdog mode, enable if configured to do so */
  REG(SMWDTHROSC_WDCTL) = WATCHDOG_ENABLE;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Writes the WDT clear sequence.
 *
 * Due to how the SMWDTHROSC_WDCTL works, it is OK to simply write these bits
 * rather than use RMW operations.
 */
void
watchdog_periodic(void)
{
  REG(SMWDTHROSC_WDCTL) = (SMWDTHROSC_WDCTL_CLR_3 | SMWDTHROSC_WDCTL_CLR_1);
  REG(SMWDTHROSC_WDCTL) = (SMWDTHROSC_WDCTL_CLR_2 | SMWDTHROSC_WDCTL_CLR_0);
}
/*---------------------------------------------------------------------------*/
/** \brief Keeps control until the WDT throws a reset signal. Starts the WDT
 * if not already started. */
void
watchdog_reboot(void)
{
  INTERRUPTS_DISABLE();

  /*
   * If the WDT is not started, set minimum interval and start
   * If the WDT is started, this will have no effect
   */
  REG(SMWDTHROSC_WDCTL) = SMWDTHROSC_WDCTL_INT | SMWDTHROSC_WDCTL_EN;

  while(1);
}
/**
 * @}
 * @}
 */
