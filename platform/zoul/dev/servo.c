/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-servo
 * @{
 *
 * \file
 *  Driver for a generic Servo driver
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/pwm.h"
#include "dev/gpio.h"
#include "servo.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
int
servo_position(uint16_t gptab, uint8_t port, uint8_t pin, uint16_t pos)
{
  uint8_t gpt_num;
  uint8_t gpt_ab;
  uint32_t count = 0;

  if((gptab < SERVO_CHANNEL_1) && (gptab > SERVO_CHANNEL_7)) {
    PRINTF("Servo: invalid servo channel\n");
    return SERVO_ERROR;
  }

  /* CC2538 has 4 ports (A-D) and up to 8 pins (0-7) */
  if((port > GPIO_D_NUM) || (pin > 7)) {
    PRINTF("Servo: Invalid pin/port settings\n");
    return SERVO_ERROR;
  }

  if(pos > SERVO_MAX_DEGREES) {
    PRINTF("Servo: invalid position (max %u)\n", SERVO_MAX_DEGREES);
    return SERVO_ERROR;
  }

  count = (SERVO_MAX_VAL - SERVO_MIN_VAL) * pos;
  count /= SERVO_MAX_DEGREES;
  count += SERVO_MIN_VAL;

  gpt_num = (uint8_t)(gptab >> 8);
  gpt_ab =  (uint8_t)(gptab & 0x00FF);

  PRINTF("Servo: F%uHz GPTNUM %u GPTAB %u --> %uº (%lu)\n", SERVO_DEFAULT_FREQ,
                                                            gpt_num, gpt_ab,
                                                            pos, count);
  /* Use count as argument instead of percentage */
  if(pwm_enable(SERVO_DEFAULT_FREQ, 0, count, gpt_num,gpt_ab) != PWM_SUCCESS) {
    PRINTF("Servo: failed to configure the pwm channel\n");
    return SERVO_ERROR;
  }

  /* Start the PWM as soon as possible, keep the pulses to lock the servo in the
   * given position
   */
  if(pwm_start(gpt_num, gpt_ab, port, pin) != PWM_SUCCESS) {
    PRINTF("Servo: failed to initialize the pwm channel\n");
    return SERVO_ERROR;
  }

  return SERVO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int
servo_stop(uint16_t gptab, uint8_t port, uint8_t pin)
{
  uint8_t gpt_num;
  uint8_t gpt_ab;

  if((gptab < SERVO_CHANNEL_1) && (gptab > SERVO_CHANNEL_7)) {
    PRINTF("Servo: invalid servo channel\n");
    return SERVO_ERROR;
  }

  /* CC2538 has 4 ports (A-D) and up to 8 pins (0-7) */
  if((port > GPIO_D_NUM) || (pin > 7)) {
    PRINTF("Servo: Invalid pin/port settings\n");
    return SERVO_ERROR;
  }

  gpt_num = (uint8_t)((gptab & 0xFF00) >> 8);
  gpt_ab =  (uint8_t)(gptab & 0x00FF);

  if(pwm_disable(gpt_num, gpt_ab, port, pin) != PWM_SUCCESS) {
    PRINTF("Servo: unable to disable the pwm channel\n");
    return SERVO_ERROR;
  }

  return SERVO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/** @} */
