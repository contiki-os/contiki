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
 *         Driver for the DS18B20 temperature sensor.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 *
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#include "onewire.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DS18B20_SCRATCHPAD_SIZE 9

typedef enum {
  DS18B20_CONVERT_TEMPERATURE = 0x44,
  DS18B20_COPY_SCRATCHPAD = 0x48,
  DS18B20_WRITE_SCRATCHPAD = 0x4E,
  DS18B20_READ_POWERSUPPLY = 0xB4,
  DS18B20_RECALL_E2 = 0xB8,
  DS18B20_READ_SCRATCHPAD = 0xBE
} ds18b20_cmd_t;

void ds18b20_init(void);
void ds18b20_convert_temperature(const ow_rom_code_t id);
uint8_t ds18b20_read_scratchpad(const ow_rom_code_t id, uint8_t *dest);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DS18B20_H_ */
