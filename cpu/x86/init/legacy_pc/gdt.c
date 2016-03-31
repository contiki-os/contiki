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

#include <stdint.h>
#include "gdt.h"
#include "gdt-layout.h"
#include "helpers.h"
#include "prot-domains.h"
#include "segmentation.h"

#define GDT_MEM_PL0  (SEG_DESCTYPE_NSYS | SEG_GRAN_PAGE)
#define GDT_CODE_PL0 (GDT_MEM_PL0 | SEG_TYPE_CODE_EXRD)
#define GDT_DATA_PL0 (GDT_MEM_PL0 | SEG_TYPE_DATA_RDWR)

typedef struct gdtr
{
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) gdtr_t;

/* From Intel Combined Manual, Vol. 3 , Section 3.5.1: The base addresses of
 * the GDT should be aligned on an eight-byte boundary to yield the best
 * processor performance.
 */
segment_desc_t __attribute__ ((aligned(8))) ATTR_BSS_GDT_START
  gdt[GDT_NUM_FIXED_DESC];

#define GDT_LEN                                    \
  ((((uintptr_t)&_ebss_gdt_addr) -                 \
    (uintptr_t)gdt)/sizeof(segment_desc_t))

/*---------------------------------------------------------------------------*/
static void ATTR_CODE_BOOT
set_descriptor(unsigned int index,
               uint32_t base,
               uint32_t len,
               uint16_t flag)
{
  segment_desc_t descriptor;

  if(GDT_LEN <= index) {
    halt();
  }

  segment_desc_init(&descriptor, base, len, flag);

  /* Save descriptor into gdt */
  gdt_insert_boot(index, descriptor);
}
/*---------------------------------------------------------------------------*/
void
gdt_copy_desc_change_dpl(unsigned int dest_idx,
                         unsigned int src_idx,
                         unsigned dpl)
{
  segment_desc_t desc;

  if((GDT_LEN <= dest_idx) || (GDT_LEN <= src_idx)) {
    halt();
  }

  gdt_lookup(src_idx, &desc);
  SEG_SET_FLAG(desc, DPL, dpl);
  gdt_insert(dest_idx, desc);
}
/*---------------------------------------------------------------------------*/
/* This function initializes the Global Descriptor Table. For simplicity, the
 * memory is initially organized following the flat model. Thus, memory appears
 * to Contiki as a single continuous address space. Code, data, and stack
 * are all contained in this address space (so called linear address space).
 * Certain protection domain implementations switch to a multi-segment memory
 * model later during boot.
 */
void
gdt_init(void)
{
  gdtr_t gdtr;

  /* Initialize gdtr structure */
  gdtr.limit = sizeof(segment_desc_t) * GDT_LEN - 1;
  gdtr.base = KERN_DATA_OFF_TO_PHYS_ADDR(gdt);

  /* Initialize descriptors */
  set_descriptor(GDT_IDX_NULL, 0, 0, 0);
  set_descriptor(GDT_IDX_CODE_FLAT, 0, 0x100000, GDT_CODE_PL0);
  set_descriptor(GDT_IDX_DATA_FLAT, 0, 0x100000, GDT_DATA_PL0);

  /* Load GDTR */
  __asm__ __volatile__ ("lgdt %0" :: "m" (gdtr));
}
/*---------------------------------------------------------------------------*/
void
gdt_insert_boot(unsigned int idx, segment_desc_t desc)
{
  ((segment_desc_t *)KERN_DATA_OFF_TO_PHYS_ADDR(gdt))[idx] = desc;
}
/*---------------------------------------------------------------------------*/
void
gdt_insert(unsigned int idx, segment_desc_t desc)
{
  if(GDT_LEN <= idx) {
    halt();
  }

  KERN_WRITEL(gdt[idx].raw_lo, desc.raw_lo);
  KERN_WRITEL(gdt[idx].raw_hi, desc.raw_hi);
}
/*---------------------------------------------------------------------------*/
void
gdt_lookup(unsigned int idx, segment_desc_t *desc)
{
  if((GDT_LEN <= idx) || (desc == NULL)) {
    halt();
  }

  KERN_READL(desc->raw_lo, gdt[idx].raw_lo);
  KERN_READL(desc->raw_hi, gdt[idx].raw_hi);
}
/*---------------------------------------------------------------------------*/
