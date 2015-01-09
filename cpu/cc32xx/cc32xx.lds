/*
 * Copyright (c) 2015, 3B Scientific GmbH.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * cc32xx linker configuration file. This is not the actual file used at link
 * stage. Rather, it is used as input for the auto-generation of the actual
 * ld script, which is called cc32xx.ld and will be in the project directory
 */

#ifdef HEAP_CONF_SIZE
#define HEAP_SIZE	HEAP_CONF_SIZE
#else
#error HEAP_CONF_SIZE is not specified. Please define HEAP_CONF_SIZE in contiki-conf.h.
#endif
 
#define SRAM_START		0x20004000
#define SRAM_END		0x20040000
#define SRAM_LENGTH		((SRAM_END - SRAM_START) - HEAP_SIZE) 

MEMORY
{
    SRAM (rwx) : ORIGIN = SRAM_START, LENGTH = SRAM_LENGTH
}

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.intvecs))
        *(.text*)
        *(.rodata*)
		*(.ARM.extab* .gnu.linkonce.armextab.*)
		. = ALIGN(8);
        _etext = .;
    } > SRAM

    .ARM : {
    	__exidx_start = .;
      	*(.ARM.exidx*)
      	__exidx_end = .;
    } > SRAM

    __init_data = .;

    .data : AT(__init_data)
    {
        _data = .;
        *(.data*)
		. = ALIGN (8);
        _edata = .;
    } > SRAM

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM

    .heap :
    {
		_heap = .;
		. = . + HEAP_SIZE;
		. = ALIGN(8);
		_eheap = .;
    }
}