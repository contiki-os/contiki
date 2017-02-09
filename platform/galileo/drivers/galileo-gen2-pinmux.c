/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
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
#include "gpio.h"
#include "gpio-pcal9535a.h"
#include "i2c.h"
#include "pwm-pca9685.h"
#include <stdio.h>

typedef enum {
  GALILEO_PINMUX_FUNC_A,
  GALILEO_PINMUX_FUNC_B,
  GALILEO_PINMUX_FUNC_C,
  GALILEO_PINMUX_FUNC_D
} GALILEO_PINMUX_FUNC;

#define GPIO_PCAL9535A_0_I2C_ADDR 0x25
#define GPIO_PCAL9535A_1_I2C_ADDR 0x26
#define GPIO_PCAL9535A_2_I2C_ADDR 0x27
#define PWM_PCA9685_0_I2C_ADDR    0x47

#define PINMUX_NUM_FUNCS 4
#define PINMUX_NUM_PATHS 4

typedef enum {
  NONE,
  EXP0,
  EXP1,
  EXP2,
  PWM0
} MUX_CHIP;

typedef enum {
  PIN_LOW = 0x00,
  PIN_HIGH = 0x01,
  DISABLED = 0xFF
} PIN_LEVEL;

struct pin_config {
  uint8_t pin_num;
  GALILEO_PINMUX_FUNC func;
};

struct mux_pin {
  MUX_CHIP chip;
  uint8_t pin;
  PIN_LEVEL level;
  uint32_t cfg;
};

struct mux_path {
  uint8_t io_pin;
  GALILEO_PINMUX_FUNC func;
  struct mux_pin path[PINMUX_NUM_PATHS];
};

struct pinmux_internal_data {
  struct gpio_pcal9535a_data exp0;
  struct gpio_pcal9535a_data exp1;
  struct gpio_pcal9535a_data exp2;
  struct pwm_pca9685_data pwm0;
};

static struct pinmux_internal_data data;

