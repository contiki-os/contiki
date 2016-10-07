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
 * \author Andreas Urke <arurke@gmail.com>
 *
 * \addtogroup cc26xx-cc13xx-i2c
 * @{
 *
 *
 * \file
 * Generic I2C driver for cc26xx/13xx
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "ti-lib.h"
#include "i2c.h"
#include "lpm.h"

#include <string.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define I2C_PIN_NOT_SET           IOID_UNUSED
#define I2C_SLAVE_ADDRESS_NOT_SET 0x00
#define I2C_PIN_PULL_NOT_SET      0x00
#define I2C_DEFAULT_SPEED         I2C_SPEED_FAST
/*---------------------------------------------------------------------------*/
static uint8_t selected_slave_addr = I2C_SLAVE_ADDRESS_NOT_SET;
static uint32_t selected_pin_sda = I2C_PIN_NOT_SET;
static uint32_t selected_pin_scl = I2C_PIN_NOT_SET;
static bool selected_speed = I2C_DEFAULT_SPEED;
static uint32_t selected_pin_pull = I2C_PIN_PULL_NOT_SET;

/*---------------------------------------------------------------------------*/
static bool
accessible(void)
{
  /* First, check the PD */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
     != PRCM_DOMAIN_POWER_ON) {
    return false;
  }

  /* Then check the 'run mode' clock gate */
  if(!(HWREG(PRCM_BASE + PRCM_O_I2CCLKGR) & PRCM_I2CCLKGR_CLK_EN)) {
    return false;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
void
i2c_wakeup()
{
  PRINTF("I2C: Waking up\n");
  /* First, make sure the SERIAL PD is on */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable the clock to I2C */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_I2C0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Enable and initialize the I2C master module */
  ti_lib_i2c_master_init_exp_clk(I2C0_BASE, ti_lib_sys_ctrl_clock_get(),
                                 selected_speed);
}
/*---------------------------------------------------------------------------*/
static bool
i2c_status()
{
  uint32_t status;

  status = ti_lib_i2c_master_err(I2C0_BASE);
  if(status & (I2C_MSTAT_DATACK_N_M | I2C_MSTAT_ADRACK_N_M)) {
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
  }

  return status == I2C_MASTER_ERR_NONE;
}
/*---------------------------------------------------------------------------*/
void
i2c_shutdown()
{
  //PRINTF("I2C: Shutting down\n");
  /* If selected, deselect first */
  i2c_deselect();

  if(accessible()) {
    ti_lib_i2c_master_disable(I2C0_BASE);
  }

  ti_lib_prcm_peripheral_run_disable(PRCM_PERIPH_I2C0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());
}
/*---------------------------------------------------------------------------*/
bool
i2c_write(uint8_t *data, uint8_t len)
{
  uint32_t i;
  bool success;

  /* Write slave address */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, selected_slave_addr, false);

  /* Write first byte */
  ti_lib_i2c_master_data_put(I2C0_BASE, data[0]);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  while(ti_lib_i2c_master_busy(I2C0_BASE));
  success = i2c_status();

  for(i = 1; i < len && success; i++) {
    /* Write next byte */
    ti_lib_i2c_master_data_put(I2C0_BASE, data[i]);
    if(i < len - 1) {
      /* Clear START */
      ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
      while(ti_lib_i2c_master_busy(I2C0_BASE));
      success = i2c_status();
    }
  }

  /* Assert stop */
  if(success) {
    /* Assert STOP */
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    while(ti_lib_i2c_master_bus_busy(I2C0_BASE));
  }

  return success;
}
/*---------------------------------------------------------------------------*/
bool
i2c_write_single(uint8_t data)
{
  /* Write slave address */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, selected_slave_addr, false);

  /* Write first byte */
  ti_lib_i2c_master_data_put(I2C0_BASE, data);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START + STOP */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
  while(ti_lib_i2c_master_busy(I2C0_BASE));

  return i2c_status();
}
/*---------------------------------------------------------------------------*/
bool
i2c_read(uint8_t *data, uint8_t len)
{
  uint8_t i;
  bool success;

  /* Set slave address */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, selected_slave_addr, true);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START + ACK */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

  i = 0;
  success = true;
  while(i < (len - 1) && success) {
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    if(success) {
      data[i] = ti_lib_i2c_master_data_get(I2C0_BASE);
      ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      i++;
    }
  }

  if(success) {
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    if(success) {
      data[len - 1] = ti_lib_i2c_master_data_get(I2C0_BASE);
      while(ti_lib_i2c_master_bus_busy(I2C0_BASE));
    }
  }

  return success;
}
/*---------------------------------------------------------------------------*/
bool
i2c_write_read(uint8_t *wdata, uint8_t wlen, uint8_t *rdata, uint8_t rlen)
{
  uint32_t i;
  bool success;

  /* Set slave address for write */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, selected_slave_addr, false);

  /* Write first byte */
  ti_lib_i2c_master_data_put(I2C0_BASE, wdata[0]);

  /* Check if another master has access */
  while(ti_lib_i2c_master_bus_busy(I2C0_BASE));

  /* Assert RUN + START */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  while(ti_lib_i2c_master_busy(I2C0_BASE));
  success = i2c_status();

  for(i = 1; i < wlen && success; i++) {
    /* Write next byte */
    ti_lib_i2c_master_data_put(I2C0_BASE, wdata[i]);

    /* Clear START */
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
  }
  if(!success) {
    return false;
  }

  /* Set slave address for read */
  ti_lib_i2c_master_slave_addr_set(I2C0_BASE, selected_slave_addr, true);

  /* Assert ACK */
  ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

  i = 0;
  while(i < (rlen - 1) && success) {
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    if(success) {
      rdata[i] = ti_lib_i2c_master_data_get(I2C0_BASE);
      ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      i++;
    }
  }

  if(success) {
    ti_lib_i2c_master_control(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(ti_lib_i2c_master_busy(I2C0_BASE));
    success = i2c_status();
    if(success) {
      rdata[rlen - 1] = ti_lib_i2c_master_data_get(I2C0_BASE);
      while(ti_lib_i2c_master_bus_busy(I2C0_BASE));
    }
  }

  return success;
}
/*---------------------------------------------------------------------------*/
bool
i2c_write_single_read_multi(uint8_t wdata, uint8_t *buf, uint8_t len)
{
  return i2c_write_read(&wdata, 1, buf, len);
}
/*---------------------------------------------------------------------------*/
void
i2c_select(uint32_t new_pin_sda,
                 uint32_t new_pin_scl,
                 uint8_t new_slave_address,
                 bool new_speed,
                 uint32_t new_pin_pull)
{
  PRINTF("I2C: Selecting: sda: %lu, scl: %lu, addr: %x, speed: %d,pull: %lu\n",
         new_pin_sda, new_pin_scl, new_slave_address, new_speed, new_pin_pull);

  if(accessible() == false) {
    i2c_wakeup();
  }

  /* Configure for new values only */
  if(new_pin_sda != selected_pin_sda ||
      new_pin_scl != selected_pin_scl ||
      new_speed != selected_speed) {

    /* If already selected, deselect first */
    i2c_deselect();

    ti_lib_i2c_master_disable(I2C0_BASE);

    ti_lib_ioc_io_port_pull_set(new_pin_sda, IOC_NO_IOPULL);
    ti_lib_ioc_io_port_pull_set(new_pin_scl, IOC_NO_IOPULL);
    ti_lib_ioc_pin_type_i2c(I2C0_BASE, new_pin_sda, new_pin_scl);

    /* Enable and initialize the I2C master module */
    ti_lib_i2c_master_init_exp_clk(I2C0_BASE, ti_lib_sys_ctrl_clock_get(),
                                   new_speed);
  }

  selected_pin_sda = new_pin_sda;
  selected_pin_scl = new_pin_scl;
  selected_speed = new_speed;
  selected_slave_addr = new_slave_address;
  selected_pin_pull = new_pin_pull;
}
/*---------------------------------------------------------------------------*/
void
i2c_deselect()
{
  /* Deselect only if selected */
  if(selected_pin_sda != I2C_PIN_NOT_SET ||
      selected_pin_scl != I2C_PIN_NOT_SET) {

    PRINTF("I2C: Deselecting sda: %lu, scl: %lu, addr: %x\n",
           selected_pin_sda, selected_pin_scl, selected_slave_addr);

    if(accessible()) {
      ti_lib_i2c_master_disable(I2C0_BASE);
    }

    /*
     * Set pins to GPIO Input and disable the output driver. Set pull
     * to selected_pin_pull
     */
    ti_lib_ioc_io_port_pull_set(selected_pin_sda, selected_pin_pull);
    ti_lib_ioc_io_port_pull_set(selected_pin_scl, selected_pin_pull);
    ti_lib_ioc_pin_type_gpio_input(selected_pin_sda);
    ti_lib_ioc_pin_type_gpio_input(selected_pin_scl);

    selected_pin_sda = I2C_PIN_NOT_SET;
    selected_pin_scl = I2C_PIN_NOT_SET;
    selected_speed = I2C_DEFAULT_SPEED;
    selected_slave_addr = I2C_SLAVE_ADDRESS_NOT_SET;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
