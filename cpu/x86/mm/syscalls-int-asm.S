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

#include "syscalls-int.h"
#include "prot-domains.h"
#include "gdt-layout.h"
#include "stacks.h"

/* Must match definitions (plus the trailing 's') in multi-segment.h */
#define SEG_MMIO fs
#define SEG_KERN fs

.text

/* Invoke the system call return dispatcher from the default privilege
 * level
 */
.global prot_domains_sysret_stub
prot_domains_sysret_stub:
  int $PROT_DOMAINS_SYSRET_DISPATCH_INT

.macro save_segs
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__SWSEG
  /* Save (and restore, in restore_segs) MMIO segment register into
   * callee-saved register in case a system call was invoked from a region in
   * which MMIO is enabled.
   */
  push %SEG_MMIO
#endif
.endm

.macro restore_segs
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__SWSEG
  pop %SEG_MMIO
#endif
.endm

/* Refresh most of the segment registers in case they were corrupted by
 * userspace code to prevent that from corrupting the operation of the
 * privileged code.
 */
.macro load_kern_segs
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__SWSEG
  mov $GDT_SEL_DATA, %eax
  mov %eax, %ds
  mov %eax, %es
  mov $GDT_SEL_DATA_KERN_EXC, %eax
  mov %eax, %SEG_KERN
#endif
.endm

/* Invoke the system call dispatcher C routine */
.global prot_domains_syscall_dispatcher
prot_domains_syscall_dispatcher:
  mov %esp, %ecx /*< interrupt_stack_t *intr_stk */
  /* EDX already set to "dom_client_data_t to_dcd" by syscall stub */
  save_segs
  push %eax /*< syscalls_entrypoint_t *syscall */
  load_kern_segs
  call prot_domains_syscall_dispatcher_impl
  /* fastcall convention, so callee pops arguments */
  restore_segs
  iret

/* Invoke the system call return dispatcher C routine */
.global prot_domains_sysret_dispatcher
prot_domains_sysret_dispatcher:
  mov %esp, %ecx /*< interrupt_stack_t *intr_stk */
  save_segs
  load_kern_segs
  call prot_domains_sysret_dispatcher_impl
  restore_segs
  /* Zero caller-saved registers in case they contain secrets.  The system call
   * handlers and dispatchers need to preserve the callee-saved registers.
   */
  xor %eax, %eax
  xor %ecx, %ecx
  xor %edx, %edx
  iret

.global prot_domains_launch_kernel
prot_domains_launch_kernel:
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
  mov $GDT_SEL_DATA, %eax
  mov %eax, %ds
  mov %eax, %es
  mov %eax, %fs
  mov %eax, %gs
#else
  mov $GDT_SEL_LDT(DOM_ID_kern), %eax
  lldt %ax
  call multi_segment_launch_kernel
#endif
  /* init interrupt return stack: */
  pushl $GDT_SEL_STK
  lea stacks_main, %eax
  /* matches STACKS_INIT_TOP, plus 4 since an address has been consumed: */
  add $(STACKS_SIZE_MAIN - 4), %eax
  pushl %eax
  pushl $EFLAGS_IOPL(PRIV_LVL_INT)
  pushl $GDT_SEL_CODE
  pushl $0 /* will be overwritten by syscall_dispatcher_impl */
  /* fastcall convention: */
  mov %esp, %ecx
  call prot_domains_launch_kernel_impl
  iretl
