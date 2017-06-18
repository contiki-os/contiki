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
#define PIN_INPUT 3

static uint32_t value;
static struct ctimer timer;

PROCESS(gpio_input_process, "GPIO Input Process");
AUTOSTART_PROCESSES(&gpio_input_process);
/*---------------------------------------------------------------------------*/
static void
timeout(void *data)
{
  uint8_t value_in;

  /* toggle pin state */
  value = !value;
  galileo_gpio_write(PIN_OUTPUT, value);

  galileo_gpio_read(PIN_INPUT, &value_in);

  if (value == value_in)
    printf("GPIO pin value match!\n");
  else
    printf("GPIO pin value DOESN'T match!\n");

  ctimer_reset(&timer);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gpio_input_process, ev, data)
{
  PROCESS_BEGIN();

  quarkX1000_gpio_clock_enable();

  ctimer_set(&timer, CLOCK_SECOND / 2, timeout, NULL);

  printf("GPIO input example is running\n");
  PROCESS_YIELD();

  quarkX1000_gpio_clock_disable();

  PROCESS_END();
}
