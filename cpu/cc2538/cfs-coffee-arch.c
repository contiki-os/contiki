/*
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
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
/**
 * \addtogroup cc2538-cfs-coffee-arch
 * @{
 *
 * \file
 * Module for the cc2538 Coffee port
 */
#include "contiki-conf.h"
#include "sys/cc.h"
#include "cfs/cfs-coffee.h"
#include "dev/cc2538-dev.h"
#include "dev/rom-util.h"
#include "dev/flash.h"
#include "dev/watchdog.h"
#include "cpu.h"
#include "cfs-coffee-arch.h"

#include <stdint.h>

#ifndef COFFEE_CONF_CUSTOM_PORT
/*---------------------------------------------------------------------------*/
#if !COFFEE_SECTOR_SIZE || COFFEE_SECTOR_SIZE % FLASH_PAGE_SIZE
#error COFFEE_SECTOR_SIZE must be a non-zero multiple of the flash page size
#endif
#if !COFFEE_PAGE_SIZE || COFFEE_SECTOR_SIZE % COFFEE_PAGE_SIZE
#error COFFEE_PAGE_SIZE must be a divisor of COFFEE_SECTOR_SIZE
#endif
#if COFFEE_PAGE_SIZE % FLASH_WORD_SIZE
#error COFFEE_PAGE_SIZE must be a multiple of the flash word size
#endif
#if COFFEE_START % FLASH_PAGE_SIZE
#error COFFEE_START must be aligned with a flash page boundary
#endif
#if COFFEE_SIZE % COFFEE_SECTOR_SIZE
#error COFFEE_SIZE must be a multiple of COFFEE_SECTOR_SIZE
#endif
#if COFFEE_SIZE / COFFEE_PAGE_SIZE > INT16_MAX
#error Too many Coffee pages for coffee_page_t
#endif
#if COFFEE_START < CC2538_DEV_FLASH_ADDR || \
    COFFEE_START + COFFEE_SIZE > FLASH_CCA_ADDR
#error Coffee does not fit in flash
#endif
/*---------------------------------------------------------------------------*/
void
cfs_coffee_arch_erase(uint16_t sector)
{
  watchdog_periodic();
  INTERRUPTS_DISABLE();
  rom_util_page_erase(COFFEE_START + sector * COFFEE_SECTOR_SIZE,
                      COFFEE_SECTOR_SIZE);
  INTERRUPTS_ENABLE();
}
/*---------------------------------------------------------------------------*/
void
cfs_coffee_arch_write(const void *buf, unsigned int size, cfs_offset_t offset)
{
  const uint32_t *src = buf;
  uint32_t flash_addr = COFFEE_START + offset;
  unsigned int align;
  uint32_t word, len;
  uint32_t page_buf[COFFEE_PAGE_SIZE / FLASH_WORD_SIZE];
  unsigned int i;

  if(size && (align = flash_addr & (FLASH_WORD_SIZE - 1))) {
    len = MIN(FLASH_WORD_SIZE - align, size);
    word = ~((*src & ((1 << (len << 3)) - 1)) << (align << 3));
    watchdog_periodic();
    INTERRUPTS_DISABLE();
    rom_util_program_flash(&word, flash_addr & ~(FLASH_WORD_SIZE - 1),
                           FLASH_WORD_SIZE);
    INTERRUPTS_ENABLE();
    *(const uint8_t **)&src += len;
    size -= len;
    flash_addr += len;
  }

  while(size >= FLASH_WORD_SIZE) {
    len = MIN(size & ~(FLASH_WORD_SIZE - 1), COFFEE_PAGE_SIZE);
    for(i = 0; i < len / FLASH_WORD_SIZE; i++) {
      page_buf[i] = ~*src++;
    }
    watchdog_periodic();
    INTERRUPTS_DISABLE();
    rom_util_program_flash(page_buf, flash_addr, len);
    INTERRUPTS_ENABLE();
    size -= len;
    flash_addr += len;
  }

  if(size) {
    word = ~(*src & ((1 << (size << 3)) - 1));
    watchdog_periodic();
    INTERRUPTS_DISABLE();
    rom_util_program_flash(&word, flash_addr, FLASH_WORD_SIZE);
    INTERRUPTS_ENABLE();
  }
}
/*---------------------------------------------------------------------------*/
void
cfs_coffee_arch_read(void *buf, unsigned int size, cfs_offset_t offset)
{
  const uint8_t *src;
  uint8_t *dst;

  watchdog_periodic();
  for(src = (const void *)(COFFEE_START + offset), dst = buf; size; size--) {
    *dst++ = ~*src++;
  }
}

#endif /* COFFEE_CONF_CUSTOM_PORT */

/** @} */
