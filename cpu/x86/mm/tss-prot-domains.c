/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
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

#include <stdint.h>
#include <string.h>
#include "gdt.h"
#include "helpers.h"
#include "idt.h"
#include "prot-domains.h"
#include "stacks.h"
#include "syscalls.h"
#include "tss.h"

uint32_t prot_domains_main_esp;
syscalls_entrypoint_t ATTR_KERN_ADDR_SPACE *prot_domains_syscall;

/*---------------------------------------------------------------------------*/
void app_main(void);
void
prot_domains_reg(dom_client_data_t ATTR_KERN_ADDR_SPACE *dcd,
                 uintptr_t mmio, size_t mmio_sz,
                 uintptr_t meta, size_t meta_sz,
                 bool pio)
{
  segment_desc_t desc;
  uint32_t eflags;
  dom_id_t dom_id;
  volatile struct dom_kern_data ATTR_KERN_ADDR_SPACE *dkd;

  KERN_READL(dom_id, dcd->dom_id);

  dkd = prot_domains_kern_data + dom_id;

  prot_domains_reg_multi_seg(dkd, mmio, mmio_sz, meta, meta_sz);

  /* Only the kernel protection domain requires port I/O access outside of the
   * interrupt handlers.
   */
  eflags = EFLAGS_IOPL(pio ? PRIV_LVL_USER : PRIV_LVL_INT);
  if(dom_id == DOM_ID_app) {
    eflags |= EFLAGS_IF;
  }

  /* Keep this initialization in sync with the register definitions in
   * tss-prot-domains-asm.S.
   */
  KERN_WRITEL(dkd->tss.ebp, 0);
  KERN_WRITEL(dkd->tss.ebx, 0);
  KERN_WRITEL(dkd->tss.esi, BIT(dom_id));
  KERN_WRITEL(dkd->tss.eip,
             (dom_id == DOM_ID_app) ?
             (uint32_t)app_main :
             (uint32_t)prot_domains_syscall_dispatcher);
  KERN_WRITEL(dkd->tss.cs, GDT_SEL_CODE);
  KERN_WRITEL(dkd->tss.ds, GDT_SEL_DATA);
  KERN_WRITEL(dkd->tss.es, GDT_SEL_DATA);
  KERN_WRITEL(dkd->tss.fs, LDT_SEL_KERN);
  KERN_WRITEL(dkd->tss.gs,
             (meta_sz == 0) ? GDT_SEL_NULL : LDT_SEL_META);
  KERN_WRITEL(dkd->tss.ss, GDT_SEL_STK);
  /* This stack pointer is only actually used in application protection domain.
   * Other domains enter at system call dispatcher, which switches to main
   * stack.
   */
  KERN_WRITEL(dkd->tss.esp,
             /* Two return addresses have been consumed: */
             STACKS_INIT_TOP + (2 * sizeof(uintptr_t)));
  KERN_WRITEL(dkd->tss.eflags, eflags);
  KERN_WRITEL(dkd->tss.ldt, GDT_SEL_LDT(dom_id));
  KERN_WRITEL(dkd->tss.esp2, STACKS_SIZE_MAIN + STACKS_SIZE_INT);
  KERN_WRITEL(dkd->tss.ss2, GDT_SEL_STK_INT);
  KERN_WRITEL(dkd->tss.esp0,
             STACKS_SIZE_MAIN + STACKS_SIZE_INT + STACKS_SIZE_EXC);
  KERN_WRITEL(dkd->tss.ss0, GDT_SEL_STK_EXC);
  KERN_WRITEW(dkd->tss.t, 0);
  KERN_WRITEW(dkd->tss.iomap_base, sizeof(tss_t));
  KERN_WRITEL(dkd->tss.cr3, 0);

  segment_desc_init(&desc,
                    KERN_DATA_OFF_TO_PHYS_ADDR((uint32_t)&(dkd->tss)),
                    sizeof(dkd->tss),
                    /* It should be possible for code at any privilege level to invoke the task's
                     * system call dispatcher.
                     */
                    SEG_FLAG(DPL, PRIV_LVL_USER) | SEG_TYPE_TSS32_AVAIL);

  gdt_insert(GDT_IDX_TSS(dom_id), desc);

  KERN_WRITEW(dcd->tss_sel, GDT_SEL(GDT_IDX_TSS(dom_id), PRIV_LVL_USER));
}
/*---------------------------------------------------------------------------*/
void dev_not_avail_isr(void);
void
prot_domains_impl_init(void)
{
  __asm__ __volatile__ ("ltr %0" :: "r" ((uint16_t)GDT_SEL_TSS(DOM_ID_kern)));
  __asm__ __volatile__ ("lldt %0" :: "r" ((uint16_t)GDT_SEL_LDT(DOM_ID_kern)));

  idt_set_intr_gate_desc(7,
                         (uint32_t)dev_not_avail_isr,
                         GDT_SEL_CODE_EXC, PRIV_LVL_EXC);
}
/*---------------------------------------------------------------------------*/
int main();
void
prot_domains_launch_kernel(void)
{
  multi_segment_launch_kernel();

  /* Activate kernel protection domain, entering the kernel at main. */
  __asm__ __volatile__ (
    "pushl %[_ss_]\n\t"
    "pushl %[_top_of_stk_]\n\t"
    "pushl %[_eflags_]\n\t"
    "pushl %[_cs_]\n\t"
    "pushl %[_kern_start_]\n\t"
    "iretl\n\t"
    :
    : [_ss_] "g" (GDT_SEL_STK),
      [_eflags_] "g" (EFLAGS_IOPL(PRIV_LVL_USER)),
      [_cs_] "g" (GDT_SEL_CODE),
      [_kern_start_] "g" (main),
      /* one address has already been consumed */
      [_top_of_stk_] "g" (STACKS_INIT_TOP + sizeof(uint32_t))
    );
}
/*---------------------------------------------------------------------------*/
void
prot_domains_launch_app()
{
  far_pointer_t app_ptr = { 0, GDT_SEL_TSS(DOM_ID_app) };
  __asm__ __volatile__ ("ljmp *%0" :: "m" (app_ptr));
}
/*---------------------------------------------------------------------------*/
