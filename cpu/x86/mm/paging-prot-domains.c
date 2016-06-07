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

#include <string.h>
#include "dma.h"
#include "gdt.h"
#include "gdt-layout.h"
#include "helpers.h"
#include "idt.h"
#include "paging.h"
#include "prot-domains.h"
#include "segmentation.h"
#include "stacks.h"
#include "syscalls.h"
#include "tss.h"

/*#define DBG_PAGE_ALLOC*/

/* Enable PAE-mode paging */
#define CR4_PAE BIT(5)

/* Extended Feature Enables MSR */
#define MSR_EFER 0xC0000080

/* Enable Execute Disable bit support */
#define EFER_NXE BIT(11)

/* Root page-directory-pointer table */
static pdpt_t root_pgtbl __attribute__((aligned(32))) ATTR_BSS_KERN;
/* Although the following page tables must be page-aligned, it is infeasible to
 * apply the "aligned(4096)" attribute for the reasons described in the linker
 * script.
 */
/* Second-level page directory */
static page_table_t
  second_lvl_pgtbl ATTR_BSS_KERN_PAGE_ALIGNED;
/* Leaf-level page table */
static page_table_t leaf_pgtbl ATTR_BSS_KERN_PAGE_ALIGNED;

#define LINEAR_ADDR_BOUND (MIN_PAGE_SIZE * ENTRIES_PER_PAGE_TABLE)

