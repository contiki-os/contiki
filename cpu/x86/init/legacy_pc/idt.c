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

#include "gdt-layout.h"
#include "prot-domains.h"
#include <stdint.h>

#include "helpers.h"
#include "segmentation.h"
#include "idt.h"

#define NUM_DESC 256

typedef struct idtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idtr_t;

typedef union intr_gate_desc {
  struct __attribute__((packed)) {
    uint16_t    offset_low;
    uint16_t    selector;   /* Segment Selector for destination code segment */
    uint16_t    fixed:11;
    uint16_t    d:1;        /* Size of gate: 1 = 32 bits; 0 = 16 bits */
    uint16_t    pad:1;
    uint16_t    dpl:2;      /* Descriptor Privilege Level */
    uint16_t    p:1;        /* Segment Present flag */
    uint16_t    offset_high;
  };
  uint64_t raw;
  struct {
    uint32_t raw_lo;
    uint32_t raw_hi;
  };
} intr_gate_desc_t;

/* According to Intel Combined Manual, Vol. 3, Section 6.10, the base addresses
 * of the IDT should be aligned on an 8-byte boundary to maximize performance
 * of cache line fills.
 */
static intr_gate_desc_t __attribute__((aligned(8))) ATTR_BSS_KERN
  idt[NUM_DESC];

/*---------------------------------------------------------------------------*/
/* XXX: If you change this function prototype, make sure you fix the assembly
 * code in SET_INT_EXC_HANDLER macro in interrupt.h. Otherwise, you might
 * face a very-hard-to-find bug in the interrupt handling system.
 */
void
idt_set_intr_gate_desc(int intr_num,
                       uint32_t offset,
                       uint16_t cs,
                       uint16_t dpl)
{
  intr_gate_desc_t desc;

  desc.offset_low = offset & 0xFFFF;
  desc.selector = cs;
  desc.fixed = BIT(9) | BIT(10);
  desc.pad = 0;
  desc.d = 1;
  desc.dpl = dpl;
  desc.p = 1;
  desc.offset_high = (offset >> 16) & 0xFFFF;

  KERN_WRITEL(idt[intr_num].raw_hi, desc.raw_hi);
  KERN_WRITEL(idt[intr_num].raw_lo, desc.raw_lo);
}
/*---------------------------------------------------------------------------*/
/* Initialize Interrupt Descriptor Table. The IDT is initialized with
 * null descriptors. Therefore, any interrupt at this point will cause
 * a triple fault.
 */
void
idt_init(void)
{
  idtr_t idtr;

  /* Initialize idtr structure */
  idtr.limit = (sizeof(intr_gate_desc_t) * NUM_DESC) - 1;
  idtr.base = KERN_DATA_OFF_TO_PHYS_ADDR((uint32_t)idt);

  /* Load IDTR register */
  __asm__("lidt %0\n\t" :: "m" (idtr));
}
