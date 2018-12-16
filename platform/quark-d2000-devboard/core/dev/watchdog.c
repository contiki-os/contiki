/*
 * Copyright (C) 2016, Intel Corporation. All rights reserved.
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

#include "qm_wdt.h"

void
watchdog_init(void)
{
  qm_wdt_config_t cfg;

  /* Assuming the system clock is set to 32 MHz, we should configure
   * watchdog with QM_WDT_2_28_CYCLES in order to get ~ 8 seconds
   * timeout.
   */
  cfg.timeout = QM_WDT_2_POW_28_CYCLES;
  cfg.mode = QM_WDT_MODE_RESET;
  cfg.callback = NULL;

  qm_wdt_set_config(QM_WDT_0, &cfg);
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  qm_wdt_start(QM_WDT_0);
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
  qm_wdt_reload(QM_WDT_0);
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
  /* XXX: QMSI driver has no API to stop the watchdog. If we really need to
   * stop the watchdog in future, we can implement a workaround which disables
   * the watchdog clock gate. This will make the watchdog's internal counter
   * stop incrementing.
   */
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
  /* Stubbed function */
}
