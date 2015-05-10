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
 *         Leaky bucket implementation.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#ifndef LEAKY_BUCKET_H_
#define LEAKY_BUCKET_H_

#include "contiki.h"

struct leaky_bucket {
  uint16_t capacity;
  uint16_t leakage_duration;
  uint16_t filling_level;
  unsigned long last_update_timestamp;
};

/**
 * \param lb pointer to the bucket in question
 * \param capacity number of drops that fit into the bucket
 * \param leakage_duration how long it takes until one drop leaks in seconds
 */
void leaky_bucket_init(struct leaky_bucket *lb,
    uint16_t capacity,
    uint16_t leakage_duration);

/**
 * \brief pours a drop in the bucket
 */
void leaky_bucket_pour(struct leaky_bucket *lb);

/**
 * \return whether the bucket is full
 */
int leaky_bucket_is_full(struct leaky_bucket *lb);

#endif /* LEAKY_BUCKET_H_ */
