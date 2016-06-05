/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
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

OUTPUT_FORMAT("elf32-i386")

ENTRY(start)

SECTIONS {
    /*
       OS-Dev Wiki says it is common for kernels to start at 1M. Addresses before that
       are used by BIOS/EFI, the bootloader and memory-mapped I/O.

       The UEFI GenFw program inserts a 0x220-byte offset between the image base and
       the .text section.  We add that same offset here to align the symbols in the
       UEFI DLL with those in the final UEFI binary to make debugging easier.
    */
    . = 1M + 0x220;

    .text.boot : ALIGN (32)
    {
        *(.multiboot)
        *(.boot_text)

        /*
           Fill out the section to the next 4K boundary so that the UEFI GenFw
           program does not shift the following .text section forward into the
           gap and perturb the symbols.  This only works if the size of this
           section is less than 4K - 0x220 bytes.
        */
        . = 4K - 0x220;
    }

    /*
       It is actually desired that each of the following sections be page-
       aligned.  However, the UEFI GenFw program ratchets up its alignment
       granularity to the maximum granularity discovered in its input file.
       Using page-alignment perturbs the symbols, hindering debugging.  Thus,
       this file simply pads each section out to the desired page alignment and
       declares a section alignment granularity of 32 bytes.
    */

    .text : ALIGN (32)
    {
        *(.text*)

        . = ALIGN(4K);
    }

    _stext_addr = ADDR(.text);
    _etext_addr = ADDR(.text) + SIZEOF(.text);

    .data.stack : ALIGN (32)
    {
        /*
           Introduce a guard band page before the stacks to facilitate stack
           overflow detection.  This approach wastes a page of memory for each
           guard band, but has the advantage of enabling an identity mapping
           for all linear to physical addresses except those in the MMIO
           regions.  The guard bands are marked not-present in the page tables
           to facilitate stack overflow detection.

           This padding must be placed inside of the section, or else it will
           get dropped when the UEFI GenFw program generates the UEFI binary.
        */
        . += 4K;

        /*
           Place the main stack first so that an overflow is detected and does
           not overwrite the interrupt or supervisor stacks.  Usage of the
           interrupt and stack is predictable, since it is only used by short
           trampoline code sequences that quickly pivot to the main stack.
        */
        *(.main_stack)
        *(.int_stack)
        *(.exc_stack)

        /*
           The combined sizes of the stacks is an even multiple of 4K, so there
           is no need to align the location counter here.
        */

       /*
           Introduce a guard band page after the stacks to detect stack underflow.
           Note that an underflow that only affects the interrupt and supervisor
           stacks will not generate a page fault.  Detecting such conditions by
           placing the interrupt and supervisor stacks on separate pages would
           substantially increase memory usage.
        */
        . += 4K;
    }

    .data : ALIGN (32)
    {
        /*
           The UEFI GenFw program treats all sections that are alloc and read-
           only as code sections.  By that criteria, .rodata would be a code
           section, but making such data executable is undesirable.  Thus, this
           script lumps in .rodata with other data.  It may be desirable in the
           future to actually write-protect this data.
        */
        *(.rodata*)
        *(.data*)

        _sdata_kern_startup_func = .;
        KEEP(*(.kern_startup_func))
        _edata_kern_startup_func = .;

        /*
           These could alternatively be treated as read-only data to prevent tampering
           from the user privilege level.
        */
        _sdata_shared_isr = .;
        KEEP(*(.shared_isr_data*))
        _edata_shared_isr = .;

        . = ALIGN(4K);
    }

    .bss : ALIGN (32)
    {
        *(COMMON)
        *(.bss*)

        . = ALIGN(4K);
    }

    _sdata_addr = ADDR(.data);
    _edata_addr = ADDR(.bss) + SIZEOF(.bss);

    .bss.kern (NOLOAD) : ALIGN (32)
    {
        /*
           Page-aligned data is output first.
           It is infeasible to apply a page-alignment attribute to them in the
           source code, because that increases the alignment of this section to
           be page-aligned, which causes problems when generating a UEFI binary
           as described above.
        */
        *(.page_aligned_kern_bss)
        *(.kern_bss)

        syscalls_entrypoints = .;
        *(.syscall_bss)
        syscalls_entrypoints_end = .;

        . = ALIGN(4K);
    }

    _ebss_syscall_addr = ADDR(.bss.kern) + SIZEOF(.bss.kern);

    .bss.kern_priv (NOLOAD) : ALIGN (32)
    {
        prot_domains_kern_data = .;
        /*
            The kernel and app protection domain control structures must always
            be placed in the first two slots in this order, so that they have
            well-known protection domain IDs:
         */
        *(.kern_prot_dom_bss)
        *(.app_prot_dom_bss)
        *(.prot_dom_bss)
        prot_domains_kern_data_end = .;

        *(.gdt_bss_start)
        *(.gdt_bss_mid)
        *(.gdt_bss)
        _ebss_gdt_addr = .;

        . = ALIGN(4K);
    }

    _sbss_kern_addr = ADDR(.bss.kern);
    _ebss_kern_addr = ADDR(.bss.kern_priv) + SIZEOF(.bss.kern_priv);

    .bss.meta (NOLOAD) : ALIGN (32)
    {
        *(.meta_bss)

        . = ALIGN(4K);
    }

    _ebss_pre_dma_addr = ALIGN(32);
}
