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

#ifndef CPU_X86_MM_PROT_DOMAINS_H_
#define CPU_X86_MM_PROT_DOMAINS_H_

#if !__ASSEMBLER__
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "helpers.h"
#endif

#define X86_CONF_PROT_DOMAINS__NONE   0
#define X86_CONF_PROT_DOMAINS__PAGING 1

/** Privilege level (ring) for exception handlers and other supervisory code */
#define PRIV_LVL_EXC  0
#if X86_CONF_PROT_DOMAINS != X86_CONF_PROT_DOMAINS__NONE
/** Privilege level for interrupt handlers */
#define PRIV_LVL_INT  2
/** Default privilege level */
#define PRIV_LVL_USER 3
#else
#define PRIV_LVL_INT  PRIV_LVL_EXC
#define PRIV_LVL_USER PRIV_LVL_EXC
#endif

#define DOM_ID_kern 0
#define DOM_ID_app 1

/** I/O Privilege Level */
#define EFLAGS_IOPL(pl) ((pl) << 12)
/** Interrupt Enable Flag */
#define EFLAGS_IF (1u << 9)

#if !__ASSEMBLER__

/** Protection domain ID */
typedef uint32_t dom_id_t;

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
#include "paging-prot-domains.h"
#endif

/* The following symbols are defined in the linker script */
/** Bounds for .text section */
extern uint32_t _stext_addr, _etext_addr;

#if X86_CONF_PROT_DOMAINS != X86_CONF_PROT_DOMAINS__NONE

/** Metadata that should not be DMA-accessible */
#define ATTR_BSS_META __attribute__((section(".meta_bss")))
/** Kernel-owned data */
#define ATTR_BSS_KERN __attribute__((section(".kern_bss")))
/** Code that should only be executable during bootup */
#define ATTR_CODE_BOOT __attribute__((section(".boot_text")))

/**
 * Domain-defined metadata must be page-aligned, which is implemented by the
 * linker script for variables with this attribute.
 */
#define ATTR_BSS_KERN_PAGE_ALIGNED                                            \
  __attribute__((section(".page_aligned_kern_bss")))

/** Bounds for .kern_data, .syscall_data, and .prot_dom_data sections */
extern uint32_t _sbss_kern_addr, _ebss_kern_addr;
/** End of .syscall_data section */
extern uint32_t _ebss_syscall_addr;
/** Bounds for other data sections */
extern uint32_t _sdata_addr, _edata_addr;

/**
 * If set, this protection domain is already in the call stack and is not
 * available for nested invocations.
 */
#define PROT_DOMAINS_FLAG_BUSY   BIT(0)
/** If set, this protection domain requires port I/O access. */
#define PROT_DOMAINS_FLAG_PIO    BIT(1)
/** If set, this protection domain is initialized. */
#define PROT_DOMAINS_FLAG_INITED BIT(2)

/**
 * Data associated with each protection domain that should be fully accessible
 * only to the kernel, with limited accesses and modifications permitted from
 * other domains.  Includes storage for system data structures.
 */
typedef struct dom_kern_data dom_kern_data_t;

extern volatile dom_kern_data_t prot_domains_kern_data[];
extern volatile dom_kern_data_t prot_domains_kern_data_end[];

#define PROT_DOMAINS_ACTUAL_CNT                                               \
  (prot_domains_kern_data_end - prot_domains_kern_data)

#define PROT_DOMAINS_GET_DOM_ID(dkd)                                          \
  ((dom_id_t)((dkd) - prot_domains_kern_data))

void prot_domains_syscall_dispatcher(void);

/**
 * Data associated with each protection domain that is owned by clients of that
 * domain and used to identify the domain.
 */
struct dom_client_data {
  dom_id_t dom_id;
} __attribute__((packed));

/** Allocate the client-owned protection domain data structure. */
#define PROT_DOMAINS_PDCS_NM(nm) _pdcs_##nm
#define PROT_DOMAINS_ALLOC(typ, nm)                                           \
  static dom_kern_data_t __attribute__((section(".prot_dom_bss")))            \
    PROT_DOMAINS_PDCS_NM(nm);                                                 \
  static typ ATTR_BSS_KERN nm
#define PROT_DOMAINS_INIT_ID(nm)                                              \
  (nm).dom_id = PROT_DOMAINS_GET_DOM_ID(&PROT_DOMAINS_PDCS_NM(nm))

/**
 * Perform early initialization during boot stage 0 to prepare for boot stage 1
 */
void prot_domains_gdt_init() ATTR_CODE_BOOT;
/**
 * Perform initialization during boot stage 1 to prepare for kernel launch
 */
