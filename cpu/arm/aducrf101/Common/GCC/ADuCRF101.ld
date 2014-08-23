/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(Reset_Handler)

MEMORY
{
        flash (rx) 	: org = 0x00000000, len = 128k
        ram (rwx)       : org = 0x20000000, len = 16k
}

SECTIONS
{
        .vectors : {
                __vectors_start = .;
                KEEP(*(.vectors))
                . = ALIGN(4);
        } >flash

        .text : {
                . = ALIGN(4);
                *(.text)
                *(.text.*)
                etext = .;
        } >flash

        .ARM.ex : {
                *(.ARM.extab* .gnu.linkonce.armextab.*)
                __exidx_start = .;
                *(.ARM.exidx* .gnu.linkonce.armexidx.*)
                __exidx_end = .;
        } >flash

        .rodata : {
                . = ALIGN(4);
                *(.rodata)
                *(.rodata.*)
                . = ALIGN(4);
                __rodata_end = .;
        } >flash

        .data : {
                /* Initialized data is placed in flash, and will get
                   copied to RAM by crt0.S */
                . = ALIGN(4);
                __data_flash_start = LOADADDR(.data);
                __data_start = .;
                *(.data)
                *(.data.*)

                /* Code that goes into RAM also ends up here, and
                   gets copied along with the data section. */
                *(.ramtext*)

                __data_end = .;
                edata = .;
        } > ram AT > flash

        .bss : {
                . = ALIGN(8);
                __bss_start = .;
                *(.bss)
                *(.bss.*)
                *(COMMON)
                . = ALIGN(4);
                __bss_end = .;
                end = .;
        } > ram

        /* Heap grows up from "end" (after bss), and stack grows down from
           the end of RAM. */
        . = ORIGIN(ram) + LENGTH(ram);
        stack_end = .;
}
