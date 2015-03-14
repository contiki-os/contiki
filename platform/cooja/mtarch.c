/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */

#include "sys/mt.h"

#ifndef MTARCH_STACKSIZE
#define MTARCH_STACKSIZE 4096
#endif /* MTARCH_STACKSIZE */

#if defined(_WIN32) || defined(__CYGWIN__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void *main_fiber;

#elif defined(__linux) || defined(__APPLE__)

#ifdef __APPLE__
/* Avoid deprecated error on Darwin */
#define _XOPEN_SOURCE
#endif

#include <stdlib.h>
#include <signal.h>
#include <ucontext.h>

struct mtarch_t {
  char stack[MTARCH_STACKSIZE];
  ucontext_t context;
};

static ucontext_t main_context;
static ucontext_t *running_context;

#endif /* _WIN32 || __CYGWIN__ || __linux */

/*--------------------------------------------------------------------------*/
void
mtarch_init(void)
{
#if defined(_WIN32) || defined(__CYGWIN__)

  main_fiber = ConvertThreadToFiber(NULL);

#endif /* _WIN32 || __CYGWIN__ */
}
/*--------------------------------------------------------------------------*/
void
mtarch_remove(void)
{
#if defined(_WIN32) || defined(__CYGWIN__)

  ConvertFiberToThread();

#endif /* _WIN32 || __CYGWIN__ */
}
/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *thread,
	     void (* function)(void *data),
	     void *data)
{
#if defined(_WIN32) || defined(__CYGWIN__)

  thread->mt_thread = CreateFiber(0, (LPFIBER_START_ROUTINE)function, data);

#elif defined(__linux)

  thread->mt_thread = malloc(sizeof(struct mtarch_t));

  getcontext(&((struct mtarch_t *)thread->mt_thread)->context);

  ((struct mtarch_t *)thread->mt_thread)->context.uc_link = NULL;
  ((struct mtarch_t *)thread->mt_thread)->context.uc_stack.ss_sp = 
			((struct mtarch_t *)thread->mt_thread)->stack;
  ((struct mtarch_t *)thread->mt_thread)->context.uc_stack.ss_size = 
			sizeof(((struct mtarch_t *)thread->mt_thread)->stack);

  /* Some notes:
     - If a CPU needs stronger alignment for the stack than malloc()
       guarantees (like i.e. IA64) then makecontext() is supposed to
       add that alignment internally.
     - According to POSIX the arguments to function() are of type int
       and there are in fact 64-bit implementations which support only
       32 bits per argument meaning that a pointer argument has to be
       splitted into two arguments.
     - Most implementations interpret context.uc_stack.ss_sp on entry
       as the lowest stack address even if the CPU stack actually grows
       downwards. Although this means that ss_sp does NOT represent the
       CPU stack pointer this behaviour makes perfectly sense as it is
       the only way to stay independent from the CPU architecture. But
       Solaris prior to release 10 interprets ss_sp as highest stack
       address thus requiring special handling. */
  makecontext(&((struct mtarch_t *)thread->mt_thread)->context,
	      (void (*)(void))function, 1, data);

#endif /* _WIN32 || __CYGWIN__ || __linux */
}
/*--------------------------------------------------------------------------*/
void
mtarch_yield(void)
{
#if defined(_WIN32) || defined(__CYGWIN__)

  SwitchToFiber(main_fiber);

#elif defined(__linux)

  swapcontext(running_context, &main_context);

#endif /* _WIN32 || __CYGWIN__ || __linux */
}
/*--------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *thread)
{
#if defined(_WIN32) || defined(__CYGWIN__)

  SwitchToFiber(thread->mt_thread);

#elif defined(__linux)

  running_context = &((struct mtarch_t *)thread->mt_thread)->context;
  swapcontext(&main_context, running_context);
  running_context = NULL;

#endif /* _WIN32 || __CYGWIN__ || __linux */
}
/*--------------------------------------------------------------------------*/
void
mtarch_stop(struct mtarch_thread *thread)
{
#if defined(_WIN32) || defined(__CYGWIN__)

  DeleteFiber(thread->mt_thread);

#elif defined(linux) || defined(__linux)

  free(thread->mt_thread);

#endif /* _WIN32 || __CYGWIN__ || __linux */
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
