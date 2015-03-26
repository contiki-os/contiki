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
 *         Insecure default seeder.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "lib/null-seeder.h"
#include "lib/random.h"

/*---------------------------------------------------------------------------*/
static void
seed_16_bytes(uint8_t *result)
{
  uint8_t i;
  uint16_t r;

  for(i = 0; i < 16; i += 2) {
    r = random_rand();
    result[i] = (r >> 8) & 0xFF;
    result[i + 1] = r & 0xFF;
  }
}
/*---------------------------------------------------------------------------*/
static void
generate_seed(struct csprng_seed *result)
{
  seed_16_bytes(result->key);
  seed_16_bytes(result->state);
}
/*---------------------------------------------------------------------------*/
const struct csprng_seeder null_seeder = {
  "null_seeder",
  generate_seed
};
/*---------------------------------------------------------------------------*/
