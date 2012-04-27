/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 * \file
 *         Hardware-dependent functions for the cc253x watchdog.
 *
 *         This file contains an ISR and must reside in the HOME bank.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "cc253x.h"
#include "sfr-bits.h"
#include "contiki-conf.h"

#define WDT_TIMEOUT_MIN    (WDCTL_INT1 | WDCTL_INT0)
/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
  WDCTL = 0; /* IDLE, Max Interval */
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  WDCTL |= WDCTL_MODE1; /* Start in Watchdog mode */
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
  /* Write the 'clear' sequence while maintaining mode and interval setting */
  WDCTL = (WDCTL & 0x0F) | WDCTL_CLR3 | WDCTL_CLR1;
  WDCTL = (WDCTL & 0x0F) | WDCTL_CLR2 | WDCTL_CLR0;
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
  WDCTL = WDT_TIMEOUT_MIN;
  /* Dis-acknowledge all interrupts while we wait for the dog to bark */
  DISABLE_INTERRUPTS();

  WDCTL |= WDCTL_MODE1; /* Just in case it's not started... */

  /* NOP till the dog barks... */
  while(1) {
    ASM(nop);
  }
}
