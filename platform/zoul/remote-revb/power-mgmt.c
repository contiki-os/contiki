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
 * \addtogroup remote-power-mgmt-revb
 * @{
 * RE-Mote power management functions.
 *
 * @{
 *
 * \author
 *         Aitor Mejias   <amejias@zolertia.com>
 *         Antonio Lignan <alinan@zolertia.com>
 */
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdint.h>
#include "contiki.h"
#include "dev/gpio.h"
#include "sys/rtimer.h"
#include "power-mgmt.h"
#include "dev/i2c.h"
/* -------------------------------------------------------------------------- */
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/* -------------------------------------------------------------------------- */
#define PM_ENABLE_LINE_SET       GPIO_SET_PIN(PM_ENABLE_PORT_BASE, \
                                              PM_ENABLE_PIN_MASK)
#define PM_ENABLE_LINE_CLR       GPIO_CLR_PIN(PM_ENABLE_PORT_BASE, \
                                              PM_ENABLE_PIN_MASK)
#define PM_ENABLE_AS_OUTPUT      GPIO_SET_OUTPUT(PM_ENABLE_PORT_BASE, \
                                                 PM_ENABLE_PIN_MASK)
#define PM_ENABLE_LINE_CMD       PM_ENABLE_LINE_SET; \
                                 clock_delay_usec(100);

