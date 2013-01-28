/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <string.h>

#include "sys/mtarch.h"

unsigned char  mtarch_asm_threadspreg;
unsigned char *mtarch_asm_threadzp;
unsigned char *mtarch_asm_threadstack;

void mtarch_asm_start(void);
void mtarch_asm_yield(void);
void mtarch_asm_exec(void);

/*--------------------------------------------------------------------------*/
void
mtarch_init(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_remove(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *thread,
             void (* function)(void *data),
             void *data)
{
  memset(thread->cpustack, 0, sizeof(thread->cpustack));
  memset(thread->cstack,   0, sizeof(thread->cstack));

  /* Copy current zero page content as template. */
  mtarch_asm_threadzp = thread->zp;
  mtarch_asm_start();

  /* Create a CPU stack frame with the appropriate values. */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 2] = ((unsigned short)function) / 0x100; /* high byte of return address */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 3] = ((unsigned short)function) % 0x100; /* low  byte of return address */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 4] = 0x21; /* processor flags */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 5] =       /* a register      */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 6] =       /* x register      */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 7] = 0x00; /* y register      */
  thread->spreg = MTARCH_CPUSTACKSIZE - 8;

  /* Setup the C stack with the data pointer. */
  thread->cstack[MTARCH_CSTACKSIZE - 2] = ((unsigned short)data) / 0x100; /* high byte of data pointer */
  thread->cstack[MTARCH_CSTACKSIZE - 3] = ((unsigned short)data) % 0x100; /* low  byte of data pointer */

  /* Setup the C stack pointer. */
  thread->zp[1] = ((size_t)&thread->cstack[MTARCH_CSTACKSIZE - 3]) / 0x100; /* high byte of C stack pointer */
  thread->zp[0] = ((size_t)&thread->cstack[MTARCH_CSTACKSIZE - 3]) % 0x100; /* low  byte of C stack pointer */
}
/*--------------------------------------------------------------------------*/
void
mtarch_yield(void)
{
  mtarch_asm_yield();
}
/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *thread)
{
  /* Switch processor stack. The call to mtarch_asm_switch() will not
     return until the process that we switch to calls yield(). */
  mtarch_asm_threadspreg = thread->spreg;

  mtarch_asm_threadstack = thread->cpustack;  
  mtarch_asm_threadzp    = thread->zp;

  mtarch_asm_exec();

  thread->spreg = mtarch_asm_threadspreg;  
}
/*--------------------------------------------------------------------------*/
void
mtarch_stop(struct mtarch_thread *thread)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstart(void)
{
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstop(void)
{
}
/*--------------------------------------------------------------------------*/
