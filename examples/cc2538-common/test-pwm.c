/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-test-pwm Test the CC2538 PWM driver
 *
 * Demonstrates the use of the CC2538 PWM driver
 *
 * @{
 *
 * \file
 *         A quick program for testing the CC2538 PWM driver
 * \author
 *         Javier Sanchez <jsanchez@zolertia.com>
 *         Antonio Lignan <alinan@zolertia.com>
 */
#include "contiki.h"
#include "cpu.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/sys-ctrl.h"
#include "pwm.h"
#include "systick.h"
#include "lpm.h"
#include "dev/ioc.h"
#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
typedef struct {
  uint8_t timer;
  uint8_t ab;
  uint8_t port;
  uint8_t pin;
  uint8_t duty;
  uint8_t off_state;
  uint32_t freq;
} pwm_config_t;
/*---------------------------------------------------------------------------*/
#define MAX_PWM  4
static const pwm_config_t pwm_num[MAX_PWM] = {
  {
    .timer = PWM_TIMER_1,
    .ab = PWM_TIMER_A,
    .port = GPIO_D_NUM,
    .pin = 5,
    .duty = 15,
    .freq = 1,
    .off_state = PWM_OFF_WHEN_STOP,
  }, {
    .timer = PWM_TIMER_1,
    .ab = PWM_TIMER_B,
    .port = GPIO_D_NUM,
    .pin = 4,
    .duty = 35,
    .freq = 100,
    .off_state = PWM_ON_WHEN_STOP,
  }, {
    .timer = PWM_TIMER_2,
    .ab = PWM_TIMER_A,
    .port = GPIO_D_NUM,
    .pin = 3,
    .duty = 50,
    .freq = 1000,
    .off_state = PWM_OFF_WHEN_STOP,
  }, {
    .timer = PWM_TIMER_2,
    .ab = PWM_TIMER_B,
    .port = GPIO_D_NUM,
    .pin = 2,
    .duty = 85,
    .freq = 160000,
    .off_state = PWM_ON_WHEN_STOP,
  }
};
static uint8_t pwm_en[MAX_PWM];
/*---------------------------------------------------------------------------*/
#if DEBUG
static const char *
gpt_name(uint8_t timer)
{
  switch(timer) {
  case PWM_TIMER_0:
    return "PWM TIMER 0";
  case PWM_TIMER_1:
    return "PWM TIMER 1";
  case PWM_TIMER_2:
    return "PWM TIMER 2";
  case PWM_TIMER_3:
    return "PWM TIMER 3";
  default:
    return "Unknown";
  }
}
#endif
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(cc2538_pwm_test, "cc2538 pwm test");
AUTOSTART_PROCESSES(&cc2538_pwm_test);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2538_pwm_test, ev, data)
{
  PROCESS_BEGIN();

  uint8_t i;
  memset(pwm_en, 0, MAX_PWM);

  PRINTF("\nStarting the test\n");

  for(i = 0; i < MAX_PWM; i++) {
    if(pwm_enable(pwm_num[i].freq, pwm_num[i].duty, 0,
                  pwm_num[i].timer, pwm_num[i].ab) == PWM_SUCCESS) {
      pwm_en[i] = 1;
      PRINTF("%s (%u) configuration OK\n", gpt_name(pwm_num[i].timer),
             pwm_num[i].ab);
    }
  }

  while(1) {
    for(i = 0; i < MAX_PWM; i++) {
      if((pwm_en[i]) &&
         (pwm_start(pwm_num[i].timer, pwm_num[i].ab,
                    pwm_num[i].port, pwm_num[i].pin) != PWM_SUCCESS)) {
        pwm_en[i] = 0;
        PRINTF("%s (%u) failed to start \n", gpt_name(pwm_num[i].timer),
               pwm_num[i].ab);
      }
    }

    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    for(i = 0; i < MAX_PWM; i++) {
      if((pwm_en[i]) &&
         (pwm_toggle_direction(pwm_num[i].timer,
                               pwm_num[i].ab) != PWM_SUCCESS)) {
        PRINTF("%s (%u) invert failed \n", gpt_name(pwm_num[i].timer),
               pwm_num[i].ab);
      }
    }

    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    for(i = 0; i < MAX_PWM; i++) {
      if((pwm_en[i]) &&
         (pwm_stop(pwm_num[i].timer, pwm_num[i].ab,
                   pwm_num[i].port, pwm_num[i].pin,
                   pwm_num[i].off_state) != PWM_SUCCESS)) {
        pwm_en[i] = 0;
        PRINTF("%s (%u) failed to stop\n", gpt_name(pwm_num[i].timer),
               pwm_num[i].ab);
      }
    }

    etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