void prot_domains_init();
void prot_domains_impl_init();

/* Return from cpu_boot_stage1 will invoke prot_domains_launch_kernel due to
 * that return address being pushed on the stack by cpu_boot_stage0.
 */
#define prot_domains_leave_boot_stage1()

/* Return from main will invoke prot_domains_launch_app due to that return
 * address being pushed on the stack by cpu_boot_stage0.
 */
#define prot_domains_leave_main()

void prot_domains_launch_kernel(void);

/* Whenever changing this, update syscalls-int-asm.S:prot_domains_launch_kernel
 * to match:
 */
#define PROT_DOMAINS_INIT_RET_ADDR_CNT 2

void app_main(void);
#define prot_domains_launch_app app_main

#else

#define ATTR_BSS_META
#define ATTR_BSS_KERN
#define ATTR_CODE_BOOT

struct dom_client_data {
  uintptr_t mmio; /**< MMIO range base address */
  uintptr_t meta; /**< Domain-defined metadata base address */
};

/** Retrieve the MMIO base address for the specified protection domain. */
#define PROT_DOMAINS_MMIO(dcd) ((dcd).mmio)

/** Retrieve the metadata base address for the specified protection domain. */
#define PROT_DOMAINS_META(dcd) ((dcd).meta)

#define PROT_DOMAINS_ALLOC(typ, nm) static typ nm
#define PROT_DOMAINS_INIT_ID(nm)

#define prot_domains_gdt_init()

#define prot_domains_init()

int main(void);
#define prot_domains_leave_boot_stage1 main
#define prot_domains_leave_main ENABLE_IRQ(); app_main

#define PROT_DOMAINS_INIT_RET_ADDR_CNT 0

#endif

/**
 * Protection domain data readable by the client.  It is used to control
 * execution, so it should be protected from modifications by clients.
 * Otherwise, there is a risk that one client could modify one of these
 * structures used by another client to issue a system call, which could then
 * cause the latter client to perform an unintended system call.
 */
typedef struct dom_client_data dom_client_data_t;

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__NONE
#define prot_domains_reg(dcd, mmio_, mmio_sz, meta_, meta_sz, pio) \
  (dcd)->mmio = (mmio_); \
  (dcd)->meta = (meta_)
#else
/**
 * \brief         Register a protection domain, which involves creating the
 *                necessary system data structures for it.
 *
 * \param dcd     Client-accessible domain information
 * \param mmio    Optional base address for per-domain memory-mapped IO region
 * \param mmio_sz Size of MMIO region
 * \param meta    Optional base address for per-domain metadata
 * \param meta_sz Size of metadata
 * \param pio     Set to true if protection domain requires port IO access
 */
void prot_domains_reg(dom_client_data_t *dcd,
                      uintptr_t mmio,
                      size_t mmio_sz,
                      uintptr_t meta,
                      size_t meta_sz,
                      bool pio);
#endif

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__NONE
#define prot_domains_lookup_meta_phys_base(drv) 0
#else
/** Lookup base physical address of metadata region for specified domain */
uintptr_t prot_domains_lookup_meta_phys_base(dom_client_data_t *drv);
#endif

#if X86_CONF_PROT_DOMAINS != X86_CONF_PROT_DOMAINS__PAGING
#define PROT_DOMAINS_META_OFF_TO_PHYS(off, meta_phys_base)                    \
  ((meta_phys_base) + (off))
#endif

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__NONE
#define PROT_DOMAINS_ENTER_ISR(...)
#define PROT_DOMAINS_LEAVE_ISR(...)
#else
#define PROT_DOMAINS_ENTER_ISR_COMMON(exc)                                    \
  ".if !" #exc "\n\t"                                                         \
  /* Save the current stack pointer into a callee-saved register. */          \
  "mov %%esp, %%ebx\n\t"                                                      \
  /* Pivot to the main stack of the interrupted context.                   */ \
  /* Interrupts never have an error code, so the offset is always 44.      */ \
  /* No interrupt handlers use anything from the original interrupt stack, */ \
  /* so there is no need to copy anything from it to the main stack.       */ \
  "mov 44(%%esp), %%esp\n\t"                                                  \
  ".endif\n\t"
#define PROT_DOMAINS_LEAVE_ISR_COMMON(exc)                                    \
  /* Restore the interrupt/exception stack pointer. */                        \
  ".if !" #exc "\n\t"                                                         \
  "mov %%ebx, %%esp\n\t"                                                      \
  ".endif\n\t"
#endif

#endif /* !__ASSEMBLER__ */

#endif /* CPU_X86_MM_PROT_DOMAINS_H_ */
