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
  * 	Z80 machine-specific difinitions for supporting multithread.
  * 
  * \author
  *	Takahide Matsutsuka <markn@markn.org>
  */
#ifndef __MTARCH_H__
#define __MTARCH_H__

/* Unit of the stack is 2byte wide. */
#ifndef MTARCH_STACKSIZE
#define MTARCH_STACKSIZE 128
#endif /* MTARCH_STACKSIZE */

struct mtarch_thread {
  /* 
   * On top of the mtarch_thread must be the address for the stack pointer.
   * See details at mtarch_switch in mtarch.c
   */
  uint16_t *sp;
  /*
   * Stack is 2-byte wide, so please note that you need 2 * MTARCH_STACKSIZE
   * bytes for the stack area for each thread.
   */
  uint16_t stack[MTARCH_STACKSIZE];
};

/*
 * A function for debugging purpose, placed here by following other implementations.
 */
int mtarch_stack_usage(struct mtarch_thread *t);

#endif /* __MTARCH_H__ */

