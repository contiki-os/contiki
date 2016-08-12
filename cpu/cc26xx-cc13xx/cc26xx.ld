/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

/* CC26XX linker script */

/* Entry Point */
ENTRY(ResetISR)

MEMORY
{
    /* Flash Size 128 KB minus the CCA area below (88 bytes) */
    FLASH (RX) : ORIGIN = 0x00000000, LENGTH = 0x0001FFA8

    /*
     * Customer Configuration Area and Bootloader Backdoor configuration
     * in flash, up to 88 bytes
     */
    FLASH_CCFG (RX) : ORIGIN = 0x0001FFA8, LENGTH = 88

    /* RAM Size 20KB */
    SRAM (RWX) : ORIGIN = 0x20000000, LENGTH = 0x00005000
    
    /* Application can use GPRAM region as RAM if cache is disabled in CCFG */
    GPRAM (RWX) : ORIGIN = 0x11000000, LENGTH = 0x00002000
}

/*. Highest address of the stack. Used in startup file .*/
_estack = ORIGIN(SRAM) + LENGTH(SRAM); /* End of SRAM */

/*. Generate a link error if heap and stack don’t fit into RAM .*/
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x100;

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH = 0

    .data :
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM AT > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > FLASH

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM

    .ccfg :
    {
        KEEP(*(.ccfg))
    } > FLASH_CCFG

    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
      . = ALIGN(4);
      . = . + _Min_Heap_Size;
      . = . + _Min_Stack_Size;
      . = ALIGN(4);
    } > SRAM
    
    .gpram :
    { 
    } > GPRAM
    
}
