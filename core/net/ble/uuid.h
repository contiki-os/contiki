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
/*---------------------------------------------------------------------------*/
#ifndef UUID_H_
#define UUID_H_
#include <stdint.h>
#include "util.h"
/*---------------------------------------------------------------------------*/
/* Struct for 128 bits uuids */
typedef struct {
  uint8_t data[16];
} uint128_t;

/* Struct for holding differents types of gatt values */
typedef struct {
  enum {
    BT_SIZE8 = 1,
    BT_SIZE16 = 2,
    BT_SIZE32 = 4,
    BT_CHARACTERISTIC = 5,
    BT_SIZE64 = 8,
    BT_SIZEMPU = 12,
    BT_SIZE128 = 16,
    BT_SIZE_STR = 30,
  } type;
  union {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    uint128_t u128;
    char str[30];
  } value;
} bt_size_t;
/*---------------------------------------------------------------------------*/
/* Transform 16 bts uuid to 128 bits */
uint128_t uuid_16_to_128(uint16_t uuid_16);
/* Transform 128 bts uuid to 16 bits */
uint16_t uuid_128_to_16(const uint128_t uuid_128);
/* Compare 128 bits uuids */
uint8_t uuid_128_compare(const uint128_t u1, const uint128_t u2);
/* Compare 16 bits uuids */
uint8_t uuid_16_compare(const uint16_t u1, const uint16_t u2);
/* Swap custom 128 bits uuids */
uint128_t swap128(const uint128_t *input);

#endif /* UUID_H_ */
