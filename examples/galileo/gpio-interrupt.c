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

#include <stdio.h>

#include "contiki.h"
#include "sys/ctimer.h"

#include "galileo-gpio.h"
#include "gpio.h"

#define PIN_OUTPUT 2
#define PIN_INTR 3

static struct ctimer timer;

PROCESS(gpio_interrupt_process, "GPIO Interrupt Process");
AUTOSTART_PROCESSES(&gpio_interrupt_process);
/*---------------------------------------------------------------------------*/
static void
timeout(void *data)
{
  /* emulate an interrupt */
  galileo_gpio_write(PIN_OUTPUT, 0);
  galileo_gpio_write(PIN_OUTPUT, 1);

  ctimer_reset(&timer);
}
/*---------------------------------------------------------------------------*/
static void
callback(uint32_t status)
{
  printf("GPIO interrupt callback called, status: %d\n", status);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gpio_interrupt_process, ev, data)
{
  PROCESS_BEGIN();

  galileo_gpio_config(PIN_INTR, QUARKX1000_GPIO_INT | QUARKX1000_GPIO_ACTIVE_HIGH | QUARKX1000_GPIO_EDGE);

  quarkX1000_gpio_set_callback(callback);

  quarkX1000_gpio_clock_enable();

  ctimer_set(&timer, CLOCK_SECOND / 2, timeout, NULL);

  printf("GPIO interrupt example is running\n");
  PROCESS_YIELD();

  quarkX1000_gpio_clock_disable();

  PROCESS_END();
}
