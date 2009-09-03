/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * $Id: cooja_mt.c,v 1.2 2009/09/03 12:57:57 nvt-se Exp $
 */
/*
 * This file is ripped from mt.c of the Contiki Multi-threading library.
 * Fredrik Osterlind <fros@sics.se>
 */

#include "contiki.h"
#include "sys/cooja_mt.h"
#include "sys/cc.h"

#define MT_STATE_READY   1
#define MT_STATE_RUNNING 2
#define MT_STATE_EXITED  5

static struct cooja_mt_thread *current;

/*--------------------------------------------------------------------------*/
void
cooja_mt_init(void)
{
  cooja_mtarch_init();
}
/*--------------------------------------------------------------------------*/
void
cooja_mt_remove(void)
{
  cooja_mtarch_remove();
}
/*--------------------------------------------------------------------------*/
void
cooja_mt_start(struct cooja_mt_thread *thread, void (* function)(void *), void *data)
{
  /* Call the architecture dependant function to set up the processor
     stack with the correct parameters. */
  cooja_mtarch_start(&thread->thread, function, data);

  thread->state = MT_STATE_READY;
}
/*--------------------------------------------------------------------------*/
void
cooja_mt_exec(struct cooja_mt_thread *thread)
{
  if(thread->state == MT_STATE_READY) {
    thread->state = MT_STATE_RUNNING;
    current = thread;
    /* Switch context to the thread. The function call will not return
       until the the thread has yielded, or is preempted. */
    cooja_mtarch_exec(&thread->thread);
  }
}
/*--------------------------------------------------------------------------*/
void
cooja_mt_exit(void)
{
  current->state = MT_STATE_EXITED;
  current = NULL;
  cooja_mtarch_yield();
}
/*--------------------------------------------------------------------------*/
void
cooja_mt_yield(void)
{
  current->state = MT_STATE_READY;
  current = NULL;
  /* This function is called from the running thread, and we call the
     switch function in order to switch the thread to the main Contiki
     program instead. For us, the switch function will not return
     until the next time we are scheduled to run. */
  cooja_mtarch_yield();
}
