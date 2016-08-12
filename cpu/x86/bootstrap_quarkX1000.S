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

#include "stacks.h"

# Multiboot
.set MAGIC_NUMBER, 0x1BADB002
.set FLAGS, 0x0
.set CHECKSUM, -MAGIC_NUMBER

.section .multiboot
.align 4
.long MAGIC_NUMBER
.long FLAGS
.long CHECKSUM

.section .boot_text
.global start
start:
        cli
#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__TSS
        /* TSS-based protection domains use a multi-segment model that defines
         * tight bounds around stacks.  That means that the bottom of the stack
         * has an offset of 0, which is the address of the stacks_main symbol.
         * The following code computes the physical load address of the top of
         * the stack, which is what should be initially used as the stack
         * pointer while the flat memory model is in use.
         */
        lea _sdata_addr, %eax
        lea (stacks_main + STACKS_SIZE_MAIN)(%eax), %esp
#else
        mov $(stacks_main + STACKS_SIZE_MAIN), %esp
#endif
        call cpu_boot_stage0
