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

#include <assert.h>
#include "idt.h"
#include "interrupt.h"
#include "pic.h"
#include "shared-isr.h"

/* Defined in linker script */
extern shared_isr_client_t _sdata_shared_isr, _edata_shared_isr;

static void __attribute__((used))
shared_handler(void)
{
  shared_isr_client_t *client;
  for(client = &_sdata_shared_isr; client < &_edata_shared_isr; client++) {
    if(client->handler()) {
      pic_eoi(client->irq);
      return;
    }
  }
}

/**
 * \brief Initialize shared ISR by iterating through all of its clients and
 *        configuring their interrupts to route to the shared ISR.
 */
void
shared_isr_init(void)
{
  shared_isr_client_t *client = &_sdata_shared_isr;
  shared_isr_client_t *consistency_check_client;
  bool prev_conf;

  void shared_isr_stub(void);
  __asm__ __volatile__ (
    ISR_STUB("shared_isr_stub", 0, "shared_handler", 0)
    :
  );

  while(client < &_edata_shared_isr) {
    consistency_check_client = &_sdata_shared_isr;

    prev_conf = false;

    while(consistency_check_client < client) {
      if((client->irq == consistency_check_client->irq) ||
         (client->pin == consistency_check_client->pin) ||
         (client->pirq == consistency_check_client->pirq)) {

        prev_conf = true;

        /* This interrupt was previously configured. */
        break;
      }

      consistency_check_client++;
    }

    if(prev_conf) {
      /* The requested configurations for each IRQ must be consistent. */
      assert((client->irq == consistency_check_client->irq) &&
             (client->agent == consistency_check_client->agent) &&
             (client->pin == consistency_check_client->pin) &&
             (client->pirq == consistency_check_client->pirq));
    } else {
      idt_set_intr_gate_desc(PIC_INT(client->irq), (uint32_t)shared_isr_stub,
                             GDT_SEL_CODE_INT, PRIV_LVL_INT);

      pci_irq_agent_set_pirq(client->agent, client->pin, client->pirq);

      pci_pirq_set_irq(client->pirq, client->irq, 1);

      pic_unmask_irq(client->irq);
    }

    client++;
  }
}
