/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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

#include "sys/clock.h"
#include "sys/etimer.h"

#include "contiki-conf.h"
#include "drivers/legacy_pc/rtc.h"

#if CLOCK_CONF_SECOND == 2
#define FREQ   RTC_2_HZ
#elif CLOCK_CONF_SECOND == 4
#define FREQ   RTC_4_HZ
#elif CLOCK_CONF_SECOND == 8
#define FREQ   RTC_8_HZ
#elif CLOCK_CONF_SECOND == 16
#define FREQ   RTC_16_HZ
#elif CLOCK_CONF_SECOND == 32
#define FREQ   RTC_32_HZ
#elif CLOCK_CONF_SECOND == 64
#define FREQ   RTC_64_HZ
#elif CLOCK_CONF_SECOND == 128
#define FREQ   RTC_128_HZ
#elif CLOCK_CONF_SECOND == 256
#define FREQ   RTC_256_HZ
#elif CLOCK_CONF_SECOND == 512
#define FREQ   RTC_512_HZ
#elif CLOCK_CONF_SECOND == 1024
#define FREQ   RTC_1024_HZ
#elif CLOCK_CONF_SECOND == 2048
#define FREQ   RTC_2048_HZ
#elif CLOCK_CONF_SECOND == 4096
#define FREQ   RTC_4096_HZ
#elif CLOCK_CONF_SECOND == 8192
#define FREQ   RTC_8192_HZ
#else
#define FREQ   -1
#error "RTC is being used thus CLOCK_CONF_SECOND has to be a value defined by rtc_frequency_t."
#endif

static volatile clock_time_t tick_count = 0;

static void
update_ticks(void)
{
  clock_time_t expire = etimer_next_expiration_time();

  tick_count++;

  /* Notify etimer library if the next event timer has expired */
  if(expire != 0 && tick_count >= expire) {
    etimer_request_poll();
  }
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  rtc_init(FREQ, update_ticks);
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return tick_count;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return tick_count / CLOCK_CONF_SECOND;
}
/*---------------------------------------------------------------------------*/
void
clock_wait(clock_time_t t)
{
  clock_time_t initial = tick_count;

  while(tick_count < t + initial);
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  /* Stubbed function */
}
/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t t)
{
  /* Stubbed function */
}
