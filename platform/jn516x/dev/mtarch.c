/*
 * Copyright (c) 2008
 * Telecooperation Office (TecO), Universitaet Karlsruhe (TH), Germany.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. Neither the name of the Universitaet Karlsruhe (TH) nor the names
 *    of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s): Philipp Scholl <scholl@teco.edu>
 *            Tokita Hiroshi <tokita.hiroshi@gmail.com>
 */

/* Copied from Philipp Scholl's (BSD) Contiki port to Jennic */
/* Added mtarch implementation by Tokita Hiroshi. */

#include <AppHardwareApi.h>
#include <stdio.h>
#include "sys/mt.h"
#include "mtarch.h"

static struct mtarch_thread *running;

static void
mtarch_wrapper(void)
{
  ((void (*)(void *))running->function)((void *)running->data);
}
void
mtarch_init(void)
{
  /* Multithreading can't works concurrently with
     overflow detection which enabled by vAHI_SetStackOverflow().
     So, stop overflow detection at start multithread. */
  vAHI_SetStackOverflow(FALSE, 0);
}
void
mtarch_remove(void)
{
}
void
mtarch_start(struct mtarch_thread *thread,
             void (*function)(void *data),
             void *data)
{
  int i;

  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    thread->stack[i] = i;
  }

  thread->data = data;
  thread->function = function;
  thread->sp = (unsigned int *)&thread->stack[MTARCH_STACKSIZE - 1];

  *thread->sp-- = (unsigned int)0x0;  /* R16 */
  *thread->sp-- = (unsigned int)0x0;  /* R15 */
  *thread->sp-- = (unsigned int)0x0;  /* R14 */
  *thread->sp-- = (unsigned int)0x0;  /* R13 */
  *thread->sp-- = (unsigned int)0x0;  /* R12 */
  *thread->sp-- = (unsigned int)0x0;  /* R11 */
  *thread->sp-- = (unsigned int)0x0;  /* R10 */
  *thread->sp-- = (unsigned int)mtarch_wrapper;  /* R9: link register */
  *thread->sp-- = (unsigned int)0x0;  /* R8 */
  *thread->sp-- = (unsigned int)0x0;  /* R7 */
  *thread->sp-- = (unsigned int)0x0;  /* R6 */
  *thread->sp-- = (unsigned int)0x0;  /* R5 */
  *thread->sp-- = (unsigned int)0x0;  /* R4 */
  *thread->sp-- = (unsigned int)0x0;  /* R3 */
  *thread->sp-- = (unsigned int)0x0;  /* R2 */
  *thread->sp = (unsigned int)0x0;    /* R1 */
}
static void
sw()
{
  register unsigned int *r7 __asm__ ("r7") = running->sp;
  register unsigned int **r8 __asm__ ("r8") = &(running->sp);

  __asm__ volatile (
    "b.di                   \n\t"

    "b.addi  r1, r1, -0x40  \n\t"
    "b.sw    0x00(r1), r1   \n\t"
    "b.sw    0x04(r1), r2   \n\t"
    "b.sw    0x08(r1), r3   \n\t"
    "b.sw    0x0c(r1), r4   \n\t"
    "b.sw    0x10(r1), r5   \n\t"
    "b.sw    0x14(r1), r6   \n\t"
    "b.sw    0x18(r1), r7   \n\t"
    "b.sw    0x1c(r1), r8   \n\t"
    "b.sw    0x20(r1), r9   \n\t"
    "b.sw    0x24(r1), r10  \n\t"
    "b.sw    0x28(r1), r11  \n\t"
    "b.sw    0x2c(r1), r12  \n\t"
    "b.sw    0x30(r1), r13  \n\t"
    "b.sw    0x34(r1), r14  \n\t"
    "b.sw    0x38(r1), r15  \n\t"
    "b.sw    0x3c(r1), r16  \n\t"

    "b.sw    0x00(%1), r1   \n\t"
    "b.mov   r1, %0         \n\t"

    /* r1: stack pointer */
    "b.lwz   r2,  0x04(r1)  \n\t"
    "b.lwz   r3,  0x08(r1)  \n\t"
    "b.lwz   r4,  0x0c(r1)  \n\t"
    "b.lwz   r5,  0x14(r1)  \n\t"
    "b.lwz   r6,  0x18(r1)  \n\t"
    /* r7: local variable */
    /* r8: local variable */
    "b.lwz   r9,  0x20(r1)  \n\t"
    "b.lwz   r10, 0x24(r1)  \n\t"
    "b.lwz   r11, 0x28(r1)  \n\t"
    "b.lwz   r12, 0x2c(r1)  \n\t"
    "b.lwz   r13, 0x30(r1)  \n\t"
    "b.lwz   r14, 0x34(r1)  \n\t"
    "b.lwz   r15, 0x38(r1)  \n\t"
    "b.lwz   r16, 0x3c(r1)  \n\t"
    "b.addi  r1, r1, 0x40   \n\t"

    "b.ei                   \n\t"
    :
    : "r" (r7), "r" (r8)
    );
}
void
mtarch_yield(void)
{
  sw();
}
void
mtarch_exec(struct mtarch_thread *thread)
{
  running = thread;
  sw();
  running = NULL;
}
void
mtarch_stop(struct mtarch_thread *thread)
{
}
void
mtarch_pstart(void)
{
}
void
mtarch_pstop(void)
{
}
