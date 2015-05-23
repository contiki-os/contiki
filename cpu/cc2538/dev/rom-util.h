/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-rom-util cc2538 ROM utility function library
 *
 * Driver for the cc2538 ROM utility function library
 * @{
 *
 * \file
 * Header file for the cc2538 ROM utility function library driver
 */
#ifndef ROM_UTIL_H_
#define ROM_UTIL_H_

#include "contiki.h"

#include <stddef.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name ROM utility function library API access table structure
 * @{
 */
struct rom_util_api {
  /** Calculate CRC32 over a given address range */
  uint32_t (*crc32)(uint8_t *data, uint32_t byte_count);

  /** Return the flash size */
  uint32_t (*get_flash_size)(void);

  /** Return the chip ID */
  uint32_t (*get_chip_id)(void);

  /** Erase flash pages */
  int32_t (*page_erase)(uint32_t flash_addr, uint32_t size);

  /** Program the flash */
  int32_t (*program_flash)(uint32_t *ram_data, uint32_t flash_addr,
                           uint32_t byte_count);

  /** Perform a system reset of the SoC */
  void (*reset_device)(void);

  /** Set a memory area to a specified value */
  void *(*memset)(void *s, int c, size_t n);

  /** Copy data from one memory area to another */
  void *(*memcpy)(void *dest, const void *src, size_t n);

  /** Compare two memory areas */
  int (*memcmp)(const void *s1, const void *s2, size_t n);

  /** Move a block of data from one memory area to another */
  void *(*memmove)(void *dest, const void *src, size_t n);
};
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Pointer to the ROM utility function library API table
 * @{
 */
#define ROM_UTIL_API            ((struct rom_util_api *)0x00000048)
/** @} */
/*---------------------------------------------------------------------------*/
/** \name ROM utility function library API accessor macros
 * @{
 */
#define rom_util_crc32(data, byte_count) \
  (ROM_UTIL_API->crc32((data), (byte_count)))
#define rom_util_get_flash_size() \
  (ROM_UTIL_API->get_flash_size())
#define rom_util_get_chip_id() \
  (ROM_UTIL_API->get_chip_id())
#define rom_util_page_erase(flash_addr, size) \
  (ROM_UTIL_API->page_erase((flash_addr), (size)))
#define rom_util_program_flash(ram_data, flash_addr, byte_count) \
  (ROM_UTIL_API->program_flash((ram_data), (flash_addr), (byte_count)))
#define rom_util_reset_device() \
  (ROM_UTIL_API->reset_device())
#define rom_util_memset(s, c, n) \
  (ROM_UTIL_API->memset((s), (c), (n)))
#define rom_util_memcpy(dest, src, n) \
  (ROM_UTIL_API->memcpy((dest), (src), (n)))
#define rom_util_memcmp(s1, s2, n) \
  (ROM_UTIL_API->memcmp((s1), (s2), (n)))
#define rom_util_memmove(dest, src, n) \
  (ROM_UTIL_API->memmove((dest), (src), (n)))
/** @} */

#endif /* ROM_UTIL_H_ */

/**
 * @}
 * @}
 */
