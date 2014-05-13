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
 * \addtogroup openmote-cc2538
 * @{
 *
 * \defgroup openmote-cc2538-antenna OpenMote-CC2538 antenna driver
 *
 * Antenna driver implementation for the OpenMote-CC2538
 * @{
 *
 * \file
 * Antenna driver implementation for the OpenMote-CC2538
 */
#include "contiki.h"
#include "reg.h"
#include "dev/antenna.h"
#include "dev/gpio.h"

#define ANTENNA_GPIO_PIN_MASK   ANTENNA_ALL

/*---------------------------------------------------------------------------*/
void
antenna_arch_init(void)
{
  GPIO_SET_OUTPUT(GPIO_C_BASE, ANTENNA_GPIO_PIN_MASK);
  GPIO_SET_OUTPUT(GPIO_C_BASE, ANTENNA_GPIO_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
void
antenna_arch_external(void)
{
  GPIO_WRITE_PIN(GPIO_C_BASE, ANTENNA_GPIO_PIN_MASK, ~ANTENNA_GPIO_PIN_INTERNAL);
  GPIO_WRITE_PIN(GPIO_C_BASE, ANTENNA_GPIO_PIN_MASK, ANTENNA_GPIO_PIN_EXTERNAL);
}
/*---------------------------------------------------------------------------*/
void
antenna_arch_internal(unsigned char leds)
{
  GPIO_WRITE_PIN(GPIO_C_BASE, ANTENNA_GPIO_PIN_MASK, ANTENNA_GPIO_PIN_INTERNAL);
  GPIO_WRITE_PIN(GPIO_C_BASE, ANTENNA_GPIO_PIN_MASK, ~ANTENNA_GPIO_PIN_EXTERNAL);
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
