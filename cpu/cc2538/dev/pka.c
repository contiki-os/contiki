/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2014 Andreas Dröscher <contiki@anticat.ch>
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
 * \addtogroup cc2538-pka
 * @{
 *
 * \file
 * Implementation of the cc2538 PKA engine driver
 */
#include "contiki.h"
#include "sys/energest.h"
#include "dev/pka.h"
#include "dev/sys-ctrl.h"
#include "dev/nvic.h"
#include "lpm.h"
#include "reg.h"

#include <stdbool.h>
#include <stdint.h>

static volatile struct process *notification_process = NULL;
/*---------------------------------------------------------------------------*/
/** \brief The PKA engine ISR
 *
 *        This is the interrupt service routine for the PKA engine.
 *
 *        This ISR is called at worst from PM0, so lpm_exit() does not need
 *        to be called.
 */
void
pka_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  nvic_interrupt_unpend(NVIC_INT_PKA);
  nvic_interrupt_disable(NVIC_INT_PKA);

  if(notification_process != NULL) {
    process_poll((struct process *)notification_process);
    notification_process = NULL;
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
static bool
permit_pm1(void)
{
  return (REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) == 0;
}
/*---------------------------------------------------------------------------*/
void
pka_init(void)
{
  volatile int i;

  lpm_register_peripheral(permit_pm1);

  pka_enable();

  /* Reset the PKA engine */
  REG(SYS_CTRL_SRSEC)   |= SYS_CTRL_SRSEC_PKA;
  for(i = 0; i < 16; i++) {
    REG(SYS_CTRL_SRSEC) &= ~SYS_CTRL_SRSEC_PKA;
  }
}
/*---------------------------------------------------------------------------*/
void
pka_enable(void)
{
  /* Enable the clock for the PKA engine */
  REG(SYS_CTRL_RCGCSEC) |= SYS_CTRL_RCGCSEC_PKA;
  REG(SYS_CTRL_SCGCSEC) |= SYS_CTRL_SCGCSEC_PKA;
  REG(SYS_CTRL_DCGCSEC) |= SYS_CTRL_DCGCSEC_PKA;
}
/*---------------------------------------------------------------------------*/
void
pka_disable(void)
{
  /* Gate the clock for the PKA engine */
  REG(SYS_CTRL_RCGCSEC) &= ~SYS_CTRL_RCGCSEC_PKA;
  REG(SYS_CTRL_SCGCSEC) &= ~SYS_CTRL_SCGCSEC_PKA;
  REG(SYS_CTRL_DCGCSEC) &= ~SYS_CTRL_DCGCSEC_PKA;
}
/*---------------------------------------------------------------------------*/
uint8_t
pka_check_status(void)
{
  return (REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) == 0;
}
void
pka_register_process_notification(struct process *p)
{
  notification_process = p;
}
/** @} */
