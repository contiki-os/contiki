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

#ifndef CPU_X86_MM_LDT_LAYOUT_H_
#define CPU_X86_MM_LDT_LAYOUT_H_

#include "gdt-layout.h"

/* Each LDT can contain up to this many descriptors, but some protection
 * domains may not use all of the slots.
 */
#define LDT_NUM_DESC 3

/**
 * Provides access to kernel data.  Most protection domains are granted at most
 * read-only access, but the kernel protection domain is granted read/write
 * access.
 */
#define LDT_IDX_KERN 0
/** Maps a device MMIO range */
#define LDT_IDX_MMIO 1
/** Maps domain-defined metadata */
#define LDT_IDX_META 2

#define LDT_SEL(idx, rpl) (GDT_SEL(idx, rpl) | (1 << 2))

#define LDT_SEL_KERN      LDT_SEL(LDT_IDX_KERN, PRIV_LVL_USER)
#define LDT_SEL_MMIO      LDT_SEL(LDT_IDX_MMIO, PRIV_LVL_USER)
#define LDT_SEL_META      LDT_SEL(LDT_IDX_META, PRIV_LVL_USER)
#define LDT_SEL_STK       LDT_SEL(LDT_IDX_STK, PRIV_LVL_USER)

#endif /* CPU_X86_MM_LDT_LAYOUT_H_ */
