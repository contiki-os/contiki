/*
 * Copyright (c) 2012, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "platform-conf.h"
#include "contiki-conf.h"
#include "dev/leds.h"
#include "stm32l1xx.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_hal_cortex.h"
#include "stm32l1xx_hal.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
#define RELOAD_VALUE        ((F_CPU / CLOCK_CONF_SECOND) - 1)
/*---------------------------------------------------------------------------*/
static volatile unsigned long seconds = 0;
static volatile clock_time_t ticks;
void st_lib_sys_tick_handler(void);
/*---------------------------------------------------------------------------*/
void
st_lib_sys_tick_handler(void)
{
  ticks++;
  if((ticks % CLOCK_SECOND) == 0) {
    seconds++;
    energest_flush();
  }
  HAL_IncTick();

  if(etimer_pending()) {
    etimer_request_poll();
  }
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  ticks = 0;
  st_lib_hal_systick_clk_source_config(SYSTICK_CLKSOURCE_HCLK);
  st_lib_hal_systick_config(RELOAD_VALUE);
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return seconds;
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  seconds = sec;
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return ticks;
}
/*---------------------------------------------------------------------------*/
void
clock_delay(unsigned int i)
{
  for(; i > 0; i--) {
    unsigned int j;
    for(j = 50; j > 0; j--) {
      asm ("nop");
    }
  }
}
/*---------------------------------------------------------------------------*/
/* Wait for a multiple of clock ticks (7.8 ms at 128 Hz). */
void
clock_wait(clock_time_t t)
{
  clock_time_t start;
  start = clock_time();
  while(clock_time() - start < (clock_time_t)t) ;
}
/*---------------------------------------------------------------------------*/
