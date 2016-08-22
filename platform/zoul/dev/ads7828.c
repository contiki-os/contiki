/*
 * Copyright (c) 2017, Indian Institute of Science
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
 * This file is part of the Contiki operating system.
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-ads7828
 * @{
 *
 * \file
 *    Driver for TI ADS7828 12-bit, 8-Channel Sampling ADC conveter with I2C
 *    interface
 * \author
 *    Sumankumar Panchal <suman@ece.iisc.ernet.in>
 *    Akshay P M <akshaypm@ece.iisc.ernet.in>
 */
/*---------------------------------------------------------------------------*/
#include "dev/i2c.h"
#include "ads7828.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
void
ads7828_init()
{

  i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
           I2C_SCL_FAST_BUS_SPEED);
}
/*---------------------------------------------------------------------------*/
uint16_t
ads7828_read_channel(uint8_t reg, uint16_t *data)
{

  uint8_t buf[2];
  if(i2c_single_send(ADS7828_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(ADS7828_ADDR, buf, 2) == I2C_MASTER_ERR_NONE) {
      *data = (256 * buf[0]) + buf[1];
      PRINTF("ADS7828: successful to read data\n");
      return ADS7828_SUCCESS;
    }
  }
  PRINTF("ADS7828: failed to read data\n");
  return ADS7828_ERROR;
}
/*---------------------------------------------------------------------------*/
/** @} */
