/*
 @file       cc26x0f128.lds
 @brief      CC26x0F128 linker configuration file for GNU compiler.

 @Revised     $Date: 2014-06-06 09:59:22 +0200 (fr, 06 jun 2014) $
 @Revision    $Revision: 13151 $

  Copyright (C) 2014 Texas Instruments Incorporated - http:www.ti.com/


  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*. Entry Point *./
ENTRY( ResetIsr )

/* System memory map */
MEMORY
{
    /* Application stored in and executes from internal flash */
    /* Flash Size 128 KB */
    FLASH (RX) : ORIGIN = 0x0, LENGTH = 0x0001FFB0
    /* Customer Configuration Area and Bootloader Backdoor configuration in flash, 80 bytes*/
    FLASH_CCFG (RX) : ORIGIN = 0x1FFB0, LENGTH = 80
    /* Application uses internal RAM for data */
    /* RAM Size 20 KB */
    SRAM (RWX) : ORIGIN = 0x20000000, LENGTH = 0x5000
}

/*. Highest address of the stack. Used in startup file .*/
_estack = ORIGIN(SRAM) + LENGTH(SRAM); /*end of SRAM .*/

/*. Generate a link error if heap and stack don’t fit into RAM .*/
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x100;


/* Section allocation in memory */
SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH= 0
    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM
    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM
    .ccfg :
    { 
    } > FLASH_CCFG
    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
      . = ALIGN(4);
      . = . + _Min_Heap_Size;
      . = . + _Min_Stack_Size;
      . = ALIGN(4);
    } >SRAM
}


