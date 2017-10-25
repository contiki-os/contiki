/*
 * Copyright (C) 2016, Intel Corporation. All rights reserved.
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

#include "galileo-pinmux.h"

#include <assert.h>
#include "cy8c9540a.h"
#include "gpio.h"
#include <stdio.h>

static cy8c9540a_bit_addr_t mux_bit_addrs[] = {
  { 3, 4 }, /* IO0 */
  { 3, 5 }, /* IO1 */
  { 1, 7 }, /* IO2 */
  { 1, 6 }, /* IO3 */
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 }, /* IO4 */
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 }, /* IO5 */
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 }, /* IO6 */
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 }, /* IO7 */
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 }, /* IO8 */
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 }, /* IO9 */
  { 3, 6 }, /* IO10 */
  { 3, 7 }, /* IO11 */
  { 5, 2 }, /* IO12 */
  { 5, 3 }, /* IO13 */
  { 3, 1 }, /* A0 */
  { 3, 0 }, /* A1 */
  { 0, 7 }, /* A2 */
  { 0, 6 }, /* A3 */
  { 0, 5 }, /* A4 (also controlled by I2C mux) */
  { 0, 4 }, /* A5 (also controlled by I2C mux) */
};

static cy8c9540a_bit_addr_t i2c_mux_bit_addr = { 1, 5 };

/*---------------------------------------------------------------------------*/
static void
flatten_pin_num(galileo_pin_group_t grp, unsigned *pin)
{
  if(grp == GALILEO_PIN_GRP_ANALOG) {
    *pin += GALILEO_NUM_DIGITAL_PINS;
  }

  assert(*pin < GALILEO_NUM_PINS);
}
/*---------------------------------------------------------------------------*/
/* See galileo-gpio.c for the declaration of this function. */
int
galileo_brd_to_cpu_gpio_pin(unsigned pin, bool *sus)
{
  assert(pin < GALILEO_NUM_PINS);
  *sus = false;
  switch(pin) {
  case 2:
    return 6;
  case 3:
    return 7;
  case 10:
    return 2;
  default:
    return -1; /* GPIO pin may be connected to the CY8C9540A chip, but not the
                  CPU. */
  }
}
/*---------------------------------------------------------------------------*/
static cy8c9540a_bit_addr_t cy8c9540a_gpio_mapping[] = {
  { 4, 6 },
  { 4, 7 },
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 },
  { CY8C9540A_BIT_ADDR_INVALID_PORT, 0 },
  { 1, 4 },
  { 0, 1 },
  { 1, 0 },
  { 1, 3 },
  { 1, 2 },
  { 0, 3 },
  { 0, 0 }, /* This driver configures IO10 to connect to CPU GPIO when setting
               IO10 to a digital mode, but hardware exists to alternately
               connect it to this pin of the CY8C9540A chip. */
  { 1, 1 },
  { 3, 2 },
  { 3, 3 },
  { 4, 0 },
  { 4, 1 },
  { 4, 2 },
  { 4, 3 },
  { 4, 4 },
  { 4, 5 }
};
/* Map a board-level GPIO pin number to the address of the CY8C9540A pin that
 * implements it.
 */
cy8c9540a_bit_addr_t
galileo_brd_to_cy8c9540a_gpio_pin(unsigned pin)
{
  assert(pin < GALILEO_NUM_PINS);
  return cy8c9540a_gpio_mapping[pin];
}
/*---------------------------------------------------------------------------*/
/* The I2C mux control must be set high to be able to access A4 and A5.
 */
