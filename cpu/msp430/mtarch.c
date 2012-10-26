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

#include <stdio.h>
#include "sys/mt.h"

#ifdef __IAR_SYSTEMS_ICC__
#define __asm__ asm
#endif

static unsigned short *sptmp;
static struct mtarch_thread *running;

/*--------------------------------------------------------------------------*/
void
mtarch_init(void)
{

}
/*--------------------------------------------------------------------------*/
static void
mtarch_wrapper(void)
{
  /* Call thread function with argument */
  ((void (*)(void *))running->function)((void*)running->data);
}
/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *t,
	     void (*function)(void *), void *data)
{
  int i;

  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    t->stack[i] = i;
  }

  t->sp = &t->stack[MTARCH_STACKSIZE - 1];

  *t->sp = (unsigned short)mt_exit;
  --t->sp;

  *t->sp = (unsigned short)mtarch_wrapper;
  --t->sp;

  /* Space for registers. */
  t->sp -= 11;

  /* Store function and argument (used in mtarch_wrapper) */
  t->data = data;
  t->function = function;
}
/*--------------------------------------------------------------------------*/

static void
sw(void)
{

  sptmp = running->sp;

  __asm__("push r4");
  __asm__("push r5");
  __asm__("push r6");
  __asm__("push r7");
  __asm__("push r8");
  __asm__("push r9");
  __asm__("push r10");
  __asm__("push r11");
  __asm__("push r12");
  __asm__("push r13");
  __asm__("push r14");
  __asm__("push r15");

#ifdef __IAR_SYSTEMS_ICC__
/*  use IAR intrinsic functions */
  running->sp = (unsigned short *) __get_SP_register();
  __set_SP_register((unsigned short) sptmp);
#else
  __asm__("mov.w r1,%0" : "=r" (running->sp));
  __asm__("mov.w %0,r1" : : "m" (sptmp));
#endif

  __asm__("pop r15");
  __asm__("pop r14");
  __asm__("pop r13");
  __asm__("pop r12");
  __asm__("pop r11");
  __asm__("pop r10");
  __asm__("pop r9");
  __asm__("pop r8");
  __asm__("pop r7");
  __asm__("pop r6");
  __asm__("pop r5");
  __asm__("pop r4");
}
/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *t)
{
  running = t;
  sw();
  running = NULL;
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
void
mtarch_stop(struct mtarch_thread *thread)
{

}
/*--------------------------------------------------------------------------*/
int
mtarch_stack_usage(struct mt_thread *t)
{
  int i;

  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    if(t->thread.stack[i] != (unsigned short)i) {
      return MTARCH_STACKSIZE - i;
    }
  }

  return MTARCH_STACKSIZE;
}
/*--------------------------------------------------------------------------*/
