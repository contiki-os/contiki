/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-gpio-interrupts
 * @{
 *
 * \file
 * Implementation of CC13xx/CC26xx GPIO interrupt handling.
 */
/*---------------------------------------------------------------------------*/
#include "ioc.h"
#include "gpio-interrupt.h"
#include "sys/energest.h"
#include "lpm.h"
#include "ti-lib.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define gpio_interrupt_isr GPIOIntHandler
/*---------------------------------------------------------------------------*/
/* Handler array */
static gpio_interrupt_handler_t handlers[NUM_IO_MAX];
/*---------------------------------------------------------------------------*/
void
gpio_interrupt_register_handler(uint8_t ioid, gpio_interrupt_handler_t f)
{
  uint8_t interrupts_disabled = ti_lib_int_master_disable();

  /* Clear interrupts on specified pins */
  ti_lib_gpio_event_clear(1 << ioid);

  handlers[ioid] = f;

  /* Re-enable interrupts */
  if(!interrupts_disabled) {
    ti_lib_int_master_enable();
  }
}
/*---------------------------------------------------------------------------*/
void
gpio_interrupt_init()
{
  int i;

  for(i = 0; i < NUM_IO_MAX; i++) {
    handlers[i] = NULL;
  }

  ti_lib_int_enable(INT_EDGE_DETECT);
}
/*---------------------------------------------------------------------------*/
void
gpio_interrupt_isr(void)
{
  uint32_t pin_mask;
  uint8_t i;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* Read interrupt flags */
  pin_mask = (HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) & GPIO_PIN_MASK);

  /* Clear the interrupt flags */
  HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) = pin_mask;

  /* Run custom ISRs */
  for(i = 0; i < NUM_GPIO_PINS; i++) {
    /* Call the handler if there is one registered for this event */
    if((pin_mask & (1 << i)) && handlers[i] != NULL) {
      handlers[i](i);
    }
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/** @} */
