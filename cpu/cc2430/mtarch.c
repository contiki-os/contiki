/*
 * Copyright (c) 2007, Takahide Matsutsuka.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 /*
  * \file
  * 	Z80 machine-specific implementation for supporting multithread.
  * \author
  * 	Takahide Matsutsuka <markn@markn.org>
  */
#include "sys/mt.h"
#include "mtarch.h"

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
  uint16_t i;
  
  for(i = 0; i < MTARCH_STACKSIZE; i++) {
    t->stack[i] = i;
  }

  t->sp = &t->stack[MTARCH_STACKSIZE - 1];

  
  /* A parameter for method for thread function. */
  *t->sp = (uint16_t)data;
  --t->sp;

  /* This will be a return address of thread function. */
  *t->sp = (uint16_t)mt_exit;
  --t->sp;

  /* The thread function, is used as a return address of mtarch_switch. */
  *t->sp = (uint16_t)function;
  --t->sp;

  /*
   * Space for registers.
   * af, bc, de, hl, ix, iy, af', bc', de', hl'
   */
  /*
   * Z80 stack basis:  
   * push stores the data AFTER decrementing sp. 
   * pop reads the data BEFORE incrementing sp.
   */

  t->sp = t->sp - 9;
}
/*--------------------------------------------------------------------------*/
static struct mtarch_thread *running_thread;
static uint16_t *sptmp;
static void
mtarch_switch()
{
  __asm
    di	; disable interrupt
    ; normal registers
    push af
    push bc
    push de
    push hl
    push ix
    push iy

    ; back registers
    ex af,af'
    push af
    exx
    push bc
    push de
    push hl

    ; swap between running_thread->sp and SP reg
    ; _running_thread in asembler below points running_thread->sp
    ; sptmp = sp;
    ld (_sptmp),sp
    
    ; sp = *(running_thread->sp);
    ld ix,(_running_thread)
    ld l,0(ix)
    ld h,1(ix)
    ld sp,hl
    
    ; running_thread->sp = sptmp;
    ld hl,(_sptmp)
    ld 0(ix),l
    ld 1(ix),h

    ; back registers
    pop hl
    pop de
    pop bc
    exx
    pop af
    ex af,af'

    ; normal registers
    pop iy
    pop ix  
    pop hl
    pop de
    pop bc
    pop af
    ei	; enable interrupt
  __endasm;
  // here sp indicates the address that point the function
}
/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *t)
{
  running_thread = t;
  mtarch_switch();
  running_thread = NULL;
}
/*--------------------------------------------------------------------------*/
void
mtarch_remove()
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_yield()
{
  if (running_thread == NULL) {
    /* ERROR! we have no runnning thread. */
    return; 
  }
  mtarch_switch();
}
/*--------------------------------------------------------------------------*/
void mtarch_stop(struct mtarch_thread *thread)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstop()
{  
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstart()
{  
}
/*--------------------------------------------------------------------------*/
int
mtarch_stack_usage(struct mtarch_thread *t)
{
  uint16_t i;
  for (i = 0; i < MTARCH_STACKSIZE; i++) {
    if (t->stack[i] != i) {
      return MTARCH_STACKSIZE - i;
    }
  }
  
  return 0;
}
/*--------------------------------------------------------------------------*/
