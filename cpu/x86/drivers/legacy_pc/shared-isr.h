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

#ifndef CPU_X86_DRIVERS_LEGACY_PC_SHARED_ISR_H_
#define CPU_X86_DRIVERS_LEGACY_PC_SHARED_ISR_H_

#include <stdbool.h>
#include "pci.h"

/**
 * The handler function should return true if and only if it handled the
 * interrupt.
 */
typedef bool (*shared_isr_handler_t)(void);

typedef struct shared_isr_client {
  uint8_t irq;
  IRQAGENT agent;
  INTR_PIN pin;
  PIRQ pirq;
  shared_isr_handler_t handler;
} shared_isr_client_t;

/* Unlike a non-shared interrupt handler function, an individual interrupt
 * handler for a shared interrupt must not issue an EOI. The EOI is issued by
 * the shared-isr subsystem.
 */
#define DEFINE_SHARED_IRQ(irq_, agent_, pin_, pirq_, handler_)                \
static struct shared_isr_client                                               \
  __attribute__((used, section(".shared_isr_data"))) _shared_irq_##irq_ = {   \
  .irq = irq_,                                                                \
  .agent = agent_,                                                            \
  .pin = pin_,                                                                \
  .pirq = pirq_,                                                              \
  .handler = handler_                                                         \
}

void shared_isr_init(void);

#endif /* CPU_X86_DRIVERS_LEGACY_PC_SHARED_ISR_H_ */
