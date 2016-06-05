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
#include "gdt-layout.h"

#include "idt.h"

struct interrupt_context {
  /* The general-purpose register values are saved by the pushal instruction in
   * the interrupt dispatcher. Having access to these saved values may be
   * useful in some future interrupt or exception handler, and saving and later
   * restoring them also enables the ISR to freely overwrite the EAX, ECX, and
   * EDX registers as is permitted by the cdecl calling convention.
   */
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  /* These two values are pushed on the stack by the CPU when it delivers an
   * exception with an associated error code.  Currently, only the double fault
   * handler accepts this structure as a parameter, and that type of exception
   * does have an associated error code.
   */
  uint32_t error_code;
  uint32_t eip;
  /* The CPU pushes additional values beyond these on the stack, specifically
   * the code segment descriptor and flags.  If a privilege-level change occurs
   * during delivery, the CPU additionally pushes the stack pointer and stack
   * segment descriptor.
   */
};

#define ISR_STUB(label_str, has_error_code, handler_str, exc)    \
  "jmp 2f\n\t"                                                   \
  ".align 4\n\t"                                                 \
  label_str ":\n\t"                                              \
  "         pushal\n\t"                                          \
  PROT_DOMAINS_ENTER_ISR(exc)                                    \
  "         call " handler_str "\n\t"                            \
  PROT_DOMAINS_LEAVE_ISR(exc)                                    \
  "         popal\n\t"                                           \
  "         .if " #has_error_code "\n\t"                         \
  "         add $4, %%esp\n\t"                                   \
  "         .endif\n\t"                                          \
  "         iret\n\t"                                            \
  "2:\n\t"

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
 * exc:             0 if this is an interrupt, which should be handled
 *                  at the interrupt privilege level.  1 if this is an
 *                  exception, which should be handled at the
 *                  exception privilege level.
 * dpl:             Privilege level for IDT descriptor, which is the
 *                  numerically-highest privilege level that can
 *                  generate this interrupt with a software interrupt
 *                  instruction.
 *
 * Since there is no easy way to write an Interrupt Service Routines
 * (ISR) in C (for further information on this, see [1]), we provide
 * this helper macro. It basically provides an assembly trampoline
 * to a C function (handler parameter) which, indeed, handles the
 * interrupt.
 *
 * [1] http://wiki.osdev.org/Interrupt_Service_Routines
 */
#define SET_INT_EXC_HANDLER(num, has_error_code, handler, exc, dpl) \
  do {                                                              \
    __asm__ __volatile__ (                                          \
      "pushl %[_dpl_]\n\t"                                          \
      "pushl %[_cs_]\n\t"                                           \
      "pushl $1f\n\t"                                               \
      "pushl %[_isr_num_]\n\t"                                      \
      "call idt_set_intr_gate_desc\n\t"                             \
      "add $16, %%esp\n\t"                                          \
      ISR_STUB("1", has_error_code, "%P[_handler_]", exc)           \
      :                                                             \
      : [_isr_num_] "g" (num),                                      \
        [_handler_] "i" (handler),                                  \
        [_cs_] "i" (exc ? GDT_SEL_CODE_EXC : GDT_SEL_CODE_INT),     \
        [_dpl_] "i" (dpl)                                           \
      /* the invocation of idt_set_intr_gate_desc may clobber   */  \
      /* the caller-saved registers:                            */  \
      : "eax", "ecx", "edx"                                         \
    );                                                              \
  } while (0)
#define SET_INTERRUPT_HANDLER(num, has_error_code, handler)         \
  SET_INT_EXC_HANDLER(num, has_error_code, handler, 0, PRIV_LVL_INT)
#define SET_EXCEPTION_HANDLER(num, has_error_code, handler)         \
  SET_INT_EXC_HANDLER(num, has_error_code, handler, 1, PRIV_LVL_EXC)

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
