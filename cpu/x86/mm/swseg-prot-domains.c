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

#include "gdt.h"
#include "helpers.h"
#include "multi-segment.h"
#include "prot-domains.h"

/*---------------------------------------------------------------------------*/
void
prot_domains_reg(dom_client_data_t ATTR_KERN_ADDR_SPACE *dcd,
                 uintptr_t mmio, size_t mmio_sz,
                 uintptr_t meta, size_t meta_sz,
                 bool pio)
{
  volatile dom_kern_data_t ATTR_KERN_ADDR_SPACE *dkd;
  dom_id_t dom_id;

  KERN_READL(dom_id, dcd->dom_id);

  if(PROT_DOMAINS_ACTUAL_CNT <= dom_id) {
    halt();
  }

  dkd = prot_domains_kern_data + dom_id;

  prot_domains_reg_multi_seg(dkd, mmio, mmio_sz, meta, meta_sz);

  KERN_WRITEL(dkd->flags, pio ? PROT_DOMAINS_FLAG_PIO : 0);
}
/*---------------------------------------------------------------------------*/
static inline void __attribute__((always_inline))
prot_domains_switch(dom_id_t from_id, dom_id_t to_id,
                    interrupt_stack_t *intr_stk)
{
  __asm__ __volatile__ (
    "lldt %[_ldt_]\n\t"
    "mov %[_meta_seg_], %%eax\n\t"
    "lsl %%eax, %%ecx\n\t"
    "jz 1f\n\t" /* ZF will only be set if the segment descriptor is valid. */
    "xor %%eax, %%eax\n\t" /* Nullify metadata selector */
    "1: mov %%eax, %%" SEG_META "s\n\t"
    "mov %[_kern_seg_], %%eax\n\t"
    "mov %%eax, %%" SEG_KERN "s\n\t"
    :
    : [_ldt_] "r" ((uint16_t)GDT_SEL_LDT(to_id)),
      [_meta_seg_] "i" (LDT_SEL_META),
      [_kern_seg_] "i" (LDT_SEL_KERN)
    : "cc", "eax", "ecx"
    );
}
/*---------------------------------------------------------------------------*/

/* Enable inter-procedural optimization with procedures in the following file:
 */
#include "syscalls-int.c"
