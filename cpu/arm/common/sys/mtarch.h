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
 * \addtogroup arm
 * @{
 *
 * \defgroup arm-cm-mtarch ARM Cortex-M support for Contiki multi-threading
 *
 * All the Cortex-M devices supported by CMSIS-CORE are supported.
 *
 * An exception handler can decide to make the main Contiki thread preempt any
 * running mt thread by calling mtarch_pstart() (e.g. to perform urgent
 * operations that have been triggered by some event or that had been
 * scheduled). If the running thread is already the main Contiki thread, then
 * nothing happens. The corresponding task switch takes place when leaving
 * Handler mode. The main Contiki thread then resumes after the call to
 * mt_exec() that yielded to the preempted mt thread.
 * @{
 *
 * \file
 * Header file for the ARM Cortex-M support for Contiki multi-threading.
 */
#ifndef MTARCH_H_
#define MTARCH_H_

#include "contiki-conf.h"
#include "sys/cc.h"

#include <stdint.h>

#ifndef MTARCH_CONF_STACKSIZE
/** Thread stack size configuration, expressed as a number of 32-bit words. */
#define MTARCH_CONF_STACKSIZE   256
#endif
/** Actual stack size, with minimum size and alignment requirements enforced. */
#define MTARCH_STACKSIZE        ((MAX(MTARCH_CONF_STACKSIZE, \
                                      sizeof(struct mtarch_thread_context) / \
                                      sizeof(uint32_t)) + 1) & ~1)

/**
 * Structure of a saved thread context.
 *
 * <tt>xpsr..r0</tt> are managed by the processor (except in mtarch_start()),
 * while the other register values are handled by the software.
 */
struct mtarch_thread_context {
#if __ARM_ARCH == 7
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
#endif
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t exc_return;
#if __ARM_ARCH == 6
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
#endif
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t pc;
  uint32_t xpsr;
};

struct mtarch_thread {
  uint32_t psp;
  union {
    struct {
      uint32_t free[MTARCH_STACKSIZE -
                    sizeof(struct mtarch_thread_context) / sizeof(uint32_t)];
      struct mtarch_thread_context context;
    } start_stack;
    uint32_t stack[MTARCH_STACKSIZE];
  } CC_ALIGN(8);
};

#endif /* MTARCH_H_ */

/**
 * @}
 * @}
 */
