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

#ifndef CPU_X86_MM_TSS_PROT_DOMAINS_H_
#define CPU_X86_MM_TSS_PROT_DOMAINS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "ldt-layout.h"
#include "segmentation.h"
#include "tss.h"

struct dom_kern_data {
  /** Task State Segment */
  tss_t tss;
  /** Local Descriptor Table with per-domain descriptors */
  segment_desc_t ldt[LDT_NUM_DESC];
} __attribute__((packed));

/* relies on dom_kern_data: */
#include "multi-segment.h"

/* relies on ATTR_KERN_ADDR_SPACE: */
#include "syscalls.h"

/**
 * Data associated with each protection domain that is owned by clients of that
 * domain and used to identify the domain.
 */
struct dom_client_data {
  dom_id_t dom_id;
  /** The selector is only 16 bits, but it is padded to 32 bits. */
  uint32_t tss_sel;
};

extern uint32_t prot_domains_main_esp;

#define SYSCALLS_STUB_MIDDLE(nm)                                              \
  /* If already in the callee protection domain, skip the protection */       \
  /* domain switch and directly invoke the system call body */                \
  "  je _syscall_" #nm "\n\t"                                                 \
  "  movl $" EXP_STRINGIFY(_syscall_ent_##nm) ", prot_domains_syscall\n\t"    \
  "  mov %esp, prot_domains_main_esp\n\t"

#define SYSCALLS_STUB(nm)                                                     \
  SYSCALLS_ALLOC_ENTRYPOINT(nm);                                              \
  asm (                                                                       \
    ".text\n\t"                                                               \
    ".global " #nm "\n\t"                                                     \
    #nm ":\n\t"                                                               \
    "  str %ax\n\t"                                                           \
    /* Compare current Task Register selector to selector for callee */       \
    /* protection domain, in tss_sel field of dom_client_data */              \
    "  cmpw %ax, 8(%esp)\n\t"                                                 \
    SYSCALLS_STUB_MIDDLE(nm)                                                  \
    /* This will treat the dom_id field as the offset for the call, but */    \
    /* that is ignored when performing a far call to a task */                \
    "  lcall *4(%esp)\n\t"                                                    \
    "  ret\n\t")

#define SYSCALLS_STUB_SINGLETON(nm, dcd)                                      \
  SYSCALLS_ALLOC_ENTRYPOINT(nm);                                              \
  asm (                                                                       \
    ".text\n\t"                                                               \
    ".global " #nm "\n\t"                                                     \
    #nm ":\n\t"                                                               \
    "  str %ax\n\t"                                                           \
    /* Compare current Task Register selector to selector for callee */       \
    /* protection domain, in tss_sel field of dom_client_data */              \
    "  cmpw %ax, %" SEG_KERN "s:(4 + " #dcd ")\n\t"                           \
    SYSCALLS_STUB_MIDDLE(nm)                                                  \
    /* This will treat the dom_id field as the offset for the call, but */    \
    /* that is ignored when performing a far call to a task */                \
    "  lcall *%" SEG_KERN "s:" #dcd "\n\t"                                    \
    "  ret\n\t")

#define PROT_DOMAINS_ENTER_ISR(exc)                                           \
  MULTI_SEGMENT_ENTER_ISR(exc)                                                \
  /* It is possible that the system call dispatcher is being interrupted, */  \
  /* and some interrupt handlers perform system calls.  Thus, it is       */  \
  /* necessary to save and restore the system call dispatcher parameters  */  \
  /* (in callee-saved registers).                                         */  \
  "mov prot_domains_main_esp, %%esi\n\t"                                      \
  "mov prot_domains_syscall, %%edi\n\t"                                       \
  PROT_DOMAINS_ENTER_ISR_COMMON(exc)
#define PROT_DOMAINS_LEAVE_ISR(exc)                                           \
  PROT_DOMAINS_LEAVE_ISR_COMMON(exc)                                          \
  "mov %%edi, prot_domains_syscall\n\t"                                       \
  "mov %%esi, prot_domains_main_esp\n\t"                                      \
  MULTI_SEGMENT_LEAVE_ISR(exc)

/* Allocate two additional GDT entries for each protection domain.  Note that
 * the particular storage allocated by this statement may actually be used for
 * some other protection domain, depending on how the linker happens to arrange
 * all of the GDT storage.  The GDT_IDX_TSS and GDT_IDX_LDT macros in
 * gdt-layout.h determine which storage is used for each protection domain.
 * Thus, this storage should not be referenced directly by its variable name.
 */
#define PROT_DOMAINS_ALLOC_IMPL(nm)                                           \
  static segment_desc_t ATTR_BSS_GDT_MID _gdt_storage_##nm[2]

#endif /* CPU_X86_MM_TSS_PROT_DOMAINS_H_ */
