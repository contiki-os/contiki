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

#ifndef CPU_X86_MM_TSS_H_
#define CPU_X86_MM_TSS_H_

#include <stdint.h>

/**
 * Task State Segment.  Used by the CPU to manage switching between
 * different protection domains (tasks).  The current task is referenced
 * by the Task Register.  When the CPU switches away from a task due to
 * a far call, etc., it updates the associated in-memory TSS with the
 * current state of the task.  It then loads CPU state from the TSS for
 * the new task.  See Intel Combined Manual, Vol. 3, Chapter 7 for more
 * details.
 */
typedef struct tss {
  uint32_t prev_tsk; /**< The selector of the task that called this one, if applicable */
  uint32_t esp0;     /**< Stack pointer for ring 0 code in this task */
  uint32_t ss0;      /**< Stack segment selector for ring 0 code in this task */
  uint32_t esp1;     /**< Stack pointer for ring 1 code in this task */
  uint32_t ss1;      /**< Stack segment selector for ring 1 code in this task */
  uint32_t esp2;     /**< Stack pointer for ring 2 code in this task */
  uint32_t ss2;      /**< Stack segment selector for ring 2 code in this task */
  uint32_t cr3;      /**< CR3 for this task when paging is enabled */
  uint32_t eip;      /**< Stored instruction pointer value */
  uint32_t eflags;   /**< Settings for EFLAGS register */
  /** General purpose register values */
  uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
  /** Segment register selector values */
  uint32_t es, cs, ss, ds, fs, gs;
  /** Selector for Local Descriptor Table */
  uint32_t ldt;
  /** Debug-related flag */
  uint16_t t;
  /** Offset from base of TSS to base of IO permission bitmap, if one is installed */
  uint16_t iomap_base;
} tss_t;

void tss_init(void);

#endif /* CPU_X86_TSS_H_ */
