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
 *	This component forwards index calls using the generic index 
 *      API to specific implementations.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki.h"
#include "lib/memb.h"
#include "lib/list.h"

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include "antelope.h"
#include "attribute.h"
#include "db-options.h"
#include "index.h"
#include "storage.h"

static index_api_t *index_components[] = {&index_inline,
	&index_maxheap};

LIST(indices);
MEMB(index_memb, index_t, DB_INDEX_POOL_SIZE);

static process_event_t load_request_event;
PROCESS(db_indexer, "DB Indexer");

static index_api_t *
find_index_api(index_type_t index_type)
{
  int i;

  for(i = 0; i < sizeof(index_components) / sizeof(index_components[0]); i++) {
      if(index_components[i]->type == index_type) {
	return index_components[i];
      }
  }

  return NULL;
}

void
index_init(void)
{
  list_init(indices);
  memb_init(&index_memb);
  process_start(&db_indexer, NULL);
}

db_result_t
index_create(index_type_t index_type, relation_t *rel, attribute_t *attr)
{
  tuple_id_t cardinality;
  index_t *index;
  index_api_t *api;

  cardinality = relation_cardinality(rel);
  if(cardinality == INVALID_TUPLE) {
    return DB_STORAGE_ERROR;
  }

  if(attr->domain != DOMAIN_INT && attr->domain != DOMAIN_LONG) {
    PRINTF("DB: Cannot create an index for a non-number attribute!\n");
    return DB_INDEX_ERROR;
  }

  api = find_index_api(index_type);
  if(api == NULL) {
    PRINTF("DB: No API for index type %d\n", (int)index_type);
    return DB_INDEX_ERROR;
  }

  if(attr->index != NULL) {
    /* Refuse to overwrite the old index. */
    PRINTF("DB: The attribute %s is already indexed\n", attr->name);
    return DB_INDEX_ERROR;
  }

  index = memb_alloc(&index_memb);
  if(index == NULL) {
    PRINTF("DB: Failed to allocate an index\n");
    return DB_ALLOCATION_ERROR;
  }

  index->rel = rel;
  index->attr = attr;
  index->api = api;
  index->flags = 0;
  index->opaque_data = NULL;
  index->descriptor_file[0] = '\0';
  index->type = index_type;

  if(DB_ERROR(api->create(index))) {
    memb_free(&index_memb, index);
    PRINTF("DB: Index-specific creation failed for attribute %s\n", attr->name);
    return DB_INDEX_ERROR;
  }

  attr->index = index;
  list_push(indices, index);

  if(index->descriptor_file[0] != '\0' &&
     DB_ERROR(storage_put_index(index))) {
    api->destroy(index);
    memb_free(&index_memb, index);
    PRINTF("DB: Failed to store index data in file \"%s\"\n",
           index->descriptor_file);
    return DB_INDEX_ERROR;
  }

  if(!(api->flags & INDEX_API_INLINE) && cardinality > 0) {
    PRINTF("DB: Created an index for an old relation; issuing a load request\n");
    index->flags = INDEX_LOAD_NEEDED;
    process_post(&db_indexer, load_request_event, NULL);
  } else {
    /* Inline indexes (i.e., those using the existing storage of the relation)
       do not need to be reloaded after restarting the system. */
    PRINTF("DB: Index created for attribute %s\n", attr->name);
    index->flags |= INDEX_READY;
  }

  return DB_OK;
}

db_result_t
index_destroy(index_t *index)
{
  if(DB_ERROR(index_release(index)) ||
     DB_ERROR(index->api->destroy(index))) {
    return DB_INDEX_ERROR;
  }

  return DB_OK;
}

db_result_t
index_load(relation_t *rel, attribute_t *attr)
{
  index_t *index;
  index_api_t *api;

  PRINTF("DB: Attempting to load an index over %s.%s\n", rel->name, attr->name);

  index = memb_alloc(&index_memb);
  if(index == NULL) {
    PRINTF("DB: No more index objects available\n");
    return DB_ALLOCATION_ERROR;
  }

  if(DB_ERROR(storage_get_index(index, rel, attr))) {
    PRINTF("DB: Failed load an index descriptor from storage\n");
    memb_free(&index_memb, index);
    return DB_INDEX_ERROR;
  }

  index->rel = rel;
  index->attr = attr;
  index->opaque_data = NULL;

  api = find_index_api(index->type);
  if(api == NULL) {
    PRINTF("DB: No API for index type %d\n", index->type);
    return DB_INDEX_ERROR;
  }

  index->api = api;

  if(DB_ERROR(api->load(index))) {
    PRINTF("DB: Index-specific load failed\n");
    return DB_INDEX_ERROR;
  }

  list_push(indices, index);
  attr->index = index;
  index->flags = INDEX_READY;

  return DB_OK;
}

db_result_t
index_release(index_t *index)
{
  if(DB_ERROR(index->api->release(index))) {
    return DB_INDEX_ERROR;
  }

  index->attr->index = NULL;
  list_remove(indices, index);
  memb_free(&index_memb, index);

  return DB_OK;
}

db_result_t
index_insert(index_t *index, attribute_value_t *value,
             tuple_id_t tuple_id)
{
  return index->api->insert(index, value, tuple_id);
}

db_result_t
index_delete(index_t *index, attribute_value_t *value)
{
  if(index->flags != INDEX_READY) {
    return DB_INDEX_ERROR;
  }

  return index->api->delete(index, value);
}

