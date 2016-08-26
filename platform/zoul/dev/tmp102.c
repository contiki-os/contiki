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
 * \addtogroup zoul-tmp102-sensor
 * @{
 *
 * \file
 *  Driver for the TMP102 temperature sensor
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "tmp102.h"

void
tmp102_init(void)
{
  i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
}
/*---------------------------------------------------------------------------*/

uint8_t
tmp102_read(uint16_t *data)
{
  uint8_t buf[2];
  uint16_t temp;

  /* Write to the temperature register to trigger a reading */
  if(i2c_single_send(TMP102_ADDR, TMP102_TEMP) == I2C_MASTER_ERR_NONE) {
    /* Read two bytes only */
    if(i2c_burst_receive(TMP102_ADDR, buf, 2) == I2C_MASTER_ERR_NONE) {
      /* 12-bit value, TMP102 SBOS397F Table 8-9 */
      temp = (buf[0] << 4) + (buf[1] >> 4);
      if(temp > 2047) {
        temp -= (1 << 12);
      }
      temp *= 0.625;
      *data = temp;
      return I2C_MASTER_ERR_NONE;
    }
  }
  return i2c_master_error();
}
/*---------------------------------------------------------------------------*/
/** @} */
