/**
 * \addtogroup mbxxx-platform
 *
 * @{
 */

/*
 * Copyright (c) 2010, STMicroelectronics.
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
 */

/**
 * \file
 *  Coffee architecture-dependent header for the STM32W108-based mb851
 *  platform. STM32W108 has 128KB of program flash.
 * \author
 *  Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 */

#ifndef CFS_COFFEE_ARCH_H
#define CFS_COFFEE_ARCH_H

#include "contiki-conf.h"

#include "hal/error.h"
#include "hal/micro/cortexm3/flash.h"

/* 
 * STM32W108 has 128KB of program flash in 128 pages of 1024 bytes each = 128KB.
 * The smallest erasable unit is one page and the smallest writable
 * unit is an aligned 16-bit half-word.
 */
/* Byte page size, starting address on page boundary, and size of file system */
#define FLASH_START               0x8000000
/* Minimum erasable unit. */
#define FLASH_PAGE_SIZE           1024
/* Last 3 pages reserved for NVM. */
#define FLASH_PAGES               125

/* Minimum reservation unit for Coffee. It can be changed by the user.  */
#define COFFEE_PAGE_SIZE          (FLASH_PAGE_SIZE/4)

/*
 * If using IAR, COFFEE_ADDRESS reflects the static value in the linker script
 * iar-cfg-coffee.icf, so it can't be passed as a parameter for Make.
 */
#ifdef __ICCARM__
#define COFFEE_ADDRESS            0x8010c00
#endif
#if (COFFEE_ADDRESS & 0x3FF) != 0
#error "COFFEE_ADDRESS not aligned to a 1024-bytes page boundary."
#endif

#define COFFEE_PAGES              ((FLASH_PAGES*FLASH_PAGE_SIZE-               \
                 (COFFEE_ADDRESS-FLASH_START))/COFFEE_PAGE_SIZE)
#define COFFEE_START              (COFFEE_ADDRESS & ~(COFFEE_PAGE_SIZE-1))
#define COFFEE_SIZE               (COFFEE_PAGES*COFFEE_PAGE_SIZE)

/* These must agree with the parameters passed to makefsdata */
#define COFFEE_SECTOR_SIZE        FLASH_PAGE_SIZE
#define COFFEE_NAME_LENGTH        20

/* These are used internally by the AVR flash read routines */
/* Word reads are faster but take 130 bytes more PROGMEM */
 /* #define FLASH_WORD_READS          1 */
/*
 * 1 = Slower reads, but no page writes after erase and 18 bytes less PROGMEM. 
 * Best for dynamic file system
 */
 /* #define FLASH_COMPLEMENT_DATA     0 */

/* These are used internally by the coffee file system */
#define COFFEE_MAX_OPEN_FILES     4
#define COFFEE_FD_SET_SIZE        8
#define COFFEE_DYN_SIZE           (COFFEE_PAGE_SIZE*1)
/* Micro logs are not needed for single page sectors */
#define COFFEE_MICRO_LOGS         0
#define COFFEE_LOG_TABLE_LIMIT    16    /* It doesnt' matter as */
#define COFFEE_LOG_SIZE           128   /* COFFEE_MICRO_LOGS is 0. */

#if COFFEE_PAGES <= 127
#define coffee_page_t int8_t
#elif COFFEE_PAGES <= 0x7FFF
#define coffee_page_t int16_t
#endif

#define COFFEE_WRITE(buf, size, offset) \
        stm32w_flash_write(COFFEE_START + offset, buf, size)

#define COFFEE_READ(buf, size, offset) \
        stm32w_flash_read(COFFEE_START + offset, buf, size)

#define COFFEE_ERASE(sector) \
        stm32w_flash_erase(sector)


void stm32w_flash_read(uint32_t address, void *data, uint32_t length);

void stm32w_flash_write(uint32_t address, const void *data, uint32_t length);

void stm32w_flash_erase(uint8_t sector);

int coffee_file_test(void);

#endif /* !COFFEE_ARCH_H */
/** @} */