/* -------------------------------------------------------------------------- */
#define PM_NUMBITS(X)            (1 << ((X)-1))
/* -------------------------------------------------------------------------- */
static uint8_t initialized = 0;
static uint8_t lbuf[5];
/* -------------------------------------------------------------------------- */
int8_t
pm_enable(void)
{
  /* Set as output/low to set IDLE state */
  GPIO_SOFTWARE_CONTROL(PM_ENABLE_PORT_BASE, PM_ENABLE_PIN_MASK);
  PM_ENABLE_AS_OUTPUT;

  i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
           I2C_SCL_NORMAL_BUS_SPEED);
  initialized = 1;
  return PM_SUCCESS;
}
/* -------------------------------------------------------------------------- */
static int
pm_write_byte(uint8_t reg, uint8_t val)
{
  if(!initialized) {
    return PM_ERROR;
  }
  lbuf[0] = reg;
  lbuf[1] = val;
  PM_ENABLE_LINE_CMD;

  if(i2c_burst_send(PWR_MNGMT_ADDR, lbuf, 2) == I2C_MASTER_ERR_NONE) {
    PM_ENABLE_LINE_CLR;
    return PM_SUCCESS;
  }
  PM_ENABLE_LINE_CLR;
  return PM_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
pm_read_byte(uint8_t reg, uint8_t *val, uint8_t len)
{
  /* Detect valid register parameter */
  if((reg < PM_VBAT) || (reg >= PM_MAX_COMMANDS) || (!len)) {
    PRINTF("PM: invalid settings/not initialized\n");
    return PM_ERROR;
  }

  PM_ENABLE_LINE_CMD;

  if(i2c_single_send(PWR_MNGMT_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(PWR_MNGMT_ADDR, val, len) == I2C_MASTER_ERR_NONE) {
      printf("PM: Data 0x%02X\n", *val);
      PM_ENABLE_LINE_CLR;
      return PM_SUCCESS;
    }
  }
  PRINTF("PM: Error reading the registers\n");
  PM_ENABLE_LINE_CLR;
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_reset_system(void)
{
  if(!initialized) {
    return PM_ERROR;
  }

  /* Reset the low-power PIC and the whole board as a bonus */
  lbuf[0] = PM_CMD_RST_HARD;
  lbuf[1] = 1;
  PM_ENABLE_LINE_CMD;

  if(i2c_burst_send(PWR_MNGMT_ADDR, lbuf, 2) == I2C_MASTER_ERR_NONE) {
    clock_delay_usec(1000);
    PM_ENABLE_LINE_CLR;
    return PM_SUCCESS;
  }
  PM_ENABLE_LINE_CLR;
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_set_timeout(uint32_t time)
{
  if(!initialized) {
    return PM_ERROR;
  }

  if(time > PM_SOFT_SHTDN_28_DAYS) {
    PRINTF("PM: maximum timeout is %u\n", (uint32_t)PM_SOFT_SHTDN_28_DAYS);
    return PM_ERROR;
  }

  lbuf[0] = PM_SOFT_TIME;
  lbuf[1] = (uint8_t)(time >> 24);
  lbuf[2] = (uint8_t)(time >> 16) & 0xFF;
  lbuf[3] = (uint8_t)(time >> 8) & 0xFF;
  lbuf[4] = (uint8_t)(time & 0xFF);
  PRINTF("PM: Timeout 0x%02x%02x%02x%02x\n", lbuf[1], lbuf[2], lbuf[3], lbuf[4]);

  PM_ENABLE_LINE_CMD;

  if(i2c_burst_send(PWR_MNGMT_ADDR, lbuf, 5) == I2C_MASTER_ERR_NONE) {
    PM_ENABLE_LINE_CLR;
    return PM_SUCCESS;
  }

  PRINTF("PM: error setting the timeout\n");
  PM_ENABLE_LINE_CLR;
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
uint32_t
pm_get_timeout(void)
{
  uint32_t retval = 0;
  PM_ENABLE_LINE_CMD;

  if(i2c_single_send(PWR_MNGMT_ADDR, PM_SOFT_TIME) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(PWR_MNGMT_ADDR, lbuf, 4) == I2C_MASTER_ERR_NONE) {
      retval |= ((uint32_t)lbuf[0] << 24);
      retval |= ((uint32_t)lbuf[1] << 16);
      retval |= ((uint32_t)lbuf[2] << 8);
      retval |= lbuf[3];
    }
  }

  PM_ENABLE_LINE_CLR;
  PRINTF("PM: Timeout 0x%02x%02x%02x%02x\n", lbuf[0], lbuf[1], lbuf[2], lbuf[3]);

  retval *= PM_SOFT_SHTDN_INTERVAL;
  retval /= 100;
  return retval;
}
/* -------------------------------------------------------------------------- */
uint32_t
pm_get_num_cycles(void)
{
  uint32_t retval = 0;
  PM_ENABLE_LINE_CMD;

  if(i2c_single_send(PWR_MNGMT_ADDR, PM_GET_NUM_CYCLES) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(PWR_MNGMT_ADDR, lbuf, 4) == I2C_MASTER_ERR_NONE) {
      retval |= ((uint32_t)lbuf[0] << 24);
      retval |= ((uint32_t)lbuf[1] << 16);
      retval |= ((uint32_t)lbuf[2] << 8);
      retval |= lbuf[3];
    }
  }
  PM_ENABLE_LINE_CLR;
  PRINTF("PM: Sleep cycles: 0x%02x%02x%02x%02x\n", lbuf[0], lbuf[1], lbuf[2],
                                                   lbuf[3]);
  return retval;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_shutdown_now(uint8_t type)
{
  if(!initialized) {
    PRINTF("PM: Not initialized\n");
    return PM_ERROR;
  }

  if((type != PM_HARD_SLEEP_CONFIG) && (type != PM_SOFT_SLEEP_CONFIG)) {
    PRINTF("PM: Invalid shutdown mode type\n");
    return PM_ERROR;
  }

  PM_ENABLE_LINE_CMD;

  if(type == PM_HARD_SLEEP_CONFIG) {
    pm_write_byte(PM_HARD_SLEEP_CONFIG, PM_ENABLE);
    PM_ENABLE_LINE_CLR;
    return PM_SUCCESS;
  }

  /* Soft sleep */
  pm_write_byte(PM_SOFT_SLEEP_CONFIG, PM_ENABLE);
  PM_ENABLE_LINE_CLR;
  return PM_SUCCESS;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_get_voltage(uint16_t *state)
{
  if(!initialized) {
    return PM_ERROR;
  }

  PM_ENABLE_LINE_CMD;
  if(i2c_single_send(PWR_MNGMT_ADDR, PM_GET_VDD) == I2C_MASTER_ERR_NONE) {
    /* Read two bytes only */
    if(i2c_burst_receive(PWR_MNGMT_ADDR, lbuf, 2) == I2C_MASTER_ERR_NONE) {
      *state = (uint16_t)lbuf[0] << 8;
      *state += lbuf[1];

      /* Delay required for the command to finish */
      clock_delay_usec(3000);

      PRINTF("PM: Voltage %u [%u][%u]\n", *state, lbuf[0], lbuf[1]);
      PM_ENABLE_LINE_CLR;
      return PM_SUCCESS;
    }
  }
  PM_ENABLE_LINE_CLR;
  return PM_ERROR;
}
/* -------------------------------------------------------------------------- */
int8_t
pm_get_fw_ver(uint8_t *fwver)
{
  if((!initialized) || (fwver == NULL)) {
    return PM_ERROR;
  }

  if(pm_read_byte(PM_FW_VERSION, fwver, 1) == PM_SUCCESS) {
    return PM_SUCCESS;
  }
  return PM_ERROR;
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
