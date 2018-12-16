/*
 * Copyright (c) 2013, Hasso-Plattner-Institut.
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
 *         An OFB-AES-128-based CSPRNG.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "lib/csprng.h"
#include "lib/aes-128.h"
#include "sys/cc.h"
#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

static struct csprng_seed seed;

/*---------------------------------------------------------------------------*/
/*
 * We use output feedback mode (OFB) for generating cryptographic pseudo-random
 * numbers [RFC 4086]. A potential problem with OFB is that OFB at some point
 * enters a cycle. However, the expected cycle length given a random key and a
 * random starting point is about 2^127 in our instantiation [Davies and Parkin,
 * The Average Cycle Size of The Key Stream in Output  Feedback Encipherment].
 */
void
csprng_rand(uint8_t *result, uint8_t len)
{
  uint16_t pos;

  AES_128.set_key(seed.key);
  for(pos = 0; pos < len; pos += 16) {
    AES_128.encrypt(seed.state);
    memcpy(result + pos, seed.state, MIN(len - pos, 16));
  }
}
/*---------------------------------------------------------------------------*/
void
csprng_init(void)
{
  CSPRNG_SEEDER.generate_seed(&seed);
#if DEBUG
  uint8_t i;

  PRINTF("csprng: seeder = %s\n", CSPRNG_SEEDER.name);
  PRINTF("csprng: key = ");
  for(i = 0; i < CSPRNG_KEY_LEN; i++) {
    PRINTF("%02x", seed.key[i]);
  }
  PRINTF("\n");
  PRINTF("csprng: state = ");
  for(i = 0; i < CSPRNG_STATE_LEN; i++) {
    PRINTF("%02x", seed.state[i]);
  }
  PRINTF("\n");
#endif
}
/*---------------------------------------------------------------------------*/
