/*
 * Copyright (c) 2017, Hasso-Plattner-Institut.
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
 *         Mixing CSPRNG seeder.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "cc2538-mix-seeder.h"
#include "cc2538-sram-seeder.h"
#include "lib/iq-seeder.h"

struct csprng_seed last_seed;

/*---------------------------------------------------------------------------*/
static void
xor(struct csprng_seed *result, struct csprng_seed *temp_result)
{
  uint16_t i;

  for(i = 0; i < sizeof(struct csprng_seed); i++) {
    ((uint8_t *)result)[i] ^= ((uint8_t *)temp_result)[i];
  }
}
/*---------------------------------------------------------------------------*/
static void
generate_seed(struct csprng_seed *result)
{
  struct csprng_seed temp_result;

#if LPM_CONF_MAX_PM >= 2
  cc2538_sram_seeder.generate_seed(&temp_result);
  xor(&last_seed, &temp_result);
#endif /* LPM_CONF_MAX_PM >= 2 */

  iq_seeder.generate_seed(&temp_result);
  xor(&last_seed, &temp_result);

  memcpy(result, &last_seed, sizeof(struct csprng_seed));
}
/*---------------------------------------------------------------------------*/
const struct csprng_seeder cc2538_mix_seeder = {
  "cc2538_mix_seeder",
  generate_seed
};
/*---------------------------------------------------------------------------*/
