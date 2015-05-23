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
 *     MaxHeap - A binary maximum heap index for flash memory.
 *
 *     The idea behind the MaxHeap index is to write entries sequentially 
 *     into small buckets, which are indexed in a binary maximum heap. 
 *     Although sequential writes make the entries unsorted within a 
 *     bucket, the time to load and scan a single bucket is small. The 
 *     sequential write is important for flash memories, which are 
 *     unable to handle multiple rewrites of the same page without doing 
 *     an expensive erase operation between the rewrites.
 *
 *     Each bucket specifies a range (a,b) of values that it accepts. 
 *     Once a bucket fills up, two buckets are created with the ranges 
 *     (a,mean) and (mean+1, b), respectively. The entries from the 
 *     original bucket are then copied into the appropriate new bucket 
 *     before the old bucket gets deleted.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/memb.h"
#include "lib/random.h"

#include "db-options.h"
#include "index.h"
#include "result.h"
#include "storage.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#define BRANCH_FACTOR	2
#define BUCKET_SIZE	128
#define NODE_LIMIT	511
#define NODE_DEPTH      9

#if (1 << NODE_DEPTH) != (NODE_LIMIT + 1)
#error "NODE_DEPTH is set incorrectly."
#endif

#define EMPTY_NODE(node)	((node)->min == 0 && (node)->max == 0)
#define EMPTY_PAIR(pair)	((pair)->key == 0 && (pair)->value == 0)

typedef uint16_t maxheap_key_t;
typedef uint16_t maxheap_value_t;

#define KEY_MIN 0
#define KEY_MAX 65535

struct heap_node {
  maxheap_key_t min;
  maxheap_key_t max;
};
typedef struct heap_node heap_node_t;

struct key_value_pair {
  maxheap_key_t key;
  maxheap_value_t value;
};

struct bucket {
  struct key_value_pair pairs[BUCKET_SIZE];
};
typedef struct bucket bucket_t;

struct heap {
  db_storage_id_t heap_storage;
  db_storage_id_t bucket_storage;
  /* Remember where the next free slot for each bucket is located. */
  uint8_t next_free_slot[NODE_LIMIT];
};
typedef struct heap heap_t;

struct bucket_cache {
  heap_t *heap;
  uint16_t bucket_id;
  bucket_t bucket;
};

/* Keep a cache of buckets read from storage. */
static struct bucket_cache bucket_cache[DB_HEAP_CACHE_LIMIT];
MEMB(heaps, heap_t, DB_HEAP_INDEX_LIMIT);

static struct bucket_cache *get_cache(heap_t *, int);
static struct bucket_cache *get_cache_free(void);
static void invalidate_cache(void);
static maxheap_key_t transform_key(maxheap_key_t);
static int heap_read(heap_t *, int, heap_node_t *);
static int heap_write(heap_t *, int, heap_node_t *);
static int heap_insert(heap_t *, maxheap_key_t, maxheap_key_t);
static int heap_find(heap_t *, maxheap_key_t key, int *iterator);
#if HEAP_DEBUG
static void heap_print(heap_t *);
#endif
static int bucket_read(heap_t *, int, bucket_t *);
static struct bucket_cache *bucket_load(heap_t *, int);
static int bucket_append(heap_t *, int, struct key_value_pair *);
static int bucket_split(heap_t *, int);

static db_result_t create(index_t *);
static db_result_t destroy(index_t *);
static db_result_t load(index_t *);
static db_result_t release(index_t *);
static db_result_t insert(index_t *, attribute_value_t *, tuple_id_t);
static db_result_t delete(index_t *, attribute_value_t *);
static tuple_id_t get_next(index_iterator_t *);

index_api_t index_maxheap = {
  INDEX_MAXHEAP,
  INDEX_API_EXTERNAL,
  create,
  destroy,
  load,
  release,
  insert,
  delete,
  get_next
};

static struct bucket_cache *
get_cache(heap_t *heap, int bucket_id)
{
  int i;

  for(i = 0; i < DB_HEAP_CACHE_LIMIT; i++) {
    if(bucket_cache[i].heap == heap && bucket_cache[i].bucket_id == bucket_id) {
      return &bucket_cache[i];
    }
  }
  return NULL;
}

