/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Flash device driver header file for the Mulle platform.
 *
 * \author
 *         Simon Aittamaa <simon.aittamaa@ltu.se>
 *
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum flash_id {
  FLASH_ID0 = 0x4,
} flash_id_t;

typedef enum flash_metrics {
  FLASH_PAGE_SIZE = 256,
  FLASH_SECTOR_SIZE = 256 * FLASH_PAGE_SIZE,
  FLASH_SECTORS = 32,
  FLASH_SIZE = FLASH_SECTOR_SIZE * FLASH_SECTORS,
} flash_metrics_t;

typedef enum flash_flags {
  FLASH_WAIT = (1 << 0),
  FLASH_FINISH = (1 << 1),
  FLASH_BLOCKING = FLASH_WAIT | FLASH_FINISH,
} flash_flags_t;

typedef enum flash_error {
  E_FLASH_OK = 0,
  E_FLASH_INVALID_STATUS,
  E_FLASH_UNKNOWN,
  E_FLASH_BUSY,
  E_FLASH_QUEUE_FULL,
  E_FLASH_QUEUE_EMPTY,
  E_FLASH_ERASE_FAILED,
  E_FLASH_WRITE_FAILED,
} flash_error_t;

typedef uint32_t flash_addr_t;

flash_error_t flash_init(void);

flash_error_t flash_status(const flash_id_t);

flash_error_t flash_read(const flash_id_t, const flash_addr_t, uint8_t *, const uint32_t, const flash_flags_t);
/* Read data inverted (~data) */
flash_error_t flash_readi(const flash_id_t, const flash_addr_t, uint8_t *, const uint32_t, const flash_flags_t);
flash_error_t flash_write(const flash_id_t, const flash_addr_t, const uint8_t *, const uint32_t, const flash_flags_t);
/* Write data inverted (~data) */
flash_error_t flash_writei(const flash_id_t, const flash_addr_t, const uint8_t *, const uint32_t, const flash_flags_t);

flash_error_t flash_write_queue(const flash_id_t, const flash_addr_t, const uint8_t *, const uint32_t, const flash_flags_t);
flash_error_t flash_write_process(const flash_flags_t);

flash_error_t flash_erase_sector(const flash_id_t, const uint32_t, const flash_flags_t);
flash_error_t flash_erase_chip(const flash_id_t, const flash_flags_t);

flash_error_t flash_sleep(const flash_id_t, const flash_flags_t);
flash_error_t flash_wakeup(const flash_id_t, const flash_flags_t);

/* Indended for debugging purposes _ONLY_  -- Simon */
flash_error_t flash_dump(const flash_id_t, void (*)(const uint8_t));

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !defined(FLASH_H_) */
