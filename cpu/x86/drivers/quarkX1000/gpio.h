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

#ifndef CPU_X86_DRIVERS_QUARKX1000_GPIO_H_
#define CPU_X86_DRIVERS_QUARKX1000_GPIO_H_

#include <stdint.h>

#include "pci.h"

#define QUARKX1000_GPIO_IN            (0 << 0)
#define QUARKX1000_GPIO_OUT           (1 << 0)
#define QUARKX1000_GPIO_INT           (1 << 1)
#define QUARKX1000_GPIO_ACTIVE_LOW    (0 << 2)
#define QUARKX1000_GPIO_ACTIVE_HIGH   (1 << 2)
#define QUARKX1000_GPIO_LEVEL         (0 << 3)
#define QUARKX1000_GPIO_EDGE          (1 << 3)
#define QUARKX1000_GPIO_DEBOUNCE      (1 << 4)
#define QUARKX1000_GPIO_CLOCK_SYNC    (1 << 5)
#define QUARKX1000_GPIO_POL_NORMAL    (0 << 6)
#define QUARKX1000_GPIO_POL_INV       (1 << 6)
#define QUARKX1000_GPIO_PUD_NORMAL    (0 << 7)
#define QUARKX1000_GPIO_PUD_PULL_UP   (1 << 7)
#define QUARKX1000_GPIO_PUD_PULL_DOWN (2 << 7)

#define QUARKX1000_GPIO_DIR_MASK        (1 << 0)
#define QUARKX1000_GPIO_POL_MASK        (1 << 6)
#define QUARKX1000_GPIO_PUD_MASK        (3 << 7)

typedef void (*quarkX1000_gpio_callback)(uint32_t);

int quarkX1000_gpio_init(void);

int quarkX1000_gpio_config(uint8_t pin, int flags);
int quarkX1000_gpio_read(uint8_t pin, uint8_t *value);
int quarkX1000_gpio_write(uint8_t pin, uint8_t value);

int quarkX1000_gpio_config_port(int flags);
int quarkX1000_gpio_read_port(uint8_t *value);
int quarkX1000_gpio_write_port(uint8_t value);

int quarkX1000_gpio_set_callback(quarkX1000_gpio_callback callback);

void quarkX1000_gpio_clock_enable(void);
void quarkX1000_gpio_clock_disable(void);

#endif /* CPU_X86_DRIVERS_QUARKX1000_GPIO_H_ */
