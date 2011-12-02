/*
 * Copyright (c) 2010, Swedish Institute of Computer Science
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
 *	A memory-resident hash map used as a DB index.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <string.h>

#include "lib/memb.h"

#include "db-options.h"
#include "index.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

static db_result_t create(index_t *);
static db_result_t destroy(index_t *);
static db_result_t load(index_t *);
static db_result_t release(index_t *);
static db_result_t insert(index_t *, attribute_value_t *, tuple_id_t);
static db_result_t delete(index_t *, attribute_value_t *);
static tuple_id_t get_next(index_iterator_t *);

index_api_t index_memhash = {
  INDEX_MEMHASH,
  INDEX_API_INTERNAL,
  create,
  destroy,
  load,
  release,
  insert,
  delete,
  get_next
};

struct hash_item {
  tuple_id_t tuple_id;
  attribute_value_t value;
};
typedef struct hash_item hash_item_t;

typedef hash_item_t hash_map_t[DB_MEMHASH_TABLE_SIZE];

MEMB(hash_map_memb, hash_map_t, DB_MEMHASH_INDEX_LIMIT);

static unsigned
calculate_hash(attribute_value_t *value)
{
  unsigned char *cp, *end;
  unsigned hash_value;

  cp = (unsigned char *)value;
  end = cp + sizeof(*value);
  hash_value = 0;

  while(cp < end) {
    hash_value = hash_value * 33 + *cp++;
  }

  return hash_value % DB_MEMHASH_TABLE_SIZE;
}

static db_result_t
create(index_t *index)
{
  int i;
  hash_map_t *hash_map;

  PRINTF("Creating a memory-resident hash map index\n");

  hash_map = memb_alloc(&hash_map_memb);
  if(hash_map == NULL) {
    return DB_ALLOCATION_ERROR;
  }

  for(i = 0; i < DB_MEMHASH_TABLE_SIZE; i++) {
    hash_map[i]->tuple_id = INVALID_TUPLE;
  }

  index->opaque_data = hash_map;

  return DB_OK;
}

static db_result_t
destroy(index_t *index)
{
  memb_free(&hash_map_memb, index->opaque_data);

  return DB_OK;
}

static db_result_t
load(index_t *index)
{
  return create(index);
}

static db_result_t
release(index_t *index)
{
  return destroy(index);
}

static db_result_t
insert(index_t *index, attribute_value_t *value, tuple_id_t tuple_id)
{
  hash_map_t *hash_map;
  uint16_t hash_value;

  hash_map = index->opaque_data;

  hash_value = calculate_hash(value);
  hash_map[hash_value]->tuple_id = tuple_id;
  hash_map[hash_value]->value = *value;

  PRINTF("DB: Inserted value %ld into the hash table\n", VALUE_LONG(value));

  return DB_OK;
}

static db_result_t
delete(index_t *index, attribute_value_t *value)
{
  hash_map_t *hash_map;
  uint16_t hash_value;

  hash_map = index->opaque_data;

  hash_value = calculate_hash(value);
  if(memcmp(&hash_map[hash_value]->value, value, sizeof(*value)) != 0) {
    return DB_INDEX_ERROR;
  }

  hash_map[hash_value]->tuple_id = INVALID_TUPLE;
  return DB_OK;
}

static tuple_id_t
get_next(index_iterator_t *iterator)
{
  hash_map_t *hash_map;
  uint16_t hash_value;

  if(iterator->next_item_no == 1) {
    /* The memhash supports only unique values at the moment. */
    return INVALID_TUPLE;
  }

  hash_map = iterator->index->opaque_data;

  hash_value = calculate_hash(&iterator->min_value);
  if(memcmp(&hash_map[hash_value]->value, &iterator->min_value, sizeof(iterator->min_value)) != 0) {
    return INVALID_TUPLE;
  }

  iterator->next_item_no++;

  PRINTF("DB: Found value %ld in the hash table\n", 
	VALUE_LONG(&iterator->min_value));

  return hash_map[hash_value]->tuple_id;
}
