/* Copyright (c) 2015, Michele Amoretti.
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

/**
 * \file
 *         Bloom filter
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */

#ifndef __BLOOM_H__
#define __BLOOM_H__

#include <limits.h>
#include <stdlib.h>
#include <stddef.h> /*for size_t*/
#include "contiki.h"


#ifdef BLOOM_SIZE_PARAM
#define BLOOM_SIZE BLOOM_SIZE_PARAM
#else
#define BLOOM_SIZE 40
#endif

#ifdef BLOOM_NFUNCS_PARAM
#define BLOOM_NFUNCS BLOOM_NFUNCS_PARAM
#else
#define BLOOM_NFUNCS 7
#endif

#define A_SIZE (BLOOM_SIZE+CHAR_BIT-1)/CHAR_BIT

typedef struct {
	unsigned char a[A_SIZE];
} BLOOM;

BLOOM bloom_create();
unsigned char bloom_count_ones(unsigned char x);
int bloom_add(BLOOM *bloom, const char *s);
int bloom_check(BLOOM *bloom, const char *s);
int bloom_print(BLOOM *bloom);
int bloom_distance(BLOOM *bloom1, BLOOM *bloom2);

#endif

