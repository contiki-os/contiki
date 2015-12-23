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

#include "i2c.h"
#include "pwm-pca9685.h"

#define REG_MODE1 0x00
#define REG_MODE2 0x01

#define REG_LED_ON_L(n)  ((4 * n) + 0x06)
#define REG_LED_ON_H(n)  ((4 * n) + 0x07)
#define REG_LED_OFF_L(n) ((4 * n) + 0x08)
#define REG_LED_OFF_H(n) ((4 * n) + 0x09)

#define MAX_PWM_OUT          16
#define PWM_ONE_PERIOD_TICKS 4096

int
pwm_pca9685_set_values(struct pwm_pca9685_data *data, uint32_t pwm, uint32_t on, uint32_t off)
{
  uint8_t buf[5] = { 0 };

  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  if(pwm > MAX_PWM_OUT) {
    return -1;
  }

  buf[0] = REG_LED_ON_L(pwm);

  if((on >= PWM_ONE_PERIOD_TICKS) || (off >= PWM_ONE_PERIOD_TICKS)) {
    /* Treat as 100% */
    buf[1] = 0x0;
    buf[2] = (1 << 4);
    buf[3] = 0x0;
    buf[4] = 0x0;
  } else if(off == 0) {
    /* Treat it as 0% */
    buf[1] = 0x0;
    buf[2] = 0x0;
    buf[3] = 0x0;
    buf[4] = (1 << 4);
  } else {
    /* Populate registers accordingly */
    buf[0] = (on & 0xFF);
    buf[1] = ((on >> 8) & 0x0F);
    buf[2] = (off & 0xFF);
    buf[3] = ((off >> 8) & 0x0F);
  }

  return quarkX1000_i2c_polling_write(buf, sizeof(buf), data->i2c_slave_addr);
}
int
pwm_pca9685_set_duty_cycle(struct pwm_pca9685_data *data, uint32_t pwm, uint8_t duty)
{
  uint32_t on, off;

  if(duty == 0) {
    on = 0;
    off = 0;
  } else if(duty >= 100) {
    on = PWM_ONE_PERIOD_TICKS + 1;
    off = PWM_ONE_PERIOD_TICKS + 1;
  } else {
    on = PWM_ONE_PERIOD_TICKS * duty / 100;
    off = PWM_ONE_PERIOD_TICKS - 1;
  }

  return pwm_pca9685_set_values(data, pwm, on, off);
}
int
pwm_pca9685_init(struct pwm_pca9685_data *data, uint16_t i2c_slave_addr)
{
  uint8_t buf[2] = { 0 };

  /* has to init after I2C master */
  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  data->i2c_slave_addr = i2c_slave_addr;

  buf[0] = REG_MODE1;
  buf[1] = (1 << 5);

  if(quarkX1000_i2c_polling_write(buf, 2, i2c_slave_addr) < 0) {
    return -1;
  }

  return 0;
}
