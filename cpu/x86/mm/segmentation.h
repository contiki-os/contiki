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

#ifndef CPU_X86_MM_SEGMENTATION_H_
#define CPU_X86_MM_SEGMENTATION_H_

#include <stdint.h>

#define SEG_FLAG(lbl, val)                                                    \
  (((val) & (~0u >> (32 - SEG_WIDTH_##lbl))) << SEG_SHAMT_##lbl)

#define SEG_SET_FLAG(desc, lbl, val)                                          \
  (desc).flags = ((desc).flags & ~SEG_FLAG(lbl, ~0u)) | SEG_FLAG(lbl, val)

#define SEG_WIDTH_TYPE       4
#define SEG_SHAMT_TYPE       0
#define SEG_WIDTH_DESCTYPE   1
#define SEG_SHAMT_DESCTYPE   4
#define SEG_WIDTH_DPL        2
#define SEG_SHAMT_DPL        5
#define SEG_WIDTH_PRESENT    1
#define SEG_SHAMT_PRESENT    7
#define SEG_WIDTH_LIMIT_HI   4
#define SEG_SHAMT_LIMIT_HI   8
#define SEG_WIDTH_AVL        1
#define SEG_SHAMT_AVL        12
#define SEG_WIDTH_LONG_MODE  1
#define SEG_SHAMT_LONG_MODE  13
/* also used to indicate default operand and address size */
#define SEG_WIDTH_DIRECTION  1
#define SEG_SHAMT_DIRECTION  14
#define SEG_WIDTH_GRAN       1
#define SEG_SHAMT_GRAN       15

#define SEG_TYPE_DATA_RDONLY SEG_FLAG(TYPE, 0x00) /* Read only */
#define SEG_TYPE_DATA_RDWR   SEG_FLAG(TYPE, 0x02) /* Read/Write */
#define SEG_TYPE_CODE_EXRD   SEG_FLAG(TYPE, 0x0A) /* Execute/Read */
#define SEG_TYPE_CODE_EX     SEG_FLAG(TYPE, 0x08) /* Execute only */
#define SEG_TYPE_LDT         SEG_FLAG(TYPE, 0x02)
#define SEG_TYPE_TSS32_AVAIL SEG_FLAG(TYPE, 0x09)

#define SEG_DESCTYPE_SYS     SEG_FLAG(DESCTYPE, 0)
#define SEG_DESCTYPE_NSYS    SEG_FLAG(DESCTYPE, 1)

#define SEG_PRESENT          SEG_FLAG(PRESENT, 1)

#define SEG_DEFL_OPSZ_32BIT  SEG_FLAG(DIRECTION, 1)

#define SEG_GRAN_BYTE        SEG_FLAG(GRAN, 0)
#define SEG_GRAN_PAGE        SEG_FLAG(GRAN, 1)

/**
 * Maximum length of segment that can be regulated with a byte-granularity
 * segment limit.
 */
#define SEG_MAX_BYTE_GRAN_LEN (1 << 20)

/**
 * Segment descriptor.  See Intel Combined Manual,
 * Vol. 3, Section 3.4.5 for more details.
 */
typedef union segment_desc {
  struct {
    uint32_t lim_lo     : 16;
    uint32_t base_lo    : 16;
    uint32_t base_mid   : 8;
    uint32_t flags      : 16;
    uint32_t base_hi    : 8;
  };
  struct {
    uint32_t raw_lo, raw_hi;
  };
  uint64_t raw;
} segment_desc_t;

#define SEG_DESC_NOT_PRESENT 0

/* The next two functions are invoked by boot code, so they must always be
 * inlined to avoid being placed in a different address space than the initial,
 * flat address space.
 */
static inline void __attribute__((always_inline))
segment_desc_set_limit(segment_desc_t *c_this, uint32_t len)
{
  uint32_t limit = len - 1;

  SEG_SET_FLAG(*c_this, LIMIT_HI, limit >> 16); /* set limit bits 19:16 */
  c_this->lim_lo = limit; /* set limit bits 15:0 */
}
/**
 * \brief        Initialize a segment descriptor.
 * \param c_this Segment descriptor to be initialized.
 * \param base   Base address of region to be covered by segment descriptor.
 * \param len    Length to be specified by segment descriptor.  The units may
 *               be bytes or pages, depending on the flags.
 * \param flags  Flags to be added to the default flags: present, default
 *               operand size of 32 bits, and high limit bits.
 */
static inline void __attribute__((always_inline))
segment_desc_init(segment_desc_t *c_this,
                  uint32_t base, uint32_t len, uint16_t flags)
{
  c_this->raw = 0;

  /* Create the high 32 bit segment */
  c_this->base_mid = base >> 16; /* set base bits 23:16 */
  c_this->base_hi = base >> 24; /* set base bits 31:24 */

  /* Create the low 32 bit segment */
  c_this->base_lo = base; /* set base bits 15:0 */

  c_this->flags = SEG_FLAG(PRESENT, 1) | SEG_DEFL_OPSZ_32BIT | flags;

  /* This must be done after setting the other flags, or else it
   * would be partially overridden.
   */
  segment_desc_set_limit(c_this, len);
}
#endif /* CPU_X86_MM_SEGMENTATION_H_ */
