/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \addtogroup cc2538-gpio
 * @{
 *
 * \file
 * Implementation of the cc2538 GPIO controller
 */
#include "contiki.h"
#include "sys/energest.h"
#include "dev/leds.h"
#include "dev/gpio.h"
#include "dev/nvic.h"
#include "reg.h"
#include "lpm.h"

#include <string.h>

/**
 * \brief Pointer to a function to be called when a GPIO interrupt is detected.
 * Callbacks for Port A, Pins[0:7] are stored in positions [0:7] of this
 * buffer, Port B callbacks in [8:15] and so on
 */
static gpio_callback_t gpio_callbacks[32];
/*---------------------------------------------------------------------------*/
void
gpio_register_callback(gpio_callback_t f, uint8_t port, uint8_t pin)
{
  gpio_callbacks[(port << 3) + pin] = f;
}
/*---------------------------------------------------------------------------*/
/** \brief Run through all registered GPIO callbacks and invoke those
 * associated with the \a port and the pins specified by \a mask
 * \param mask Search callbacks associated with pins specified by this mask
 * \param port Search callbacks associated with this port. Here, port is
 * specified as a number between 0 and 3. Port A: 0, Port B: 1 etc */
void
notify(uint8_t mask, uint8_t port)
{
  uint8_t i;
  gpio_callback_t *f = &gpio_callbacks[port << 3];

  for(i = 0; i < 8; i++) {
    if(mask & (1 << i)) {
      if((*f) != NULL) {
        (*f)(port, i);
      }
    }
    f++;
  }
}
/*---------------------------------------------------------------------------*/
/** \brief Interrupt service routine for Port A */
void
gpio_port_a_isr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  lpm_exit();

  notify(REG(GPIO_A_BASE | GPIO_MIS), GPIO_A_NUM);

  REG(GPIO_A_BASE | GPIO_IC) = 0xFF;

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/** \brief Interrupt service routine for Port B */
void
gpio_port_b_isr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  lpm_exit();

  notify(REG(GPIO_B_BASE | GPIO_MIS), GPIO_B_NUM);

  REG(GPIO_B_BASE | GPIO_IC) = 0xFF;

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/** \brief Interrupt service routine for Port C */
void
gpio_port_c_isr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  lpm_exit();

  notify(REG(GPIO_C_BASE | GPIO_MIS), GPIO_C_NUM);

  REG(GPIO_C_BASE | GPIO_IC) = 0xFF;

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/** \brief Interrupt service routine for Port D */
void
gpio_port_d_isr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  lpm_exit();

  notify(REG(GPIO_D_BASE | GPIO_MIS), GPIO_D_NUM);

  REG(GPIO_D_BASE | GPIO_IC) = 0xFF;

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
gpio_init()
{
  memset(gpio_callbacks, 0, sizeof(gpio_callbacks));
}
/** @} */
