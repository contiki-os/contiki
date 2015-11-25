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
 * \addtogroup cc2538-nvic
 * @{
 *
 * \file
 * Driver for the cc2538 NVIC
 * All interrupt-related functionality is implemented here
 */
#include "contiki.h"
#include "dev/nvic.h"
#include "dev/scb.h"
#include "reg.h"

#include <stdint.h>

static uint32_t *interrupt_enable;
static uint32_t *interrupt_disable;
static uint32_t *interrupt_pend;
static uint32_t *interrupt_unpend;
/*---------------------------------------------------------------------------*/
void
nvic_init()
{
  interrupt_enable = (uint32_t *)NVIC_EN0;
  interrupt_disable = (uint32_t *)NVIC_DIS0;
  interrupt_pend = (uint32_t *)NVIC_PEND0;
  interrupt_unpend = (uint32_t *)NVIC_UNPEND0;

  /* Provide our interrupt table to the NVIC */
  REG(SCB_VTABLE) = NVIC_VTABLE_ADDRESS;
}
/*---------------------------------------------------------------------------*/
void
nvic_interrupt_enable(uint32_t intr)
{
  /* Writes of 0 are ignored, which is why we can simply use = */
  interrupt_enable[intr >> 5] = 1 << (intr & 0x1F);
}
/*---------------------------------------------------------------------------*/
void
nvic_interrupt_disable(uint32_t intr)
{
  /* Writes of 0 are ignored, which is why we can simply use = */
  interrupt_disable[intr >> 5] = 1 << (intr & 0x1F);
}
/*---------------------------------------------------------------------------*/
void
nvic_interrupt_en_restore(uint32_t intr, uint8_t v)
{
  if(v != 1) {
    return;
  }

  interrupt_enable[intr >> 5] = 1 << (intr & 0x1F);
}
/*---------------------------------------------------------------------------*/
uint8_t
nvic_interrupt_en_save(uint32_t intr)
{
  uint8_t rv = ((interrupt_enable[intr >> 5] & (1 << (intr & 0x1F)))
      > NVIC_INTERRUPT_DISABLED);

  nvic_interrupt_disable(intr);

  return rv;
}
/*---------------------------------------------------------------------------*/
void
nvic_interrupt_pend(uint32_t intr)
{
  /* Writes of 0 are ignored, which is why we can simply use = */
  interrupt_pend[intr >> 5] = 1 << (intr & 0x1F);
}
/*---------------------------------------------------------------------------*/
void
nvic_interrupt_unpend(uint32_t intr)
{
  /* Writes of 0 are ignored, which is why we can simply use = */
  interrupt_unpend[intr >> 5] = 1 << (intr & 0x1F);
}
/** @} */