static struct bucket_cache *
get_cache_free(void)
{
  int i;

  for(i = 0; i < DB_HEAP_CACHE_LIMIT; i++) {
    if(bucket_cache[i].heap == NULL) {
      return &bucket_cache[i];
    }
  }
  return NULL;
}

static void
invalidate_cache(void)
{
  int i;

  for(i = 0; i < DB_HEAP_CACHE_LIMIT; i++) {
    if(bucket_cache[i].heap != NULL) {
      bucket_cache[i].heap = NULL;
      break;
    }
  }
}

static maxheap_key_t
transform_key(maxheap_key_t key)
{
  random_init(key);
  return random_rand();
}

static int
heap_read(heap_t *heap, int bucket_id, heap_node_t *node)
{
  if(DB_ERROR(storage_read(heap->heap_storage, node,
			   DB_MAX_FILENAME_LENGTH + (unsigned long)bucket_id * sizeof(*node), sizeof(*node)))) {
    return 0;
  }

  return 1;
}

static int
heap_write(heap_t *heap, int bucket_id, heap_node_t *node)
{
  if(DB_ERROR(storage_write(heap->heap_storage, node,
			    DB_MAX_FILENAME_LENGTH + (unsigned long)bucket_id * sizeof(*node), sizeof(*node)))) {
    return 0;
  }

  return 1;
}

static int
heap_insert(heap_t *heap, maxheap_key_t min, maxheap_key_t max)
{
  int i;
  heap_node_t node;

  PRINTF("DB: Insert node (%ld,%ld) into the heap\n", (long)min, (long)max);

  if(min > max) {
    return -1;
  }

  for(i = 0; i < NODE_LIMIT;) {
    if(heap_read(heap, i, &node) == 0) {
      PRINTF("DB: Failed to read heap node %d\n", i);
      return -1;
    }

    if(EMPTY_NODE(&node)) {
      node.min = min;
      node.max = max;
      if(heap_write(heap, i, &node) == 0) {
	PRINTF("DB: Failed to write heap node %d\n", i);
	return -1;
      }
      return i;
    } else if(node.min <= min && max <= node.max) {
      i = BRANCH_FACTOR * i + 1;
    } else {
      i++;
    }
  }

  PRINTF("DB: No more nodes available\n");
  return -1;
}

static int
heap_find(heap_t *heap, maxheap_key_t key, int *iterator)
{
  maxheap_key_t hashed_key;
  int i;
  int first_child;
  static heap_node_t node;

  hashed_key = transform_key(key);

  for(i = *iterator; i < NODE_LIMIT;) {
    if(heap_read(heap, i, &node) == 0) {
      break;
    }
    if(EMPTY_NODE(&node)) {
      break;
    } else if(node.min <= hashed_key && hashed_key <= node.max) {
      first_child = BRANCH_FACTOR * i + 1;

      if(first_child >= NODE_LIMIT) {
	break;
      }
      *iterator = first_child;
      return i;
    } else {
      i++;
    }
  }

  return -1;
}

#if HEAP_DEBUG
static void
heap_print(heap_t *heap)
{
  int level_count;
  int branch_count;
  int branch_amount;
  int i, j;
  heap_node_t node;

  level_count = 0;
  branch_count = 0;
  branch_amount = BRANCH_FACTOR;

  for(i = 0;; i++) {
    if(heap_read(heap, i, &node) == 0 || EMPTY_NODE(&node)) {
      break;
    }

    for(j = 0; j < level_count; j++) {
      PRINTF("\t");
    }
    PRINTF("(%ld,%ld)\n", (long)node.min, (long)node.max);
    if(level_count == 0) {
      level_count++;
    } else if(branch_count + 1 == branch_amount) {
      level_count++;
      branch_count = 0;
      branch_amount = branch_amount * BRANCH_FACTOR;
    } else {
      branch_count++;
    }
  }
}
#endif /* HEAP_DEBUG */

