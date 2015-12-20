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

#include "contiki.h"
#include "gpio.h"
#include "gpio-pcal9535a.h"
#include "i2c.h"
#include "stdio.h"

#define REG_INPUT_PORT0              0x00
#define REG_INPUT_PORT1              0x01
#define REG_OUTPUT_PORT0             0x02
#define REG_OUTPUT_PORT1             0x03
#define REG_POL_INV_PORT0            0x04
#define REG_POL_INV_PORT1            0x05
#define REG_CONF_PORT0               0x06
#define REG_CONG_PORT1               0x07
#define REG_OUT_DRV_STRENGTH_PORT0_L 0x40
#define REG_OUT_DRV_STRENGTH_PORT0_H 0x41
#define REG_OUT_DRV_STRENGTH_PORT1_L 0x42
#define REG_OUT_DRV_STRENGTH_PORT1_H 0x43
#define REG_INPUT_LATCH_PORT0        0x44
#define REG_INPUT_LATCH_PORT1        0x45
#define REG_PUD_EN_PORT0             0x46
#define REG_PUD_EN_PORT1             0x47
#define REG_PUD_SEL_PORT0            0x48
#define REG_PUD_SEL_PORT1            0x49
#define REG_INT_MASK_PORT0           0x4A
#define REG_INT_MASK_PORT1           0x4B
#define REG_INT_STATUS_PORT0         0x4C
#define REG_INT_STATUS_PORT1         0x4D
#define REG_OUTPUT_PORT_CONF         0x4F

#define READ_PORT_TIMEOUT (CLOCK_SECOND / 100)
#define READ_PORT_TRIES 5

