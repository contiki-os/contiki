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
 *         Implementation of K60 RTC driver.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include "rtc.h"
#include "K60.h"
#include "config-clocks.h"
#include "config-board.h"

void
rtc_init(void)
{
  /* System clock initialization, early boot */

  /* Enable clock gate for RTC module */
  /* side note: It is ironic that we need to enable the clock gate for a clock module */
  BITBAND_REG(SIM->SCGC6, SIM_SCGC6_RTC_SHIFT) = 1;

  /* Reset the RTC status */
  RTC->SR = 0;

  /* Enable RTC clock, enable load capacitance as configured by config-board.h */
  RTC->CR |= RTC_CR_OSCE_MASK | BOARD_RTC_LOAD_CAP_BITS;
}

void
rtc_start(void) {
  /* Enable RTC seconds counter. The RTC module has been initialized beforehand. */
  BITBAND_REG(RTC->SR, RTC_SR_TCE_SHIFT) = 1;
}

void
rtc_stop(void) {
  /* Disable RTC seconds counter. */
  BITBAND_REG(RTC->SR, RTC_SR_TCE_SHIFT) = 0;
}

void
rtc_time_set(uint32_t seconds) {
  /* Set the time */
  RTC->TSR = seconds;
}

uint32_t
rtc_time_get(void) {
  return RTC->TSR;
}

void
rtc_alarm_set(uint32_t alarm_time) {
  RTC->TAR = alarm_time;
}
