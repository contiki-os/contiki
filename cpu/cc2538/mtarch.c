/*
 * Copyright (c) 2014, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 *
 * Author: Andreas Dröscher <contiki@anticat.ch>
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
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS "AS IS" AND
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
 */
#include "mtarch.h"
#include "reg.h"
#include "scb.h"
#include "mt.h"

#include <string.h>

/**
 * Pointer to the next thread or NULL if the next thread is main
 */
static struct mtarch_thread *next_thread = NULL;

/**
 * Pointer to the current thread or NULL the main thread is running
 */
static struct mtarch_thread *current_thread = NULL;

/**
 * Main stack pointer (used for kernel and protothreads)
 */
static void *msp = NULL;

void
mtarch_init(void)
{
}

void
mtarch_start(struct mtarch_thread *thread,
             void (*function)(void *data),
             void *data)
{
  /* Calculate SP */
  mtarch_stack_t *sp = (mtarch_stack_t *)&thread->stack[MTARCH_STACKSIZE - MTARCH_REGISTER_COUNT];
  thread->sp = sp;

  /* Zero stack */
  memset(thread->stack, '\0', sizeof(void *) * MTARCH_STACKSIZE);

  /* Fill stack */
  sp->r0 = data;
  sp->lr = mt_exit;
  sp->pc = function;
  sp->psr = (void *)0x01000000;
}

void
mtarch_yield(void)
{
  /* Trigger PendSV the handler will switch to main */
  REG(SCB_INTCTRL) = SCB_INTCTRL_PEND_SV;
}

void
mtarch_exec(struct mtarch_thread *thread)
{
  /* Point to the next thread */
  next_thread = thread;

  /* Trigger PendSV, it will do the switch */
  REG(SCB_INTCTRL) = SCB_INTCTRL_PEND_SV;
}

void
mtarch_pstop(void)
{
}

void
mtarch_stop(struct mtarch_thread *thread)
{
}

/**
 * The schedule switches back and forth between
 * main stack and thread stack(s). NULL pointers in
 * next_thread and current_thread are used to indicate
 * main stack.
 *
 * \param sp current stack pointer
 * \return next stack pointer
 */
void *
schedule(void *sp)
{
  if(current_thread != NULL) {
    current_thread->sp = sp;
  } else {
    msp = sp;
  }

  /* Switch to next thread */
  current_thread = next_thread;
  next_thread = NULL;

  if(current_thread) {
    sp = current_thread->sp;
  } else {
    sp = msp;
  }
  return sp;
}

/* Disable prologue and epilogue of ISR */
void pend_sv_isr(void)  __attribute__ ((isr, naked));

/**
 * The PendSV interupt service routine backs up current
 * context, calls the scheduler and restores the newly
 * selected context.
 */
void
pend_sv_isr(void)
{
  /* Backup current thread context */
  asm volatile ("cpsid i             \n"           /* Disable interrupts */
                "dsb                 \n"           /* Data synchronization barrier */
                "isb                 \n"           /* Instruction synchronization barrier */
                "mrs r0, msp         \n"           /* Read SP to r0 */
                "stmdb r0!, {r4-r11} \n"           /* Push registers onto stack */
                "msr msp, r0");                    /* Update stack pointer */

  /* Call sheduler Note: r0 is used as argument as well as result value */
  asm volatile ("mov r4, lr\n"                     /* backup lr into callee saved register */
                "bl schedule\n"                    /* Call scheduler */
                "mov lr, r4\n");                   /* restore lr from callee saved register */

  /* Restore next thread context */
  asm volatile ("ldmfd r0!, {r4-r11} \n"           /* Pop registers from stack */
                "msr msp, r0         \n"           /* Update stack pointer */
                "cpsie i");                        /* Re-Enable interrupts */

  asm volatile ("bx lr\n");                        /* Restore state and return  */
}
