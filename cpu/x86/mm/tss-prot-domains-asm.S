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

.text

/* Initialize the TSS fields in prot_domains_reg accordingly:
 * Note: Each of these must be a callee-saved register, so that they are
 * restored to their original values prior to the task returning.  This will
 * result in the same values being loaded when the task is next invoked.
 */
#define CUR_DOM_ID_BITMAP                esi

/* Must match SEG_KERN (plus the trailing 's') in multi-segment.h */
#define SEG_KERN fs

.global prot_domains_syscall_dispatcher
prot_domains_syscall_dispatcher:
#define PROT_DOMAINS_SYSCALL eax
  mov prot_domains_syscall, %PROT_DOMAINS_SYSCALL
  cmp $syscalls_entrypoints, %PROT_DOMAINS_SYSCALL
  jl halt
  cmp $syscalls_entrypoints_end, %PROT_DOMAINS_SYSCALL
  jnl halt
#define SYSCALLS_ENTRYPOINTS_ALIGN_MASK  ebp
  mov $3, %SYSCALLS_ENTRYPOINTS_ALIGN_MASK
  and %PROT_DOMAINS_SYSCALL, %SYSCALLS_ENTRYPOINTS_ALIGN_MASK
  jnz halt

  /* Compare allowed domains bitmask against current domain ID bitmap.  If
   * the check fails, then the current domain ID bitmap value will be zeroed
   * out, which could cause incorrect behavior in the future.  However, the
   * response to a failed check is to halt the system, so destroying the
   * current domain ID bitmap value will have no effect.
   */
  and %SEG_KERN:4(%PROT_DOMAINS_SYSCALL), %CUR_DOM_ID_BITMAP
  jz halt

  mov prot_domains_main_esp, %esp

  /* Must be a callee-saved register: */
#define ORIG_RET_ADDR edi
  /* Update the caller's stack to return back to here */
  pop %ORIG_RET_ADDR
  push $sysret_dispatcher
  /* Jump to the system call body */
  jmp *%SEG_KERN:(%PROT_DOMAINS_SYSCALL)

sysret_dispatcher:
  push %ORIG_RET_ADDR

  iret

  /* The task will resume here for the next system call, so it is necessary
   * to jump back to the top.
   */
  jmp prot_domains_syscall_dispatcher

.global dev_not_avail_isr
dev_not_avail_isr:
  clts
  iret
