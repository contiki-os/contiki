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
 * \defgroup cc2538-clock cc2538 Clock
 *
 * Implementation of the clock module for the cc2538
 *
 * To implement the clock functionality, we use the SysTick peripheral on the
 * cortex-M3. We run the system clock at 16 MHz and we set the SysTick to give
 * us 128 interrupts / sec
 * @{
 *
 * \file
 * Clock driver implementation for the TI cc2538
 */
#include "contiki.h"
#include "systick.h"
#include "reg.h"
#include "cpu.h"
#include "dev/gptimer.h"
#include "dev/sys-ctrl.h"

#include "sys/energest.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define RELOAD_VALUE (125000 - 1)  /** Fire 128 times / sec */

static volatile clock_time_t count;
static volatile unsigned long secs = 0;
static volatile uint8_t second_countdown = CLOCK_SECOND;
/*---------------------------------------------------------------------------*/
/**
 * \brief Arch-specific implementation of clock_init for the cc2538
 *
 * We initialise the SysTick to fire 128 interrupts per second, giving us a
 * value of 128 for CLOCK_SECOND
 *
 * We also initialise GPT0:Timer A, which is used by clock_delay_usec().
 * We use 16-bit range (individual), count-down, one-shot, no interrupts.
 * The system clock is at 16MHz giving us 62.5 nano sec ticks for Timer A.
 * Prescaled by 16 gives us a very convenient 1 tick per usec
 */
void
clock_init(void)
{
  count = 0;

  REG(SYSTICK_STRELOAD) = RELOAD_VALUE;

  /* System clock source, Enable */
  REG(SYSTICK_STCTRL) |= SYSTICK_STCTRL_CLK_SRC | SYSTICK_STCTRL_ENABLE;

  /* Enable the SysTick Interrupt */
  REG(SYSTICK_STCTRL) |= SYSTICK_STCTRL_INTEN;

  /*
   * Remove the clock gate to enable GPT0 and then initialise it
   * We only use GPT0 for clock_delay_usec. We initialise it here so we can
   * have it ready when it's needed
   */
  REG(SYS_CTRL_RCGCGPT) |= SYS_CTRL_RCGCGPT_GPT0;

  /* Make sure GPT0 is off */
  REG(GPT_0_BASE | GPTIMER_CTL) = 0;


  /* 16-bit */
  REG(GPT_0_BASE | GPTIMER_CFG) = 0x04;

  /* One-Shot, Count Down, No Interrupts */
  REG(GPT_0_BASE | GPTIMER_TAMR) = GPTIMER_TAMR_TAMR_ONE_SHOT;

  /* Prescale by 16 (thus, value 15 in TAPR) */
  REG(GPT_0_BASE | GPTIMER_TAPR) = 0x0F;
}
/*---------------------------------------------------------------------------*/
CCIF clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  secs = sec;
}
/*---------------------------------------------------------------------------*/
CCIF unsigned long
clock_seconds(void)
{
  return secs;
}
/*---------------------------------------------------------------------------*/
void
clock_wait(clock_time_t i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Arch-specific implementation of clock_delay_usec for the cc2538
 * \param len Delay \e len uSecs
 *
 * See clock_init() for GPT0 Timer A's configuration
 */
void
clock_delay_usec(uint16_t len)
{
  REG(GPT_0_BASE | GPTIMER_TAILR) = len;
  REG(GPT_0_BASE | GPTIMER_CTL) |= GPTIMER_CTL_TAEN;

  /* One-Shot mode: TAEN will be cleared when the timer reaches 0 */
  while(REG(GPT_0_BASE | GPTIMER_CTL) & GPTIMER_CTL_TAEN);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Obsolete delay function but we implement it here since some code
 * still uses it
 */
void
clock_delay(unsigned int i)
{
  clock_delay_usec(i);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Adjust the clock by moving it forward by a number of ticks
 * \param ticks The number of ticks
 *
 * This function is useful when coming out of PM1/2, during which the system
 * clock is stopped. We adjust the clock by moving it forward by a number of
 * ticks equal to the deep sleep duration. We update the seconds counter if
 * we have to and we also do some housekeeping so that the next second will
 * increment when it is meant to.
 *
 * \note This function is only meant to be used by lpm_exit(). Applications
 * should really avoid calling this
 */
void
clock_adjust(clock_time_t ticks)
{
  /* Halt the SysTick while adjusting */
  REG(SYSTICK_STCTRL) &= ~SYSTICK_STCTRL_ENABLE;

  /* Moving forward by more than a second? */
  secs += ticks >> 7;

  /* Increment tick count */
  count += ticks;

  /*
   * Update internal second countdown so that next second change will actually
   * happen when it's meant to happen.
   */
  second_countdown -= ticks;

  if(second_countdown == 0 || second_countdown > 128) {
    secs++;
    second_countdown -= 128;
  }

  /* Re-Start the SysTick */
  REG(SYSTICK_STCTRL) |= SYSTICK_STCTRL_ENABLE;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief The clock Interrupt Service Routine. It polls the etimer process
 * if an etimer has expired. It also updates the software clock tick and
 * seconds counter since reset.
 */
void
clock_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  count++;
  if(etimer_pending()) {
    etimer_request_poll();
  }

  if(--second_countdown == 0) {
    secs++;
    second_countdown = CLOCK_SECOND;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
