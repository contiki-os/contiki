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

#ifndef CPU_X86_MM_SWSEG_PROT_DOMAINS_H_
#define CPU_X86_MM_SWSEG_PROT_DOMAINS_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ldt-layout.h"
#include "paging.h"
#include "segmentation.h"
#include "syscalls-int.h"

struct dom_kern_data {
  /** Local Descriptor Table with per-domain descriptors */
  segment_desc_t ldt[LDT_NUM_DESC];
  /** Flags are defined with the prefix PROT_DOMAINS_FLAG in prot-domains.h */
  uint32_t flags;
  /**
   * Original return address from call stack when this protection domain
   * invoked some other protection domain.  This serves to control the return
   * entrypoint.  The callee is not permitted to modify this value (unless the
   * callee is the kernel protection domain).
   */
  uintptr_t orig_ret_addr;

  /* This structure is precisely 32 bytes in length, a power of 2.  If its size
   * changes, add an alignment attribute to keep it aligned at a power of 2 so
   * that dereferencing arrays of these structures uses shift instructions
   * instead of multiplication.  Shifting is faster than multiplication.
   */
};

/* relies on dom_kern_data: */
#include "multi-segment.h"

#define PROT_DOMAINS_ENTER_ISR(exc)                                           \
  MULTI_SEGMENT_ENTER_ISR(exc)                                                \
  PROT_DOMAINS_ENTER_ISR_COMMON(exc)
#define PROT_DOMAINS_LEAVE_ISR(exc)                                           \
  PROT_DOMAINS_LEAVE_ISR_COMMON(exc)                                          \
  MULTI_SEGMENT_LEAVE_ISR(exc)

#define prot_domains_impl_init syscalls_int_init

#define prot_domains_set_wp(en)

/* Allocate one additional GDT entry for each protection domain.  Note that
 * the particular storage allocated by this statement may actually be used for
 * some other protection domain, depending on how the linker happens to arrange
 * all of the GDT storage.  The GDT_IDX_LDT macro in gdt-layout.h determine
 * which storage is used for each protection domain.  Thus, this storage should
 * not be referenced directly by its variable name.
 */
#define PROT_DOMAINS_ALLOC_IMPL(nm)                                           \
  static segment_desc_t ATTR_BSS_GDT_MID _gdt_storage_##nm

#endif /* CPU_X86_MM_SWSEG_PROT_DOMAINS_H_ */