static int
bucket_read(heap_t *heap, int bucket_id, bucket_t *bucket)
{
  size_t size;

  if(heap->next_free_slot[bucket_id] == 0) {
    size = BUCKET_SIZE;
  } else {
    size = heap->next_free_slot[bucket_id];
  }

  size *= sizeof(struct key_value_pair);

  if(DB_ERROR(storage_read(heap->bucket_storage, bucket,
                           (unsigned long)bucket_id * sizeof(*bucket), size))) {
    return 0;
  }

  return 1;
}

static struct bucket_cache *
bucket_load(heap_t *heap, int bucket_id)
{
  int i;
  struct bucket_cache *cache;

  cache = get_cache(heap, bucket_id);
  if(cache != NULL) {
    return cache;
  }

  cache = get_cache_free();
  if(cache == NULL) {
    invalidate_cache();
    cache = get_cache_free();
    if(cache == NULL) {
      return NULL;
    }
  }

  if(bucket_read(heap, bucket_id, &cache->bucket) == 0) {
    return NULL;
  }

  cache->heap = heap;
  cache->bucket_id = bucket_id;

  if(heap->next_free_slot[bucket_id] == 0) {
    for(i = 0; i < BUCKET_SIZE; i++) {
      if(EMPTY_PAIR(&cache->bucket.pairs[i])) {
        break;
      }
    }

    heap->next_free_slot[bucket_id] = i;
  }

  PRINTF("DB: Loaded bucket %d, the next free slot is %u\n", bucket_id,
	 (unsigned)heap->next_free_slot[bucket_id]);

  return cache;
}

static int
bucket_append(heap_t *heap, int bucket_id, struct key_value_pair *pair)
{
  unsigned long offset;

  if(heap->next_free_slot[bucket_id] >= BUCKET_SIZE) {
    PRINTF("DB: Invalid write attempt to the full bucket %d\n", bucket_id);
    return 0;
  }

  offset = (unsigned long)bucket_id * sizeof(bucket_t);
  offset += heap->next_free_slot[bucket_id] * sizeof(struct key_value_pair);

  if(DB_ERROR(storage_write(heap->bucket_storage, pair, offset, sizeof(*pair)))) {
    return 0;
  }

  heap->next_free_slot[bucket_id]++;

  return 1;
}

static int
bucket_split(heap_t *heap, int bucket_id)
{
  heap_node_t node;
  maxheap_key_t mean;
  int small_bucket_index;
  int large_bucket_index;

  if(heap_read(heap, bucket_id, &node) == 0) {
    return 0;
  }

  mean = node.min + ((node.max - node.min) / 2);

  PRINTF("DB: Split bucket %d (%ld, %ld) at mean value %ld\n", bucket_id, 
         (long)node.min, (long)node.max, (long)mean);

  small_bucket_index = heap_insert(heap, node.min, mean);
  if(small_bucket_index < 0) {
    return 0;
  }

  large_bucket_index = heap_insert(heap, mean + 1, node.max);
  if(large_bucket_index < 0) {
    /*heap_remove(small_bucket);*/
    return 0;
  }

  return 1;
}

int
insert_item(heap_t *heap, maxheap_key_t key, maxheap_value_t value)
{
  int heap_iterator;
  int bucket_id, last_good_bucket_id;
  struct key_value_pair pair;

  for(heap_iterator = 0, last_good_bucket_id = -1;;) {
    bucket_id = heap_find(heap, key, &heap_iterator);
    if(bucket_id < 0) {
      break;
    }
    last_good_bucket_id = bucket_id;
  }
  bucket_id = last_good_bucket_id;

  if(bucket_id < 0) {
    PRINTF("DB: No bucket for key %ld\n", (long)key);
    return 0;
  }

  pair.key = key;
  pair.value = value;

  if(heap->next_free_slot[bucket_id] == BUCKET_SIZE) {
    PRINTF("DB: Bucket %d is full\n", bucket_id);
    if(bucket_split(heap, bucket_id) == 0) {
      return 0;
    }

    /* Select one of the newly created buckets. */
    bucket_id = heap_find(heap, key, &heap_iterator);
    if(bucket_id < 0) {
      return 0;
    }
  }

  if(bucket_append(heap, bucket_id, &pair) == 0) {
    return 0;
  }

  PRINTF("DB: Inserted key %ld (hash %ld) into the heap at bucket_id %d\n",
	 (long)key, (long)transform_key(key), bucket_id);

  return 1;
}

