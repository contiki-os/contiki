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

#ifndef CPU_X86_MM_SYSCALLS_H_
#define CPU_X86_MM_SYSCALLS_H_

#include "helpers.h"
#include "prot-domains.h"
#include <stdbool.h>

typedef uint32_t dom_id_bitmap_t;

typedef struct syscalls_entrypoint {
  uintptr_t entrypoint;
  dom_id_bitmap_t doms;
} syscalls_entrypoint_t;
extern syscalls_entrypoint_t ATTR_KERN_ADDR_SPACE syscalls_entrypoints[];
extern syscalls_entrypoint_t ATTR_KERN_ADDR_SPACE syscalls_entrypoints_end[];

#define SYSCALLS_ACTUAL_CNT (syscalls_entrypoints_end - syscalls_entrypoints)

#if X86_CONF_PROT_DOMAINS != X86_CONF_PROT_DOMAINS__NONE

#define SYSCALLS_ALLOC_ENTRYPOINT(nm)                                         \
  syscalls_entrypoint_t __attribute__((section(".syscall_bss")))              \
    ATTR_KERN_ADDR_SPACE _syscall_ent_##nm

#define SYSCALLS_INIT(nm)                                                     \
  KERN_WRITEL(_syscall_ent_##nm.entrypoint, (uintptr_t)_syscall_##nm);        \
  KERN_WRITEL(_syscall_ent_##nm.doms, 0)

#define SYSCALLS_DEFINE(nm, ...)                                              \
  void _syscall_##nm(__VA_ARGS__);                                            \
  SYSCALLS_STUB(nm);                                                          \
  void _syscall_##nm(__VA_ARGS__)

#define SYSCALLS_DEFINE_SINGLETON(nm, dcd, ...)                               \
  void _syscall_##nm(__VA_ARGS__);                                            \
  SYSCALLS_STUB_SINGLETON(nm, dcd);                                           \
  void _syscall_##nm(__VA_ARGS__)

#define SYSCALLS_AUTHZ_UPD(nm, drv, set)                                      \
  {                                                                           \
    dom_id_t _sc_tmp_id;                                                      \
    dom_id_bitmap_t _sc_tmp_bm;                                               \
    KERN_READL(_sc_tmp_id, (drv).dom_id);                                     \
    KERN_READL(_sc_tmp_bm, _syscall_ent_##nm.doms);                           \
    if(set) {                                                                 \
      _sc_tmp_bm |= BIT(_sc_tmp_id);                                          \
    } else {                                                                  \
      _sc_tmp_bm &= ~BIT(_sc_tmp_id);                                         \
    }                                                                         \
    KERN_WRITEL(_syscall_ent_##nm.doms, _sc_tmp_bm);                          \
  }

/**
 * Check that any untrusted pointer that could have been influenced by a caller
 * (i.e. a stack parameter or global variable) refers to a location at or above
 * a certain stack boundary and halt otherwise.  This is used to prevent a
 * protection domain from calling a different protection domain and passing a
 * pointer that references a location in the callee's stack other than its
 * parameters.
 *
 * This also checks that the pointer is either within the stack region or the
 * shared data region, which is important for preventing redirection of data
 * accesses to MMIO or metadata regions.  This check is omitted for multi-
 * segment protection domain implementations, since the segment settings
 * already enforce this property for pointers dereferenced in DS.  Pointers
 * that can be influenced by a caller should not be dereferenced in any other
 * segment.
 *
 * The pointer is both validated and copied to a new storage location, which
 * must be within the callee's local stack region (excluding the parameter
 * region).  This is to mitigate scenarios such as two pointers being validated
 * and an adversary later inducing a write through one of the pointers to the
 * other pointer to corrupt the latter pointer before it is used.
 *
 * The frame address is adjusted to account for the first word pushed on the
 * local frame and the return address, since neither of those should ever be
 * referenced by an incoming pointer.  In particular, if an incoming pointer
 * references the return address, it could potentially redirect execution with
 * the privileges of the callee protection domain.
 */
#if X86_CONF_PROT_DOMAINS_MULTI_SEG
#define PROT_DOMAINS_VALIDATE_PTR(validated, untrusted, sz)                   \
  validated = untrusted;                                                      \
  if(((uintptr_t)(validated)) <                                               \
      ((2 * sizeof(uintptr_t)) + (uintptr_t)__builtin_frame_address(0))) {    \
    halt();                                                                   \
  }
#else
#define PROT_DOMAINS_VALIDATE_PTR(validated, untrusted, sz)                   \
  validated = untrusted;                                                      \
  if((((uintptr_t)(validated)) <                                              \
      ((2 * sizeof(uintptr_t)) + (uintptr_t)__builtin_frame_address(0))) ||   \
     (((uintptr_t)&_edata_addr) <= (((uintptr_t)(validated)) + (sz)))) {      \
    halt();                                                                   \
  }
#endif

#else

#define SYSCALLS_ALLOC_ENTRYPOINT(nm)
#define SYSCALLS_INIT(nm)
#define SYSCALLS_DEFINE(nm, ...) void nm(__VA_ARGS__)
#define SYSCALLS_DEFINE_SINGLETON(nm, dcd, ...) void nm(__VA_ARGS__)
#define SYSCALLS_AUTHZ_UPD(nm, drv, set)
#define PROT_DOMAINS_VALIDATE_PTR(validated, untrusted, sz) validated = untrusted

#endif

#define SYSCALLS_AUTHZ(nm, drv) SYSCALLS_AUTHZ_UPD(nm, drv, true)
#define SYSCALLS_DEAUTHZ(nm, drv) SYSCALLS_AUTHZ_UPD(nm, drv, false)

#endif /* CPU_X86_MM_SYSCALLS_H_ */
