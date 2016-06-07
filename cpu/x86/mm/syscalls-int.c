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

#include "prot-domains.h"
#include "tss.h"
#include "helpers.h"
#include "stacks.h"
#include "idt.h"
#include "syscalls.h"
#include "gdt.h"
#include "gdt-layout.h"
#include "interrupt.h"

/**
 * Current protection domain.  Not protected, since it is just a convenience
 * variable to avoid unneeded protection domain switches.
 */
dom_id_t cur_dom = DOM_ID_app;

/* defined in syscalls-int-asm.S */
void prot_domains_sysret_dispatcher(void);

/* Maximum depth of inter-domain call stack */
#define MAX_INTER_DOM_CALL_STK_SZ 4

/* Protected call stack for inter-domain system calls.  The stack grows up. */
static volatile dom_id_t ATTR_BSS_KERN
  inter_dom_call_stk[MAX_INTER_DOM_CALL_STK_SZ];

/* Pointer to the next (free) slot in the inter-domain call stack */
static int ATTR_BSS_KERN inter_dom_call_stk_ptr;

/*---------------------------------------------------------------------------*/
static inline void __attribute__((always_inline))
update_eflags(dom_id_t from_id, dom_id_t to_id, interrupt_stack_t *intr_stk)
{
  if((to_id == DOM_ID_app) &&
     (DT_SEL_GET_RPL(intr_stk->cs) == PRIV_LVL_USER)) {
    /* Only enable interrupts in the application protection domain cooperative
     * scheduling context.
     */
    intr_stk->eflags |= EFLAGS_IF;
  } else {
    intr_stk->eflags &= ~EFLAGS_IF;
  }
}
/*---------------------------------------------------------------------------*/
static inline void __attribute__((always_inline))
dispatcher_tail(dom_id_t from_id, dom_id_t to_id, interrupt_stack_t *intr_stk)
{
  cur_dom = to_id;

  prot_domains_switch(from_id, to_id, intr_stk);

  prot_domains_set_wp(true);

  update_eflags(from_id, to_id, intr_stk);
}
/*---------------------------------------------------------------------------*/
int main(void);
static inline void __attribute__((always_inline))
syscall_dispatcher_tail(interrupt_stack_t *intr_stk,
                        dom_id_t to_id,
                        uint32_t syscall_eip)
{
  dom_id_t from_id;
  uint32_t tmp;
  volatile dom_kern_data_t ATTR_KERN_ADDR_SPACE *from_dkd, *to_dkd;

  uint32_t loc_call_stk_ptr;

  to_dkd = prot_domains_kern_data + to_id;

  /* This implementation of protection domains is non-reentrant.  For example,
   * it stores the return address taken from the stack of a caller domain
   * while dispatching a system call and stores it in a single field in the
   * kernel data associated with that protection domain.  That model does not
   * permit reentrancy.
   */
  KERN_READL(tmp, to_dkd->flags);
  if((tmp & PROT_DOMAINS_FLAG_BUSY) == PROT_DOMAINS_FLAG_BUSY) {
    halt();
  }
  tmp |= PROT_DOMAINS_FLAG_BUSY;
  KERN_WRITEL(to_dkd->flags, tmp);

  /* Update the interrupt stack so that the IRET instruction will return to the
   * system call entrypoint.
   */
  intr_stk->eip = syscall_eip;

  KERN_READL(loc_call_stk_ptr, inter_dom_call_stk_ptr);
  /* Lookup the information for the caller */
  KERN_READL(from_id, inter_dom_call_stk[loc_call_stk_ptr - 1]);
  from_dkd = prot_domains_kern_data + from_id;

  /* Save the current return address from the unprivileged stack to a protected
   * location in the kernel-owned data structure.  This enforces return
   * entrypoint control.
   */
  KERN_WRITEL(from_dkd->orig_ret_addr, *(uintptr_t *)intr_stk->esp);
  /* Update the unprivileged stack so that when the system call body is
   * complete, it will invoke the system call return stub.
   */
  *((uintptr_t *)intr_stk->esp) = (uintptr_t)prot_domains_sysret_stub;

  if(MAX_INTER_DOM_CALL_STK_SZ <= loc_call_stk_ptr) {
    halt();
  }
  KERN_WRITEL(inter_dom_call_stk[loc_call_stk_ptr], to_id);

  loc_call_stk_ptr++;
  KERN_WRITEL(inter_dom_call_stk_ptr, loc_call_stk_ptr);

  dispatcher_tail(from_id, to_id, intr_stk);
}
/*---------------------------------------------------------------------------*/
void __attribute__((fastcall))
prot_domains_syscall_dispatcher_impl(interrupt_stack_t *intr_stk,
                                     dom_id_t to_id,
                                     syscalls_entrypoint_t *syscall)
{
  uint32_t tmp;
  uint32_t syscall_eip;

  if(PROT_DOMAINS_ACTUAL_CNT <= to_id) {
    halt();
  }

  /* Get the approved entrypoint for the system call being invoked */

  if(!((((uintptr_t)syscalls_entrypoints) <= (uintptr_t)syscall) &&
       (((uintptr_t)syscall) < (uintptr_t)syscalls_entrypoints_end) &&
       (((((uintptr_t)syscall) - (uintptr_t)syscalls_entrypoints)
           % sizeof(syscalls_entrypoint_t)) == 0))) {
    /* Assert is not usable when switching protection domains */
    halt();
  }

  KERN_READL(tmp, syscall->doms);
  if((BIT(to_id) & tmp) == 0) {
    halt();
  }

  KERN_READL(syscall_eip, syscall->entrypoint);

  prot_domains_set_wp(false);

  syscall_dispatcher_tail(intr_stk, to_id, syscall_eip);
}
/*---------------------------------------------------------------------------*/
int main(void);
void __attribute__((fastcall))
prot_domains_launch_kernel_impl(interrupt_stack_t *intr_stk)
{
  KERN_WRITEL(inter_dom_call_stk[0], DOM_ID_app);

  KERN_WRITEL(inter_dom_call_stk_ptr, 1);

  syscall_dispatcher_tail(intr_stk, DOM_ID_kern, (uint32_t)main);
}
/*---------------------------------------------------------------------------*/
void __attribute__((fastcall))
prot_domains_sysret_dispatcher_impl(interrupt_stack_t *intr_stk)
{
  dom_id_t from_id, to_id;
  uint32_t loc_call_stk_ptr;
  uint32_t flags;

  KERN_READL(loc_call_stk_ptr, inter_dom_call_stk_ptr);
  if(loc_call_stk_ptr <= 1) {
    halt();
  }

  KERN_READL(from_id, inter_dom_call_stk[loc_call_stk_ptr - 1]);
  KERN_READL(to_id, inter_dom_call_stk[loc_call_stk_ptr - 2]);

  KERN_READL(intr_stk->eip,
            prot_domains_kern_data[to_id].orig_ret_addr);

  prot_domains_set_wp(false);

  KERN_READL(flags, prot_domains_kern_data[from_id].flags);
  flags &= ~PROT_DOMAINS_FLAG_BUSY;
  KERN_WRITEL(prot_domains_kern_data[from_id].flags, flags);

  KERN_WRITEL(inter_dom_call_stk_ptr, loc_call_stk_ptr - 1);

  dispatcher_tail(from_id, to_id, intr_stk);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief  Lookup the current protection domain.
 * \return Kernel data structure for the current protection domain.
 */
static volatile dom_kern_data_t ATTR_KERN_ADDR_SPACE *
get_current_domain(void)
{
  uint32_t loc_call_stk_ptr;
  dom_id_t id;
  KERN_READL(loc_call_stk_ptr, inter_dom_call_stk_ptr);
  KERN_READL(id, inter_dom_call_stk[loc_call_stk_ptr - 1]);
  return prot_domains_kern_data + id;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief     Check whether the protection domain is authorized to perform port
 *            I/O from the cooperative scheduling context.
 * \param dkd Protection domain to check
 * \return    Result of the check as a Boolean value
 */
static bool
needs_port_io(volatile dom_kern_data_t ATTR_KERN_ADDR_SPACE *dkd)
{
  uint32_t dkd_flags;
  KERN_READL(dkd_flags, dkd->flags);
  return (dkd_flags & PROT_DOMAINS_FLAG_PIO) == PROT_DOMAINS_FLAG_PIO;
}
/*---------------------------------------------------------------------------*/
/* Mark the context parameter as volatile so that writes to it will not get
 * optimized out.  This parameter is not handled like ordinary function
 * parameters.  It actually partially includes the contents of the exception
 * stack, so updates to those locations can affect the operation of the
 * subsequent interrupt return.
 */
static void
gp_fault_handler(volatile struct interrupt_context context)
{
  uint32_t cs_lim;
  uint8_t opcode;

  volatile dom_kern_data_t ATTR_KERN_ADDR_SPACE *dkd = get_current_domain();
  if (needs_port_io(dkd)) {
    __asm__ __volatile__ (
      "mov %%cs, %0\n\t"
      "lsl %0, %0\n\t"
      : "=r"(cs_lim));

    if (cs_lim < context.eip) {
      halt();
    }

    /* Load first byte of faulting instruction */
    __asm__ __volatile__ (
      "movb %%cs:%1, %0"
      : "=q"(opcode)
      : "m"(*(uint8_t *)context.eip));

    switch (opcode) {
    case 0xEC: /* inb */
      context.eax = (context.eax & ~0xFF) | inb((uint16_t)context.edx);
      break;
    case 0xED: /* inl */
      context.eax = inl((uint16_t)context.edx);
      break;
    case 0xEE: /* outb */
      outb((uint16_t)context.edx, (uint8_t)context.eax);
      break;
    case 0xEF: /* outl */
      outl((uint16_t)context.edx, context.eax);
      break;
    default:
      halt();
    }

    /* Skip the faulting port I/O instruction that was emulated. */
    context.eip++;
  } else {
    halt();
  }
}
/*---------------------------------------------------------------------------*/
void
syscalls_int_init(void)
{
  tss_init();

  SET_EXCEPTION_HANDLER(13, 1, gp_fault_handler);

  /* Register system call dispatchers: */

  idt_set_intr_gate_desc(PROT_DOMAINS_SYSCALL_DISPATCH_INT,
                         (uint32_t)prot_domains_syscall_dispatcher,
                         GDT_SEL_CODE_EXC,
                         PRIV_LVL_USER);
  idt_set_intr_gate_desc(PROT_DOMAINS_SYSRET_DISPATCH_INT,
                         (uint32_t)prot_domains_sysret_dispatcher,
                         GDT_SEL_CODE_EXC,
                         PRIV_LVL_USER);
}
/*---------------------------------------------------------------------------*/