static db_result_t
create(index_t *index)
{
  char heap_filename[DB_MAX_FILENAME_LENGTH];
  char bucket_filename[DB_MAX_FILENAME_LENGTH];
  char *filename;
  db_result_t result;
  heap_t *heap;

  heap = NULL;
  filename = NULL;
  bucket_filename[0] = '\0';

  /* Generate the heap file, which is the main index file that is
     referenced from the metadata of the relation. */
  filename = storage_generate_file("heap",
				   (unsigned long)NODE_LIMIT * sizeof(heap_node_t));
  if(filename == NULL) {
    PRINTF("DB: Failed to generate a heap file\n");
    return DB_INDEX_ERROR;
  }

  memcpy(index->descriptor_file, filename,
	 sizeof(index->descriptor_file));

  PRINTF("DB: Generated the heap file \"%s\" using %lu bytes of space\n",
	 index->descriptor_file, (unsigned long)NODE_LIMIT * sizeof(heap_node_t));

  index->opaque_data = heap = memb_alloc(&heaps);
  if(heap == NULL) {
    PRINTF("DB: Failed to allocate a heap\n");
    result = DB_ALLOCATION_ERROR;
    goto end;
  }
  heap->heap_storage = -1;
  heap->bucket_storage = -1;

  /* Generate the bucket file, which stores the (key, value) pairs. */
  filename = storage_generate_file("bucket",
				   (unsigned long)NODE_LIMIT * sizeof(bucket_t));
  if(filename == NULL) {
    PRINTF("DB: Failed to generate a bucket file\n");
    result = DB_INDEX_ERROR;
    goto end;
  }
  memcpy(bucket_filename, filename, sizeof(bucket_filename));

  PRINTF("DB: Generated the bucket file \"%s\" using %lu bytes of space\n",
	 bucket_filename, (unsigned long)NODE_LIMIT * sizeof(bucket_t));

  /* Initialize the heap. */
  memset(&heap->next_free_slot, 0, sizeof(heap->next_free_slot));

  heap->heap_storage = storage_open(index->descriptor_file);
  heap->bucket_storage = storage_open(bucket_filename);
  if(heap->heap_storage < 0 || heap->bucket_storage < 0) {
    result = DB_STORAGE_ERROR;
    goto end;
  }

  if(DB_ERROR(storage_write(heap->heap_storage, &bucket_filename, 0,
			    sizeof(bucket_filename)))) {
    result = DB_STORAGE_ERROR;
    goto end;
  }

  if(heap_insert(heap, KEY_MIN, KEY_MAX) < 0) {
    PRINTF("DB: Heap insertion error\n");
    result = DB_INDEX_ERROR;
    goto end;
  }

  PRINTF("DB: Created a heap index\n");
  result = DB_OK;

 end:
  if(result != DB_OK) {
    if(heap != NULL) {
      storage_close(heap->bucket_storage);
      storage_close(heap->heap_storage);
      memb_free(&heaps, heap);
    }
    if(index->descriptor_file[0] != '\0') {
      cfs_remove(heap_filename);
      index->descriptor_file[0] = '\0';
    }
    if(bucket_filename[0] != '\0') {
      cfs_remove(bucket_filename);
    }
  }
  return result;
}

static db_result_t
destroy(index_t *index)
{
  release(index);
  return DB_INDEX_ERROR;
}

static db_result_t
load(index_t *index)
{
  heap_t *heap;
  db_storage_id_t fd;
  char bucket_file[DB_MAX_FILENAME_LENGTH];

  index->opaque_data = heap = memb_alloc(&heaps);
  if(heap == NULL) {
    PRINTF("DB: Failed to allocate a heap\n");
    return DB_ALLOCATION_ERROR;
  }

  fd = storage_open(index->descriptor_file);
  if(fd < 0) {
    return DB_STORAGE_ERROR;
  }

  if(storage_read(fd, bucket_file, 0, sizeof(bucket_file)) !=
     sizeof(bucket_file)) {
    storage_close(fd);
    return DB_STORAGE_ERROR;
  }

  storage_close(fd);

  heap->heap_storage = storage_open(index->descriptor_file);
  heap->bucket_storage = storage_open(bucket_file);

  memset(&heap->next_free_slot, 0, sizeof(heap->next_free_slot));

  PRINTF("DB: Loaded max-heap index from file %s and bucket file %s\n",
	 index->descriptor_file, bucket_file);

  return DB_OK;
}

