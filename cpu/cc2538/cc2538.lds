/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
/*
 * cc2538 linker configuration file. This is not the actual file used at link
 * stage. Rather, it is used as input for the auto-generation of the actual
 * ld script, which is called cc2538.ld and will be in the project directory
 */
#if (LPM_CONF_MAX_PM==2) && (LPM_CONF_ENABLE != 0)
#define NRSRAM_START    0x20000000
#define NRSRAM_LEN      0x00004000
#define SRAM_START      0x20004000
#define SRAM_LEN        0x00004000
#else
#define SRAM_START      0x20000000
#define SRAM_LEN        0x00008000
#endif

#ifdef FLASH_CONF_ORIGIN
#define FLASH_ORIGIN    FLASH_CONF_ORIGIN
#else
#error FLASH_CONF_ORIGIN is not specified. Please define FLASH_CONF_ORIGIN in contiki-conf.h.
#endif

#ifdef FLASH_CONF_SIZE
#define FLASH_SIZE      FLASH_CONF_SIZE
#else
#error FLASH_CONF_SIZE is not specified. Please define FLASH_CONF_SIZE in contiki-conf.h.
#endif

#define FLASH_CCA_LENGTH 44
#define FLASH_LENGTH     (FLASH_SIZE - FLASH_CCA_LENGTH)
#define FLASH_CCA_ORIGIN (FLASH_ORIGIN + FLASH_LENGTH)

MEMORY
{
    FLASH (rx) : ORIGIN = FLASH_ORIGIN, LENGTH = FLASH_LENGTH
    FLASH_CCA (RX) : ORIGIN = FLASH_CCA_ORIGIN, LENGTH = FLASH_CCA_LENGTH
#if (LPM_CONF_MAX_PM==2) && (LPM_CONF_ENABLE != 0)
    NRSRAM (RWX) : ORIGIN = NRSRAM_START, LENGTH = NRSRAM_LEN
#endif
    SRAM (RWX) : ORIGIN = SRAM_START, LENGTH = SRAM_LEN
}

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

    .socdata (NOLOAD) :
    {
        *(.udma_channel_control_table)
    } > SRAM

    .data : ALIGN(4)
    {
        _data = .;
        *(.data*)
        _edata = .;
    } > SRAM AT > FLASH
    _ldata = LOADADDR(.data);

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

    .stack (NOLOAD) :
    {
        *(.stack)
    } > SRAM

#if (LPM_CONF_MAX_PM==2) && (LPM_CONF_ENABLE != 0)
    .nrdata (NOLOAD) :
    {
        _nrdata = .;
        *(.nrdata*)
        _enrdata = .;
    } > NRSRAM
#endif

    .flashcca :
    {
        KEEP(*(.flashcca))
    } > FLASH_CCA
}
