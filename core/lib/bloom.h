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

/**
 * \file
 *         Header file for the Bloom filter
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */
 
 /** \addtogroup lib
 * @{ */

/**
 * \defgroup bloom Bloom filter
 *
 * A Bloom filter is a space-efficient probabilistic data structure 
 * conceived by Burton Howard Bloom in 1970, which is used to test 
 * whether an element is a member of a set. False positive matches are possible, 
 * but false negatives are not, thus a Bloom filter has a 100% recall rate.
 *
 * @{
 */

#ifndef __BLOOM_H__
#define __BLOOM_H__

#include <limits.h>
#include <stdlib.h>
#include <stddef.h> /*for size_t*/
#include "contiki.h"


#ifdef BLOOM_SIZE_PARAM
#define BLOOM_SIZE BLOOM_SIZE_PARAM
#else /* BLOOM_SIZE_PARAM */
#define BLOOM_SIZE 40
#endif /* BLOOM_SIZE_PARAM */

#ifdef BLOOM_NFUNCS_PARAM
#define BLOOM_NFUNCS BLOOM_NFUNCS_PARAM
#else /* BLOOM_NFUNCS_PARAM */
#define BLOOM_NFUNCS 7
#endif /* BLOOM_NFUNCS_PARAM */

#define A_SIZE (BLOOM_SIZE+CHAR_BIT-1)/CHAR_BIT

/**
 * Structure of Bloom filters.
 */
typedef struct {
	unsigned char a[A_SIZE];
} BLOOM;


/**
 * \brief      Create and initialize a Bloom filter.
 * \return     The Bloom filter.
 *
 *             This function creates an instance of Bloom filter
 *             and sets all its elements to 0.
 *
 */
BLOOM bloom_create();

/**
 * \brief      Counts how many bits are set to 1 in the passed parameter.
 * \param x    The parameter to be checked.
 * \return     The number of bits that are set to 1.
 *
 *             This function gets a small data chunk as input
 *             and counts how many of its bits are set to 1.
 *
 */
unsigned char bloom_count_ones(unsigned char x);

/**
 * \brief      Add a keyword to a Bloom filter.
 * \param bloom    The target Bloom filter.
 * \param s    The keyword to be added.
 *
 *             This function adds a keyword to a Bloom filter,
 *             by setting to 1 BLOOM_NFUNCS of its bits, 
 *             namely those resulting from the application of BLOOM_NFUNCS  
 *             different CRC16 hash functions to the keyword.
 *
 */
void bloom_add(BLOOM *bloom, const char *s);

/**
 * \brief      Check a keyword against a Bloom filter.
 * \param bloom    The target Bloom filter.
 * \param s    The keyword to be checked.
 * \return     The result of the performed check.
 *
 *             This function returns 0 if the keyword is not in the Bloom filter,
 *             otherwise it returns 1 if the keyword may be in the Bloom filter.
 *
 */
int bloom_check(BLOOM *bloom, const char *s);

/**
 * \brief      Prints a Bloom filter to stdout.
 * \param bloom    The target Bloom filter.
 *
 *             This function prints the Bloom filter as a string of 0/1 characters.
 *
 */
void bloom_print(BLOOM *bloom);

/**
 * \brief      Computes the distance between two Bloom filters.
 * \param bloom1    The first Bloom filter.
 * \param bloom2    The second Bloom filter.
 * \return     The distance between the two Bloom filters.
 *
 *             This function performs a bitwise XOR of the two Bloom filters
 *             and returns the number of resulting bits that are set to 1.
 *
 */
int bloom_distance(BLOOM *bloom1, BLOOM *bloom2);

#endif /* BLOOM_H_ */

/** @} */
/** @} */
