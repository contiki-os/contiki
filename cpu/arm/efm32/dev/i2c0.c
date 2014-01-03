/*
 * Copyright (c) 2013, Kerlink
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
 */
/**
 * \addtogroup efm32-devices
 * @{
 */
/**
 * \file
 *         EFM32 I2C0 driver
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <efm32.h>
#include <dev/i2c.h>

#include "clock.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_i2c.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

int i2c_init(uint32_t clock)           __attribute__ ((alias ("i2c0_init")));
int i2c_arch_master_transfer(struct i2c_msg *msgs, int num)   __attribute__ ((alias ("i2c0_arch_master_transfer")));


/*---------------------------------------------------------------------------*/
int i2c0_transfer(struct i2c_msg *msg)
{
  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;
  uint32_t                   timeout = 3000;

  memset(&seq,0,sizeof(seq));

  seq.addr = msg->address << 1;
  seq.flags = 0;
  if(msg->mode & I2C_WRITE)
  {
    seq.flags |= I2C_FLAG_WRITE;
    seq.buf[1].data = msg->buf;
    seq.buf[1].len = msg->len;
  }
  if(msg->mode & I2C_READ)
  {
    seq.flags |= I2C_FLAG_READ;
    seq.buf[0].data = msg->buf;
    seq.buf[0].len = msg->len;
  }


  ret = I2C_TransferInit(I2C0, &seq);
  while (ret == i2cTransferInProgress && timeout--)
  {
    ret = I2C_Transfer(I2C0);
  }
  if(ret != i2cTransferDone) return -EIO;

  return 1;
}

/*---------------------------------------------------------------------------*/
int i2c0_arch_master_transfer(struct i2c_msg *msgs, int num)
{
  int i=0, ret=0;

  for(i=0;i<num;i++)
  {
      ret = i2c0_transfer(&msgs[i]);
      if(ret < 0) return ret;
  }

  return (ret < 0) ? ret : num;

}

/*---------------------------------------------------------------------------*/
int i2c0_init(uint32_t clock)
{
  int i=0;

  // Initialize to 93,5 kHz (4:4 duty cycle)
  I2C_Init_TypeDef i2cinit = I2C_INIT_DEFAULT;

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_I2C0, true);
  // Set to 400kHz (but change 1/2 duty cycle)
  //i2cinit.freq = I2C_FREQ_FAST_MAX;

  /* In some situations (after a reset during an I2C transfer), the slave */
  /* device may be left in an unknown state. Send 9 clock pulses just in case. */
  for (i = 0; i < 9; i++)
  {
    gpio_set_value(GPIO_I2C0_SCL,0);
    usleep(100);
    gpio_set_value(GPIO_I2C0_SCL,1);
    usleep(100);
  }

  /* Enable pins at location 0 */
  I2C0->ROUTE = I2C_ROUTE_SDAPEN |
                  I2C_ROUTE_SCLPEN |
                  (I2C0_LOCATION << _I2C_ROUTE_LOCATION_SHIFT);

  I2C_Init(I2C0,&i2cinit);

  //I2C_Enable(I2C0, true);
#if 0
  if (I2C0->STATE & I2C_STATE_BUSY)
  {
  I2C0->CMD = I2C_CMD_ABORT;
  }

  I2C0->TXDATA = (0x48 << 1) | 1;
  I2C0->CMD = I2C_CMD_START;

  PRINTF("State %X, Status = %X\n\r",I2C0->STATE, I2C0->STATUS );
  I2C0->TXDATA = (0x48 << 1) | 1;
  PRINTF("State %X, Status = %X\n\r",I2C0->STATE, I2C0->STATUS );
#endif

//#define I2C0_TEST
#ifdef I2C0_TEST
  /*----------------- TEST -----------*/
  I2C_TransferSeq_TypeDef    seq;
  uint8_t                    offsetLoc[1];
  uint8_t data[3];
  I2C_TransferReturn_TypeDef ret;
  uint32_t timeout = 300000;

  memset(data,0,sizeof(data));

  seq.addr  = 0x48 << 1;
  seq.flags = I2C_FLAG_WRITE_READ;
  /* Select offset to start reading from */
  offsetLoc[0]    = (uint8_t) 0;
  seq.buf[0].data = offsetLoc;
  seq.buf[0].len  = 1;
  /* Select location/length of data to be read */
  seq.buf[1].data = data;
  seq.buf[1].len  = 3;

  ret = I2C_TransferInit(I2C0, &seq);
  while (ret == i2cTransferInProgress && timeout--)
  {
    ret = I2C_Transfer(I2C0);
  }

  PRINTF("I2C transfert test : ret = %d, data = %X\r\n", ret, data[0]);
#endif
  /*
  I2C0->CMD = I2C_CMD_ABORT;
  I2C0->
*/
}

/** @} */
