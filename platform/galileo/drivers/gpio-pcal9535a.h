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

#ifndef CPU_X86_DRIVERS_GPIO_PCAL9535A_H_
#define CPU_X86_DRIVERS_GPIO_PCAL9535A_H_

#include <stdint.h>

union gpio_pcal9535a_port_data {
  uint16_t all;
  uint8_t port[2];
  uint8_t byte[2];
};

struct gpio_pcal9535a_data {
  uint16_t i2c_slave_addr;
  uint32_t out_pol_inv;

  struct {
    union gpio_pcal9535a_port_data output;
    union gpio_pcal9535a_port_data pol_inv;
    union gpio_pcal9535a_port_data dir;
    union gpio_pcal9535a_port_data pud_en;
    union gpio_pcal9535a_port_data pud_sel;
  } reg_cache;
};

int gpio_pcal9535a_init(struct gpio_pcal9535a_data *data, uint16_t i2c_slave_addr);

int gpio_pcal9535a_config(struct gpio_pcal9535a_data *data, uint32_t pin, int flags);
int gpio_pcal9535a_read(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t *value);
int gpio_pcal9535a_write(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t value);

int gpio_pcal9535a_config_port(struct gpio_pcal9535a_data *data, uint32_t pin, int flags);
int gpio_pcal9535a_read_port(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t *value);
int gpio_pcal9535a_write_port(struct gpio_pcal9535a_data *data, uint32_t pin, uint32_t value);

#endif /* CPU_X86_DRIVERS_GPIO_PCAL9535A_H_ */
