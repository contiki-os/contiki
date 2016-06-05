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

#ifndef CPU_X86_MM_MULTI_SEGMENT_H_
#define CPU_X86_MM_MULTI_SEGMENT_H_

#include <stdint.h>
#include <stdlib.h>
#include "helpers.h"
#include "ldt-layout.h"

#ifdef __clang__
#define __SEG_FS
#define __seg_fs __attribute__((address_space(257)))
#define __SEG_GS
#define __seg_gs __attribute__((address_space(256)))
#endif

#ifdef __SEG_FS
#define ATTR_MMIO_ADDR_SPACE __seg_fs
#define ATTR_KERN_ADDR_SPACE __seg_fs
#else
#define ATTR_KERN_ADDR_SPACE
#endif
#ifdef __SEG_GS
#define ATTR_META_ADDR_SPACE __seg_gs
#endif

void prot_domains_reg_multi_seg(volatile struct dom_kern_data ATTR_KERN_ADDR_SPACE *dkd,
                                uintptr_t mmio, size_t mmio_sz,
                                uintptr_t meta, size_t meta_sz);
void multi_segment_launch_kernel(void);

#define MULTI_SEGMENT_ENTER_ISR(exc)                                          \
  "mov $" EXP_STRINGIFY(GDT_SEL_DATA) ", %%eax\n\t"                           \
  /* Refresh DS and ES in case the userspace code corrupted them.         */  \
  "mov %%eax, %%ds\n\t"                                                       \
  "mov %%eax, %%es\n\t"                                                       \
  /* Refresh SEG_KERN.                                                    */  \
  "mov $" EXP_STRINGIFY(LDT_SEL_KERN) ", %%eax\n\t"                           \
  "mov %%eax, %%" SEG_KERN "s\n\t"                                            \
  ".if " #exc "\n\t"                                                          \
  /* It is possible that a routine performing MMIO is being interrupted.  */  \
  /* Thus, it is necessary to save and restore the MMIO segment register  */  \
  /* (in a callee-saved register).                                        */  \
  "mov %%" SEG_MMIO "s, %%ebp\n\t"                                            \
  "mov $" EXP_STRINGIFY(GDT_SEL_DATA_KERN_EXC) ", %%eax\n\t"                  \
  "mov %%eax, %%" SEG_KERN "s\n\t"                                            \
  ".endif\n\t"
#define MULTI_SEGMENT_LEAVE_ISR(exc)                                          \
  ".if " #exc "\n\t"                                                          \
  "mov %%ebp, %%" SEG_MMIO "s\n\t"                                            \
  ".endif\n\t"

/**
 * The MMIO region is tightly bounded within a segment, so its base offset is
 * always 0.
 */
#define PROT_DOMAINS_MMIO(dcd) 0
/**
 * The metadata region is tightly bounded within a segment, so its base offset
 * is always 0.
 */
#define PROT_DOMAINS_META(dcd) 0

#define SEG_MMIO "f" /**< For MMIO accesses, when enabled. */
#define SEG_KERN "f" /**< For kernel data accesses */
#define SEG_META "g" /**< For metadata accesses */

#define _SEG_READL(seg, dst, src)                                             \
  __asm__ __volatile__ (                                                      \
    "movl %%" seg "s:%[src_], %[dst_]" : [dst_]"=r"(dst) : [src_]"m"(src))

#define _SEG_READW(seg, dst, src)                                             \
  __asm__ __volatile__ (                                                      \
    "movw %%" seg "s:%[src_], %[dst_]" : [dst_]"=r"(dst) : [src_]"m"(src))

#define _SEG_READB(seg, dst, src)                                             \
  __asm__ __volatile__ (                                                      \
    "movb %%" seg "s:%[src_], %[dst_]" : [dst_]"=q"(dst) : [src_]"m"(src))

#define _SEG_WRITEL(seg, dst, src)                                            \
  __asm__ __volatile__ (                                                      \
    "movl %[src_], %%" seg "s:%[dst_]"                                        \
    : [dst_]"=m"(dst) : [src_]"r"((uint32_t)(src)))

#define _SEG_WRITEW(seg, dst, src)                                            \
  __asm__ __volatile__ (                                                      \
    "movw %[src_], %%" seg "s:%[dst_]"                                        \
    : [dst_]"=m"(dst) : [src_]"r"((uint16_t)(src)))

