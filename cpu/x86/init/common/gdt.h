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

#ifndef GDT_H
#define GDT_H

#include "gdt-layout.h"
#include "prot-domains.h"
#include "segmentation.h"

extern segment_desc_t gdt[];
extern int _ebss_gdt_addr;

#define GDT_IDX_OF_DESC(ptr)                      \
       ((((uintptr_t)(ptr)) - ((uintptr_t)&gdt))/ \
        sizeof(segment_desc_t))

/**
 * \brief     Compute the selector for a GDT entry allocated somewhere besides gdt.c.
 * \param ptr Pointer to GDT descriptor.
 * \param rpl Requested Privilege Level.
 */
#define GDT_SEL_OF_DESC(ptr, rpl) GDT_SEL(GDT_IDX_OF_DESC(ptr), rpl)

#define ATTR_BSS_GDT       __attribute__((section(".gdt_bss")))
#define ATTR_BSS_GDT_START __attribute__((section(".gdt_bss_start")))

void gdt_copy_desc_change_dpl(unsigned int dest_idx,
                              unsigned int src_idx,
                              unsigned dpl);
void gdt_init(void) ATTR_CODE_BOOT;
void gdt_insert(unsigned int idx, segment_desc_t desc);
void gdt_lookup(unsigned int idx, segment_desc_t *desc);

#endif /* GDT_H */
