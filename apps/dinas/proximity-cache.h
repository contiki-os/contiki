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
 *         Proximity Cache
 * \author
 *         Michele Amoretti <michele.amoretti@unipr.it> 
 */

#ifndef __PROXIMITY_CACHE_H__
#define __PROXIMITY_CACHE_H__

#include <stdlib.h>
#include <stddef.h> /*for size_t*/
#include "sys/clock.h"
#include "contiki.h"
#include "net/ip/uip.h"
#include "bloom.h"


#ifdef CACHE_SIZE_PARAM
#define CACHE_SIZE CACHE_SIZE_PARAM
#else 
#define CACHE_SIZE 3
#endif

#ifdef T1_PARAM
#define T1 T1_PARAM
#else 
#define T1 30 /* this is a percentage; also T2 */
#endif

#ifdef T2_PARAM
#define T2 T2_PARAM
#else 
#define T2 90 /* T2 > T1 always! */
#endif



#define SHORT_ADDR_SIZE 19

typedef struct cache_item {
	BLOOM bloomname;
	uip_ipaddr_t owner_addr;
	uip_ipaddr_t provider_neighbor_addr;
	clock_time_t timestamp;
} CACHEITEM;

int proximity_cache_init();
int proximity_cache_size();
int proximity_cache_add_item(CACHEITEM ci);
int proximity_cache_print();
int proximity_cache_check_item(CACHEITEM* ci);
CACHEITEM* proximity_cache_get_item(int i);
CACHEITEM* proximity_cache_get_most_similar_item(BLOOM* bloomname, uip_ipaddr_t* provider_ipaddr);

#endif
