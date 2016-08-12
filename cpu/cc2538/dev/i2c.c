/*
 * Copyright (c) 2015, Mehdi Migault
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
 * \addtogroup cc2538-i2c cc2538 I2C Control
 * @{
 *
 * \file
 * Implementation file of the I2C Control module
 *
 * \author
 * Mehdi Migault
 */

#include "i2c.h"

#include <stdint.h>
#include "clock.h"
#include "sys-ctrl.h"
/*---------------------------------------------------------------------------*/
void
i2c_init(uint8_t port_sda, uint8_t pin_sda, uint8_t port_scl, uint8_t pin_scl,
         uint32_t bus_speed)
{
  /* Enable I2C clock in different modes */
  REG(SYS_CTRL_RCGCI2C) |= 1; /* Run mode */

  /* Reset I2C peripheral */
  REG(SYS_CTRL_SRI2C) |= 1; /* Reset position */

  /* Delay for a little bit */
  clock_delay_usec(50);

  REG(SYS_CTRL_SRI2C) &= ~1;  /* Normal position */

  /* Set pins in input */
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(port_sda), GPIO_PIN_MASK(pin_sda));
  GPIO_SET_INPUT(GPIO_PORT_TO_BASE(port_scl), GPIO_PIN_MASK(pin_scl));

  /* Set peripheral control for the pins */
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(port_sda), GPIO_PIN_MASK(pin_sda));
  GPIO_PERIPHERAL_CONTROL(GPIO_PORT_TO_BASE(port_scl), GPIO_PIN_MASK(pin_scl));

  /* Set the pad to no drive type */
  ioc_set_over(port_sda, pin_sda, IOC_OVERRIDE_DIS);
  ioc_set_over(port_scl, pin_scl, IOC_OVERRIDE_DIS);

  /* Set pins as peripheral inputs */
  REG(IOC_I2CMSSDA) = ioc_input_sel(port_sda, pin_sda);
  REG(IOC_I2CMSSCL) = ioc_input_sel(port_scl, pin_scl);

  /* Set pins as peripheral outputs */
  ioc_set_sel(port_sda, pin_sda, IOC_PXX_SEL_I2C_CMSSDA);
  ioc_set_sel(port_scl, pin_scl, IOC_PXX_SEL_I2C_CMSSCL);

  /* Enable the I2C master module */
  i2c_master_enable();

  /* t the master clock frequency */
  i2c_set_frequency(bus_speed);
}
/*---------------------------------------------------------------------------*/
void
i2c_master_enable(void)
{
  REG(I2CM_CR) |= 0x10; /* Set MFE bit */
}
/*---------------------------------------------------------------------------*/
void
i2c_master_disable(void)
{
  REG(I2CM_CR) &= ~0x10;  /* Reset MFE bit */
}
/*---------------------------------------------------------------------------*/
void
i2c_set_frequency(uint32_t freq)
{
  /* Peripheral clock setting, using the system clock */
  REG(I2CM_TPR) = ((SYS_CTRL_SYS_CLOCK + (2 * 10 * freq) - 1) /
                   (2 * 10 * freq)) - 1;
}
/*---------------------------------------------------------------------------*/
void
i2c_master_set_slave_address(uint8_t slave_addr, uint8_t access_mode)
{
  if(access_mode) {
    REG(I2CM_SA) = ((slave_addr << 1) | 1);
  } else {
    REG(I2CM_SA) = (slave_addr << 1);
  }
}
/*---------------------------------------------------------------------------*/
void
i2c_master_data_put(uint8_t data)
{
  REG(I2CM_DR) = data;
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_master_data_get(void)
{
  return REG(I2CM_DR);
}
/*---------------------------------------------------------------------------*/
void
i2c_master_command(uint8_t cmd)
{
  REG(I2CM_CTRL) = cmd;
  /* Here we need a delay, otherwise the I2C module keep the receiver mode */
  clock_delay_usec(1);
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_master_busy(void)
{
  return REG(I2CM_STAT) & I2CM_STAT_BUSY;
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_master_error(void)
{
  uint8_t temp = REG(I2CM_STAT);  /* Get all status */
  if(temp & I2CM_STAT_BUSY) {   /* No valid if BUSY bit is set */
    return I2C_MASTER_ERR_NONE;
  } else if(temp & (I2CM_STAT_ERROR | I2CM_STAT_ARBLST)) {
    return temp;  /* Compare later */
  }
  return I2C_MASTER_ERR_NONE;
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_single_send(uint8_t slave_addr, uint8_t data)
{
  i2c_master_set_slave_address(slave_addr, I2C_SEND);
  i2c_master_data_put(data);
  i2c_master_command(I2C_MASTER_CMD_SINGLE_SEND);

  while(i2c_master_busy());

  /* Return the STAT register of I2C module if error occured, I2C_MASTER_ERR_NONE otherwise */
  return i2c_master_error();
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_single_receive(uint8_t slave_addr, uint8_t *data)
{
  uint8_t temp;

  i2c_master_set_slave_address(slave_addr, I2C_RECEIVE);
  i2c_master_command(I2C_MASTER_CMD_SINGLE_RECEIVE);

  while(i2c_master_busy());
  temp = i2c_master_error();
  if(temp == I2C_MASTER_ERR_NONE) {
    *data = i2c_master_data_get();
  }
  return temp;
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_burst_send(uint8_t slave_addr, uint8_t *data, uint8_t len)
{
  uint8_t sent;
  if((len == 0) || (data == NULL)) {
    return I2CM_STAT_INVALID;
  }
  if(len == 1) {
    return i2c_single_send(slave_addr, data[0]);
  }
  i2c_master_set_slave_address(slave_addr, I2C_SEND);
  i2c_master_data_put(data[0]);
  i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
  while(i2c_master_busy());
  if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
    for(sent = 1; sent <= (len - 2); sent++) {
      i2c_master_data_put(data[sent]);
      i2c_master_command(I2C_MASTER_CMD_BURST_SEND_CONT);
      while(i2c_master_busy());
    }
    /* This should be the last byte, stop sending */
    i2c_master_data_put(data[len - 1]);
    i2c_master_command(I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(i2c_master_busy());
  }

  /* Return the STAT register of I2C module if error occurred, I2C_MASTER_ERR_NONE otherwise */
  return i2c_master_error();
}
/*---------------------------------------------------------------------------*/
uint8_t
i2c_burst_receive(uint8_t slave_addr, uint8_t *data, uint8_t len)
{
  uint8_t recv = 0;
  if((len == 0) || data == NULL) {
    return I2CM_STAT_INVALID;
  }
  if(len == 1) {
    return i2c_single_receive(slave_addr, &data[0]);
  }
  i2c_master_set_slave_address(slave_addr, I2C_RECEIVE);
  i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_START);
  while(i2c_master_busy());
  if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
    data[0] = i2c_master_data_get();
    /* If we got 2 or more bytes pending to be received, keep going*/
    for(recv = 1; recv <= (len - 2); recv++) {
      i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      while(i2c_master_busy());
      data[recv] = i2c_master_data_get();
    }
    /* This should be the last byte, stop receiving */
    i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(i2c_master_busy());
    data[len - 1] = i2c_master_data_get();
  }
  return i2c_master_error();
}
/*---------------------------------------------------------------------------*/
/** @} */
