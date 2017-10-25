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
 * \addtogroup cc2538-rtimer
 * @{
 *
 * \file
 * Implementation of the arch-specific rtimer functions for the cc2538
 *
 */
#include "contiki.h"
#include "sys/energest.h"
#include "sys/rtimer.h"
#include "dev/nvic.h"
#include "dev/smwdthrosc.h"
#include "cpu.h"
#include "lpm.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
static volatile rtimer_clock_t next_trigger;
/*---------------------------------------------------------------------------*/
/**
 * \brief We don't need to explicitly initialise anything but this
 *        routine is required by the API.
 *
 *        The Sleep Timer starts ticking automatically as soon as the device
 *        turns on. We don't need to turn on interrupts before the first call
 *        to rtimer_arch_schedule()
 */
void
rtimer_arch_init(void)
{
  return;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Schedules an rtimer task to be triggered at time t
 * \param t The time when the task will need executed. This is an absolute
 *          time, in other words the task will be executed AT time \e t,
 *          not IN \e t ticks
 */
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  rtimer_clock_t now;

  /* STLOAD must be 1 */
  while((REG(SMWDTHROSC_STLOAD) & SMWDTHROSC_STLOAD_STLOAD) != 1);

  INTERRUPTS_DISABLE();

  now = RTIMER_NOW();

  /*
   * New value must be 5 ticks in the future. The ST may tick once while we're
   * writing the registers. We play it safe here and we add a bit of leeway
   */
  if((int32_t)(t - now) < 7) {
    t = now + 7;
  }

  /* ST0 latches ST[1:3] and must be written last */
  REG(SMWDTHROSC_ST3) = (t >> 24) & 0x000000FF;
  REG(SMWDTHROSC_ST2) = (t >> 16) & 0x000000FF;
  REG(SMWDTHROSC_ST1) = (t >> 8) & 0x000000FF;
  REG(SMWDTHROSC_ST0) = t & 0x000000FF;

  INTERRUPTS_ENABLE();

  /* Store the value. The LPM module will query us for it */
  next_trigger = t;

  NVIC_EnableIRQ(SMT_IRQn);
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_next_trigger()
{
  return next_trigger;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns the current real-time clock time
 * \return The current rtimer time in ticks
 */
rtimer_clock_t
rtimer_arch_now()
{
  rtimer_clock_t rv;

  /* SMWDTHROSC_ST0 latches ST[1:3] and must be read first */
  rv = REG(SMWDTHROSC_ST0);
  rv |= (REG(SMWDTHROSC_ST1) << 8);
  rv |= (REG(SMWDTHROSC_ST2) << 16);
  rv |= (REG(SMWDTHROSC_ST3) << 24);

  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief The rtimer ISR
 *
 *        Interrupts are only turned on when we have an rtimer task to schedule
 *        Once the interrupt fires, the task is called and then interrupts no
 *        longer get acknowledged until the next task needs scheduled.
 */
void
rtimer_isr()
{
  /*
   * If we were in PM1+, call the wake-up sequence first. This will make sure
   * that the 32MHz OSC is selected as the clock source. We need to do this
   * before calling the next rtimer_task, since the task may need the RF.
   */
  lpm_exit();

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  next_trigger = 0;

  NVIC_ClearPendingIRQ(SMT_IRQn);
  NVIC_DisableIRQ(SMT_IRQn);

  rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/** @} */
