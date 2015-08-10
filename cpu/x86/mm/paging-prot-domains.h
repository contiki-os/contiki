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

#ifndef CPU_X86_MM_PAGING_PROT_DOMAINS_H_
#define CPU_X86_MM_PAGING_PROT_DOMAINS_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dma.h"
#include "helpers.h"
#include "paging.h"
#include "syscalls-int.h"

struct dom_kern_data {
  /** Base physical address of optional MMIO region */
  uintptr_t mmio;
  /** Number of (contiguous) pages in MMIO region */
  size_t mmio_sz;
  /** Base physical address of optional metadata region */
  uintptr_t meta;
  /** Number of (contiguous) pages in metadata region */
  size_t meta_sz;
  /** Flags are defined with the prefix PROT_DOMAINS_FLAG in prot-domains.h */
  uint32_t flags;
  /**
   * Original return address from call stack when this protection domain
   * invoked some other protection domain.  This serves to control the return
   * entrypoint.  The callee is not permitted to modify this value (unless the
   * callee is the kernel protection domain).
   */
  uintptr_t orig_ret_addr;

  /* align to next-larger power of 2 to enable usage of shifting instead of
   * multiplication to index an array of these structures.
   */
} __attribute__((aligned(32)));

/** Linear base address at which to map the MMIO region. */
#define PROT_DOMAINS_MMIO_LINEAR_BASE (MIN_PAGE_SIZE + (uintptr_t)&_ebss_kern_addr)

/** Maximum supported size of MMIO region */
#define PROT_DOMAINS_MAX_MMIO_SZ 0x4000

/** Linear base address at which to map the metadata region */
#define PROT_DOMAINS_META_LINEAR_BASE \
  (MIN_PAGE_SIZE + (PROT_DOMAINS_MMIO_LINEAR_BASE + PROT_DOMAINS_MAX_MMIO_SZ))

#define PROT_DOMAINS_META_OFF_TO_PHYS(off, meta_phys_base)                    \
  ((meta_phys_base) + ((off) - PROT_DOMAINS_META_LINEAR_BASE))

/** Any MMIO region mapping always starts at a particular linear address. */
#define PROT_DOMAINS_MMIO(dcd) PROT_DOMAINS_MMIO_LINEAR_BASE
/**
 * Any metadata region mapping always starts at a particular linear address.
 */
#define PROT_DOMAINS_META(dcd) PROT_DOMAINS_META_LINEAR_BASE

#define PROT_DOMAINS_ENTER_ISR(exc)                                           \
  PROT_DOMAINS_ENTER_ISR_COMMON(exc)
#define PROT_DOMAINS_LEAVE_ISR(exc) PROT_DOMAINS_LEAVE_ISR_COMMON(exc)

/* Enable paging */
#define CR0_PG BIT(31)
/* Enable write protection in supervisor mode */
#define CR0_WP BIT(16)
/* Enable protected mode */
#define CR0_PE BIT(0)

/**
 * \brief    Enable or disable write protection enforcement in supervisor mode.
 *           When disabled, supervisory code (i.e. code running at ring levels
 *           0-2) is permitted to write to pages that are marked read-only in
 *           page tables.
 *
 * \param en Set to true to enable write protection enforcement.
 */
static inline void prot_domains_set_wp(bool en)
{
  uint32_t cr0_val = CR0_PG | CR0_PE;
  if(en) {
    cr0_val |= CR0_WP;
  }
  __asm__ __volatile__ ("mov %0, %%cr0" :: "r"(cr0_val));
}

#endif /* CPU_X86_MM_PAGING_PROT_DOMAINS_H_ */
