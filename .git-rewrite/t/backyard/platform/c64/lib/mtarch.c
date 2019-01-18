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
 * $Id: mtarch.c,v 1.1 2007/05/23 23:11:28 oliverschmidt Exp $
 */
#include "mtarch.h"
#include <string.h>

unsigned char  mtarch_asm_threadspreg;
unsigned char *mtarch_asm_threadsp;
unsigned char *mtarch_asm_threadzp;
unsigned char *mtarch_asm_threadstack;

void mtarch_asm_start(void);
void mtarch_asm_exec(void);


/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *thread,
	     void (* function)(void *data),
	     void *data)
{
  memset(thread->cpustack, 0, sizeof(thread->cpustack));
  memset(thread->cstack, 0, sizeof(thread->cstack));
  
  /* Create a CPU stack frame with the appropriate values... */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 2] = ((unsigned short)function) >> 8; /* high byte of return address. */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 3] = ((unsigned short)function) & 0xff; /* low byte of return address. */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 4] = 0x21; /* processor flags. */
  thread->cpustack[MTARCH_CPUSTACKSIZE - 5] =       /* a register */
    thread->cpustack[MTARCH_CPUSTACKSIZE - 6] =     /* x register */
    thread->cpustack[MTARCH_CPUSTACKSIZE - 7] = 0;  /* y register */
  thread->spreg = MTARCH_CPUSTACKSIZE - 8;

  /* Setup the C stack with the data pointer. */
  thread->sp = &thread->cstack[MTARCH_CSTACKSIZE - 1];
  
  mtarch_asm_threadzp    = &(thread->zp);  
  mtarch_asm_start();
}
/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *thread)
{
  /* Switch processor stack. The call to mtarch_asm_switch() will not
     return until the process that we switch to calls yield(). */
  mtarch_asm_threadspreg = thread->spreg;
  mtarch_asm_threadsp    = thread->sp;

  mtarch_asm_threadstack = &(thread->cpustack[0]);  
  mtarch_asm_threadzp    = &(thread->zp[0]);
  
  mtarch_asm_exec();

  thread->sp = mtarch_asm_threadsp;
  thread->spreg = mtarch_asm_threadspreg;  
}
/*--------------------------------------------------------------------------*/
void
mtarch_init(void) {

}
/*--------------------------------------------------------------------------*/
void
mtarch_remove(void)
{

}
