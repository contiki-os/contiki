/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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

#include "i2c-eeprom.h"
#include "i2c.h"

/**
 * \addtogroup saker-eeprom
 *
 * @{
 *
 * \file
 *      Implementation of the platform's I2C-EEPROM driver
 */

/*---------------------------------------------------------------------------*/
/** Page size */
#define EEPROM_PAGE_SIZE                8
/** Write cycle time in usec */
#define EEPROM_TWC_USEC                 6000
/*---------------------------------------------------------------------------*/
#define BUSYWAIT_UNTIL(cond, max_time)                                       \
  do {                                                                       \
    rtimer_clock_t t0;                                                       \
    t0 = RTIMER_NOW();                                                       \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) {}      \
  } while(0)
/*---------------------------------------------------------------------------*/
int
i2c_eeprom_write_block(const uint8_t *data, uint16_t addr,
                       uint16_t data_len)
{

  int i = data_len;

  if((addr + data_len) - 1 >= EEPROM_START_ADDR_WP_BLOCK) {
    /* Can't write into write protected block */
    return 0;
  }

  if(data_len == 0) {
    /* Nothing to write */
    return 0;
  }

  /* Initialize I2C */
  i2c_init(I2C_SDA_PORT,
           I2C_SDA_PIN,
           I2C_SCL_PORT,
           I2C_SCL_PIN,
           I2C_SCL_FAST_BUS_SPEED);

  /* Write initial word address */
  i2c_master_set_slave_address(EEPROM_ADDR, I2C_SEND);
  i2c_master_data_put((uint8_t)addr);
  i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
  BUSYWAIT_UNTIL(!(i2c_master_busy()), RTIMER_SECOND / 100);
  if(i2c_master_busy() || (i2c_master_error() != I2C_MASTER_ERR_NONE)) {
    /* Timeout or I2C error */
    i = 0;
  } else {

    int j = 0;

    while(data_len != 0) {

      while(1) {
        i2c_master_data_put(data[j++]);
        addr++;
        data_len--;
        if(((addr % EEPROM_PAGE_SIZE) != 0) && (data_len != 0)) {
          i2c_master_command(I2C_MASTER_CMD_BURST_SEND_CONT);
          while(i2c_master_busy()) ;
        } else {
          /*
           * Page boundary or last data byte,
           * send stop condition and break out of for-loop
           */
          i2c_master_command(I2C_MASTER_CMD_BURST_SEND_FINISH);
          while(i2c_master_busy()) ;
          break;
        }
      }

      /* Give some time for the ACK. */
      clock_delay_usec(EEPROM_TWC_USEC);

      if(data_len != 0) {
        /* Write start address of next page */
        i2c_master_data_put((uint8_t)addr);
        i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
        while(i2c_master_busy()) ;
      }
    } /* while(data_len != 0) */
  }

  /* Disable I2C */
  i2c_master_disable();

  /* Disable I2C clock */
  REG(SYS_CTRL_RCGCI2C) = 0;

  return i;
}
/*---------------------------------------------------------------------------*/
int
i2c_eeprom_read_block(uint8_t *buf, uint16_t addr, uint16_t buf_len)
{

  int i = buf_len;

  if((addr + buf_len) > (EEPROM_END_ADDR + 1)) {
    /* Can not read behind end address */
    return 0;
  }

  if(buf_len == 0) {
    /* Nothing to read */
    return 0;
  }

  /* Initialize I2C */
  i2c_init(I2C_SDA_PORT,
           I2C_SDA_PIN,
           I2C_SCL_PORT,
           I2C_SCL_PIN,
           I2C_SCL_FAST_BUS_SPEED);

  /*
   * Sequential read from random address:
   * Select address to read from using a write operation
   */
  i2c_master_set_slave_address(EEPROM_ADDR, I2C_SEND);
  i2c_master_data_put((uint8_t)addr);
  i2c_master_command(I2C_MASTER_CMD_SINGLE_SEND);
  BUSYWAIT_UNTIL(!(i2c_master_busy()), RTIMER_SECOND / 100);
  if(i2c_master_busy() || (i2c_master_error() != I2C_MASTER_ERR_NONE)) {
    /* Timeout or I2C error */
    i = 0;
  } else {
    /* Perform sequential read */
    i2c_burst_receive(EEPROM_ADDR, buf, (uint8_t)buf_len);
  }

  /* Disable I2C */
  i2c_master_disable();

  /* Disable I2C clock */
  REG(SYS_CTRL_RCGCI2C) = 0;

  return i;
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 */
