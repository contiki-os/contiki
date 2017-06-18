/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup arm-cm-mtarch
 * @{
 *
 * \file
 * Implmentation of the ARM Cortex-M support for Contiki multi-threading.
 */
#include CMSIS_DEV_HDR
#include "sys/mt.h"

#include <stdint.h>

#define EXC_RETURN_PROCESS_THREAD_BASIC_FRAME   0xfffffffd

/* Check whether EXC_RETURN[3:0] in LR indicates a preempted process thread. */
#if __ARM_ARCH == 7
#define PREEMPTED_PROCESS_THREAD() \
  "and r0, lr, #0xf\n\t" \
  "cmp r0, #0xd\n\t"
#elif __ARM_ARCH == 6
#define PREEMPTED_PROCESS_THREAD() \
  "mov r0, lr\n\t" \
  "movs r1, #0xf\n\t" \
  "and r0, r1\n\t" \
  "cmp r0, #0xd\n\t"
#else
#error Unsupported ARM architecture
#endif
/*----------------------------------------------------------------------------*/
/**
 * \brief SVCall system handler
 *
 * This exception handler executes the action requested by the corresponding
 * \c svc instruction, which is a task switch from the main Contiki thread to an
 * mt thread or the other way around.
 */
__attribute__ ((__naked__))
void
svcall_handler(void)
{
  /* This is a controlled system handler, so do not use ENERGEST_TYPE_IRQ. */

  /*
   * Decide whether to switch to the main thread or to a process thread,
   * depending on the type of the thread preempted by SVCall.
   */
  __asm__ (PREEMPTED_PROCESS_THREAD()
#if __ARM_ARCH == 7
           "it eq\n\t"
#endif
           "beq switch_to_main_thread\n\t"

  /*
   * - Retrieve from the main stack the PSP passed to SVCall through R0. Note
   *   that it cannot be retrieved directly from R0 on exception entry because
   *   this register may have been overwritten by other exceptions on SVCall
   *   entry.
   * - Save the main thread context to the main stack.
   * - Restore the process thread context from the process stack.
   * - Return to Thread mode, resuming the process thread.
   */
#if __ARM_ARCH == 7
           "ldr r0, [sp]\n\t"
           "push {r4-r11, lr}\n\t"
           "add r1, r0, #9 * 4\n\t"
           "msr psp, r1\n\t"
           "ldmia r0, {r4-r11, pc}");
#elif __ARM_ARCH == 6
           "mov r0, r8\n\t"
           "mov r1, r9\n\t"
           "mov r2, r10\n\t"
           "mov r3, r11\n\t"
           "push {r0-r7, lr}\n\t"
           "ldr r0, [sp, #9 * 4]\n\t"
           "ldmia r0!, {r4-r7}\n\t"
           "mov r8, r4\n\t"
           "mov r9, r5\n\t"
           "mov r10, r6\n\t"
           "mov r11, r7\n\t"
           "ldmia r0!, {r3-r7}\n\t"
           "msr psp, r0\n\t"
           "bx r3");
#endif
}
/*----------------------------------------------------------------------------*/
/**
 * \brief PendSV system handler
 *
 * This exception handler executes following a call to mtarch_pstart() from
 * another exception handler. It performs a task switch to the main Contiki
 * thread if it is not already running.
 */
