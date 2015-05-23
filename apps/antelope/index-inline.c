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
 *	A binary search index for attributes that are constrained to be 
 *      monotonically increasing, which is a rather common pattern for
 *      time series or keys. Since this index has no storage overhead,
 *      it does not wear out the flash memory nor does it occupy any
 *      space. Furthermore, unlike B+-trees, it has a O(1) memory
 *      footprint in relation to the number of data items.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <stdlib.h>
#include <string.h>

#include "index.h"
#include "relation.h"
#include "result.h"
#include "storage.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

struct search_handle {
  index_t *index;
  tuple_id_t start_row;
  tuple_id_t end_row;
};

struct search_handle handle;

static db_result_t null_op(index_t *);
static db_result_t insert(index_t *, attribute_value_t *, tuple_id_t);
static db_result_t delete(index_t *, attribute_value_t *);
static tuple_id_t get_next(index_iterator_t *);

/*
 * The create, destroy, load, release, insert, and delete operations
 * of the index API always succeed because the index does not store
 * items separately from the row file. The four former operations share 
 * the same signature, and are thus implemented by the null_op function 
 * to save space.
 */
index_api_t index_inline = {
  INDEX_INLINE,
  INDEX_API_EXTERNAL | INDEX_API_COMPLETE | INDEX_API_RANGE_QUERIES,
  null_op,
  null_op,
  null_op,
  null_op,
  insert,
  delete,
  get_next
};

static attribute_value_t *
get_value(tuple_id_t *index, relation_t *rel, attribute_t *attr)
{
  unsigned char row[rel->row_length];
  static attribute_value_t value;

  if(DB_ERROR(storage_get_row(rel, index, row))) {
    return NULL;
  }

  if(DB_ERROR(relation_get_value(rel, attr, row, &value))) {
    PRINTF("DB: Unable to retrieve a value from tuple %ld\n", (long)(*index));
    return NULL;
  }

  return &value;
}

static tuple_id_t
binary_search(index_iterator_t *index_iterator,
              attribute_value_t *target_value,
              int exact_match)
{
  relation_t *rel;
  attribute_t *attr;
  attribute_value_t *cmp_value;
  tuple_id_t min;
  tuple_id_t max;
  tuple_id_t center;

  rel = index_iterator->index->rel;
  attr = index_iterator->index->attr;

  max = relation_cardinality(rel);
  if(max == INVALID_TUPLE) {
    return INVALID_TUPLE;
  }
  max--;
  min = 0;

  do {
    center = min + ((max - min) / 2);

    cmp_value = get_value(&center, rel, attr);
    if(cmp_value == NULL) {
      PRINTF("DB: Failed to get the center value, index = %ld\n",
	(long)center);
      return INVALID_TUPLE;
    }

    if(db_value_to_long(target_value) > db_value_to_long(cmp_value)) {
      min = center + 1;
    } else {
      max = center - 1;
    }
  } while(min <= max &&
          db_value_to_long(target_value) != db_value_to_long(cmp_value));

  if(exact_match &&
     db_value_to_long(target_value) != db_value_to_long(cmp_value)) {
    PRINTF("DB: Could not find value %ld in the inline index\n",
           db_value_to_long(target_value));
    return INVALID_TUPLE;
  }

  return center;
}

static tuple_id_t
range_search(index_iterator_t *index_iterator,
             tuple_id_t *start, tuple_id_t *end)
{
  attribute_value_t *low_target;
  attribute_value_t *high_target;
  int exact_match;

  low_target = &index_iterator->min_value;
  high_target = &index_iterator->max_value;

  PRINTF("DB: Search index for value range (%ld, %ld)\n",
    db_value_to_long(low_target), db_value_to_long(high_target));

  exact_match = db_value_to_long(low_target) == db_value_to_long(high_target);

  /* Optimize later so that the other search uses the result
     from the first one. */
  *start = binary_search(index_iterator, low_target, exact_match);
  if(*start == INVALID_TUPLE) {
    return DB_INDEX_ERROR;
  }

  *end = binary_search(index_iterator, high_target, exact_match);
  if(*end == INVALID_TUPLE) {
    return DB_INDEX_ERROR;
  }
  return DB_OK;
}

static db_result_t
null_op(index_t *index)
{
  return DB_OK;
}

static db_result_t
insert(index_t *index, attribute_value_t *value, tuple_id_t tuple_id)
{
  return DB_OK;
}

static db_result_t
delete(index_t *index, attribute_value_t *value)
{
  return DB_OK;
}

static tuple_id_t
get_next(index_iterator_t *iterator)
{
  static tuple_id_t cached_start;
  static tuple_id_t cached_end;

  if(iterator->next_item_no == 0) {
    /*
     * We conduct the actual index search when the caller attempts to 
     * access the first item in the iteration. The first and last tuple 
     * id:s of the result get cached for subsequent iterations.
     */
    if(DB_ERROR(range_search(iterator, &cached_start, &cached_end))) {
      cached_start = 0;
      cached_end = 0;
      return INVALID_TUPLE;
    }
    PRINTF("DB: Cached the tuple range (%ld,%ld)\n", 
           (long)cached_start, (long)cached_end);
    ++iterator->next_item_no;
    return cached_start;
  } else if(cached_start + iterator->next_item_no <= cached_end) {
    return cached_start + iterator->next_item_no++;
  }

  return INVALID_TUPLE;
}
