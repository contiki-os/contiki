/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include <stddef.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "sys/mt.h"

#ifndef __WORDSIZE
#define __WORDSIZE 32
#endif /* __WORDSIZE */

#ifndef ON_64BIT_ARCH
#if __WORDSIZE == 64
#define ON_64BIT_ARCH 1
#else /* ON_64BIT_ARCH */
#define ON_64BIT_ARCH 0
#endif /* __WORDSIZE == 64 */
#endif /* ON_64BIT_ARCH */

struct frame {
  unsigned long flags;
#if ON_64BIT_ARCH
  unsigned long rbp;
  unsigned long rdi;
  unsigned long rsi;
  unsigned long rdx;
  unsigned long rcx;
  unsigned long rbx;
  unsigned long rax;
#else /* ON_64BIT_ARCH */
  unsigned long ebp;
  unsigned long edi;
  unsigned long esi;
  unsigned long edx;
  unsigned long ecx;
  unsigned long ebx;
  unsigned long eax;
#endif /* ON_64BIT_ARCH */
  unsigned long retaddr;
  unsigned long retaddr2;
#if !ON_64BIT_ARCH
  unsigned long data;
#endif
};
/*--------------------------------------------------------------------------*/
void
mtarch_init(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *t,
    void (*function)(void *), void *data)
{
  struct frame *f = (struct frame *)&t->stack[MTARCH_STACKSIZE - sizeof(struct frame)/sizeof(unsigned long)];
  int i;

  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    t->stack[i] = i;
  }

  memset(f, 0, sizeof(struct frame));
  f->retaddr = (unsigned long)function;
  t->sp      = (unsigned long)&f->flags;
#if ON_64BIT_ARCH
  f->rdi     = (unsigned long)data;
  f->rbp     = (unsigned long)&f->rax;
#else /* ON_64BIT_ARCH */
  f->data    = (unsigned long)data;
  f->ebp     = (unsigned long)&f->eax;
#endif /* ON_64BIT_ARCH */
}
/*--------------------------------------------------------------------------*/
static struct mtarch_thread *running_thread;
/*--------------------------------------------------------------------------*/
void sw(void)
{
  /* Store registers */
#if ON_64BIT_ARCH
  __asm__ (
      "pushq %rax\n\t"
      "pushq %rbx\n\t"
      "pushq %rcx\n\t"
      "pushq %rdx\n\t"
      "pushq %rsi\n\t"
      "pushq %rdi\n\t"
      "pushq %rbp\n\t"
      "pushq %rbp\n\t");
#else /* ON_64BIT_ARCH */
  __asm__ (
      "pushl %eax\n\t"
      "pushl %ebx\n\t"
      "pushl %ecx\n\t"
      "pushl %edx\n\t"
      "pushl %esi\n\t"
      "pushl %edi\n\t"
      "pushl %ebp\n\t"
      "pushl %ebp\n\t");
#endif /* ON_64BIT_ARCH */

  /* Switch stack pointer */
#if ON_64BIT_ARCH
  __asm__ ("movq %0, %%rax\n\t" : : "m" (running_thread));
  __asm__ (
      "movq (%rax), %rbx\n\t"
      "movq %rsp, (%rax)\n\t"
      "movq %rbx, %rsp\n\t"
  );
#else /* ON_64BIT_ARCH */
  __asm__ ("movl %0, %%eax\n\t" : : "m" (running_thread));
  __asm__ (
      "movl (%eax), %ebx\n\t"
      "movl %esp, (%eax)\n\t"
      "movl %ebx, %esp\n\t"
  );
#endif /* ON_64BIT_ARCH */

  /* Restore previous registers */
#if ON_64BIT_ARCH
  __asm__ (
      "popq %rbp\n\t"
      "popq %rbp\n\t"
      "popq %rdi\n\t"
      "popq %rsi\n\t"
      "popq %rdx\n\t"
      "popq %rcx\n\t"
      "popq %rbx\n\t"
      "popq %rax\n\t"

      "leave\n\t"
      "ret\n\t"
  );
#else /* ON_64BIT_ARCH */
  __asm__ (
      "popl %ebp\n\t"
      "popl %ebp\n\t"
      "popl %edi\n\t"
      "popl %esi\n\t"
      "popl %edx\n\t"
      "popl %ecx\n\t"
      "popl %ebx\n\t"
      "popl %eax\n\t"

      "leave\n\t"
      "ret\n\t"
  );
#endif /* ON_64BIT_ARCH */

}

/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *t)
{
  running_thread = t;
  sw();
  running_thread = NULL;
}
/*--------------------------------------------------------------------------*/
void
mtarch_remove(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_yield(void)
{
  sw();
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstop(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstart(void)
{
}
/*--------------------------------------------------------------------------*/
int
mtarch_stack_usage(struct mt_thread *t)
{
  int i;
  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    if(t->thread.stack[i] != i) {
      return MTARCH_STACKSIZE - i;
    }
  }
  return -1;
}
/*--------------------------------------------------------------------------*/