db_result_t
index_get_iterator(index_iterator_t *iterator, index_t *index, 
                   attribute_value_t *min_value,
                   attribute_value_t *max_value)
{
  tuple_id_t cardinality;
  unsigned long range;
  unsigned long max_range;
  long max;
  long min;

  cardinality = relation_cardinality(index->rel);
  if(cardinality == INVALID_TUPLE) {
    return DB_STORAGE_ERROR;
  }

  if(index->flags != INDEX_READY) {
    return DB_INDEX_ERROR;
  }

  min = db_value_to_long(min_value);
  max = db_value_to_long(max_value);

  range = (unsigned long)max - min;
  if(range > 0) {
    /*
     * Index structures that do not have a natural ability to handle 
     * range queries (e.g., a hash index) can nevertheless emulate them.
     *
     * The range query emulation attempts to look up the key for each 
     * value in the search range. If the search range is sparse, this 
     * iteration will incur a considerable overhead per found key.
     *
     * Hence, the emulation is preferable when an external module wants 
     * to iterate over a narrow range of keys, for which the total 
     * search cost is smaller than that of an iteration over all tuples
     * in the relation.
     */
    if(!(index->api->flags & INDEX_API_RANGE_QUERIES)) {
      PRINTF("DB: Range query requested for an index that does not support it\n");
      max_range = cardinality / DB_INDEX_COST;
      if(range > max_range) {
        return DB_INDEX_ERROR;
      }
      PRINTF("DB: Using the index anyway because the range is small enough (%lu <= %lu)\n",
             range, max_range);
    }
  }

  iterator->index = index;
  iterator->min_value = *min_value;
  iterator->max_value = *max_value;
  iterator->next_item_no = 0;

  PRINTF("DB: Acquired an index iterator for %s.%s over the range (%ld,%ld)\n", 
         index->rel->name, index->attr->name,
         min_value->u.long_value, max_value->u.long_value);

  return DB_OK;
}

tuple_id_t
index_get_next(index_iterator_t *iterator)
{
  long min;
  long max;

  if(iterator->index == NULL) {
    /* This attribute is not indexed. */
    return INVALID_TUPLE;
  }

  if((iterator->index->attr->flags & ATTRIBUTE_FLAG_UNIQUE) &&
     iterator->next_item_no == 1) {
    min = db_value_to_long(&iterator->min_value);
    max = db_value_to_long(&iterator->max_value);
    if(min == max) {
      /*
       * We stop if this is an equivalence search on an attribute
       * whose values are unique, and we already found one item.
       */
      PRINTF("DB: Equivalence search finished\n");
      return INVALID_TUPLE;
    }
  }

  return iterator->index->api->get_next(iterator);
}

int
index_exists(attribute_t *attr)
{
  index_t *index;

  index = (index_t *)attr->index;
  if(index == NULL || index->flags != INDEX_READY) {
    return 0;
  }

  return 1;
}

static index_t *
get_next_index_to_load(void)
{
  index_t *index;

  for(index = list_head(indices); index != NULL; index = index->next) {
    if(index->flags & INDEX_LOAD_NEEDED) {
      return index;
    }
  }

  return NULL;
}

PROCESS_THREAD(db_indexer, ev, data)
{
  static index_t *index;
  static db_handle_t handle;
  static tuple_id_t row;
  db_result_t result;
  attribute_value_t value;
  int column;

  PROCESS_BEGIN();
  load_request_event = process_alloc_event();

  for(;;) {
    PROCESS_WAIT_EVENT_UNTIL(ev == load_request_event);

    index = get_next_index_to_load();
    if(index == NULL) {
      PRINTF("DB: Request to load an index, but no index is set to be loaded\n");
      continue;
    }

    PRINTF("DB: Loading the index for %s.%s...\n",
	index->rel->name, index->attr->name);

    /* Project the values of the indexed attribute from all tuples in 
       the relation, and insert them into the index again. */
    if(DB_ERROR(db_query(&handle, "SELECT %s FROM %s;", index->attr->name, index->rel->name))) {
      index->flags |= INDEX_LOAD_ERROR;
      index->flags &= ~INDEX_LOAD_NEEDED;
      continue;
    }

    for(;; row++) {
      PROCESS_PAUSE();

      result = db_process(&handle);
      if(DB_ERROR(result)) {
        PRINTF("DB: Index loading failed while processing: %s\n",
               db_get_result_message(result));
        index->flags |= INDEX_LOAD_ERROR;
        goto cleanup;
      }
      if(result == DB_FINISHED) {
        break;
      }

      for(column = 0; column < handle.ncolumns; column++) {
        if(DB_ERROR(db_get_value(&value, &handle, column))) {
          index->flags |= INDEX_LOAD_ERROR;
          goto cleanup;
        }

	if(DB_ERROR(index_insert(index, &value, row))) {
	  index->flags |= INDEX_LOAD_ERROR;
	  goto cleanup;
	}
      }
    }

    PRINTF("DB: Loaded %lu rows into the index\n",
	(unsigned long)handle.current_row);

cleanup:
    if(index->flags & INDEX_LOAD_ERROR) {
      PRINTF("DB: Failed to load the index for %s.%s\n",
	index->rel->name, index->attr->name);
    }
    index->flags &= ~INDEX_LOAD_NEEDED;
    index->flags |= INDEX_READY;
    db_free(&handle);
  }


  PROCESS_END();
}
