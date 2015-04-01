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

#define NUM_DESC 3

/* Each define here is for a specific flag in the descriptor. Refer to Intel
 * Combined Manual (Intel 64 and IA-32 Architectures Software Developer's
 * Manual), Vol. 3, Section 3.4.5 for a description of each flag.
 */
#define SEG_DESCTYPE(x)  ((x) << 0x04) /* Descriptor type (0 for system, 1 for code/data) */
#define SEG_PRES(x)      ((x) << 0x07) /* Present */
#define SEG_SAVL(x)      ((x) << 0x0C) /* Available for system use */
#define SEG_LONG(x)      ((x) << 0x0D) /* Long mode */
#define SEG_SIZE(x)      ((x) << 0x0E) /* Size (0 for 16-bit, 1 for 32) */
#define SEG_GRAN(x)      ((x) << 0x0F) /* Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB) */
#define SEG_PRIV(x)      (((x) &  0x03) << 0x05) /* Set privilege level (0 - 3) */

#define SEG_DATA_RDWR    0x02 /* Read/Write */
#define SEG_CODE_EXRD    0x0A /* Execute/Read */

#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_CODE_EXRD

#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_DATA_RDWR

typedef struct gdtr
{
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) gdtr_t;

typedef uint64_t segment_desc_t;

/* From Intel Combined Manual, Vol. 3 , Section 3.5.1: The base addresses of
 * the GDT should be aligned on an eight-byte boundary to yield the best
 * processor performance.
 */
static segment_desc_t gdt[NUM_DESC] __attribute__ ((aligned (8)));

static void
set_descriptor(unsigned int index, uint32_t base, uint32_t limit, uint16_t flag)
{
  segment_desc_t descriptor;

  if (index >= NUM_DESC)
    return;

  /* Create the high 32 bit segment */
  descriptor  =  limit       & 0x000F0000; /* set limit bits 19:16 */
  descriptor |= (flag <<  8) & 0x00F0FF00; /* set type, p, dpl, s, g, d/b, l and avl fields */
  descriptor |= (base >> 16) & 0x000000FF; /* set base bits 23:16 */
  descriptor |=  base        & 0xFF000000; /* set base bits 31:24 */

  /* Shift by 32 to allow for low part of segment */
  descriptor <<= 32;

  /* Create the low 32 bit segment */
  descriptor |= base  << 16; /* set base bits 15:0 */
  descriptor |= limit  & 0x0000FFFF; /* set limit bits 15:0 */

  /* Save descriptor into gdt */
  gdt[index] = descriptor;
}


/* This function initializes the Global Offset Table. For simplicity, the
 * memory is organized following the flat model. Thus, memory appears to
 * Contiki as a single continuous address space. Code, data, and stack
 * are all contained in this address space (so called linear address space).
 */
void
gdt_init(void)
{
  gdtr_t gdtr;

  /* Initialize gdtr structure */
  gdtr.limit = sizeof(segment_desc_t) * NUM_DESC - 1;
  gdtr.base = (uint32_t) &gdt;

  /* Initialize descriptors */
  set_descriptor(0, 0, 0, 0);
  set_descriptor(1, 0, 0x0FFFFF, GDT_CODE_PL0);
  set_descriptor(2, 0, 0x0FFFFF, GDT_DATA_PL0);

  /* Load GDTR register and update segment registers.
   *
   * In protected mode, segment registers should be loaded according to
   * the offset in GDT. So DS, SS, ES, FS and GS registers should be
   * loadded with 0x10 while CS with 0x08. CS register cannot be changed
   * directly. For that reason, we do a far jump.
   */
  __asm__ ("lgdt %0\n\t"
           "jmp $0x08, $1f\n\t"
           "1:\n\t"
           "mov $0x10, %%ax\n\t"
           "mov %%ax, %%ds\n\t"
           "mov %%ax, %%ss\n\t"
           "mov %%ax, %%es\n\t"
           "mov %%ax, %%fs\n\t"
           "mov %%ax, %%gs\n\t"
      :
      : "m" (gdtr)
  );
}
