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
MEMORY
{
    FLASH_FW (rx) : ORIGIN = FLASH_FW_ADDR, LENGTH = FLASH_FW_SIZE
    FLASH_CCA (RX) : ORIGIN = FLASH_CCA_ADDR, LENGTH = FLASH_CCA_SIZE

    /*
     * If PM2 is enabled, then the PM2 SRAM limitations apply, i.e. the
     * regular-leakage SRAM is a non-retention SRAM and the low-leakage SRAM is
     * a full-retention SRAM.
     * Else, the data in the regular-leakage SRAM is always retained, so there
     * are virtually a non-retention SRAM with a size of 0 bytes and a
     * full-retention SRAM spanning the whole SRAM, which is more convenient to
     * use.
     */
#if LPM_CONF_ENABLE && LPM_CONF_MAX_PM >= 2
    NRSRAM (RWX) : ORIGIN = CC2538_DEV_RLSRAM_ADDR,
                   LENGTH = CC2538_DEV_RLSRAM_SIZE
    FRSRAM (RWX) : ORIGIN = CC2538_DEV_LLSRAM_ADDR,
                   LENGTH = CC2538_DEV_LLSRAM_SIZE
#else
    NRSRAM (RWX) : ORIGIN = CC2538_DEV_RLSRAM_ADDR, LENGTH = 0
    FRSRAM (RWX) : ORIGIN = CC2538_DEV_SRAM_ADDR, LENGTH = CC2538_DEV_SRAM_SIZE
#endif
}

ENTRY(flash_cca_lock_page)
SECTIONS
{
    .text :
    {
        _text = .;
        *(.vectors)
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH_FW= 0

    .socdata (NOLOAD) :
    {
        *(.udma_channel_control_table)
    } > FRSRAM

    .data : ALIGN(4)
    {
        _data = .;
        *(.data*)
        _edata = .;
    } > FRSRAM AT > FLASH_FW
    _ldata = LOADADDR(.data);

    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > FLASH_FW

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > FRSRAM

    .stack (NOLOAD) :
    {
        *(.stack)
    } > FRSRAM

    _heap = .;
    _eheap = ORIGIN(FRSRAM) + LENGTH(FRSRAM);

    .nrdata (NOLOAD) :
    {
        _nrdata = .;
        *(.nrdata*)
        _enrdata = .;
    } > NRSRAM

    .flashcca :
    {
        *(.flashcca)
    } > FLASH_CCA
}
