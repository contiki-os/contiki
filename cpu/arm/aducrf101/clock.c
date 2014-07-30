/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Jim Paris <jim.paris@rigado.com>
 */

#include <aducrf101-contiki.h>
#include <sys/clock.h>
#include <sys/etimer.h>

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

#define SAMPLE_STACK_POINTER
#ifdef SAMPLE_STACK_POINTER
volatile uint32_t *__min_sampled_sp = (uint32_t *)0xFFFFFFFF;
#endif

void
SysTick_Handler(void)
{
#ifdef SAMPLE_STACK_POINTER
  /* Take note of the lowest stack pointer we ever saw.
     When compiling against newlib, the total free bytes of
     RAM not ever used by heap or stack can be found via GDB:
       (gdb) p (char *)__min_sampled_sp - (char *)_sbrk(0)
  */
  uint32_t *sp = (uint32_t *)&sp;
  if (sp < __min_sampled_sp)
    __min_sampled_sp = sp;
#endif

  current_clock++;
  if(etimer_pending()) {
    etimer_request_poll();
  }
  if(--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
}
/*---------------------------------------------------------------------------*/
void
clock_init()
{
  SysTick_Config(F_CPU / CLOCK_SECOND);
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return current_clock;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return current_seconds;
}
/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t usec)
{
  /* Delay by watching the SysTick value change. */
  int32_t remaining = (int32_t)usec * F_CPU / 1000000;
  int32_t old = SysTick->VAL;
  while(remaining > 0) {
    int32_t new = SysTick->VAL;
    if(new > old) { /* wraparound */
      old += SysTick->LOAD;
    }
    remaining -= (old - new);
    old = new;
  }
}
