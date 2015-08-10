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

#ifndef CPU_X86_MM_GDT_LAYOUT_H_
#define CPU_X86_MM_GDT_LAYOUT_H_

#include "prot-domains.h"

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
/**
 * Number of fixed GDT descriptors.  Additional descriptors may be defined
 * outside of gdt.c.
 */
#define GDT_NUM_FIXED_DESC    7
#else
#define GDT_NUM_FIXED_DESC    3
#endif

#define GDT_IDX_NULL          0
/**
 * Flat code segment, used at boot and also for the rest of the system's
 * runtime when protection domains are disabled
 */
#define GDT_IDX_CODE_FLAT     1
/**
 * Flat data segment, used at boot and also for the rest of the system's
 * runtime when protection domains are disabled
 */
#define GDT_IDX_DATA_FLAT     2

#if X86_CONF_PROT_DOMAINS != X86_CONF_PROT_DOMAINS__NONE
/** Default (post-boot) code segment */
#define GDT_IDX_CODE          3
/**
 * Same bounds and permissions as default code segment, but at the interrupt
 * handler privilege level
 */
#define GDT_IDX_CODE_INT      4
/** Stack segment for interrupt handlers */
#define GDT_IDX_STK_INT       5

#define GDT_IDX_CODE_EXC      GDT_IDX_CODE_FLAT
/** Default data segment used by code at all privilege levels */
#define GDT_IDX_DATA          6
#define GDT_IDX_STK           GDT_IDX_DATA
#define GDT_IDX_STK_EXC       GDT_IDX_DATA_FLAT
#else
#define GDT_IDX_CODE          GDT_IDX_CODE_FLAT
#define GDT_IDX_CODE_INT      GDT_IDX_CODE_FLAT
#define GDT_IDX_CODE_EXC      GDT_IDX_CODE_FLAT
#define GDT_IDX_DATA          GDT_IDX_DATA_FLAT
#define GDT_IDX_STK           GDT_IDX_DATA_FLAT
#define GDT_IDX_STK_INT       GDT_IDX_DATA_FLAT
#define GDT_IDX_STK_EXC       GDT_IDX_DATA_FLAT
#endif

#define GDT_SEL(idx, rpl)     (((idx) << 3) | (rpl))

#define DT_SEL_GET_IDX(sel)   ((sel) >> 3)

#define DT_SEL_GET_RPL(sel)   ((sel) & 3)

#define GDT_SEL_NULL          GDT_SEL(GDT_IDX_NULL, 0)
#define GDT_SEL_CODE_FLAT     GDT_SEL(GDT_IDX_CODE_FLAT, PRIV_LVL_EXC)
#define GDT_SEL_DATA_FLAT     GDT_SEL(GDT_IDX_DATA_FLAT, PRIV_LVL_EXC)

#define GDT_SEL_CODE          GDT_SEL(GDT_IDX_CODE, PRIV_LVL_USER)
#define GDT_SEL_CODE_INT      GDT_SEL(GDT_IDX_CODE_INT, PRIV_LVL_INT)
#define GDT_SEL_CODE_EXC      GDT_SEL(GDT_IDX_CODE_EXC, PRIV_LVL_EXC)

#define GDT_SEL_DATA          GDT_SEL(GDT_IDX_DATA, PRIV_LVL_EXC)

#define GDT_SEL_STK           GDT_SEL(GDT_IDX_STK, PRIV_LVL_USER)
#define GDT_SEL_STK_INT       GDT_SEL(GDT_IDX_STK_INT, PRIV_LVL_INT)
#define GDT_SEL_STK_EXC       GDT_SEL(GDT_IDX_STK_EXC, PRIV_LVL_EXC)

#endif /* CPU_X86_MM_GDT_LAYOUT_H_ */

