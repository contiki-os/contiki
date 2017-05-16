/*
 * Copyright (c) 2017, Arthur Courtel
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
 *
 * Author: Arthur Courtel <arthurcourtel@gmail.com>
 *
 */

#include "uuid.h"
#include <string.h>
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define BASE_UUID16_OFFSET  2

/* Base BLE UUID */
static uint128_t bluetooth_base_uuid = {
  .data = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
            0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
};

inline uint128_t
uuid_16_to_128(uint16_t uuid_16)
{
  uint128_t result;
  /* Set base uuid */
  result = bluetooth_base_uuid;
  uuid_16 = swap16(uuid_16);

  memcpy(&result.data[BASE_UUID16_OFFSET], &uuid_16, sizeof(uuid_16));

  return result;
}
inline uint16_t
uuid_128_to_16(const uint128_t uuid_128)
{
  uint16_t result;

  memcpy(&result, &uuid_128.data[BASE_UUID16_OFFSET], sizeof(result));

  result = swap16(result);
  return result;
}
inline uint8_t
uuid_128_compare(const uint128_t u1, const uint128_t u2)
{
  for(uint8_t i = 0; i < sizeof(uint128_t); i++) {
    if(u1.data[i] != u2.data[i]) {
      return 0;
    }
  }
  return 1;
}
inline uint8_t
uuid_16_compare(const uint16_t u1, const uint16_t u2)
{
  if(u1 != u2) {
    return 0;
  }

  return 1;
}
inline uint128_t
swap128(const uint128_t *input)
{
  uint128_t output;
  for(uint8_t i = 0; i < sizeof(uint128_t); i++) {
    output.data[i] = input->data[sizeof(uint128_t) - i - 1];
  }
  return output;
}
