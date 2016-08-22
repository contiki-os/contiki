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

#include "galileo-gpio.h"
#include <assert.h>
#include "gpio.h"

/* Must be implemented in board-specific pinmux file to map a board-level GPIO
 * pin number to the corresponding CPU GPIO pin number.
 *
 * The return value should always be a positive number. An assertion within the
 * function should check the validity of the pin number.
 */
int galileo_brd_to_cpu_gpio_pin(unsigned pin, bool *sus);

static int
brd_to_cpu_pin(unsigned pin)
{
  int cpu_pin;
  bool sus;

  cpu_pin = galileo_brd_to_cpu_gpio_pin(pin, &sus);
  assert(!sus);

  return cpu_pin;
}

void galileo_gpio_config(uint8_t pin, int flags)
{
  assert(quarkX1000_gpio_config(brd_to_cpu_pin(pin), flags) == 0);
}

/**
 * \brief     Read from GPIO.
 * \param pin Board-level IO pin number.
 */
void galileo_gpio_read(uint8_t pin, uint8_t *value)
{
  assert(quarkX1000_gpio_read(brd_to_cpu_pin(pin), value) == 0);
}

void galileo_gpio_write(uint8_t pin, uint8_t value)
{
  assert(quarkX1000_gpio_write(brd_to_cpu_pin(pin), value) == 0);
}