static struct mux_path galileo_pinmux_paths[GALILEO_NUM_PINS * PINMUX_NUM_FUNCS] = {
  {0, GALILEO_PINMUX_FUNC_A, {
          { EXP1,  0,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* GPIO3 out */
          { EXP1,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {0, GALILEO_PINMUX_FUNC_B, {
          { EXP1,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO3 in */
          { EXP1,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {0, GALILEO_PINMUX_FUNC_C, {
          { EXP1,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* UART0_RXD */
          { EXP1,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {0, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {1, GALILEO_PINMUX_FUNC_A, {
          { EXP1, 13,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO4 out */
          { EXP0, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0, 13,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {1, GALILEO_PINMUX_FUNC_B, {
          { EXP1, 13,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO4 in */
          { EXP0, 12,  PIN_HIGH, (QUARKX1000_GPIO_OUT)},
          { EXP0, 13,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {1, GALILEO_PINMUX_FUNC_C, {
          { EXP1, 13,  PIN_HIGH, (QUARKX1000_GPIO_OUT)}, /* UART0_TXD */
          { EXP0, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0, 13,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {1, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {2, GALILEO_PINMUX_FUNC_A, {
          { PWM0, 13,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* GPIO5 out */
          { EXP1,  2,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP1,  3,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {2, GALILEO_PINMUX_FUNC_B, {
          { PWM0, 13,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* GPIO5 in */
          { EXP1,  2,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { EXP1,  3,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {2, GALILEO_PINMUX_FUNC_C, {
          { PWM0, 13,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* UART1_RXD */
          { EXP1,  2,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { EXP1,  3,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {2, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {3, GALILEO_PINMUX_FUNC_A, {
          { PWM0,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO6 out */
          { PWM0, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {3, GALILEO_PINMUX_FUNC_B, {
          { PWM0,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) },  /* GPIO6 in */
          { PWM0, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  0,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { EXP0,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {3, GALILEO_PINMUX_FUNC_C, {
          { PWM0,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) },  /* UART1_TXD */
          { PWM0, 12,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { EXP0,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {3, GALILEO_PINMUX_FUNC_D, {
          { PWM0,  0,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },  /* PWM.LED1 */
          { PWM0, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},

  {4, GALILEO_PINMUX_FUNC_A, {
          { EXP1,  4,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS4 out */
          { EXP1,  5,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {4, GALILEO_PINMUX_FUNC_B, {
          { EXP1,  4,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS4 in */
          { EXP1,  5,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {4, GALILEO_PINMUX_FUNC_C, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {4, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {5, GALILEO_PINMUX_FUNC_A, {
          { PWM0,  2,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO8 (out) */
          { EXP0,  2,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  3,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {5, GALILEO_PINMUX_FUNC_B, {
          { PWM0,  2,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO8 (in) */
          { EXP0,  2,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { EXP0,  3,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {5, GALILEO_PINMUX_FUNC_C, {
          { PWM0,  2,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* PWM.LED3 */
          { EXP0,  2,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  3,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {5, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {6, GALILEO_PINMUX_FUNC_A, {
          { PWM0,  4,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO9 (out) */
          { EXP0,  4,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  5,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {6, GALILEO_PINMUX_FUNC_B, {
          { PWM0,  4,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO9 (in) */
          { EXP0,  4,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { EXP0,  5,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {6, GALILEO_PINMUX_FUNC_C, {
          { PWM0,  4,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* PWM.LED5 */
          { EXP0,  4,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  5,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {6, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {7, GALILEO_PINMUX_FUNC_A, {
          { EXP1,  6,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS0 (out) */
          { EXP1,  7,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {7, GALILEO_PINMUX_FUNC_B, {
          { EXP1,  6,   PIN_LOW, (QUARKX1000_GPIO_IN ) }, /* GPIO_SUS0 (in) */
          { EXP1,  7,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {7, GALILEO_PINMUX_FUNC_C, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {7, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {8, GALILEO_PINMUX_FUNC_A, {
          { EXP1,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS1 (out) */
          { EXP1,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {8, GALILEO_PINMUX_FUNC_B, {
          { EXP1,  8,   PIN_LOW, (QUARKX1000_GPIO_IN ) }, /* GPIO_SUS1 (in) */
          { EXP1,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {8, GALILEO_PINMUX_FUNC_C, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {8, GALILEO_PINMUX_FUNC_D, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {9, GALILEO_PINMUX_FUNC_A, {
          { PWM0,  6,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS2 (out) */
          { EXP0,  6,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  7,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {9, GALILEO_PINMUX_FUNC_B, {
          { PWM0,  6,   PIN_LOW, (QUARKX1000_GPIO_OUT) },  /* GPIO_SUS2 (in) */
          { EXP0,  6,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
          { EXP0,  7,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {9, GALILEO_PINMUX_FUNC_C, {
          { PWM0,  6,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* PWM.LED7 */
          { EXP0,  6,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { EXP0,  7,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {9, GALILEO_PINMUX_FUNC_C, {
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
          { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {10, GALILEO_PINMUX_FUNC_A, {
           { PWM0, 10,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO2 (out) */
           { EXP0, 10,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {10, GALILEO_PINMUX_FUNC_B, {
           { PWM0, 10,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO2 (in) */
           { EXP0, 10,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP0, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {10, GALILEO_PINMUX_FUNC_C, {
           { PWM0, 10,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* PWM.LED11 */
           { EXP0, 10,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {10, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {11, GALILEO_PINMUX_FUNC_A, {
           { EXP1, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS3 (out) */
           { PWM0,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {11, GALILEO_PINMUX_FUNC_B, {
           { EXP1, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS3 (in) */
           { PWM0,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0,  8,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP0,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {11, GALILEO_PINMUX_FUNC_C, {
           { EXP1, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* PWM.LED9 */
           { PWM0,  8,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP0,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {11, GALILEO_PINMUX_FUNC_D, {
           { EXP1, 12,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* SPI1_MOSI */
           { PWM0,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},

  {12, GALILEO_PINMUX_FUNC_A, {
           { EXP1, 10,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO7 (out) */
           { EXP1, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {12, GALILEO_PINMUX_FUNC_B, {
           { EXP1, 10,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* GPIO7 (in) */
           { EXP1, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {12, GALILEO_PINMUX_FUNC_C, {
           { EXP1, 10,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* SPI1_MISO */
           { EXP1, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {12, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {13, GALILEO_PINMUX_FUNC_A, {
           { EXP1, 14,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* GPIO_SUS5 (out) */
           { EXP0, 14,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0, 15,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {13, GALILEO_PINMUX_FUNC_B, {
           { EXP1, 14,   PIN_LOW, (QUARKX1000_GPIO_OUT) },  /* GPIO_SUS5 (in) */
           { EXP0, 14,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP0, 15,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {13, GALILEO_PINMUX_FUNC_C, {
           { EXP1, 14,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* SPI1_CLK */
           { EXP0, 14,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP0, 15,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {13, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {14, GALILEO_PINMUX_FUNC_A, {
           { EXP2,  0,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* EXP2.P0_0 (out)/ADC.IN0 */
           { EXP2,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {14, GALILEO_PINMUX_FUNC_B, {
           { EXP2,  0,   PIN_LOW, (QUARKX1000_GPIO_IN ) }, /* EXP2.P0_0 (in)/ADC.IN0 */
           { EXP2,  1,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {14, GALILEO_PINMUX_FUNC_C, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {14, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {15, GALILEO_PINMUX_FUNC_A, {
           { EXP2,  2,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* EXP2.P0_2 (out)/ADC.IN1 */
           { EXP2,  3,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {15, GALILEO_PINMUX_FUNC_B, {
           { EXP2,  2,   PIN_LOW, (QUARKX1000_GPIO_IN ) }, /* EXP2.P0_2 (in)/ADC.IN1 */
           { EXP2,  3,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {15, GALILEO_PINMUX_FUNC_C, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {15, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {16, GALILEO_PINMUX_FUNC_A, {
           { EXP2,  4,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* EXP2.P0_4 (out)/ADC.IN2 */
           { EXP2,  5,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {16, GALILEO_PINMUX_FUNC_B, {
           { EXP2,  4,   PIN_LOW, (QUARKX1000_GPIO_IN ) }, /* EXP2.P0_4 (in)/ADC.IN2 */
           { EXP2,  5,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {16, GALILEO_PINMUX_FUNC_C, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {16, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {17, GALILEO_PINMUX_FUNC_A, {
           { EXP2,  6,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* EXP2.P0_6 (out)/ADC.IN3 */
           { EXP2,  7,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {17, GALILEO_PINMUX_FUNC_B, {
           { EXP2,  6,   PIN_LOW, (QUARKX1000_GPIO_IN ) }, /* EXP2.P0_6 (in)/ADC.IN3 */
           { EXP2,  7,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {17, GALILEO_PINMUX_FUNC_C, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {17, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {18, GALILEO_PINMUX_FUNC_A, {
           { PWM0, 14,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* EXP2.P1_0 (out)/ADC.IN4 */
           { EXP2, 12,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP2,  8,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP2,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {18, GALILEO_PINMUX_FUNC_B, {
           { PWM0, 14,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* EXP2.P1_0 (in)/ADC.IN4 */
           { EXP2, 12,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP2,  8,   PIN_LOW, (QUARKX1000_GPIO_IN ) },
           { EXP2,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {18, GALILEO_PINMUX_FUNC_C, {
           { PWM0, 14,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* I2C SDA */
           { EXP2,  9,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP2, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {18, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},

  {19, GALILEO_PINMUX_FUNC_A, {
           { PWM0, 15,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* EXP2.P1_2 (out)/ADC.IN5 */
           { EXP2, 12,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP2, 10,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP2, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {19, GALILEO_PINMUX_FUNC_B, {
           { PWM0, 15,   PIN_LOW, (QUARKX1000_GPIO_OUT) }, /* EXP2.P1_2 (in)/ADC.IN5 */
           { EXP2, 12,  PIN_HIGH, (QUARKX1000_GPIO_OUT) },
           { EXP2, 10,   PIN_LOW, (QUARKX1000_GPIO_IN ) },
           { EXP2, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) }}},
  {19, GALILEO_PINMUX_FUNC_C, {
           { PWM0, 15,  PIN_HIGH, (QUARKX1000_GPIO_OUT) }, /* I2C SCL */
           { EXP2, 11,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { EXP2, 12,   PIN_LOW, (QUARKX1000_GPIO_OUT) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
  {19, GALILEO_PINMUX_FUNC_D, {
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }, /* NONE */
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) },
           { NONE,  0,  DISABLED, (QUARKX1000_GPIO_IN ) }}},
};

static int
galileo_pinmux_set_pin(uint8_t pin, GALILEO_PINMUX_FUNC func)
{
  struct mux_path *mux_path;
  uint8_t index, i;

  if(pin >= GALILEO_NUM_PINS) {
    return -1;
  }

  index = PINMUX_NUM_FUNCS * pin;
  index += func;

  mux_path = &galileo_pinmux_paths[index];

  for(i = 0; i < PINMUX_NUM_PATHS; i++) {
    struct gpio_pcal9535a_data *exp = NULL;
    switch(mux_path->path[i].chip) {
    case EXP0:
      exp = &data.exp0;
      break;
    case EXP1:
      exp = &data.exp1;
      break;
    case EXP2:
      exp = &data.exp2;
      break;
    case PWM0:
      if(pwm_pca9685_set_duty_cycle(&data.pwm0, mux_path->path[i].pin, mux_path->path[i].level ? 100 : 0) < 0) {
        return -1;
      }
      continue;
    case NONE:
      continue;
    }

    assert(exp != NULL);

    if(gpio_pcal9535a_write(exp, mux_path->path[i].pin, mux_path->path[i].level) < 0) {
      return -1;
    }
    if(gpio_pcal9535a_config(exp, mux_path->path[i].pin, mux_path->path[i].cfg) < 0) {
      return -1;
    }
  }

  return 0;
}
static void
flatten_pin_num(galileo_pin_group_t grp, unsigned *pin)
{
  if(grp == GALILEO_PIN_GRP_ANALOG) {
    *pin += GALILEO_NUM_DIGITAL_PINS;
  }

  assert(*pin < GALILEO_NUM_PINS);
}
/* See galileo-gpio.c for the declaration of this function. */
int
galileo_brd_to_cpu_gpio_pin(unsigned pin, bool *sus)
{
  static const int SUS = 0x100;
  unsigned pins[GALILEO_NUM_DIGITAL_PINS] = {
          3,       4,       5,       6,
    SUS | 4,       8,       9, SUS | 0,
    SUS | 1, SUS | 2,       2, SUS | 3,
          7, SUS | 5
  };
  int cpu_pin;

  /* GPIOs in the analog pin space are implemented by EXP2, not the CPU. */
  assert(pin < GALILEO_NUM_DIGITAL_PINS);
  cpu_pin = pins[pin];

  *sus = (cpu_pin & SUS) == SUS;

  return cpu_pin & ~SUS;
}
void
galileo_pinmux_select_din(galileo_pin_group_t grp, unsigned pin)
{
  bool sus;
  int cpu_pin;

  flatten_pin_num(grp, &pin);

  assert(galileo_pinmux_set_pin(pin, GALILEO_PINMUX_FUNC_B) == 0);

  cpu_pin = galileo_brd_to_cpu_gpio_pin(pin, &sus);
  /* GPIO_SUS pins are currently unsupported. */
  assert(!sus);
  quarkX1000_gpio_config(cpu_pin, QUARKX1000_GPIO_IN);
}
void
galileo_pinmux_select_dout(galileo_pin_group_t grp, unsigned pin)
{
  bool sus;
  int cpu_pin;

  flatten_pin_num(grp, &pin);

  assert(galileo_pinmux_set_pin(pin, GALILEO_PINMUX_FUNC_A) == 0);

  cpu_pin = galileo_brd_to_cpu_gpio_pin(pin, &sus);
  /* GPIO_SUS pins are currently unsupported. */
  assert(!sus);
  quarkX1000_gpio_config(cpu_pin, QUARKX1000_GPIO_OUT);
}
void
galileo_pinmux_select_pwm(unsigned pin)
{
  GALILEO_PINMUX_FUNC func = GALILEO_PINMUX_FUNC_C;
  switch(pin) {
  case 3:
    func = GALILEO_PINMUX_FUNC_D;
    break;
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

  assert(galileo_pinmux_set_pin(pin, func) == 0);
}
void
galileo_pinmux_select_serial(unsigned pin)
{
  assert(pin < 4);

  assert(galileo_pinmux_set_pin(pin, GALILEO_PINMUX_FUNC_C) == 0);
}
void
galileo_pinmux_select_i2c(void)
{
  assert(galileo_pinmux_set_pin(18, GALILEO_PINMUX_FUNC_C) == 0);
  assert(galileo_pinmux_set_pin(19, GALILEO_PINMUX_FUNC_C) == 0);
}
void
galileo_pinmux_select_spi(void)
{
  assert(galileo_pinmux_set_pin(11, GALILEO_PINMUX_FUNC_D) == 0);
  assert(galileo_pinmux_set_pin(12, GALILEO_PINMUX_FUNC_C) == 0);
  assert(galileo_pinmux_set_pin(13, GALILEO_PINMUX_FUNC_C) == 0);
}
void
galileo_pinmux_select_analog(unsigned pin)
{
  assert(pin < GALILEO_NUM_ANALOG_PINS);

  pin += GALILEO_NUM_DIGITAL_PINS;

  assert(galileo_pinmux_set_pin(pin, GALILEO_PINMUX_FUNC_B) == 0);
}
int
galileo_pinmux_initialize(void)
{
  /* has to init after I2C master */
  if(!quarkX1000_i2c_is_available()) {
    return -1;
  }

  if(gpio_pcal9535a_init(&data.exp0, GPIO_PCAL9535A_0_I2C_ADDR) < 0) {
    return -1;
  }

  if(gpio_pcal9535a_init(&data.exp1, GPIO_PCAL9535A_1_I2C_ADDR) < 0) {
    return -1;
  }

  if(gpio_pcal9535a_init(&data.exp2, GPIO_PCAL9535A_2_I2C_ADDR) < 0) {
    return -1;
  }

  if(pwm_pca9685_init(&data.pwm0, PWM_PCA9685_0_I2C_ADDR) < 0) {
    return -1;
  }

  /* Activate default pinmux configuration. */
  /* Some of the following lines are commented out due to the GPIO_SUS pins
   * being currently unsupported.
   */
  galileo_pinmux_select_serial(0);
  galileo_pinmux_select_serial(1);
  galileo_pinmux_select_dout(GALILEO_PIN_GRP_DIGITAL, 2);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 3);
  /*galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 4);*/
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 5);
  galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 6);
  /*galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 7);*/
  /*galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 8);*/
  /*galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 9);*/
  galileo_pinmux_select_dout(GALILEO_PIN_GRP_DIGITAL, 10);
  /*galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 11);*/
  galileo_pinmux_select_dout(GALILEO_PIN_GRP_DIGITAL, 12);
  /*galileo_pinmux_select_din(GALILEO_PIN_GRP_DIGITAL, 13);*/
  galileo_pinmux_select_analog(0);
  galileo_pinmux_select_analog(1);
  galileo_pinmux_select_analog(2);
  galileo_pinmux_select_analog(3);
  galileo_pinmux_select_i2c();

  return 0;
}
