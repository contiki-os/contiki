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
#include "contiki.h"
#include "platform-conf.h"

#include "sys/rtimer.h"
#include "sys/process.h"
#include "dev/watchdog.h"

#include "stm32l1xx.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_hal_tim.h"
#include "stm32l1xx_hal_cortex.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
volatile uint32_t rtimer_clock = 0uL;
/*---------------------------------------------------------------------------*/
st_lib_tim_handle_typedef htim2;
/*---------------------------------------------------------------------------*/
void
st_lib_tim2_irq_handler(void)
{
  /* clear interrupt pending flag */
  st_lib_hal_tim_clear_it(&htim2, TIM_IT_UPDATE);

  rtimer_clock++;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  st_lib_tim_clock_config_typedef s_clock_source_config;
  st_lib_tim_oc_init_typedef s_config_oc;

  st_lib_tim2_clk_enable();
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = PRESCALER;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1;

  st_lib_hal_tim_base_init(&htim2);
  st_lib_hal_tim_base_start_it(&htim2);

  s_clock_source_config.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  st_lib_hal_tim_config_clock_source(&htim2, &s_clock_source_config);

  st_lib_hal_tim_oc_init(&htim2);

  s_config_oc.OCMode = TIM_OCMODE_TIMING;
  s_config_oc.Pulse = 0;
  s_config_oc.OCPolarity = TIM_OCPOLARITY_HIGH;
  st_lib_hal_tim_oc_config_channel(&htim2, &s_config_oc, TIM_CHANNEL_1);

  st_lib_hal_tim_clear_flag(&htim2, TIM_FLAG_UPDATE);

  /* Enable TIM2 Update interrupt */
  st_lib_hal_tim_enable_it(&htim2, TIM_IT_UPDATE);

  st_lib_hal_tim_enable(&htim2);

  st_lib_hal_nvic_set_priority((st_lib_irq_n_type)TIM2_IRQn, 0, 0);
  st_lib_hal_nvic_enable_irq((st_lib_irq_n_type)(TIM2_IRQn));
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now(void)
{
  return rtimer_clock;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
}
/*---------------------------------------------------------------------------*/
