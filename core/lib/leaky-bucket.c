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
 *         Leaky Bucket Counter (LBC)
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "lib/leaky-bucket.h"
#include "sys/clock.h"
#include "sys/cc.h"

/*---------------------------------------------------------------------------*/
static void
update_filling_level(struct leaky_bucket *lb)
{
  uint16_t leaked_drops;

  leaked_drops = (clock_seconds() - lb->last_update_timestamp)
      / lb->leakage_duration;
  lb->last_update_timestamp += leaked_drops * lb->leakage_duration;

  if(leaked_drops >= lb->filling_level) {
    lb->filling_level = 0;
  } else {
    lb->filling_level -= leaked_drops;
  }
}
/*---------------------------------------------------------------------------*/
void
leaky_bucket_init(struct leaky_bucket *lb,
    uint16_t capacity,
    uint16_t leakage_duration)
{
  lb->capacity = capacity;
  lb->leakage_duration = leakage_duration;
  lb->filling_level = 0;
  lb->last_update_timestamp = clock_seconds();
}
/*---------------------------------------------------------------------------*/
void
leaky_bucket_pour(struct leaky_bucket *lb)
{
  update_filling_level(lb);
  lb->filling_level = MIN(lb->filling_level + 1, lb->capacity);
}
/*---------------------------------------------------------------------------*/
int
leaky_bucket_is_full(struct leaky_bucket *lb)
{
  update_filling_level(lb);
  return lb->filling_level == lb->capacity;
}
/*---------------------------------------------------------------------------*/
