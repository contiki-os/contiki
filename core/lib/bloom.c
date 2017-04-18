/* 
 * Copyright (c) 2015, Michele Amoretti.
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
*/

/** \addtogroup bloom
 * @{ */

/**
 * \file
 *         Implementation of the Bloom filter
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "bloom.h"
#include "lib/crc16.h"
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define SETBIT(a, n) (a[n/CHAR_BIT] |= (1<<(n%CHAR_BIT)))  
#define GETBIT(a, n) (a[n/CHAR_BIT] & (1<<(n%CHAR_BIT)))

const unsigned char oneBits[] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};


/*---------------------------------------------------------------------------*/
BLOOM 
bloom_create() {
  BLOOM bloom;
  int n;

  for(n=0; n < A_SIZE; n++) {
    bloom.a[n] = 0;
  }

return bloom;
}
/*---------------------------------------------------------------------------*/
unsigned char 
bloom_count_ones(unsigned char x)
{
  unsigned char results;
  results = oneBits[x&0x0f];
  results += oneBits[x>>4];
  return results;
}
/*---------------------------------------------------------------------------*/
void 
bloom_add(BLOOM *bloom, const char *s)
{
  size_t n;

  /* using the n hash functions of the Bloom filter */
  for(n = 0; n < BLOOM_NFUNCS; ++n) {
    SETBIT(bloom->a, crc16_data(s, strlen(s), n)%BLOOM_SIZE);
  }
}
/*---------------------------------------------------------------------------*/
int 
bloom_check(BLOOM *bloom, const char *s)
{
  size_t n;

  /* using the n hash functions of the Bloom filter */
  for(n = 0; n < BLOOM_NFUNCS; ++n) {
    if(!(GETBIT(bloom->a, crc16_data( s, strlen(s), n)%BLOOM_SIZE))) 
      return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
void
bloom_print(BLOOM *bloom)
{
  size_t n;

  for(n = 0; n < BLOOM_SIZE; ++n) {
    PRINTF("%s",(GETBIT(bloom->a,n))?"1":"0");
  }
  PRINTF("\n");	
}
/*---------------------------------------------------------------------------*/
int 
bloom_distance(BLOOM *bloom1, BLOOM *bloom2) 
{ 
  int dist;
  int i;

  dist = 0;
  for(i = 0; i < A_SIZE; ++i) {
    dist = dist + bloom_count_ones(bloom1->a[i] ^ bloom2->a[i]);
  }

  return dist;
}
/*---------------------------------------------------------------------------*/

/** @} */

