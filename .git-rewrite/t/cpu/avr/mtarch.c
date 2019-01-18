/*
 * Copyright (c) 2006, Technical University of Munich
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
 * \file AVR specific implementation of multithreading architecture
 *
 * \author Adam Dunkels <adam@sics.se>
 * \author Simon Barner <barner@in.tum.de>
 *
 * @(#)$Id: mtarch.c,v 1.1 2006/12/22 16:55:53 barner Exp $
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "sys/mt.h"
#include "dev/rs232.h"

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
  /* Initialize stack with number sequence (used for
   * measuring stack usage */
  uint8_t i;

  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    t->stack[i] = i;
  }

  /*
   * Push pointer to mt_exit and the thread onto our stack:
   * Caveats:
   *  - The stack is defined as an array of bytes, but pointers are 16 bit wide
   *  - Function pointers are 16-bit addresses in flash ROM, but e.g.
   *    avr-objdump displays byte addresses
   *  - Get the high and low byte of the addresses onto the stack in the
   *    right order
   */

  /* Initialize stack. This is done in reverse order ("pushing") the
   *  pre-allocated array */

  /* mt_exit function that is to be invoked if the thread dies */
  t->stack[MTARCH_STACKSIZE -  1] = (unsigned char)((unsigned short)mt_exit) & 0xff;
  t->stack[MTARCH_STACKSIZE -  2] = (unsigned char)((unsigned short)mt_exit >> 8) & 0xff;

  /* The thread handler. Invoked when RET is called in mtarch_exec */
  t->stack[MTARCH_STACKSIZE -  3] = (unsigned char)((unsigned short)function) & 0xff;
  t->stack[MTARCH_STACKSIZE -  4] = (unsigned char)((unsigned short)function >> 8) & 0xff;

  /* Register r0-r23 in t->stack[MTARCH_STACKSIZE -  5] to
   *  t->stack[MTARCH_STACKSIZE -  28].
   *
   * Per calling convention, the argument to the thread handler function 
   *  (i.e. the 'data' pointer) is passed via r24-r25.
   * See http://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_reg_usage) */
  t->stack[MTARCH_STACKSIZE - 29] = (unsigned char)((unsigned short)data) & 0xff;
  t->stack[MTARCH_STACKSIZE - 30] = (unsigned char)((unsigned short)data >> 8) & 0xff;

  /* Initialize stack pointer: Space for 2 2-byte-addresses and 32 registers,
   * post-decrement POP / pre-increment PUSH scheme */
  t->sp = &t->stack[MTARCH_STACKSIZE - 1 - 4 - 32];
}

/*--------------------------------------------------------------------------*/
static unsigned char *sptmp;
static struct mtarch_thread *running;

static void
sw(void)
{
  /* Disable interrupts while we perform the context switch */
  cli ();

  /* Push 32 general purpuse registers */
  __asm__("push r0");
  __asm__("push r1");
  __asm__("push r2");
  __asm__("push r3");
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
  __asm__("push r16");
  __asm__("push r17");
  __asm__("push r18");
  __asm__("push r19");
  __asm__("push r20");
  __asm__("push r21");
  __asm__("push r22");
  __asm__("push r23");
  __asm__("push r24");
  __asm__("push r25");
  __asm__("push r26");
  __asm__("push r27");
  __asm__("push r28");
  __asm__("push r29");
  __asm__("push r30");
  __asm__("push r31");

  /* Switch stack pointer */
  sptmp = running->sp;
  running->sp = (unsigned char*)SP;
  SP = (unsigned short)sptmp;

  /* Pop 32 general purpose registers */
  __asm__("pop r31");
  __asm__("pop r30");
  __asm__("pop r29");
  __asm__("pop r28");
  __asm__("pop r27");
  __asm__("pop r26");
  __asm__("pop r25");
  __asm__("pop r24");
  __asm__("pop r23");
  __asm__("pop r22");
  __asm__("pop r21");
  __asm__("pop r20");
  __asm__("pop r19");
  __asm__("pop r18");
  __asm__("pop r17");
  __asm__("pop r16");
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
  __asm__("pop r3");
  __asm__("pop r2");
  __asm__("pop r1");
  __asm__("pop r0");

  /* Renable interrupts */
  sei ();
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
int
mtarch_stack_usage(struct mt_thread *t)
{
  uint8_t i;
  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    if(t->thread.stack[i] != i) {
      break;
    }
  }
  return MTARCH_STACKSIZE - i;
}
/*--------------------------------------------------------------------------*/
