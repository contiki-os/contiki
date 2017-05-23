/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup sensortag-cc26xx-sensor-common
 * @{
 *
 * \file
 * Utilities common among SensorTag sensors
 */
/*---------------------------------------------------------------------------*/
#include "sensor-common.h"
#include "board-i2c.h"
/*---------------------------------------------------------------------------*/
/* Data to use when an error occurs */
#define ERROR_DATA                         0xCC
/*---------------------------------------------------------------------------*/
static uint8_t buffer[32];
/*---------------------------------------------------------------------------*/
bool
sensor_common_read_reg(uint8_t addr, uint8_t *buf, uint8_t len)
{
  return board_i2c_write_read(&addr, 1, buf, len);
}
/*---------------------------------------------------------------------------*/
bool
sensor_common_write_reg(uint8_t addr, uint8_t *buf, uint8_t len)
{
  uint8_t i;
  uint8_t *p = buffer;

  /* Copy address and data to local buffer for burst write */
  *p++ = addr;
  for(i = 0; i < len; i++) {
    *p++ = *buf++;
  }
  len++;

  /* Send data */
  return board_i2c_write(buffer, len);
}
/*---------------------------------------------------------------------------*/
void
sensor_common_set_error_data(uint8_t *buf, uint8_t len)
{
  while(len > 0) {
    len--;
    buf[len] = ERROR_DATA;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