#define _SEG_WRITEB(seg, dst, src)                                            \
  __asm__ __volatile__ (                                                      \
    "movb %[src_], %%" seg "s:%[dst_]"                                        \
    : [dst_]"=m"(dst) : [src_]"q"((uint8_t)(src)))

#ifndef __SEG_FS
#define MMIO_READL(dst, src)  _SEG_READL(SEG_MMIO, dst, src)
#define MMIO_READW(dst, src)  _SEG_READW(SEG_MMIO, dst, src)
#define MMIO_READB(dst, src)  _SEG_READB(SEG_MMIO, dst, src)
#define MMIO_WRITEL(dst, src) _SEG_WRITEL(SEG_MMIO, dst, src)
#define MMIO_WRITEW(dst, src) _SEG_WRITEW(SEG_MMIO, dst, src)
#define MMIO_WRITEB(dst, src) _SEG_WRITEB(SEG_MMIO, dst, src)

#define KERN_READL(dst, src)  _SEG_READL(SEG_KERN, dst, src)
#define KERN_READW(dst, src)  _SEG_READW(SEG_KERN, dst, src)
#define KERN_READB(dst, src)  _SEG_READB(SEG_KERN, dst, src)
#define KERN_WRITEL(dst, src) _SEG_WRITEL(SEG_KERN, dst, src)
#define KERN_WRITEW(dst, src) _SEG_WRITEW(SEG_KERN, dst, src)
#define KERN_WRITEB(dst, src) _SEG_WRITEB(SEG_KERN, dst, src)
#endif

#ifndef __SEG_GS
#define META_READL(dst, src)  _SEG_READL(SEG_META, dst, src)
#define META_READW(dst, src)  _SEG_READW(SEG_META, dst, src)
#define META_READB(dst, src)  _SEG_READB(SEG_META, dst, src)
#define META_WRITEL(dst, src) _SEG_WRITEL(SEG_META, dst, src)
#define META_WRITEW(dst, src) _SEG_WRITEW(SEG_META, dst, src)
#define META_WRITEB(dst, src) _SEG_WRITEB(SEG_META, dst, src)
#endif

#define MEMCPY_FROM_META(dst, src, sz)                                        \
  {                                                                           \
    uintptr_t __dst = (uintptr_t)(dst);                                       \
    uintptr_t __src = (uintptr_t)(src);                                       \
    size_t __sz = (size_t)(sz);                                               \
    __asm__ __volatile__ (                                                    \
      "rep movsb %%" SEG_META "s:(%%esi), %%es:(%%edi)\n\t"                   \
      : "+D"(__dst), "+S"(__src), "+c"(__sz));                                \
  }

#define MEMCPY_TO_META(dst, src, sz)                                          \
  {                                                                           \
    uintptr_t __dst = (uintptr_t)(dst);                                       \
    uintptr_t __src = (uintptr_t)(src);                                       \
    size_t __sz = (size_t)(sz);                                               \
    __asm__ __volatile__ (                                                    \
      "push %%es\n\t"                                                         \
      "push %%" SEG_META "s\n\t"                                              \
      "pop %%es\n\t"                                                          \
      "rep movsb\n\t"                                                         \
      "pop %%es\n\t"                                                          \
      : "+D"(__dst), "+S"(__src), "+c"(__sz));                                \
  }

/** Compute physical address from offset into kernel data space */
#define KERN_DATA_OFF_TO_PHYS_ADDR(x)                                         \
  (((uintptr_t)&_sbss_kern_addr) + (uintptr_t)(x))
/** Compute physical address from offset into default data space */
#define DATA_OFF_TO_PHYS_ADDR(x)                                              \
  (((uintptr_t)&_sdata_addr) + (uintptr_t)(x))
/** Compute kernel data offset from physical address in kernel data space */
#define PHYS_ADDR_TO_KERN_DATA_OFF(x)                                         \
  (((uintptr_t)(x)) - (uintptr_t)&_sbss_kern_addr)

/**
 * In multi-segment protection domain implementations, it is sufficient to just
 * compare incoming pointers against the frame pointer.  All incoming pointers
 * are dereferenced in the main data segment, which only maps the stacks and
 * the shared data section.  Since the shared data section is at a higher
 * address range than the stacks, the frame pointer check is sufficient.
 */
#define PROT_DOMAINS_CHECK_INCOMING_PTR PROT_DOMAINS_CHECK_INCOMING_PTR_EBP

void prot_domains_enable_mmio(void);
void prot_domains_disable_mmio(void);

#endif /* CPU_X86_MM_MULTI_SEGMENT_H_ */
