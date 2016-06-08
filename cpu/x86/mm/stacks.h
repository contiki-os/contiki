/*
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CPU_X86_MM_STACKS_H_
#define CPU_X86_MM_STACKS_H_

#include "prot-domains.h"

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__NONE
#define STACKS_SIZE_INT 0
#else
/**
 * The necessary amount of space for the interrupt and exception stacks is
 * determined by the amount of data pushed on the stack by the CPU when
 * delivering an interrupt or exception, and by the additional data pushed
 * on the stack by the interrupt dispatcher.  See interrupt.h for more details.
 */
#define STACKS_SIZE_INT (14 * 4)
#endif

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
/**
 * The system call and return dispatchers use this stack, so its size was
 * determined by observing their behavior.  It is possible that the dispatchers
 * could overflow the stack and overwrite data on the other stacks.  An
 * alternative design that would facilitate detection of such overflows would
 * place the exception handler stack on a separate page surrounded by guard
 * bands, but that would consume a substantial amount of additional memory.
 *
 * All stack sizes should be a multiple of 4 to accommodate a 4-byte alignment.
 */
#ifdef __clang__
#define STACKS_SIZE_EXC 512
#else
#define STACKS_SIZE_EXC 256
#endif
#elif X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__SWSEG
#ifdef __clang__
#define STACKS_SIZE_EXC 512
#else
#define STACKS_SIZE_EXC 256
#endif
#elif X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__TSS
/**
 * This should be large enough to execute the exception handler with the
 * largest stack requirement: double_fault_handler:
 *  - 1 word for the return address from calling double_fault_handler
 *  - 1 word for the saved frame pointer in double_fault_handler
 *  - 2 words that GCC has been observed to skip on the stack to align it
 *    to a preferred boundary
 *  - 1 word for the return address for calling halt
 */
#define STACKS_SIZE_EXC (STACKS_SIZE_INT + (6 * 4))
#else
#define STACKS_SIZE_EXC STACKS_SIZE_INT
#endif
/**
 * The combined size of the stacks should be an even multiple of the 4K page
 * size so that they precisely fill some number of pages when paging-based
 * protection domains are in use.  The stacks are arranged contiguously by
 * the linker scripts.  See those and README.md for more details.
 */
#define STACKS_SIZE_MAIN (8192 - (STACKS_SIZE_INT + STACKS_SIZE_EXC))

#if !__ASSEMBLER__
/**
 * Stack for exception handlers.  Also used for system call and return
 * dispatchers when paging-based protection domains are enabled.
 */
extern uint8_t stacks_exc[STACKS_SIZE_EXC];
/** Stack for interrupt handlers. */
extern uint8_t stacks_int[STACKS_SIZE_INT];
/** Main C stack. */
extern uint8_t stacks_main[STACKS_SIZE_MAIN];

#define STACKS_INIT_TOP \
  ((uintptr_t)stacks_main + STACKS_SIZE_MAIN - \
   (PROT_DOMAINS_INIT_RET_ADDR_CNT * sizeof(uintptr_t)))

#endif

#endif /* CPU_X86_MM_STACKS_H_ */
