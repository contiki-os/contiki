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

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#include "idt.h"

struct interrupt_context {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t error_code;
  uint32_t eip;
};

/* Helper macro to register interrupt handler function.
 *
 * num:             Interrupt number (0-255)
 * has_error_code:  0 if interrupt doesn't push error code onto the
 *                  stack. Otherwise, set this argument to 1.
 * handler:         Pointer to function that should be called once the
 *                  interrupt is raised. In case has_error_code == 0
 *                  the function prototype should be the following:
 *                  void handler(void)
 *                  Otherwise, it should be:
 *                  void handler(struct interrupt_context context)
 *
 * Since there is no easy way to write an Interrupt Service Routines
 * (ISR) in C (for further information on this, see [1]), we provide
 * this helper macro. It basically provides an assembly trampoline
 * to a C function (handler parameter) which, indeed, handles the
 * interrupt.
 *
 * [1] http://wiki.osdev.org/Interrupt_Service_Routines
 */
#define SET_INTERRUPT_HANDLER(num, has_error_code, handler)      \
  do {                                                           \
    __asm__ __volatile__ (                                       \
      "push $1f\n\t"                                             \
      "push %0\n\t"                                              \
      "call %P1\n\t"                                             \
      "add $8, %%esp\n\t"                                        \
      "jmp 2f\n\t"                                               \
      ".align 4\n\t"                                             \
      "1:\n\t"                                                   \
      "         pushal\n\t"                                      \
      "         call %P2\n\t"                                    \
      "         popal\n\t"                                       \
      "         .if " #has_error_code "\n\t"                     \
      "         add $4, %%esp\n\t"                               \
      "         .endif\n\t"                                      \
      "         iret\n\t"                                        \
      "2:\n\t"                                                   \
      :: "g" (num), "i" (idt_set_intr_gate_desc), "i" (handler)  \
      : "eax", "ecx", "edx"                                      \
    );                                                           \
  } while (0)

/* Disable maskable hardware interrupts */
#define DISABLE_IRQ()                                            \
  do {                                                           \
    __asm__ ("cli");                                             \
  } while (0)

/* Enable maskable hardware interrupts */
#define ENABLE_IRQ()                                             \
  do {                                                           \
    __asm__ ("sti");                                             \
  } while (0)

#endif /* INTERRUPT_H */
