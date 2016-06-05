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
#include "prot-domains.h"
#include "segmentation.h"
#include "stacks.h"

/*---------------------------------------------------------------------------*/
static uint32_t
segment_desc_compute_base(segment_desc_t desc)
{
  return (desc.base_hi << 24) | (desc.base_mid << 16) | desc.base_lo;
}
/*---------------------------------------------------------------------------*/
void
prot_domains_reg_multi_seg(volatile struct dom_kern_data ATTR_KERN_ADDR_SPACE *dkd,
                           uintptr_t mmio, size_t mmio_sz,
                           uintptr_t meta, size_t meta_sz)
{
  segment_desc_t desc;
  dom_id_t dom_id = PROT_DOMAINS_GET_DOM_ID(dkd);
  uint32_t kern_data_len;
  uint32_t tmp;

  if((dkd < prot_domains_kern_data) ||
     (prot_domains_kern_data_end <= dkd) ||
     (((((uintptr_t)dkd) - (uintptr_t)prot_domains_kern_data) %
         sizeof(dom_kern_data_t)) != 0)) {
    halt();
  }

  KERN_READL(tmp, dkd->ldt[DT_SEL_GET_IDX(LDT_SEL_KERN)].raw_hi);
  if(tmp != 0) {
    /* This PDCS was previously initialized, which is disallowed. */
    halt();
  }

  /* Initialize descriptors */

  if(dom_id == DOM_ID_kern) {
    kern_data_len = (uint32_t)&_ebss_kern_addr;
  } else {
    /* Non-kernel protection domains do not need to access the protection
     * domain control structures, and they may contain saved register values
     * that are private to each domain.
     */
    kern_data_len = (uint32_t)&_ebss_syscall_addr;
  }
  kern_data_len -= (uint32_t)&_sbss_kern_addr;

  segment_desc_init(&desc, (uint32_t)&_sbss_kern_addr, kern_data_len,
    /* Every protection domain requires at least read-only access to kernel
       data to read dom_client_data structures and to support the system call
       dispatcher, if applicable.  Only the kernel protection domain is granted
       read/write access to the kernel data. */
                    ((dom_id == DOM_ID_kern) ?
                        SEG_TYPE_DATA_RDWR :
                        SEG_TYPE_DATA_RDONLY) |
                    SEG_FLAG(DPL, PRIV_LVL_USER) |
                    SEG_GRAN_BYTE | SEG_DESCTYPE_NSYS);

  KERN_WRITEL(dkd->ldt[LDT_IDX_KERN].raw_lo, desc.raw_lo);
  KERN_WRITEL(dkd->ldt[LDT_IDX_KERN].raw_hi, desc.raw_hi);

  if(mmio_sz != 0) {
    if(SEG_MAX_BYTE_GRAN_LEN < mmio_sz) {
      halt();
    }

    segment_desc_init(&desc, mmio, mmio_sz,
                      SEG_FLAG(DPL, PRIV_LVL_USER) | SEG_GRAN_BYTE |
                      SEG_DESCTYPE_NSYS | SEG_TYPE_DATA_RDWR);
  } else {
    desc.raw = SEG_DESC_NOT_PRESENT;
  }

  KERN_WRITEL(dkd->ldt[LDT_IDX_MMIO].raw_lo, desc.raw_lo);
  KERN_WRITEL(dkd->ldt[LDT_IDX_MMIO].raw_hi, desc.raw_hi);

  if(meta_sz != 0) {
    if(SEG_MAX_BYTE_GRAN_LEN < meta_sz) {
      halt();
    }

    segment_desc_init(&desc, meta, meta_sz,
                      SEG_FLAG(DPL, PRIV_LVL_USER) | SEG_GRAN_BYTE |
                      SEG_DESCTYPE_NSYS | SEG_TYPE_DATA_RDWR);
  } else {
    desc.raw = SEG_DESC_NOT_PRESENT;
  }

  KERN_WRITEL(dkd->ldt[LDT_IDX_META].raw_lo, desc.raw_lo);
  KERN_WRITEL(dkd->ldt[LDT_IDX_META].raw_hi, desc.raw_hi);

  segment_desc_init(&desc,
                    KERN_DATA_OFF_TO_PHYS_ADDR(dkd->ldt),
                    sizeof(dkd->ldt),
                    SEG_FLAG(DPL, PRIV_LVL_USER) | SEG_GRAN_BYTE |
                    SEG_DESCTYPE_SYS | SEG_TYPE_LDT);
  gdt_insert(GDT_IDX_LDT(dom_id), desc);
}
/*---------------------------------------------------------------------------*/
void
prot_domains_gdt_init()
{
  int i;
  segment_desc_t desc;

  segment_desc_init(&desc,
                    (uint32_t)&_stext_addr,
                    ((uint32_t)&_etext_addr) - (uint32_t)&_stext_addr,
                    SEG_FLAG(DPL, PRIV_LVL_EXC) | SEG_GRAN_BYTE |
                    SEG_DESCTYPE_NSYS |
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__SWSEG
  /* The general protection fault handler requires read access to CS */
                    SEG_TYPE_CODE_EXRD
#else
                    SEG_TYPE_CODE_EX
#endif
                    );
  gdt_insert_boot(GDT_IDX_CODE_EXC, desc);

  segment_desc_init(&desc,
                    (uint32_t)&_sdata_addr,
                    ((uint32_t)&_edata_addr) - (uint32_t)&_sdata_addr,
                    SEG_FLAG(DPL, PRIV_LVL_USER) | SEG_GRAN_BYTE |
                    SEG_DESCTYPE_NSYS | SEG_TYPE_DATA_RDWR);
  gdt_insert_boot(GDT_IDX_DATA, desc);

  segment_desc_init(&desc,
                    (uint32_t)&_sbss_kern_addr,
                    ((uint32_t)&_ebss_kern_addr) -
                    (uint32_t)&_sbss_kern_addr,
                    SEG_FLAG(DPL, PRIV_LVL_EXC) | SEG_GRAN_BYTE |
                    SEG_DESCTYPE_NSYS | SEG_TYPE_DATA_RDWR);
  gdt_insert_boot(GDT_IDX_DATA_KERN_EXC, desc);

  segment_desc_init(&desc,
                    (uint32_t)DATA_OFF_TO_PHYS_ADDR(stacks_main),
                    STACKS_SIZE_MAIN,
                    SEG_FLAG(DPL, PRIV_LVL_USER) | SEG_GRAN_BYTE |
                    SEG_DESCTYPE_NSYS | SEG_TYPE_DATA_RDWR);
  gdt_insert_boot(GDT_IDX_STK, desc);

  segment_desc_set_limit(&desc, STACKS_SIZE_MAIN + STACKS_SIZE_INT);
  SEG_SET_FLAG(desc, DPL, PRIV_LVL_INT);
  gdt_insert_boot(GDT_IDX_STK_INT, desc);

  segment_desc_set_limit(&desc,
                         STACKS_SIZE_MAIN +
                         STACKS_SIZE_INT +
                         STACKS_SIZE_EXC);
  SEG_SET_FLAG(desc, DPL, PRIV_LVL_EXC);
  gdt_insert_boot(GDT_IDX_STK_EXC, desc);

  /* Not all domains will necessarily be initialized, so this initially marks
   * all per-domain descriptors not-present.
   */
  desc.raw = SEG_DESC_NOT_PRESENT;
  for(i = 0; i < PROT_DOMAINS_ACTUAL_CNT; i++) {
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__TSS
    gdt_insert_boot(GDT_IDX_TSS(i), desc);
#endif
    gdt_insert_boot(GDT_IDX_LDT(i), desc);
  }

  __asm__ __volatile__ (
      "mov %[_default_data_], %%ds\n\t"
      "mov %[_default_data_], %%es\n\t"
      "mov %[_kern_data_], %%" SEG_KERN "s\n\t"
      :
      : [_default_data_] "r"(GDT_SEL_DATA),
        [_kern_data_] "r"(GDT_SEL_DATA_KERN_EXC));
}
/*---------------------------------------------------------------------------*/
void
multi_segment_launch_kernel(void)
{
  /* Update segment registers. */
  __asm__ __volatile__ (
    "mov %[_data_seg_], %%ds\n\t"
    "mov %[_data_seg_], %%es\n\t"
    "mov %[_kern_seg_], %%" SEG_KERN "s\n\t"
    "mov %[_data_seg_], %%" SEG_META "s\n\t"
    :
    : [_data_seg_] "r" (GDT_SEL_DATA),
      [_kern_seg_] "r" (LDT_SEL_KERN)
    );
}
/*---------------------------------------------------------------------------*/
void
prot_domains_enable_mmio(void)
{
  __asm__ __volatile__ ("mov %0, %%" SEG_MMIO "s" :: "r" (LDT_SEL_MMIO));
}
/*---------------------------------------------------------------------------*/
void
prot_domains_disable_mmio(void)
{
  __asm__ __volatile__ ("mov %0, %%" SEG_KERN "s" :: "r" (LDT_SEL_KERN));
}
/*---------------------------------------------------------------------------*/
uintptr_t
prot_domains_lookup_meta_phys_base(dom_client_data_t ATTR_KERN_ADDR_SPACE *drv)
{
  dom_id_t dom_id;
  segment_desc_t desc;
  volatile dom_kern_data_t ATTR_KERN_ADDR_SPACE *dkd;

  KERN_READL(dom_id, drv->dom_id);

  dkd = prot_domains_kern_data + dom_id;

  KERN_READL(desc.raw_lo, dkd->ldt[DT_SEL_GET_IDX(LDT_SEL_META)].raw_lo);
  KERN_READL(desc.raw_hi, dkd->ldt[DT_SEL_GET_IDX(LDT_SEL_META)].raw_hi);

  return segment_desc_compute_base(desc);
}
/*---------------------------------------------------------------------------*/
