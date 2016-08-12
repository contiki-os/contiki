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

#ifndef CPU_X86_MM_SYSCALLS_INT_H_
#define CPU_X86_MM_SYSCALLS_INT_H_

/** Software interrupt number for dispatching a system call */
#define PROT_DOMAINS_SYSCALL_DISPATCH_INT 100
/** Software interrupt number for returning from a system call */
#define PROT_DOMAINS_SYSRET_DISPATCH_INT  101

#if !__ASSEMBLER__

#include <stdint.h>

extern dom_id_t cur_dom;

#define SYSCALLS_STUB_EPILOGUE(nm)                                            \
  /* Load the system call identifier into EAX, as required by */              \
  /* prot_domains_syscall_dispatcher: */                                      \
  "  mov $" EXP_STRINGIFY(_syscall_ent_##nm) ", %eax\n\t"                     \
  /* Check whether the server protection domain is already active: */         \
  "  cmp %edx, cur_dom\n\t"                                                   \
  /* If so, skip the system call dispatcher and directly invoke the */        \
  /* system call body: */                                                     \
  "  je _syscall_" #nm "\n\t"                                                 \
  "  int $" EXP_STRINGIFY(PROT_DOMAINS_SYSCALL_DISPATCH_INT) "\n\t"

#define SYSCALLS_STUB(nm)                                                     \
  SYSCALLS_ALLOC_ENTRYPOINT(nm);                                              \
  asm (                                                                       \
    ".text\n\t"                                                               \
    ".global " #nm "\n\t"                                                     \
    #nm ":\n\t"                                                               \
    /* First, load server protection domain ID into EDX, as required by */    \
    /* prot_domains_syscall_dispatcher: */                                    \
    /* Skip past return address on stack to obtain address of protection */   \
    /* domain ID parameter: */                                                \
    "  mov 4(%esp), %edx\n\t"                                                 \
    SYSCALLS_STUB_EPILOGUE(nm))

#define SYSCALLS_STUB_SINGLETON(nm, dcd)                                      \
  SYSCALLS_ALLOC_ENTRYPOINT(nm);                                              \
  asm (                                                                       \
    ".text\n\t"                                                               \
    ".global " #nm "\n\t"                                                     \
    #nm ":\n\t"                                                               \
    /* First, load server protection domain ID into EDX, as required by */    \
    /* prot_domains_syscall_dispatcher: */                                    \
    "  mov %" SEG_KERN "s:" #dcd ", %edx\n\t"                                 \
    SYSCALLS_STUB_EPILOGUE(nm))

void syscalls_int_init(void);

void prot_domains_sysret_stub(void);

/* Inter-privilege level interrupt stack with no error code. */
typedef struct interrupt_stack {
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t esp;
  uint32_t ss;
} interrupt_stack_t;

#if 0
/* Declaration only included for documentation purposes: */
/**
 * \brief         Switch to a different protection domain.
 * \param from_id Origin protection domain.
 * \param to_id   Destination protection domain.
 * \return        Segment selector for kernel data access (only used for
 *                multi-segment implementations).
 */
uint32_t prot_domains_switch(dom_id_t from_id,
                             dom_id_t to_id,
                             interrupt_stack_t *intr_stk);
#endif

#endif

#endif /* CPU_X86_MM_SYSCALLS_INT_H_ */
