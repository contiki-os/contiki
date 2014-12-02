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

#include "onewire.h"
#include "ds18b20.h"
#include "stdio.h"
#include "stdbool.h"

/**
 * Initialize the DS18B20 driver.
 *
 * Not much to do here yet.
 */
void
ds18b20_init(void)
{
}
/**
 * Tell a DS18B20 sensor to initiate a temperature conversion.
 *
 * \param id The ROM code of the sensor, 0 for SKIP ROM.
 *
 * \note Parasite power is not supported yet!
 */
void
ds18b20_convert_temperature(const ow_rom_code_t id)
{
  static const ds18b20_cmd_t cmd = DS18B20_CONVERT_TEMPERATURE;

  ow_skip_or_match_rom(id);
  ow_write_bytes((const uint8_t *)&cmd, 1);
  /* Keep reading to see status of the conversion, the response will be 1 for as
   * long as the conversion is in progress, then change to 0. */
  /*uint8_t status = 0;*/
  /*ow_read_bytes(&status, 1);*/
  /*printf("status: %x\n");*/
}
/**
 * Read the scratchpad of a DS18B20 sensor.
 *
 * \param id The ROM code of the sensor, 0 for SKIP ROM.
 * \param dest A destination buffer, must be at least 9 bytes long.
 *
 * \return 0 if the CRC is correct, non-zero otherwise.
 */
uint8_t
ds18b20_read_scratchpad(const ow_rom_code_t id, uint8_t *dest)
{
  static const ds18b20_cmd_t cmd = DS18B20_READ_SCRATCHPAD;
  uint16_t buf;
  int i;

  ow_skip_or_match_rom(id);
  ow_write_bytes((const uint8_t *)&cmd, 1);
  ow_read_bytes(&dest[0], DS18B20_SCRATCHPAD_SIZE);
  printf("Scratchpad: ");
  for(i = 0; i < DS18B20_SCRATCHPAD_SIZE / 2; ++i) {
    buf = (dest[2 * i] << 8) | (dest[2 * i + 1]);
    printf("%x", buf);
  }
  printf("\n");
  printf("CRC: %x (should be %x)\n", dest[8], ow_compute_crc(dest, 8));
  buf = (dest[1] << 8) | dest[0];
  printf("Temp (celsius): %d.%d\n", (buf >> 4), (buf & 0x0f) * 625);
  return ow_compute_crc(dest, DS18B20_SCRATCHPAD_SIZE);
}
