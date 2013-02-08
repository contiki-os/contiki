/*
 * Contiki PIC32 Port project
 * 
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup pic32 PIC32 Contiki Port
 *
 * @{
 */

/**
 * \file   rtimer-arch.c
 * \brief  PIC32MX RTIMER routines
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-04-11
 */

/*
 * PIC32MX795F512L - Specific Functions
 * 
 * All the functions in this part of the file are specific for the
 * pic32mx795f512l that is characterized by registers' name that differ from
 * the 3xx and 4xx families of the pic32mx.
 */

#include <pic32_timer.h>

#include "contiki.h"

#include <sys/rtimer.h>
#include <sys/clock.h>

#include <stdio.h>

#include <p32xxxx.h>

static uint32_t schedule;

/*---------------------------------------------------------------------------*/
void
rtimer_callback(void)
{
  pic32_timer23_disable_irq();
  TMR2 = schedule;

  rtimer_run_next();
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  pic32_timer23_disable_irq();
  IPC3CLR = _IPC3_T3IP_MASK | _IPC3_T3IS_MASK;
  IPC3SET = (7 << _IPC3_T3IP_POSITION) | (3 << _IPC3_T3IS_POSITION);
  T2CON = 0;
  T3CON = 0;     
  T2CONSET = _T2CON_T32_MASK | (TIMER_B_PRESCALE_256 << _T2CON_TCKPS_POSITION);
  PR2 =  0xFFFFFFFF;
  TMR2 = 0;
  pic32_timer23_start();
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now(void)
{
  return TMR2;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  schedule = t;
  PR2 =  t;
  pic32_timer23_enable_irq();
}
/*---------------------------------------------------------------------------*/

TIMER_INTERRUPT(3, rtimer_callback);

/** @} */