__attribute__ ((__naked__))
void
pendsv_handler(void)
{
  /* This is a controlled system handler, so do not use ENERGEST_TYPE_IRQ. */

  /*
   * Return without doing anything if PendSV has not preempted a process thread.
   * This can occur either because PendSV has preempted the main thread, in
   * which case there is nothing to do, or because mtarch_pstart() has been
   * called from an exception handler without having called mt_init() first, in
   * which case PendSV may have preempted an exception handler and nothing must
   * be done because mt is not active.
   */
  __asm__ (  PREEMPTED_PROCESS_THREAD()
#if __ARM_ARCH == 7
             "it ne\n\t"
             "bxne lr\n"
#elif __ARM_ARCH == 6
             "beq switch_to_main_thread\n\t"
             "bx lr\n"
#endif

  /*
   * - Save the process thread context to the process stack.
   * - Place into the main stack the updated PSP that SVCall must return through
   *   R0.
   * - Restore the main thread context from the main stack.
   * - Return to Thread mode, resuming the main thread.
   */
           "switch_to_main_thread:\n\t"
             "mrs r0, psp\n\t"
#if __ARM_ARCH == 7
             "stmdb r0!, {r4-r11, lr}\n\t"
             "str r0, [sp, #9 * 4]\n\t"
             "pop {r4-r11, pc}");
#elif __ARM_ARCH == 6
             "mov r3, lr\n\t"
             "sub r0, #5 * 4\n\t"
             "stmia r0!, {r3-r7}\n\t"
             "mov r4, r8\n\t"
             "mov r5, r9\n\t"
             "sub r0, #9 * 4\n\t"
             "mov r6, r10\n\t"
             "mov r7, r11\n\t"
             "stmia r0!, {r4-r7}\n\t"
             "pop {r4-r7}\n\t"
             "sub r0, #4 * 4\n\t"
             "mov r8, r4\n\t"
             "mov r9, r5\n\t"
             "str r0, [sp, #5 * 4]\n\t"
             "mov r10, r6\n\t"
             "mov r11, r7\n\t"
             "pop {r4-r7, pc}");
#endif
}
/*----------------------------------------------------------------------------*/
void
mtarch_init(void)
{
  SCB->CCR = (SCB->CCR
#ifdef SCB_CCR_NONBASETHRDENA_Msk
              /*
               * Make sure that any attempt to enter Thread mode with exceptions
               * active faults.
               *
               * Only SVCall and PendSV are allowed to forcibly enter Thread
               * mode, and they are configured with the same, lowest exception
               * priority, so no other exceptions may be active.
               */
              & ~SCB_CCR_NONBASETHRDENA_Msk
#endif
             /*
              * Force 8-byte stack pointer alignment on exception entry in order
              * to be able to use AAPCS-conforming functions as exception
              * handlers.
              */
             ) | SCB_CCR_STKALIGN_Msk;

  /*
   * Configure SVCall and PendSV with the same, lowest exception priority.
   *
   * This makes sure that they cannot preempt each other, and that the processor
   * executes them after having handled all other exceptions. If both are
   * pending at the same time, then SVCall takes precedence because of its lower
   * exception number. In addition, the associated exception handlers do not
   * have to check whether they are returning to Thread mode, because they
   * cannot preempt any other exception.
   */
  NVIC_SetPriority(SVCall_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
  NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

  /*
   * Force the preceding configurations to take effect before further
   * operations.
   */
  __DSB();
  __ISB();
}
/*----------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *thread,
             void (*function)(void *data), void *data)
{
  struct mtarch_thread_context *context = &thread->start_stack.context;

  /*
   * Initialize the thread context with the appropriate values to call
   * function() with data and to make function() return to mt_exit() without
   * having to call it explicitly.
   */
  context->exc_return = EXC_RETURN_PROCESS_THREAD_BASIC_FRAME;
  context->r0 = (uint32_t)data;
  context->lr = (uint32_t)mt_exit;
  context->pc = (uint32_t)function;
  context->xpsr = xPSR_T_Msk;
  thread->psp = (uint32_t)context;
}
/*----------------------------------------------------------------------------*/
void
mtarch_exec(struct mtarch_thread *thread)
{
  /* Pass the PSP to SVCall, and get the updated PSP as its return value. */
  register uint32_t psp __asm__ ("r0") = thread->psp;
  __asm__ volatile ("svc #0"
                    : "+r" (psp)
                    :: "memory");
  thread->psp = psp;
}
/*----------------------------------------------------------------------------*/
__attribute__ ((__naked__))
void
mtarch_yield(void)
{
  /* Invoke SVCall. */
  __asm__ ("svc #0\n\t"
           "bx lr");
}
/*----------------------------------------------------------------------------*/
void
mtarch_stop(struct mtarch_thread *thread)
{
}
/*----------------------------------------------------------------------------*/
void
mtarch_pstart(void)
{
  /* Trigger PendSV. */
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}
/*----------------------------------------------------------------------------*/
void
mtarch_pstop(void)
{
}
/*----------------------------------------------------------------------------*/
void
mtarch_remove(void)
{
}
/*----------------------------------------------------------------------------*/

/** @} */
