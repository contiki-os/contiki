/*
 * Copyright (c) 2010, Vrije Universiteit Brussel
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
 *
 * Author: Joris Borms <joris.borms@vub.ac.be>
 *
 */
#ifndef NEIGHBORATTR_H_
#define NEIGHBORATTR_H_

#include "net/rime.h"

/**
 * define how many neighbors you can store
 */
#ifdef NEIGHBOR_CONF_MAX_NEIGHBORS
#define NEIGHBOR_ATTR_MAX_NEIGHBORS NEIGHBOR_CONF_MAX_NEIGHBORS
#else                           /* NEIGHBOR_CONF_MAX_NEIGHBORS */
#define NEIGHBOR_ATTR_MAX_NEIGHBORS 12
#endif                          /* NEIGHBOR_CONF_MAX_NEIGHBORS */

/**
 * \brief      properties of a single neighbor
 */
struct neighbor_addr {
  struct neighbor_addr *next;
  rimeaddr_t addr;
  uint16_t time;
  uint16_t index;
};

/**
 * \brief      properties that define a neighbor attribute
 */
struct neighbor_attr {
  struct neighbor_attr *next;
  uint16_t size;
  void *default_value;
  void *data;
};

/**
 * \brief      Define space for additional parameters in neighbor table entries.
 * \param type The type of the attribute.
 * \param name The name of the attribute.
 * \param def  A ptr to the default value for this attribute. If NULL, attribute will
 *             be filled with zeroes by default.
 *
 *             The attribute 'name' should be registered with 'neighbor_attr_register'
 *             during initialization.
 */
#define NEIGHBOR_ATTRIBUTE(type, name, default_value_ptr) \
  static type _##name##_mem[NEIGHBOR_ATTR_MAX_NEIGHBORS]; \
  static struct neighbor_attr name = \
    {NULL, sizeof(type), default_value_ptr, (void *)_##name##_mem}

/** Same as NEIGHBOR_ATTRIBUTE, only the attr is not declared static
 * this way you can say <tt>extern struct neighbor_attr name</tt> in header to declare
 * a global neighbor attribute
 */
#define NEIGHBOR_ATTRIBUTE_GLOBAL(type, name, default_value_ptr) \
	  static type _##name##_mem[NEIGHBOR_ATTR_MAX_NEIGHBORS]; \
	  struct neighbor_attr name = \
	    {NULL, sizeof(type), default_value_ptr, (void *)_##name##_mem}

#define NEIGHBOR_ATTRIBUTE_DECLARE(name) extern struct neighbor_attr name

/**
 * \brief      register a neighbor attribute
 * \retval     non-zero if successful, zero if not
 */
int neighbor_attr_register(struct neighbor_attr *);

/**
 * \retval     head of neighbor list, useful for iterating over all neighbors
 */
struct neighbor_addr *neighbor_attr_list_neighbors(void);

/**
 * \brief      Check if a neighbor is already added to the neighbor table
 * \retval     non-zero if present, zero if not
 */
int neighbor_attr_has_neighbor(const rimeaddr_t *addr);

/**
 * \brief      Add a neighbor entry to neighbor table
 * \retval     -1 if unsuccessful, 0 if the neighbor was already
 *             in the table, and 1 if successful
 */
int neighbor_attr_add_neighbor(const rimeaddr_t *addr);

/**
 * \brief      Remove a neighbor entry to neighbor table
 * \retval     -1 if unsuccessful, 0 if the neighbor was removed
 */
int neighbor_attr_remove_neighbor(const rimeaddr_t *addr);

/**
 * \brief      Get pointer to neighbor table data specified by id
 * \param      requested attribute
 * \param addr requested neighbor
 * \retval     pointer to data, NULL if neighbor was not found
 *
 *             Searches neighbor table for addr and returns pointer to data section
 *             specified by attribute type and addr.
 *             This pointer should not be saved, as it may point to data from another
 *             neighbor in the future if neighbors get removed/added over time.
 */
void *neighbor_attr_get_data(struct neighbor_attr *, const rimeaddr_t *addr);

/**
 * \brief      Copy data to neighbor table
 * \retval     non-zero if successful, zero if not
 *
 *             Copies data to specific part of the neighbor table, specified by
 *             neighbor and attribute type, and resets timeout for that neighbor.
 *             If neighbor was not found, this will add a new neighbor to the table.
 */
int neighbor_attr_set_data(struct neighbor_attr *, const rimeaddr_t *addr,
                           void *data);

/**
 * \brief      Set global lifetime of neighbor entries.
 * \param      Lifetime in seconds. If 0, entries will not time out
 */
void neighbor_attr_set_timeout(uint16_t);

/**
 * \brief      get global lifetime of neighbor entries. If 0, entries will not time out
 */
uint16_t neighbor_attr_get_timeout(void);

/**
 * \brief      reset timeout of a neighbor to prevent it from being removed
 */
void neighbor_attr_tick(const rimeaddr_t *);

#endif                          /* NEIGHBORATTR_H_ */