static int
read_port_regs(struct gpio_pcal9535a_data *data, uint8_t reg, union gpio_pcal9535a_port_data *buf)
{
  int r;
  uint8_t tries = READ_PORT_TRIES;

  buf->byte[0] = reg;
  buf->byte[1] = 0;

  if(quarkX1000_i2c_write(buf->byte, 1, data->i2c_slave_addr) < 0) {
    return -1;
  }

  do {
    clock_wait(READ_PORT_TIMEOUT);

    r = quarkX1000_i2c_read(buf->byte, 2, data->i2c_slave_addr);
    if(r == 0) {
      break;
    }
  } while(tries--);

  if(r < 0) {
    return -1;
  }

  return 0;
}
static int
write_port_regs(struct gpio_pcal9535a_data *data, uint8_t reg, union gpio_pcal9535a_port_data *buf)
{
  uint8_t cmd[] = { reg, buf->byte[0], buf->byte[1] };

  if(quarkX1000_i2c_polling_write(cmd, sizeof(cmd), data->i2c_slave_addr) < 0) {
    return -1;
  }

  return 0;
}
static int
setup_pin_dir(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  union gpio_pcal9535a_port_data *port = &data->reg_cache.dir;
  uint16_t bit_mask, new_value = 0;

  bit_mask = 1 << pin;

  if((flags & QUARKX1000_GPIO_DIR_MASK) == QUARKX1000_GPIO_IN) {
    new_value = 1 << pin;
  }

  port->all &= ~bit_mask;
  port->all |= new_value;

  return write_port_regs(data, REG_CONF_PORT0, port);
}
static int
setup_pin_pullupdown(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  union gpio_pcal9535a_port_data *port;
  uint16_t bit_mask, new_value = 0;

  if((flags & QUARKX1000_GPIO_PUD_MASK) != QUARKX1000_GPIO_PUD_NORMAL) {
    port = &data->reg_cache.pud_sel;
    bit_mask = 1 << pin;

    if((flags & QUARKX1000_GPIO_PUD_MASK) == QUARKX1000_GPIO_PUD_PULL_UP) {
      new_value = 1 << pin;
    }

    port->all &= ~bit_mask;
    port->all |= new_value;

    if(write_port_regs(data, REG_PUD_SEL_PORT0, port) < 0) {
      return -1;
    }
  }

  port = &data->reg_cache.pud_en;
  bit_mask = 1 << pin;

  if((flags & QUARKX1000_GPIO_PUD_MASK) != QUARKX1000_GPIO_PUD_NORMAL) {
    new_value = 1 << pin;
  }

  port->all &= ~bit_mask;
  port->all |= new_value;

  return write_port_regs(data, REG_PUD_EN_PORT0, port);
}
static int
setup_pin_polarity(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  union gpio_pcal9535a_port_data *port = &data->reg_cache.pol_inv;
  uint16_t bit_mask, new_value = 0;

  bit_mask = 1 << pin;

  if((flags & QUARKX1000_GPIO_POL_MASK) == QUARKX1000_GPIO_POL_INV) {
    new_value = 1 << pin;
  }

  port->all &= ~bit_mask;
  port->all |= new_value;

  if(write_port_regs(data, REG_POL_INV_PORT0, port) < 0) {
    return -1;
  }

  data->out_pol_inv = port->all;

  return 0;
}
int
gpio_pcal9535a_write(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t value)
{
  union gpio_pcal9535a_port_data *port = &data->reg_cache.output;
  uint16_t bit_mask, new_value;

  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  bit_mask = 1 << pin;

  new_value = (value << pin) & bit_mask;
  new_value ^= (data->out_pol_inv & bit_mask);
  new_value &= bit_mask;

  port->all &= ~bit_mask;
  port->all |= new_value;

  return write_port_regs(data, REG_OUTPUT_PORT0, port);
}
int
gpio_pcal9535a_read(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t *value)
{
  union gpio_pcal9535a_port_data buf;

  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  if(read_port_regs(data, REG_INPUT_PORT0, &buf) < 0) {
    return -1;
  }

  *value = (buf.all >> pin) & 0x01;

  return 0;
}
int
gpio_pcal9535a_config(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  if(setup_pin_dir(data, pin, flags) < 0) {
    return -1;
  }

  if(setup_pin_polarity(data, pin, flags) < 0) {
    return -1;
  }

  if(setup_pin_pullupdown(data, pin, flags) < 0) {
    return -1;
  }

  return 0;
}
static int
setup_port_dir(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  union gpio_pcal9535a_port_data *port = &data->reg_cache.dir;

  port->all = ((flags & QUARKX1000_GPIO_DIR_MASK) == QUARKX1000_GPIO_IN) ? 0xFFFF : 0x0;

  return write_port_regs(data, REG_CONF_PORT0, port);
}
static int
setup_port_pullupdown(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  union gpio_pcal9535a_port_data *port;

  if((flags & QUARKX1000_GPIO_PUD_MASK) != QUARKX1000_GPIO_PUD_NORMAL) {
    port = &data->reg_cache.pud_sel;
    port->all = ((flags & QUARKX1000_GPIO_PUD_MASK) == QUARKX1000_GPIO_PUD_PULL_UP) ? 0xFFFF : 0x0;

    if(write_port_regs(data, REG_PUD_SEL_PORT0, port) < 0) {
      return -1;
    }
  }

  port = &data->reg_cache.pud_en;
  port->all = ((flags & QUARKX1000_GPIO_PUD_MASK) != QUARKX1000_GPIO_PUD_NORMAL) ? 0xFFFF : 0x0;

  return write_port_regs(data, REG_PUD_EN_PORT0, port);
}
static int
setup_port_polarity(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  union gpio_pcal9535a_port_data *port = &data->reg_cache.pol_inv;

  port->all = ((flags & QUARKX1000_GPIO_POL_MASK) == QUARKX1000_GPIO_POL_INV) ? 0xFFFF : 0x0;

  if(write_port_regs(data, REG_POL_INV_PORT0, port) < 0) {
    return -1;
  }

  data->out_pol_inv = port->all;

  return 0;
}
int
gpio_pcal9535a_write_port(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t value)
{
  union gpio_pcal9535a_port_data *port = &data->reg_cache.output;
  uint16_t bit_mask, new_value;

  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  port->all = value;
  bit_mask = data->out_pol_inv;

  new_value = value & bit_mask;
  new_value ^= data->out_pol_inv;
  new_value &= bit_mask;

  port->all &= ~bit_mask;
  port->all |= new_value;

  return write_port_regs(data, REG_OUTPUT_PORT0, port);
}
int
gpio_pcal9535a_read_port(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t *value)
{
  union gpio_pcal9535a_port_data buf;

  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  if(read_port_regs(data, REG_INPUT_PORT0, &buf) < 0) {
    return -1;
  }

  *value = buf.all;

  return 0;
}
int
gpio_pcal9535a_config_port(struct gpio_pcal9535a_data *data, uint32_t pin, int flags)
{
  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  if(setup_port_dir(data, pin, flags) < 0) {
    return -1;
  }

  if(setup_port_polarity(data, pin, flags) < 0) {
    return -1;
  }

  if(setup_port_pullupdown(data, pin, flags) < 0) {
    return -1;
  }

  return 0;
}
int
gpio_pcal9535a_init(struct gpio_pcal9535a_data *data, uint16_t i2c_slave_addr)
{
  /* has to init after I2C master */
  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  data->i2c_slave_addr = i2c_slave_addr;

  /* default for registers according to datasheet */
  data->reg_cache.output.all = 0xFFFF;
  data->reg_cache.pol_inv.all = 0x0;
  data->reg_cache.dir.all = 0xFFFF;
  data->reg_cache.pud_en.all = 0x0;
  data->reg_cache.pud_sel.all = 0xFFFF;

  return 0;
}
