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

#include "gdt.h"
#include "gdt-layout.h"
#include "prot-domains.h"
#include "segmentation.h"
#include "stacks.h"
#include "tss.h"

/* System-wide TSS */
tss_t ATTR_BSS_KERN sys_tss;

static segment_desc_t ATTR_BSS_GDT sys_tss_desc;

/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize system-wide TSS.
 */
void
tss_init(void)
{
  segment_desc_t seg_desc;

  /* Initialize TSS */
  KERN_WRITEW(sys_tss.iomap_base, sizeof(sys_tss));
  KERN_WRITEL(sys_tss.esp2, ((uint32_t)stacks_int) + STACKS_SIZE_INT);
  KERN_WRITEL(sys_tss.ss2, GDT_SEL_STK_INT);
  KERN_WRITEL(sys_tss.esp0, ((uint32_t)stacks_exc) + STACKS_SIZE_EXC);
  KERN_WRITEL(sys_tss.ss0, GDT_SEL_STK_EXC);

  segment_desc_init(&seg_desc,
                    KERN_DATA_OFF_TO_PHYS_ADDR(&sys_tss), sizeof(sys_tss),
                    SEG_FLAG(DPL, PRIV_LVL_EXC) |
                    SEG_DESCTYPE_SYS | SEG_TYPE_TSS32_AVAIL);
  gdt_insert(GDT_IDX_OF_DESC(&sys_tss_desc), seg_desc);

  __asm__ __volatile__ (
    "ltr %0"
    :
    : "r" ((uint16_t)GDT_SEL_OF_DESC(&sys_tss_desc, 0)));
}
/*---------------------------------------------------------------------------*/
