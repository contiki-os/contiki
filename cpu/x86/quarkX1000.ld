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

SECTIONS {
    /*
       OS-Dev Wiki says it is common for kernels to start at 1M. Addresses before that
       are used by BIOS/EFI, the bootloader and memory-mapped I/O.

       The UEFI GenFw program inserts a 0x240-byte offset between the image base and
       the .text section.  We add that same offset here to align the symbols in the
       UEFI DLL with those in the final UEFI binary to make debugging easier.  We also
       apply 32-byte alignments to sections rather than more conventional 4K-byte
       alignments to avoid symbols being shifted from the intermediate DLL to the
       final UEFI image as would occur if the GenFw program shifted the .text section
       from a higher, 4K-aligned offset to the 0x240-byte offset from the image base.
       Such shifting may make debugging more difficult by preventing the DLL from
       being a directly-useful source of symbol information.  The debugging symbols
       are not included in the final UEFI image.  The GenFw program uses a minimum
       section alignment of 32 bytes, so smaller alignment granularities may also
       result in symbol perturbation.
    */
    . = 1M + 0x240;

    .text ALIGN (32) :
    {
        KEEP(*(.multiboot))
        *(.text*)
    }

    .rodata ALIGN (32) :
    {
        *(.rodata*)
    }

    .data ALIGN (32) :
    {
        *(.data*)
    }

    .bss ALIGN (32) :
    {
        *(COMMON)
        *(.bss*)
    }
}