static void
set_i2c_mux(bool val)
{
  cy8c9540a_write(i2c_mux_bit_addr, val);
}
/*---------------------------------------------------------------------------*/
static void
select_gpio_pwm(unsigned flat_pin, bool pwm)
{
  bool mux_val;
  cy8c9540a_bit_addr_t mux_bit_addr;
  mux_bit_addr = mux_bit_addrs[flat_pin];
  if(mux_bit_addr.port != CY8C9540A_BIT_ADDR_INVALID_PORT) {
    mux_val = pwm || !(flat_pin == 2 || flat_pin == 3 || flat_pin == 10);
    cy8c9540a_write(mux_bit_addr, mux_val);
  }
  if((GALILEO_NUM_DIGITAL_PINS + 4) <= flat_pin) {
    /* This single control switches away from both I2C pins. */
    set_i2c_mux(true);
  }
}
/*---------------------------------------------------------------------------*/
static void
select_gpio(galileo_pin_group_t grp, unsigned pin, bool out)
{
  bool sus;
  int cpu_pin;
  cy8c9540a_bit_addr_t gpio_bit_addr;

  flatten_pin_num(grp, &pin);
  select_gpio_pwm(pin, false);

  cpu_pin = galileo_brd_to_cpu_gpio_pin(pin, &sus);
  if(cpu_pin == -1) {
    gpio_bit_addr = galileo_brd_to_cy8c9540a_gpio_pin(pin);
    cy8c9540a_set_port_dir(gpio_bit_addr,
                           out?
                             CY8C9540A_PORT_DIR_OUT :
                             CY8C9540A_PORT_DIR_IN);
    cy8c9540a_set_drive_mode(gpio_bit_addr,
                             out?
                               CY8C9540A_DRIVE_STRONG :
                               CY8C9540A_DRIVE_HI_Z);
  } else {
    quarkX1000_gpio_config(cpu_pin,
                           out? QUARKX1000_GPIO_OUT : QUARKX1000_GPIO_IN);
  }
}
/*---------------------------------------------------------------------------*/
void
galileo_pinmux_select_din(galileo_pin_group_t grp, unsigned pin)
{
  select_gpio(grp, pin, false);
}
/*---------------------------------------------------------------------------*/
void galileo_pinmux_select_dout(galileo_pin_group_t grp, unsigned pin)
{
  select_gpio(grp, pin, true);
}
/*---------------------------------------------------------------------------*/
void galileo_pinmux_select_pwm(unsigned pin)
{
  switch(pin) {
  case 3:
  case 5:
  case 6:
  case 9:
  case 10:
  case 11:
    break;
  default:
    fprintf(stderr, "%s: invalid pin: %d.\n", __FUNCTION__, pin);
    halt();
  }

  select_gpio_pwm(pin, true);
}
/*---------------------------------------------------------------------------*/
void galileo_pinmux_select_serial(unsigned pin)
{
  assert(pin == 0 || pin == 1);

  cy8c9540a_write(mux_bit_addrs[pin], false);
}
/*---------------------------------------------------------------------------*/
void galileo_pinmux_select_i2c(void)
{
  set_i2c_mux(false);
}
/*---------------------------------------------------------------------------*/
void galileo_pinmux_select_spi(void)
{
  unsigned pin;
  for(pin = 11; pin <= 13; pin++) {
    cy8c9540a_write(mux_bit_addrs[pin], false);
  }
}
/*---------------------------------------------------------------------------*/
void galileo_pinmux_select_analog(unsigned pin)
{
  assert(pin < GALILEO_NUM_ANALOG_PINS);
  pin += GALILEO_NUM_DIGITAL_PINS;

  cy8c9540a_write(mux_bit_addrs[pin], false);

  if(4 <= pin) {
    /* This single control switches away from both I2C pins. */
    set_i2c_mux(true);
  }
}
/*---------------------------------------------------------------------------*/
int
galileo_pinmux_initialize(void)
{
  int i;

  cy8c9540a_init();

  /* Configure all mux control pins as outputs. */
  for(i = 0; i < GALILEO_NUM_PINS; i++) {
    if(mux_bit_addrs[i].port == CY8C9540A_BIT_ADDR_INVALID_PORT) {
      continue;
    }

    cy8c9540a_set_port_dir(mux_bit_addrs[i], CY8C9540A_PORT_DIR_OUT);
    cy8c9540a_set_drive_mode(mux_bit_addrs[i], CY8C9540A_DRIVE_STRONG);
  }
  cy8c9540a_set_port_dir(i2c_mux_bit_addr, CY8C9540A_PORT_DIR_OUT);
  cy8c9540a_set_drive_mode(i2c_mux_bit_addr, CY8C9540A_DRIVE_STRONG);

  /* Activate default pinmux configuration. */
  galileo_pinmux_select_serial(0);
  galileo_pinmux_select_serial(1);
  galileo_pinmux_select_dout(GALILEO_PIN_GRP_DIGITAL, 2);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 3);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 4);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 5);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 6);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 7);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 8);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 9);
  galileo_pinmux_select_dout(GALILEO_PIN_GRP_DIGITAL, 10);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 11);
  galileo_pinmux_select_dout(GALILEO_PIN_GRP_DIGITAL, 12);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 13);
  galileo_pinmux_select_analog(0);
  galileo_pinmux_select_analog(1);
  galileo_pinmux_select_analog(2);
  galileo_pinmux_select_analog(3);
  galileo_pinmux_select_i2c();

  return 0;
}
/*---------------------------------------------------------------------------*/
