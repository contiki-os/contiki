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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc2538-pwm-driver
 * @{
 *
 * \file
 *  Driver for the CC2538 PWM
 *
 * \author
 *         Javier Sanchez <jsanchez@zolertia.com>
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/sys-ctrl.h"
#include "dev/pwm.h"
#include "lpm.h"
#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define PWM_GPTIMER_NUM_TO_BASE(x) ((GPT_0_BASE) + ((x) << 12))
/*---------------------------------------------------------------------------*/
static uint8_t
pwm_configured(uint8_t timer, uint8_t ab)
{
  uint8_t offset;
  uint32_t gpt_base;
  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  offset = (ab) ? 4 : 0;

  if((REG(gpt_base + GPTIMER_TAMR + offset) & GPTIMER_TAMR_TAAMS) &&
     (REG(gpt_base + GPTIMER_TAMR + offset) & GPTIMER_TAMR_TAMR_PERIODIC)) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static bool
permit_pm1(void)
{
  uint8_t timer, ab;

  for(timer = PWM_TIMER_0; timer <= PWM_TIMER_3; timer++)
    for(ab = PWM_TIMER_A; ab <= PWM_TIMER_B; ab++)
      if(pwm_configured(timer, ab) &&
         REG(PWM_GPTIMER_NUM_TO_BASE(timer) + GPTIMER_CTL) &
           (ab == PWM_TIMER_A ? GPTIMER_CTL_TAEN : GPTIMER_CTL_TBEN))
        return false;

  return true;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_enable(uint32_t freq, uint8_t duty, uint32_t count, uint8_t timer,
           uint8_t ab)
{
  uint8_t offset = 0;
  uint32_t interval_load, duty_count, copy;
  uint32_t gpt_base, gpt_en, gpt_dir;

  if((freq < PWM_FREQ_MIN) || (freq > PWM_FREQ_MAX) ||
     (duty < PWM_DUTY_MIN) || (duty > PWM_DUTY_MAX) ||
     (timer > PWM_TIMER_MAX) || (timer < PWM_TIMER_MIN)) {
    PRINTF("PWM: Invalid PWM settings\n");
    return PWM_ERROR;
  }

  /* GPT0 timer A is used for clock_delay_usec() in clock.c */
  if((ab == PWM_TIMER_A) && (timer == PWM_TIMER_0)) {
    PRINTF("PWM: GPT0 (timer A) is reserved for clock_delay_usec()\n");
    return PWM_ERROR;
  }

  PRINTF("PWM: F%08luHz: %u%%/%lu on GPT%u-%u\n", freq, duty, count, timer, ab);

  lpm_register_peripheral(permit_pm1);

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  gpt_en = GPTIMER_CTL_TAEN;
  gpt_dir = GPTIMER_CTL_TAPWML;

  if(ab == PWM_TIMER_B) {
    offset = 4;
    gpt_en = GPTIMER_CTL_TBEN;
    gpt_dir = GPTIMER_CTL_TBPWML;
  }

  PRINTF("PWM: GPT_x_BASE 0x%08lX (%u)\n", gpt_base, offset);

  /* Restore later, ensure GPTIMER_CTL_TxEN and GPTIMER_CTL_TxPWML are clear */
  copy = REG(gpt_base + GPTIMER_CTL);
  copy &= ~(gpt_en | gpt_dir);

  /* Enable module clock for the GPTx in Active mode */
  REG(SYS_CTRL_RCGCGPT) |= (SYS_CTRL_RCGCGPT_GPT0 << timer);
  /* Enable module clock for the GPTx in Sleep mode */
  REG(SYS_CTRL_SCGCGPT) |= (SYS_CTRL_SCGCGPT_GPT0 << timer);
  /* Enable module clock for the GPTx in PM0, in PM1 and below this doesn't matter */
  REG(SYS_CTRL_DCGCGPT) |= (SYS_CTRL_DCGCGPT_GPT0 << timer);

  /* Stop the timer */
  REG(gpt_base + GPTIMER_CTL) = 0;
  /* Use 16-bit timer */
  REG(gpt_base + GPTIMER_CFG) = PWM_GPTIMER_CFG_SPLIT_MODE;
  /* Configure PWM mode */
  REG(gpt_base + GPTIMER_TAMR + offset) = 0;
  REG(gpt_base + GPTIMER_TAMR + offset) |= GPTIMER_TAMR_TAAMS;
  REG(gpt_base + GPTIMER_TAMR + offset) |= GPTIMER_TAMR_TAMR_PERIODIC;

  /* If the duty cycle is zero, leave the GPTIMER configured as PWM to pass a next
   * configured check, but do nothing else */
  if((!duty) && (!count)) {
    REG(gpt_base + GPTIMER_CTL) |= (copy | gpt_dir);
    return PWM_SUCCESS;
  }

  /* Get the peripheral clock and equivalent deassert count, depending on the
   * value given by the user, either use the count number of the duty cycle in
   * percentage
   */
  interval_load = sys_ctrl_get_sys_clock() / freq;
  if(duty) {
    duty_count = ((interval_load * duty) + 1) / 100;
  } else {
    duty_count = count;
  }

  PRINTF("PWM: sys %luHz: %lu %lu\n", sys_ctrl_get_sys_clock(),
         interval_load, duty_count);

  /* Set the start value (period), count down */
  REG(gpt_base + GPTIMER_TAILR + offset) = ((uint16_t *)&interval_load)[0] - 1;
  /* Set the deassert period */
  REG(gpt_base + GPTIMER_TAMATCHR + offset) = ((uint16_t *)&duty_count)[0] - 1;
  /* Set the prescaler if required */
  REG(gpt_base + GPTIMER_TAPR + offset) = ((uint8_t *)&interval_load)[2];
  /* Set the prescaler match if required */
  REG(gpt_base + GPTIMER_TAPMR + offset) = ((uint8_t *)&duty_count)[2];
  /* Restore the register content */
  REG(gpt_base + GPTIMER_CTL) |= (copy | gpt_dir);

  PRINTF("PWM: TnILR %lu ", REG(gpt_base + (GPTIMER_TAILR + offset)));
  PRINTF("TnMATCHR %lu  ", REG(gpt_base + (GPTIMER_TAMATCHR + offset)));
  PRINTF("TnPR %lu  ", REG(gpt_base + (GPTIMER_TAPR + offset)));
  PRINTF("TnPMR %lu\n", REG(gpt_base + (GPTIMER_TAPMR + offset)));

  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_stop(uint8_t timer, uint8_t ab, uint8_t port, uint8_t pin, uint8_t state)
{
  uint32_t gpt_base, gpt_dis;

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  /* CC2538 has 4 ports (A-D) and up to 8 pins (0-7) */
  if((port > GPIO_D_NUM) || (pin > 7)) {
    PRINTF("PWM: Invalid pin/port settings\n");
    return PWM_ERROR;
  }

  /* CC2538 has 4 ports (A-D) and up to 8 pins (0-7) */
  if((state != PWM_OFF_WHEN_STOP) && (state != PWM_ON_WHEN_STOP)) {
    PRINTF("PWM: Invalid pin state when PWM is halt\n");
    return PWM_ERROR;
  }

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  gpt_dis = (ab == PWM_TIMER_B) ? GPTIMER_CTL_TBEN : GPTIMER_CTL_TAEN;
  REG(gpt_base + GPTIMER_CTL) &= ~gpt_dis;

  /* Configure the port/pin as GPIO, input */
  ioc_set_over(port, pin, IOC_OVERRIDE_DIS);
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  if(state) {
    GPIO_SET_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  } else {
    GPIO_CLR_PIN(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  }
  PRINTF("PWM: OFF -> Timer %u (%u)\n", timer, ab);
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_start(uint8_t timer, uint8_t ab, uint8_t port, uint8_t pin)
{
  uint32_t gpt_base, gpt_en, gpt_sel;

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  /* CC2538 has 4 ports (A-D) and up to 8 pins (0-7) */
  if((port > GPIO_D_NUM) || (pin > 7)) {
    PRINTF("PWM: Invalid pin/port settings\n");
    return PWM_ERROR;
  }

  /* Map to given port/pin */
  gpt_sel = IOC_PXX_SEL_GPT0_ICP1 + (timer * 2);
  if(ab == PWM_TIMER_B) {
    gpt_sel++;
  }
  ioc_set_sel(port, pin, gpt_sel);
  ioc_set_over(port, pin, IOC_OVERRIDE_OE);
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  gpt_en = (ab == PWM_TIMER_B) ? GPTIMER_CTL_TBEN : GPTIMER_CTL_TAEN;
  REG(gpt_base + GPTIMER_CTL) |= gpt_en;
  PRINTF("PWM: ON -> Timer %u (%u) IOC_PXX_SEL_GPTx_IPCx 0x%08lX\n", timer, ab,
         gpt_sel);
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_set_direction(uint8_t timer, uint8_t ab, uint8_t dir)
{
  uint32_t gpt_base, gpt_dir;

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX) || (dir > PWM_SIGNAL_INVERTED)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  gpt_dir = (ab == PWM_TIMER_B) ? GPTIMER_CTL_TBPWML : GPTIMER_CTL_TAPWML;
  if(dir) {
    REG(gpt_base + GPTIMER_CTL) |= gpt_dir;
  } else {
    REG(gpt_base + GPTIMER_CTL) &= ~gpt_dir;
  }

  PRINTF("PWM: Signal direction (%u) -> Timer %u (%u)\n", dir, timer, ab);
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_toggle_direction(uint8_t timer, uint8_t ab)
{
  uint32_t gpt_base, gpt_dir;

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  gpt_dir = (ab == PWM_TIMER_B) ? GPTIMER_CTL_TBPWML : GPTIMER_CTL_TAPWML;
  if(REG(gpt_base + GPTIMER_CTL) & gpt_dir) {
    REG(gpt_base + GPTIMER_CTL) &= ~gpt_dir;
  } else {
    REG(gpt_base + GPTIMER_CTL) |= gpt_dir;
  }

  PRINTF("PWM: direction toggled -> Timer %u (%u)\n", timer, ab);
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_disable(uint8_t timer, uint8_t ab, uint8_t port, uint8_t pin)
{
  uint32_t gpt_base;
  uint8_t offset = (ab == PWM_TIMER_B) ? 4 : 0;
  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  /* CC2538 has 4 ports (A-D) and up to 8 pins (0-7) */
  if((port > GPIO_D_NUM) || (pin > 7)) {
    PRINTF("PWM: Invalid pin/port settings\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  /* Stop the PWM */
  pwm_stop(timer, ab, port, pin, PWM_OFF_WHEN_STOP);
  /* Disable the PWM mode */
  REG(gpt_base + (GPTIMER_TAMR + offset)) = 0;
  /* Restart the interval load and deassert values */
  REG(gpt_base + (GPTIMER_TAILR + offset)) = 0;
  REG(gpt_base + (GPTIMER_TAMATCHR + offset)) = 0;

  /* Configure the port/pin as GPIO, input */
  ioc_set_over(port, pin, IOC_OVERRIDE_DIS);
  GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(port), GPIO_PIN_MASK(pin));

  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/** @} */
