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

#ifndef PIC_H
#define PIC_H

#include "helpers.h"

#define PIC1_CMD_PORT   0x20
#define PIC1_DATA_PORT  0x21
#define PIC2_CMD_PORT   0xA0
#define PIC2_DATA_PORT  0xA1

/* This function initializes the daisy-chained Master and Slave 8259 PICs.
 * It is only called once, so let's give the compiler the option to inline it.
 * For more information about the ICWs, please refer to http://stanislavs.org/helppc/8259.html.
 */
static inline void
pic_init(void)
{
  /* ICW1: Initialization. */
  outb(PIC1_CMD_PORT, 0x11);
  outb(PIC2_CMD_PORT, 0x11);

  /* ICW2: Remap IRQs by setting an IDT Offset for each PIC. */
  outb(PIC1_DATA_PORT, 0x20);
  outb(PIC2_DATA_PORT, 0x28);

  /* ICW3: Setup Slave to Master's IRQ2. */
  outb(PIC1_DATA_PORT, 0x04);
  outb(PIC2_DATA_PORT, 0x02);

  /* ICW4: Environment setup. Set PIC1 as master and PIC2 as slave. */
  outb(PIC1_DATA_PORT, 0x01);
  outb(PIC2_DATA_PORT, 0x01);

  /* Set the IMR register, masking all hardware interrupts but IRQ 2.
   * We will have to unmask each IRQ when registering them. */
  outb(PIC1_DATA_PORT, 0xfb);
  outb(PIC2_DATA_PORT, 0xff);
}

#endif /* PIC_H */
