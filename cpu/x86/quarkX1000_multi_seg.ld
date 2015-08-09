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

OUTPUT_FORMAT("elf32-i386")

ENTRY(start)

/*
   The TSS-based protection domain implementation does not explicitly reference
   these symbols, so we list them here to prevent them from being garbage-
   collected.
*/
EXTERN(stacks_int)
EXTERN(stacks_exc)

PHDRS {
    boot_text PT_LOAD;
    text PT_LOAD;
    data PT_LOAD;
}

SECTIONS {
    /*
       OS-Dev Wiki says it is common for kernels to start at 1M. Addresses before that
       are used by BIOS/EFI, the bootloader and memory-mapped I/O.

       The UEFI GenFw program inserts a 0x220 byte offset between the image base and
       the .text section.  We add that same offset here to align the symbols in the
       UEFI DLL with those in the final UEFI binary to make debugging easier.
    */
    . = 1M + 0x220;

    /*
       The GenFw program in the EDK2 UEFI toolchain outputs UEFI images with a
       section alignment of at least 32 bytes.  Thus, it is desirable to use at
       least that alignment granularity to avoid symbols being shifted from the
       intermediate DLL to the final UEFI image.  Such shifting may make
       debugging more difficult by preventing the DLL from being a useful
       source of symbol information.  The debugging symbols are not included in
       the final UEFI image.
    */
    .text.boot : ALIGN (32)
    {
        *(.multiboot)
        /*
           The initial bootstrap code expects to operate in a flat address
           space with an identity mapping between linear and physical
           addresses.
        */
        *(.boot_text)
    } :boot_text

    /* The post-boot code segments define tight bounds around the code
       section, so this directive resets the virtual address to 0. */
    . = 0;

    /* The virtual address differs from the load address. */
    .text : AT(LOADADDR(.text.boot) + ALIGN(SIZEOF(.text.boot), 32)) ALIGN (32)
    {
        /*
           These BYTE directives emit a UD2 instruction to cause execution to
           halt if the control flow ever deviates to address 0.  This also
           prevents other code from being placed at address 0.  Some code
           considers a function pointer to address 0 to be a null function
           pointer.
        */
        BYTE(0x0F);
        BYTE(0x0B);
        *(.text*)

        /*
           An alternative design to eliminate the need for ALIGN directives
           within the AT directives in later sections could have padded
           each section out to a 32-byte boundary.  However, that would have
           enabled unneeded software accesses to the padding past the end of actual
           code/data in each section, since segments are also configured based on
           the values of the SIZEOF expressions.  As a general principle, accesses
           should be as restricted as is feasible.
        */
    } :text

    _stext_addr = LOADADDR(.text);
    _etext_addr = LOADADDR(.text) + SIZEOF(.text);

    . = 0;

    .data : AT(ALIGN(_etext_addr, 32)) ALIGN (32)
    {
        *(.main_stack)
        *(.int_stack)
        *(.exc_stack)
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
    } :data

    .bss : ALIGN (32)
    {
        *(COMMON)
        *(.bss*)
    }

    _sdata_addr = LOADADDR(.data);
    _edata_addr = LOADADDR(.bss) + SIZEOF(.bss);

    . = 0;

    .bss.kern (NOLOAD) : AT(ALIGN(_edata_addr, 32)) ALIGN (32)
    {
        /*
           This directive prevents any data from being allocated at address
           zero, since the address 0 is commonly used to represent null
           pointers.
        */
        LONG(0);
        *(.kern_bss)

        syscalls_entrypoints = .;
        *(.syscall_bss)
        syscalls_entrypoints_end = .;
    }

    _ebss_syscall_addr = LOADADDR(.bss.kern) + SIZEOF(.bss.kern);

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
        KEEP(*(.gdt_bss_mid))
        *(.gdt_bss)
        _ebss_gdt_addr = .;
    }

    _sbss_kern_addr = LOADADDR(.bss.kern);
    _ebss_kern_addr = LOADADDR(.bss.kern_priv) + SIZEOF(.bss.kern_priv);

    . = _ebss_kern_addr;

    .bss.meta (NOLOAD) : AT(ALIGN(_ebss_kern_addr, 32)) ALIGN (32)
    {
        *(.meta_bss)
    }

    /* .bss.meta may be empty, so this uses .bss.kern_priv as a base instead: */
    _ebss_pre_dma_addr = ALIGN(ALIGN(_ebss_kern_addr, 32) + SIZEOF(.bss.meta), 32);
}