/*---------------------------------------------------------------------------*/
void
prot_domains_reg(dom_client_data_t *dcd,
                 uintptr_t mmio,
                 size_t mmio_sz,
                 uintptr_t meta,
                 size_t meta_sz,
                 bool pio)
{
  dom_id_t dom_id = dcd->dom_id;
  volatile struct dom_kern_data *dkd =
    prot_domains_kern_data + dom_id;

  /* All addresses and sizes must be page-aligned */
  if((PROT_DOMAINS_ACTUAL_CNT <= dom_id) ||
     ((mmio & (MIN_PAGE_SIZE - 1)) != 0) ||
     ((mmio_sz & (MIN_PAGE_SIZE - 1)) != 0) ||
     ((meta & (MIN_PAGE_SIZE - 1)) != 0) ||
     ((meta_sz & (MIN_PAGE_SIZE - 1)) != 0) ||
     (PROT_DOMAINS_MAX_MMIO_SZ < mmio_sz) ||
     (LINEAR_ADDR_BOUND < (PROT_DOMAINS_META_LINEAR_BASE + meta_sz))) {
    halt();
  }

  if((dkd->flags & PROT_DOMAINS_FLAG_INITED) == PROT_DOMAINS_FLAG_INITED) {
    halt();
  }

  dkd->mmio = mmio;
  dkd->mmio_sz = mmio_sz;
  dkd->meta = meta;
  dkd->meta_sz = meta_sz;
  dkd->flags = PROT_DOMAINS_FLAG_INITED;
  if(pio) {
    dkd->flags |= PROT_DOMAINS_FLAG_PIO;
  }
}
/*---------------------------------------------------------------------------*/
static void __attribute__((regparm(3)))
set_ptes(uintptr_t start_la, uintptr_t start_pa, uintptr_t end_pa,
         pte_t template)
{
#ifdef DBG_PAGE_ALLOC
#warning Checking page allocations at runtime.

  if(((start_la & (MIN_PAGE_SIZE - 1)) != 0) ||
     ((start_pa & (MIN_PAGE_SIZE - 1)) != 0) ||
     ((start_la & (MIN_PAGE_SIZE - 1)) != 0) ||
     ((end_pa & (MIN_PAGE_SIZE - 1)) != 0) ||
     (LINEAR_ADDR_BOUND <= (start_la + (end_pa - start_pa)))) {
    halt();
  }
#endif

  while(start_pa < end_pa) {
    template.addr = start_pa >> 12;

    leaf_pgtbl[start_la >> MIN_PAGE_SIZE_SHAMT] = template;

#ifdef X86_CONF_USE_INVLPG
    __asm__("invlpg %0" :: "m" (*(uint8_t *)start_la));
#endif

    start_la += MIN_PAGE_SIZE;
    start_pa += MIN_PAGE_SIZE;
  }
}
/*---------------------------------------------------------------------------*/
static void __attribute__((fastcall))
set_ptes_identity_map(uintptr_t start_pa, uintptr_t end_pa, pte_t template)
{
  set_ptes(start_pa, start_pa, end_pa, template);
}
/*---------------------------------------------------------------------------*/
static inline uint32_t __attribute__((always_inline))
prot_domains_switch(dom_id_t from_id, dom_id_t to_id,
                    interrupt_stack_t *intr_stk)
{
  volatile dom_kern_data_t *from, *to;

  from = prot_domains_kern_data + from_id;
  to = prot_domains_kern_data + to_id;

  if((from_id == DOM_ID_kern) ||
     (to_id == DOM_ID_kern)) {
    pte_t to_kern_data_pte = { .raw = 0 };
    to_kern_data_pte.present = 1;
    to_kern_data_pte.exec_disable = 1;
    /* The kernel data region should always be accessible to supervisory code,
     * but it is only accessible to user mode in the kernel protection domain.
     */
    to_kern_data_pte.user_accessible = 1;
    if(to_id == DOM_ID_kern) {
      to_kern_data_pte.writable = 1;
    }

    set_ptes_identity_map((uintptr_t)&_sbss_kern_addr,
                          (uintptr_t)&_ebss_syscall_addr,
                          to_kern_data_pte);

    if(to_id != DOM_ID_kern) {
      to_kern_data_pte.user_accessible = 0;
      to_kern_data_pte.writable = 0;
    }

    set_ptes_identity_map((uintptr_t)&_ebss_syscall_addr,
                          (uintptr_t)&_ebss_kern_addr,
                          to_kern_data_pte);
  }

  if(to->mmio_sz != 0) {
    pte_t pte = { .raw = 0 };
    pte.present = 1;
    pte.exec_disable = 1;
    pte.user_accessible = 1;
    pte.writable = 1;
    /* disable caching of MMIO accesses */
    pte.pcd = 1;

    set_ptes(PROT_DOMAINS_MMIO_LINEAR_BASE,
             to->mmio,
             to->mmio + to->mmio_sz,
             pte);
  }
  if(to->mmio_sz < from->mmio_sz) {
    pte_t pte = { .raw = 0 };

    set_ptes_identity_map(PROT_DOMAINS_MMIO_LINEAR_BASE + to->mmio_sz,
                          PROT_DOMAINS_MMIO_LINEAR_BASE + from->mmio_sz,
                          pte);
  }

  if(to->meta_sz != 0) {
    pte_t pte = { .raw = 0 };
    pte.present = 1;
    pte.exec_disable = 1;
    pte.user_accessible = 1;
    pte.writable = 1;

    set_ptes(PROT_DOMAINS_META_LINEAR_BASE,
             to->meta,
             to->meta + to->meta_sz,
             pte);
  }
  if(to->meta_sz < from->meta_sz) {
    pte_t pte = { .raw = 0 };

    set_ptes_identity_map(PROT_DOMAINS_META_LINEAR_BASE + to->mmio_sz,
                          PROT_DOMAINS_META_LINEAR_BASE + from->mmio_sz,
                          pte);
  }

#ifndef X86_CONF_USE_INVLPG
  __asm__ __volatile__ ("mov %%cr3, %%eax\n\t"
                        "mov %%eax, %%cr3\n\t" ::: "eax");
#endif

  return 0;
}
/*---------------------------------------------------------------------------*/
void
prot_domains_gdt_init(void)
{
  gdt_copy_desc_change_dpl(GDT_IDX_DATA, GDT_IDX_DATA_FLAT, PRIV_LVL_USER);
  gdt_copy_desc_change_dpl(GDT_IDX_STK_INT, GDT_IDX_STK_EXC, PRIV_LVL_INT);
}
/*---------------------------------------------------------------------------*/
void
prot_domains_impl_init(void)
{
  pte_t pte = { .raw = 0 };

  syscalls_int_init();

  /* Initialize page table: */

  pte.present = 1;
  pte.addr = ((uint32_t)second_lvl_pgtbl) >> MIN_PAGE_SIZE_SHAMT;

  root_pgtbl[0] = pte;

  pte.writable = 1;
  pte.user_accessible = 1;
  pte.addr = ((uint32_t)leaf_pgtbl) >> MIN_PAGE_SIZE_SHAMT;

  second_lvl_pgtbl[0] = pte;

  /* Map code sections: */

  pte.writable = 0;
  set_ptes_identity_map((uintptr_t)&_stext_addr, (uintptr_t)&_etext_addr, pte);

  /* Map data sections: */

  pte.writable = 1;
  pte.exec_disable = 1;
  set_ptes_identity_map((uintptr_t)stacks_main,
                        (uintptr_t)stacks_main +
                        STACKS_SIZE_MAIN +
                        STACKS_SIZE_EXC +
                        STACKS_SIZE_INT,
                        pte);
  set_ptes_identity_map((uintptr_t)&_sdata_addr, (uintptr_t)&_edata_addr, pte);

  /* Enable XD bit support */
  __asm__ __volatile__ ("wrmsr" :: "c" (MSR_EFER), "a" (EFER_NXE), "d" (0));

  /* Enable PAE */
  __asm__ __volatile__ ("mov %%cr4, %%eax\n\t"
                        "or %0, %%eax\n\t"
                        "mov %%eax, %%cr4\n\t"
                        :
                        : "r" (CR4_PAE)
                        : "eax");

  /* Load CR3 */
  __asm__ __volatile__ ("mov %0, %%cr3" :: "r" (root_pgtbl));
}
/*---------------------------------------------------------------------------*/
uintptr_t
prot_domains_lookup_meta_phys_base(dom_client_data_t *drv)
{
  return prot_domains_kern_data[drv->dom_id].meta;
}
/*---------------------------------------------------------------------------*/

/* Enable inter-procedural optimization with procedures in the following file:
 */
#include "syscalls-int.c"
