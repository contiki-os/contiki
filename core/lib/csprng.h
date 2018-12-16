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

#ifndef CSPRNG_H_
#define CSPRNG_H_

#include "contiki.h"

#define CSPRNG_KEY_LEN 16
#define CSPRNG_STATE_LEN 16

#ifdef CSPRNG_CONF_SEEDER
#define CSPRNG_SEEDER CSPRNG_CONF_SEEDER
#else /* CSPRNG_CONF_SEEDER */
#define CSPRNG_SEEDER null_seeder
#endif /* CSPRNG_CONF_SEEDER */

struct csprng_seed {
  uint8_t key[CSPRNG_KEY_LEN];
  uint8_t state[CSPRNG_STATE_LEN];
};

/**
 * The structure of a seeder.
 */
struct csprng_seeder {
  char *name;
  void (* generate_seed)(struct csprng_seed *result);
};

extern const struct csprng_seeder CSPRNG_SEEDER;

/**
 * \brief Initializes this CSPRNG.
 */
void csprng_init(void);

/**
 * \brief Generates a cryptographic random number of length len
 */
void csprng_rand(uint8_t *result, uint8_t len);

#endif /* CSPRNG_H_ */
