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

#include "gdt.h"
#include <stdio.h>
#include "interrupt.h"
#include <stdint.h>
#include <assert.h>
#include "syscalls.h"
#include "stacks.h"

static dom_kern_data_t __attribute__((section(".kern_prot_dom_bss")))
  ATTR_KERN_ADDR_SPACE PROT_DOMAINS_PDCS_NM(kern_dcd);
PROT_DOMAINS_ALLOC_IMPL(kern_dcd);
static dom_client_data_t ATTR_BSS_KERN kern_dcd;
static dom_kern_data_t __attribute__((section(".app_prot_dom_bss")))
  ATTR_KERN_ADDR_SPACE PROT_DOMAINS_PDCS_NM(app_dcd);
PROT_DOMAINS_ALLOC_IMPL(app_dcd);
static dom_client_data_t ATTR_BSS_KERN app_dcd;

/*---------------------------------------------------------------------------*/
void
prot_domains_init(void)
{
  segment_desc_t desc;

  gdt_lookup(GDT_IDX_CODE_EXC, &desc);
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__SWSEG
  /* The exception code segment needs to be readable so that the general
   * protection fault handler can decode instructions, but the interrupt and
   * user level code segments should not be.
   */
  SEG_SET_FLAG(desc, TYPE, SEG_TYPE_CODE_EX);
#endif

  SEG_SET_FLAG(desc, DPL, PRIV_LVL_INT);
  gdt_insert(GDT_IDX_CODE_INT, desc);

  SEG_SET_FLAG(desc, DPL, PRIV_LVL_USER);
  gdt_insert(GDT_IDX_CODE, desc);

  PROT_DOMAINS_INIT_ID(kern_dcd);
  prot_domains_reg(&kern_dcd, 0, 0, 0, 0, true);
  PROT_DOMAINS_INIT_ID(app_dcd);
  prot_domains_reg(&app_dcd, 0, 0, 0, 0, false);

  prot_domains_impl_init();
}
/*---------------------------------------------------------------------------*/
