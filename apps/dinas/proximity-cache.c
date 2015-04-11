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

#include<stdio.h>
#include<string.h>
#include <stddef.h> /*for size_t*/
#include "proximity-cache.h"
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

CACHEITEM cache[CACHE_SIZE];
int cs = 0; /* number of items in cache */

unsigned int local_similarity(CACHEITEM *ci);
int get_oldest_item();


/*---------------------------------------------------------------------------*/
int proximity_cache_init() {
	/*PRINTF("CACHE_SIZE = %d\n", CACHE_SIZE);*/
	memset(cache, 0, CACHE_SIZE*sizeof(int));
	cs = 0;	
	return 0;
}


/*---------------------------------------------------------------------------*/
int proximity_cache_size() 
{
  return cs;
}


/*---------------------------------------------------------------------------*/
int proximity_cache_add_item(CACHEITEM ci) {
	int i = proximity_cache_check_item(&ci);
	int D = cs*BLOOM_SIZE;
	
	/*PRINTF("add: i = %d \n", i);*/
	/*
	PRINTF("add: D = %d \n", D);
	PRINTF("add: T1*D = %d \n", T1*D);
	PRINTF("cs = %d\n", cs);
	*/
	
	if (cs < CACHE_SIZE) 
	  {
	  	/* A) the cache is not full */
		if (i == CACHE_SIZE) { /* the item is not in cache */
			/*printf("A, i==0 .. \n");*/
			/* compute similitude degree with elements in cache */
			if ((cs == 0) || (local_similarity(&ci) > T1*D)) {
				/*printf("list_add \n");*/
				cache[cs] = ci;
				cs += 1; 
			}
			else
			{
				//PRINTF("rej \n");
			}
		}
		else {
			/*
			PRINTF("A, i>0 .. \n");
			PRINTF("The item is already in the cache, in position %d.\n", i);
			*/
			/* add the item only if the owner is different */
			/*if (strcmp(cache[i].owner_addr,ci.owner_addr) != 0) */
			if (!uip_ipaddr_cmp(&(cache[i].owner_addr), &(ci.owner_addr))) 
			{
				/*PRINTF("The owner is different, let's add the new item.\n");*/
				cache[cs] = ci;
				cs += 1; 
			}
			else 
				cache[i].timestamp = ci.timestamp;
		}
	  }
	else 
	  {
	  	/* B) the cache is full */
		if (i == CACHE_SIZE) { /* the item is not in cache */
			/*PRINTF("B, i==0 .. \n");	*/
			/* compute similitude degree with elements in cache */
			if (local_similarity(&ci) > T2*D) 
			{
				int oldest = get_oldest_item();
				cache[oldest] = ci;
			}
			else 
			{
				/*PRINTF("rej \n");*/
			}
		}
		else {
			/*
			PRINTF("B, i>0 .. \n");
			PRINTF("The item is already in the cache, in position %d.\n", i);
			*/
			/* replace the item only if the owner is different */
			/* if (strcmp(cache[i].owner_addr,ci.owner_addr) != 0) */
			if (!uip_ipaddr_cmp(&(cache[i].owner_addr), &(ci.owner_addr)))
			{
				/*PRINTF("The owner is different, let's replace the old item.\n");*/
				cache[i] = ci;
			}
			else 
				cache[i].timestamp = ci.timestamp;
		}
	  }
	return 0;	
}


/*---------------------------------------------------------------------------*/
int proximity_cache_print() 
{
	int i;
	for (i = 0; i < CACHE_SIZE; ++i) 
	{
    	/*PRINTF("%d : ", i);*/
    	bloom_print(&cache[i].bloomname);
    	/*
    	PRINTF("%d - owner_addr: %s\n", i, cache[i].owner_addr);
    	PRINTF("%d - provider_neighbor_addr: %s\n", i, cache[i].provider_neighbor_addr);
    	PRINTF("%d - timestamp: %d\n", i, (int)cache[i].timestamp);
    	*/
	}    
	return 0;	
}


/*---------------------------------------------------------------------------*/
CACHEITEM* proximity_cache_get_item(int i)
{
	return &cache[i];
}


/*---------------------------------------------------------------------------*/
/* checks is the item's bloomname is already in the cache */
int proximity_cache_check_item(CACHEITEM *ci) {
	int i;
	for (i = 0; i < cs; ++i) {
    	if (bloom_distance(&(cache[i].bloomname), &(ci->bloomname)) == 0)
    		return i;	
    }    
    return CACHE_SIZE;
}


/*---------------------------------------------------------------------------*/
unsigned int local_similarity(CACHEITEM *ci) 
{ 
	int D = cs*BLOOM_SIZE;
	int sum = 0;
	int i;
	unsigned int sim;
	
	for (i = 0; i < cs; ++i) {
    	sum = sum + bloom_distance(&(cache[i].bloomname), &(ci->bloomname));
    }
  
    sim = 100*(D - sum);
    
    /*
    PRINTF("D = %d \n", D);
    PRINTF("sum = %d \n", sum);
    PRINTF("sim = %u \n", sim);
    */
    
	return sim;
}


/*---------------------------------------------------------------------------*/
int get_oldest_item() 
{
	int i;
	int oldest = 0;
	clock_time_t lowest_timestamp = cache[0].timestamp;
	return oldest;
	
	for (i = 0; i < cs; ++i)
	{
		if (cache[i].timestamp < lowest_timestamp) 
		{
			lowest_timestamp = cache[i].timestamp;
			oldest = i;
		}
    }
}


/*---------------------------------------------------------------------------*/
/* avoid the provider, unless it is the only one in cache */
CACHEITEM *proximity_cache_get_most_similar_item(BLOOM* bloomname, uip_ipaddr_t* provider_ipaddr) 
{
  int i;
  int dist = BLOOM_SIZE; 
  int temp_dist;
  CACHEITEM* ci;
  ci = &(cache[0]); 
  for (i = 0; i < cs; i++)
  {
  	if (!uip_ipaddr_cmp(&(cache[i].provider_neighbor_addr), provider_ipaddr)) {
  	  temp_dist = bloom_distance(&(cache[i].bloomname), bloomname);
  	  if (temp_dist < dist) {
  		dist = temp_dist;
  		ci = &(cache[i]);
  	  }
  	}		
  }
  return ci;
}



