/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
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

#include "drivers/legacy_pc/pic.h"
#include "interrupt.h"
#include "irq.h"

#define IRQ7_INT  PIC_INT(7)

static void
spurious_irq7_handler(void)
{
  /*
   * NOTE: Originally IRQ7 was used for the parallel port interrupts. Nowadays,
   * though, it is only used if some other IRQ (i.e.: a PCIx interrupt) is
   * mapped to it. In this case we will have to check the PIC ISR register in
   * order to confirm this was a real interrupt.
   *
   * In case of a spurious interrupt, we should NEVER send an EOI here so the PIC
   * doesn't trigger the next queued interrupt.
   */
}
/*---------------------------------------------------------------------------*/
void
irq_init(void)
{
  pic_init();

  /* Set a 'fake' handler for the Spurious IRQ7 interrupts.
   * Refer to http://wiki.osdev.org/PIC .
   */
  SET_INTERRUPT_HANDLER(IRQ7_INT, 0, spurious_irq7_handler);
}
