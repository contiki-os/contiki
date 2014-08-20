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
 *         One wire driver using hardware UART as a bus master.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 *
 * \note
 *         This is heavily based on Maxim document TUTORIAL 214
 *         "Using a UART to Implement a 1-Wire Bus Master"
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>

#define ONEWIRE_UART_BAUD_SLOW 9600
#define ONEWIRE_UART_BAUD_FAST 115200

/* common 1-wire ROM commands */
typedef enum {
  ONEWIRE_CMD_SEARCH_ROM = 0xF0,
  ONEWIRE_CMD_READ_ROM = 0x33,
  ONEWIRE_CMD_MATCH_ROM = 0x55,
  ONEWIRE_CMD_SKIP_ROM = 0xCC,
  ONEWIRE_CMD_ALARM_SEARCH = 0xEC
} ow_rom_cmd_t;

/* Set ONEWIRE_ALWAYS_SKIP_ROM to 1 to always ignore ROM codes and only use the
 * SKIP ROM command when communicating with the bus */
#define ONEWIRE_ALWAYS_SKIP_ROM 0

#define ONEWIRE_ROM_CODE_LENGTH 8
typedef uint64_t ow_rom_code_t;

void ow_init(void);
void ow_reset(void);
void ow_write_bytes(const uint8_t *src, const uint8_t count);
void ow_write_byte(const uint8_t data);
void ow_read_bytes(uint8_t *dest, const uint8_t count);
uint8_t ow_compute_crc(const uint8_t *data, const uint8_t count);
ow_rom_code_t ow_read_rom(void);
void ow_skip_rom(void);
void ow_match_rom(const ow_rom_code_t id);
void ow_skip_or_match_rom(const ow_rom_code_t id);

#endif
