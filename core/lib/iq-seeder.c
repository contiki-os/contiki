/*
 * Copyright (c) 2015, Hasso-Plattner-Institut.
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         I/Q LSBs-based seeder.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "lib/iq-seeder.h"
#include "net/netstack.h"
#include "lib/aes-128.h"
#include <string.h>

#ifdef IQ_SEEDER_CONF_SAMPLING_RATE
#define SAMPLING_RATE IQ_SEEDER_CONF_SAMPLING_RATE
#else /* IQ_SEEDER_CONF_SAMPLING_RATE */
#define SAMPLING_RATE 200 /* Hz */
#endif /* IQ_SEEDER_CONF_SAMPLING_RATE */

#define COLUMN_COUNT  50
#define ROW_COUNT     16

static const uint8_t toeplitz[COLUMN_COUNT + ROW_COUNT - 1] =
    {  93 ,  50 , 210 , 134 ,  79 ,  52 , 237 , 192 ,  40 , 201 ,
        3 , 184 , 152 ,  74 ,  27 ,  28 ,  32 , 111 ,  79 , 222 ,
      174 ,  51 , 223 ,  66 , 152 , 211 , 234 , 124 ,  92 ,  64 ,
      206 , 169 , 227 , 155 , 106 ,  87 , 207 , 135 , 238 , 101 ,
      254 , 163 ,  55 ,  76 ,  50 ,  40 ,   4 , 149 ,  27 ,   1 ,
      127 , 159 , 160 ,  91 , 251 , 179 , 186 , 200 , 225 ,  47 ,
      235 , 223 ,  39 , 117 ,  19 };

/*---------------------------------------------------------------------------*/
static uint8_t
get_toeplitz_element(uint8_t row, uint8_t column)
{
  uint8_t min;

  min = row < column ? row : column;
  row -= min;
  column -= min;

  return toeplitz[row ? COLUMN_COUNT - 1 + row : column];
}
/*---------------------------------------------------------------------------*/
/* from http://en.wikipedia.org/wiki/Finite_field_arithmetic */
static uint8_t
mul_gf_256(uint8_t a, uint8_t b)
{
  uint8_t p;
  uint8_t i;

  p = 0;
  for(i = 0; i < 8; i++) {
    if(b & 1) {
      p ^= a;
      a <<= 1;
      if(a & 0x100) {
        a ^= 0x11b;
      }
      b >>= 1;
    }
  }
  return p;
}
/*---------------------------------------------------------------------------*/
static void
extract(uint8_t *target, uint8_t *source)
{
  uint8_t row;
  uint8_t column;

  /**
   * Toeplitz matrix-based extractor. For theory, see "True Random Number
   * Generators Secure in a Changing Environment: Improved Security Bounds"
   * by Skorski.
   */
  for(row = 0; row < ROW_COUNT; row++) {
    target[row] = 0;
    for(column = 0; column < COLUMN_COUNT; column++) {
      target[row] ^= mul_gf_256(get_toeplitz_element(row, column), source[column]);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
seed_16_bytes(uint8_t *result)
{
  uint8_t bit_pos;
  uint8_t byte_pos;
  uint16_t iq_count;
  uint8_t accumulator[COLUMN_COUNT];
  radio_value_t iq;

  bit_pos = 0;
  byte_pos = 0;
  memset(accumulator, 0, COLUMN_COUNT);

  NETSTACK_RADIO.on();
  for(iq_count = 0; iq_count <= (COLUMN_COUNT * 8 / 2); iq_count++) {
    NETSTACK_RADIO.get_value(RADIO_PARAM_IQ_LSBS, &iq);

    if(!iq_count) {
      /* drop first I/Q reading because it is always 0 for some reason */
      continue;
    }

    /* append I/Q LSBs to accumulator */
    accumulator[byte_pos] |= iq << bit_pos;
    bit_pos += 2;
    if(bit_pos == 8) {
      bit_pos = 0;
      byte_pos++;
    }
  }
  NETSTACK_RADIO.off();
  extract(result, accumulator);
}
/*---------------------------------------------------------------------------*/
static void
generate_seed(struct csprng_seed *result)
{
  seed_16_bytes(result->key);
  seed_16_bytes(result->state);
}
/*---------------------------------------------------------------------------*/
const struct csprng_seeder iq_seeder = {
  "iq_seeder",
  generate_seed
};
/*---------------------------------------------------------------------------*/
