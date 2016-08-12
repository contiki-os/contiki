/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
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
 * \addtogroup zoul-grove-gyro-sensor
 * @{
 *
 * \file
 *         Grove's 3-axis gyroscope driver
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/i2c.h"
#include "dev/grove-gyro.h"
#include "lib/sensors.h"
#include "dev/watchdog.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define GROVE_GYRO_INT_PORT_BASE  GPIO_PORT_TO_BASE(I2C_INT_PORT)
#define GROVE_GYRO_INT_PIN_MASK   GPIO_PIN_MASK(I2C_INT_PIN)
/*---------------------------------------------------------------------------*/
static uint8_t enabled;
static uint8_t power_mgmt;
static uint8_t int_en;
/*---------------------------------------------------------------------------*/
grove_gyro_values_t gyro_values;
/*---------------------------------------------------------------------------*/
void (*grove_gyro_int_callback)(uint8_t value);
/*---------------------------------------------------------------------------*/
static uint16_t
grove_gyro_read_reg(uint8_t reg, uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    return GROVE_GYRO_ERROR;
  }

  i2c_master_enable();
  if(i2c_single_send(GROVE_GYRO_ADDR, reg) == I2C_MASTER_ERR_NONE) {
    if(i2c_burst_receive(GROVE_GYRO_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
      return GROVE_GYRO_SUCCESS;
    }
  }

  PRINTF("Gyro: failed to read from sensor\n");
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_write_reg(uint8_t *buf, uint8_t num)
{
  if((buf == NULL) || (num <= 0)) {
    PRINTF("Gyro: invalid write values\n");
    return GROVE_GYRO_ERROR;
  }

  i2c_master_enable();
  if(i2c_burst_send(GROVE_GYRO_ADDR, buf, num) == I2C_MASTER_ERR_NONE) {
    return GROVE_GYRO_SUCCESS;
  }
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_sampdiv(uint8_t value)
{
  uint8_t buf[2];
  buf[0] = GROVE_GYRO_SMPLRT_DIV;
  buf[1] = value;
  if(grove_gyro_write_reg(buf, 2) == GROVE_GYRO_SUCCESS) {
    PRINTF("Gyro: new sampdiv 0x%02X\n", value);
    return GROVE_GYRO_SUCCESS;
  }
  PRINTF("Gyro: failed to set sampdiv\n");
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static uint8_t
grove_gyro_clear_interrupt(void)
{
  uint8_t aux = 0;

  /* Clear interrupt */
  grove_gyro_read_reg(GROVE_GYRO_INT_STATUS, &aux, 1);
  
  if(aux & GROVE_GYRO_INT_STATUS_DATA_RDY_MASK) {
    return GROVE_GYRO_INT_STATUS_DATA_RDY_MASK;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_interrupt(uint8_t value)
{
  uint8_t buf[2];
  buf[0] = GROVE_GYRO_INT_CFG;
  buf[1] = value;
  if(grove_gyro_write_reg(buf, 2) == GROVE_GYRO_SUCCESS){
    PRINTF("Gyro: interrupt cfg 0x%02X\n", value);
    return GROVE_GYRO_SUCCESS;
  }
  PRINTF("Gyro: failed to change interrupt config\n");
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_reset(void)
{
  uint8_t buf[2];
  buf[0] = GROVE_GYRO_PWR_MGMT;

  /* Read the power management status as well to force sync */
  if(grove_gyro_read_reg(GROVE_GYRO_PWR_MGMT, &power_mgmt, 1) ==
    GROVE_GYRO_SUCCESS) {
    PRINTF("Gyro: current power mgmt 0x%02X\n", power_mgmt);
    buf[1] = power_mgmt + GROVE_GYRO_PWR_MGMT_RESET;
    if(grove_gyro_write_reg(buf, 2) == GROVE_GYRO_SUCCESS) {
      PRINTF("Gyro: restarted with 0x%02X, now with default values\n", buf[1]);
      return GROVE_GYRO_SUCCESS;
    }
  }
  PRINTF("Gyro: failed to restart\n");
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_osc(uint8_t value)
{
  uint8_t buf[2];
  buf[0] = GROVE_GYRO_PWR_MGMT;

  /* Read the power management status as well to force sync */
  if(grove_gyro_read_reg(GROVE_GYRO_PWR_MGMT, &power_mgmt, 1) ==
    GROVE_GYRO_SUCCESS) {
    PRINTF("Gyro: current power mgmt 0x%02X\n", power_mgmt);
    power_mgmt &= ~GROVE_GYRO_PWR_MGMT_CLK_SEL_MASK;
    buf[1] = power_mgmt + value;
    if(grove_gyro_write_reg(buf, 2) == GROVE_GYRO_SUCCESS) {
      PRINTF("Gyro: new clock source 0x%02X\n", buf[1]);
      return GROVE_GYRO_SUCCESS;
    }
  }
  PRINTF("Gyro: failed to change the clock source\n");
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_power_mgmt(uint8_t value, uint8_t type)
{
  uint8_t buf[2];
  buf[0] = GROVE_GYRO_PWR_MGMT;

  if((type != GROVE_GYRO_POWER_ON) && (type != GROVE_GYRO_POWER_OFF)) {
    PRINTF("Gyro: invalid power command type\n");
    return GROVE_GYRO_ERROR;
  }

  /* Read the power management status as well to force sync */
  if(grove_gyro_read_reg(GROVE_GYRO_PWR_MGMT, &power_mgmt, 1) ==
    GROVE_GYRO_SUCCESS) {
    PRINTF("Gyro: current power mgmt 0x%02X\n", power_mgmt);

    if(type == GROVE_GYRO_POWER_ON) {
      power_mgmt &= ~value;
    } else {
      power_mgmt |= value;
    }

    buf[1] = power_mgmt;
    if(grove_gyro_write_reg(buf, 2) == GROVE_GYRO_SUCCESS) {
      PRINTF("Gyro: new power management register value 0x%02X\n", power_mgmt);

      /* Power-up delay */
      if(type == GROVE_GYRO_POWER_ON) {
        clock_delay_usec(25000);
      }

      return GROVE_GYRO_SUCCESS;
    }
  }
  PRINTF("Gyro: power management fail\n");

  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_dlpf(uint8_t value)
{
  uint8_t buf[2];
  buf[0] = GROVE_GYRO_DLPF_FS;
  buf[1] = GROVE_GYRO_DLPF_FS_SEL + value;

  if(grove_gyro_write_reg(buf, 2) == GROVE_GYRO_SUCCESS) {
    /* Double-check */
    if(grove_gyro_read_reg(GROVE_GYRO_DLPF_FS, &buf[0], 1) ==
      GROVE_GYRO_SUCCESS) {
      if(buf[0] == buf[1]) {
        PRINTF("Gyro: updated lp/sr 0x%02X\n", buf[0]);
        return GROVE_GYRO_SUCCESS;
      } else {
        PRINTF("Gyro: DLPF register value mismatch\n");
        return GROVE_GYRO_ERROR;
      }
    }
  }

  PRINTF("Gyro: failed to change the lp/sr\n");
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static uint16_t
grove_gyro_convert_to_value(uint16_t val)
{
  uint32_t aux;

  /* Convert from 2C's to 10's, as we care about º/s negative quantifier doesn't
   * matter, so we ommit flaging the sign
   */
  if(val & 0x8000) {
    val = (~val + 1);
  }

  /* ITG-3200 datasheet: sensitivity 14.375 LSB/(º/s) to get º/s */
  aux = val * 6956;
  aux /= 1000;

  return (uint16_t)aux;
}
/*---------------------------------------------------------------------------*/
static void
grove_gyro_convert(uint8_t *buf, uint8_t type)
{
  uint16_t aux;

  if(type & GROVE_GYRO_X) {
    aux = (buf[0] << 8) + buf[1];
    PRINTF("Gyro: X_axis (raw) 0x%02X\n", aux);
    gyro_values.x = grove_gyro_convert_to_value(aux);
  }

  if(type & GROVE_GYRO_Y) {
    aux = (buf[2] << 8) + buf[3];
    PRINTF("Gyro: Y_axis (raw) 0x%02X\n", aux);
    gyro_values.y = grove_gyro_convert_to_value(aux);
  }

  if(type & GROVE_GYRO_Z) {
    aux = (buf[4] << 8) + buf[5];
    PRINTF("Gyro: Z_axis (raw) 0x%02X\n", aux);
    gyro_values.z = grove_gyro_convert_to_value(aux);
  }

  if(type == GROVE_GYRO_TEMP) {
    aux = (buf[0] << 8) + buf[1];
    PRINTF("Gyro: Temp (raw) 0x%02X\n", aux);
    /* ITG-3200 datasheet: offset -13200, sensitivity 280 LSB/ºC */
    aux = (aux + 13200) / 28;
    aux += 350;
    gyro_values.temp = (int16_t)aux;
  }
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_read(int type)
{
  uint8_t reg;
  uint8_t len;
  uint8_t buf_ptr;
  uint8_t buf[GROVE_GYRO_MAX_DATA];

  len = (type == GROVE_GYRO_XYZ) ? GROVE_GYRO_MAX_DATA : 2;

  switch(type) {
  case GROVE_GYRO_X:
  case GROVE_GYRO_XYZ:
    buf_ptr = 0;
    reg = GROVE_GYRO_XOUT_H;
    break;
  case GROVE_GYRO_Y:
    buf_ptr = 2;
    reg = GROVE_GYRO_YOUT_H;
    break;
  case GROVE_GYRO_Z:
    buf_ptr = 4;
    reg = GROVE_GYRO_ZOUT_H;
    break;
  case GROVE_GYRO_TEMP:
    buf_ptr = 0;
    reg = GROVE_GYRO_TEMP_OUT_H;
    break;
  case GROVE_GYRO_ADDR:
    buf_ptr = 0;
    len = 1;
    reg = GROVE_GYRO_WHO_AM_I;
    break;
  default:
    PRINTF("Gyro: invalid value requested\n");
    return GROVE_GYRO_ERROR;
  }

  if(grove_gyro_read_reg(reg, &buf[buf_ptr], len) == GROVE_GYRO_SUCCESS) {
    if(type == GROVE_GYRO_ADDR) {
      PRINTF("Gyro: I2C_addr 0x%02X\n", buf[0]);
      return buf[0];
    }
    grove_gyro_convert(buf, type);
    return GROVE_GYRO_SUCCESS;
  }

  PRINTF("Gyro: failed to change the lp/sr\n");
  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
static int
grove_gyro_calibrate(void)
{
  uint8_t i;
  uint8_t buf[GROVE_GYRO_MAX_DATA];
  uint8_t power_mgmt_backup;
  uint32_t x, y, z;

  /* Disable interrupts */
  if(int_en) {
    if(grove_gyro_interrupt(GROVE_GYRO_INT_CFG_DISABLE) == GROVE_GYRO_ERROR) {
      PRINTF("Gyro: failed to disable the interrupts\n");
      return GROVE_GYRO_ERROR;
    }
    GPIO_DISABLE_INTERRUPT(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
  }

  /* Turn on the 3-axis, save the current config */
  if(grove_gyro_read_reg(GROVE_GYRO_PWR_MGMT, &power_mgmt_backup, 1) ==
    GROVE_GYRO_ERROR) {
    PRINTF("Gyro: failed to read power mgmt config\n");
    return GROVE_GYRO_ERROR;
  }

  if(grove_gyro_power_mgmt(GROVE_GYRO_ALL, GROVE_GYRO_POWER_ON) ==
    GROVE_GYRO_ERROR) {
    PRINTF("Gyro: failed to bring sensor up\n");
    return GROVE_GYRO_ERROR;
  }

  x = 0;
  y = 0;
  z = 0;

  for (i = 0; i < GROVE_GYRO_CALIB_SAMPLES; i++){
    clock_delay_usec(GROVE_GYRO_CALIB_TIME_US);
    watchdog_periodic();
    if(grove_gyro_read_reg(GROVE_GYRO_XOUT_H, buf, GROVE_GYRO_MAX_DATA) ==
      GROVE_GYRO_SUCCESS) {
      x += (buf[0] << 8) + buf[1];
      y += (buf[2] << 8) + buf[3];
      z += (buf[4] << 8) + buf[5];
    }
  }

  gyro_values.x_offset = ABS(x)/GROVE_GYRO_CALIB_SAMPLES;
  gyro_values.y_offset = ABS(y)/GROVE_GYRO_CALIB_SAMPLES;
  gyro_values.z_offset = ABS(z)/GROVE_GYRO_CALIB_SAMPLES;

  PRINTF("Gyro: x_offset (RAW) 0x%02X\n", gyro_values.x_offset);
  PRINTF("Gyro: y_offset (RAW) 0x%02X\n", gyro_values.y_offset);
  PRINTF("Gyro: z_offset (RAW) 0x%02X\n", gyro_values.z_offset);

  gyro_values.x_offset = grove_gyro_convert_to_value(gyro_values.x_offset);
  gyro_values.y_offset = grove_gyro_convert_to_value(gyro_values.y_offset);
  gyro_values.z_offset = grove_gyro_convert_to_value(gyro_values.z_offset);

  PRINTF("Gyro: x_offset (converted) %d\n", gyro_values.x_offset);
  PRINTF("Gyro: y_offset (converted) %d\n", gyro_values.y_offset);
  PRINTF("Gyro: z_offset (converted) %d\n", gyro_values.z_offset);

  /* Cleaning up */
  buf[0] = GROVE_GYRO_PWR_MGMT;
  buf[1] = power_mgmt_backup;

  if(grove_gyro_write_reg(&buf[0], 2) != GROVE_GYRO_SUCCESS) {
    PRINTF("Gyro: failed restoring power mgmt (0x%02X)\n", power_mgmt_backup);
    return GROVE_GYRO_ERROR;
  }

  if(int_en) {
    if(grove_gyro_interrupt(GROVE_GYRO_INT_CFG_RAW_READY_EN +
                            GROVE_GYRO_INT_CFG_LATCH_EN) == GROVE_GYRO_ERROR) {
      PRINTF("Gyro: failed to enable the interrupt\n");
      return GROVE_GYRO_ERROR;
    }

    GPIO_ENABLE_INTERRUPT(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
  }

  return GROVE_GYRO_SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS(grove_gyro_int_process, "Grove gyroscope interrupt process handler");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(grove_gyro_int_process, ev, data)
{
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();

  static uint8_t axis_to_read = 0;

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    if(grove_gyro_clear_interrupt() == GROVE_GYRO_INT_STATUS_DATA_RDY_MASK) {

      axis_to_read += (power_mgmt & GROVE_GYRO_X) ? 0: GROVE_GYRO_X;
      axis_to_read += (power_mgmt & GROVE_GYRO_Y) ? 0: GROVE_GYRO_Y;
      axis_to_read += (power_mgmt & GROVE_GYRO_Z) ? 0: GROVE_GYRO_Z;

      if(grove_gyro_read(axis_to_read) == GROVE_GYRO_SUCCESS) {
        grove_gyro_int_callback(GROVE_GYRO_SUCCESS);
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
grove_gyro_interrupt_handler(uint8_t port, uint8_t pin)
{
  process_poll(&grove_gyro_int_process);
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  if(!enabled) {
    PRINTF("Gyro: sensor not started\n");
    return GROVE_GYRO_ERROR;
  }

  if((type != GROVE_GYRO_X) && (type != GROVE_GYRO_Y) &&
    (type != GROVE_GYRO_Z) && (type != GROVE_GYRO_XYZ) &&
    (type != GROVE_GYRO_TEMP) && (type != GROVE_GYRO_ADDR)) {
    PRINTF("Gyro: invalid value requested 0x%02X\n", type);
    return GROVE_GYRO_ERROR;
  }

  if((type != GROVE_GYRO_TEMP) && (type != GROVE_GYRO_ADDR) &&
    (type & power_mgmt)) {
    PRINTF("Gyro: axis not enabled (0x%02X vs 0x%02X)\n", power_mgmt, type);
    return GROVE_GYRO_ERROR;
  }

  return grove_gyro_read(type);
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  if((type != GROVE_GYRO_ACTIVE) && (type != GROVE_GYRO_SAMPLE_RATE) &&
    (type != GROVE_GYRO_SAMPLE_RATE_DIVIDER) && (type != GROVE_GYRO_POWER_ON) &&
    (type != GROVE_GYRO_POWER_OFF) && (type != GROVE_GYRO_DATA_INTERRUPT) &&
    (type != GROVE_GYRO_CALIBRATE_ZERO)) {
    PRINTF("Gyro: option not supported\n");
    return GROVE_GYRO_ERROR;
  }

  switch(type) {
  case GROVE_GYRO_ACTIVE:
    if(value) {
      i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN,
               I2C_SCL_FAST_BUS_SPEED);

       /* Initialize the data structure values */
       gyro_values.x = 0;
       gyro_values.y = 0;
       gyro_values.z = 0;
       gyro_values.temp = 0;
       gyro_values.x_offset = 0;
       gyro_values.y_offset = 0;
       gyro_values.z_offset = 0;

       /* Make sure the sensor is on */
       if(grove_gyro_power_mgmt(GROVE_GYRO_ALL, GROVE_GYRO_POWER_ON) !=
         GROVE_GYRO_SUCCESS) {
         PRINTF("Gyro: failed to power on the sensor\n");
         return GROVE_GYRO_ERROR;
       }

      /* Reset and configure as default with internal oscillator, 8KHz @ 2000
       * degrees/s, no divider (full scale)
       */
      if(grove_gyro_reset() == GROVE_GYRO_SUCCESS) {
        if(grove_gyro_osc(GROVE_GYRO_DEFAULT_OSC) == GROVE_GYRO_SUCCESS) {
          if(grove_gyro_dlpf(GROVE_GYRO_DLPF_FS_CGF_8KHZ_LP256HZ) ==
            GROVE_GYRO_SUCCESS) {
            PRINTF("Gyro: started and configured\n");
            /* Disable interrupts as default */
            if(grove_gyro_interrupt(GROVE_GYRO_INT_CFG_DISABLE) ==
              GROVE_GYRO_SUCCESS) {
              PRINTF("Gyro: interrupts disabled\n");
              /* And finally put the device in SLEEP mode, set also X, Y and Z
               * in stand-by mode, whenever an axis is not used it should stay
               * in this state to save power
               */
              if(grove_gyro_power_mgmt(GROVE_GYRO_ALL, GROVE_GYRO_POWER_OFF) ==
                GROVE_GYRO_SUCCESS) {
                enabled = 1;
                PRINTF("Gyro: axis and gyroscope in low-power mode now\n");

                return GROVE_GYRO_SUCCESS;
              }
            }
          }
        }
      }
      return GROVE_GYRO_ERROR;

    } else {
      enabled = 0;
      int_en = 0;
      GPIO_DISABLE_INTERRUPT(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
      grove_gyro_int_callback = NULL;
      if(grove_gyro_interrupt(GROVE_GYRO_INT_CFG_DISABLE) ==
        GROVE_GYRO_SUCCESS) {
        return grove_gyro_power_mgmt(GROVE_GYRO_ALL, GROVE_GYRO_POWER_OFF);
      }
      PRINTF("Gyro: hw interrupt disabled but failed to disable sensor\n");
      return GROVE_GYRO_ERROR;
    }

  if(!enabled) {
    PRINTF("Gyro: sensor not started\n");
    return GROVE_GYRO_ERROR;
  }

  case GROVE_GYRO_DATA_INTERRUPT:

    if(!value) {

      /* Ensure the GPIO doesn't generate more interrupts, this may affect others
       * I2C digital sensors using the bus and sharing this pin, so an user may
       * comment the line below
       */
      int_en = 0;
      GPIO_DISABLE_INTERRUPT(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
      return grove_gyro_interrupt(GROVE_GYRO_INT_CFG_DISABLE);
    }

    /* Enable interrupt and latch the pin until cleared */
    if(grove_gyro_interrupt(GROVE_GYRO_INT_CFG_RAW_READY_EN +
                            GROVE_GYRO_INT_CFG_LATCH_EN) == GROVE_GYRO_ERROR) {
      PRINTF("Gyro: failed to enable the interrupt\n");
      return GROVE_GYRO_ERROR;
    }

    /* Default register configuration is active high, push-pull */
    GPIO_SOFTWARE_CONTROL(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
    GPIO_SET_INPUT(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
    GPIO_DETECT_EDGE(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
    GPIO_TRIGGER_SINGLE_EDGE(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
    GPIO_DETECT_FALLING(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
    gpio_register_callback(grove_gyro_interrupt_handler, I2C_INT_PORT,
                           I2C_INT_PIN);

    /* Spin process until an interrupt is received */
    process_start(&grove_gyro_int_process, NULL);

    /* Enable interrupts */
    int_en = 1;
    GPIO_ENABLE_INTERRUPT(GROVE_GYRO_INT_PORT_BASE, GROVE_GYRO_INT_PIN_MASK);
    ioc_set_over(I2C_INT_PORT, I2C_INT_PIN, IOC_OVERRIDE_PUE);
    nvic_interrupt_enable(I2C_INT_VECTOR);

    PRINTF("Gyro: Data interrupt configured\n");
    return GROVE_GYRO_SUCCESS;

  case GROVE_GYRO_SAMPLE_RATE:
    if((value < GROVE_GYRO_DLPF_FS_CGF_8KHZ_LP256HZ) ||
      (value > GROVE_GYRO_DLPF_FS_CGF_1KHZ_LP5HZ)) {
      PRINTF("Gyro: invalid sample rate/filter configuration\n");
      return GROVE_GYRO_ERROR;
    }
    return grove_gyro_dlpf(value);

  case GROVE_GYRO_SAMPLE_RATE_DIVIDER:
    if((value < 0) && (value > 0xFF)) {
      PRINTF("Gyro: invalid sampling rate div, it must be an 8-bit value\n");
      return GROVE_GYRO_ERROR;
    }
    return grove_gyro_sampdiv((uint8_t)value);

  case GROVE_GYRO_POWER_ON:
  case GROVE_GYRO_POWER_OFF:
    /* We accept mask values to enable more than one axis at the same time */
    if((value < GROVE_GYRO_Z) || (value > GROVE_GYRO_ALL)) {
      PRINTF("Gyro: invalid power management setting\n");
      return GROVE_GYRO_ERROR;
    }
    return grove_gyro_power_mgmt(value, type);

  case GROVE_GYRO_CALIBRATE_ZERO:
    return grove_gyro_calibrate();

  default:
    return GROVE_GYRO_ERROR;
  }

  return GROVE_GYRO_ERROR;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(grove_gyro, GROVE_GYRO_STRING, value, configure, NULL);
/*---------------------------------------------------------------------------*/
/** @} */
