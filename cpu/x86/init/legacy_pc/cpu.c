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

#include "cpu.h"
#include "gdt.h"
#include "helpers.h"
#include "idt.h"
#include "interrupt.h"
#include "irq.h"
#include "stacks.h"

static void
double_fault_handler(struct interrupt_context context)
{
  halt();
}
/*---------------------------------------------------------------------------*/
/* The OS has switched to its own segment descriptors.  When multi-segment
 * protection domain support is enabled, this routine runs with the
 * necessary address translations configured to invoke other routines that
 * require those translations to be in place.  However, the protection domain
 * support, if enabled, has not yet been fully activated.
 */
static void
boot_stage1(void)
{
  idt_init();

  /* Set an interrupt handler for Double Fault exception. This way, we avoid
   * the system to triple fault, leaving no trace about what happened.
   */
  SET_EXCEPTION_HANDLER(8, 1, double_fault_handler);

  /* Initialize protection domain support, if enabled */
  prot_domains_init();

  prot_domains_leave_boot_stage1();
}
/*---------------------------------------------------------------------------*/
int main(void);
/* This routine runs with the initial, flat address space, even if protection
 * domain support is enabled.  The goal behind the design of this routine is to
 * keep it as short as possible, since it is unable to directly reference data
 * and invoke functions that are intended to be accessible later after the
 * system has booted when a multi-segment protection domain model is in use.
 */
void
cpu_boot_stage0(void)
{
  /* Reserve three stack slots for return addresses */
  uintptr_t top_of_stack = STACKS_INIT_TOP;

#if X86_CONF_PROT_DOMAINS != X86_CONF_PROT_DOMAINS__NONE
  uintptr_t *top_of_stack_ptr =
    (uintptr_t *)DATA_OFF_TO_PHYS_ADDR(top_of_stack);

  top_of_stack_ptr[0] = (uintptr_t)prot_domains_launch_kernel;
  top_of_stack_ptr[1] = (uintptr_t)prot_domains_launch_app;
#endif

  /* Perform common GDT initialization */
  gdt_init();

  /* Switch all data segment registers to the newly-initialized flat data
   * descriptor.
   */
  __asm__(
    "mov %0, %%ds\n\t"
    "mov %0, %%es\n\t"
    "mov %0, %%fs\n\t"
    "mov %0, %%gs\n\t"
    :
    : "r" (GDT_SEL_DATA_FLAT)
    );

  /**
   * Perform specific GDT initialization tasks for the protection domain
   * implementation that is enabled, if any.
   */
  prot_domains_gdt_init();

  /* Do not pass memory operands to the asm block below, since it is
   * switching from the flat address space to a multi-segment address space
   * model if such a model is used by the enabled protection domain
   * implementation, if any.
   */
  __asm__(
    "mov %[_ss_], %%ss\n\t"
    "mov %[_esp_], %%esp\n\t"
    "ljmp %[_cs_], %[_stage1_]\n\t"
    :
    : [_ss_] "r" (GDT_SEL_STK_EXC),
      [_esp_] "r" (top_of_stack),
      [_cs_] "i" ((uint16_t)GDT_SEL_CODE_EXC),
      [_stage1_] "i" (boot_stage1)
    );
}