static db_result_t
release(index_t *index)
{
  heap_t *heap;

  heap = index->opaque_data;

  storage_close(heap->bucket_storage);
  storage_close(heap->heap_storage);
  memb_free(&heaps, index->opaque_data);
  return DB_INDEX_ERROR;
}

static db_result_t
insert(index_t *index, attribute_value_t *key, tuple_id_t value)
{
  heap_t *heap;
  long long_key;

  heap = (heap_t *)index->opaque_data;

  long_key = db_value_to_long(key);

  if(insert_item(heap, (maxheap_key_t)long_key,
		 (maxheap_value_t)value) == 0) {
    PRINTF("DB: Failed to insert key %ld into a max-heap index\n", long_key);
    return DB_INDEX_ERROR;
  }
  return DB_OK;
}

static db_result_t
delete(index_t *index, attribute_value_t *value)
{
  return DB_INDEX_ERROR;
}

static tuple_id_t
get_next(index_iterator_t *iterator)
{
  struct iteration_cache {
    index_iterator_t *index_iterator;
    int heap_iterator;
    tuple_id_t found_items;
    uint8_t start;
    int visited_buckets[NODE_DEPTH];
    int end;
  };
  static struct iteration_cache cache;
  heap_t *heap;
  maxheap_key_t key;
  int bucket_id;
  int tmp_heap_iterator;
  int i;
  struct bucket_cache *bcache;
  uint8_t next_free_slot;

  heap = (heap_t *)iterator->index->opaque_data;
  key = *(maxheap_key_t *)&iterator->min_value;

  if(cache.index_iterator != iterator || iterator->next_item_no == 0) {
    /* Initialize the cache for a new search. */
    cache.end = NODE_DEPTH - 1;
    cache.found_items = cache.start = 0;
    cache.index_iterator = iterator;

    /* Find the downward path through the heap consisting of all nodes
       that could possibly contain the key. */
    for(i = tmp_heap_iterator = 0; i < NODE_DEPTH; i++) {
      cache.visited_buckets[i] = heap_find(heap, key, &tmp_heap_iterator);
      if(cache.visited_buckets[i] < 0) {
        cache.end = i - 1;
	break;
      }
    }
    cache.heap_iterator = cache.end;
  }

  /*
   * Search for the key in each heap node, starting from the bottom
   * of the heap. Because the bottom nodes contain are very narrow 
   * range of keys, there is a much higher chance that the key will be
   * there rather than at the top.
   */
  for(; cache.heap_iterator >= 0; cache.heap_iterator--) {
    bucket_id = cache.visited_buckets[cache.heap_iterator];

    PRINTF("DB: Find key %lu in bucket %d\n", (unsigned long)key, bucket_id);

    if((bcache = bucket_load(heap, bucket_id)) == NULL) {
      PRINTF("DB: Failed to load bucket %d\n", bucket_id);
      return INVALID_TUPLE;
    }

    /* Because keys are stored in an unsorted order in the bucket, we
     * need to search the bucket sequentially. */
    next_free_slot = heap->next_free_slot[bucket_id];
    for(i = cache.start; i < next_free_slot; i++) {
      if(bcache->bucket.pairs[i].key == key) {
        if(cache.found_items++ == iterator->next_item_no) {
	  iterator->next_item_no++;
          cache.start = i + 1;
          PRINTF("DB: Found key %ld with value %lu\n", (long)key,
		 (unsigned long)bcache->bucket.pairs[i].value);
	  return (tuple_id_t)bcache->bucket.pairs[i].value;
        }
      }
    }
  }

  if(VALUE_INT(&iterator->min_value) == VALUE_INT(&iterator->max_value)) {
    PRINTF("DB: Could not find key %ld in the index\n", (long)key);
    return INVALID_TUPLE;
  }

  iterator->next_item_no = 0;
  VALUE_INT(&iterator->min_value)++;

  return get_next(iterator);
}
