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
 *	.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef INDEX_H
#define INDEX_H

#include "relation.h"

typedef enum {
  INDEX_NONE = 0,
  INDEX_INLINE = 1,
  INDEX_MEMHASH = 2,
  INDEX_MAXHEAP = 3
} index_type_t;

#define INDEX_READY		0x00
#define INDEX_LOAD_NEEDED	0x01
#define INDEX_LOAD_ERROR	0x02

#define INDEX_API_INTERNAL	0x01
#define INDEX_API_EXTERNAL	0x02
#define INDEX_API_INLINE	0x04
#define INDEX_API_COMPLETE	0x08
#define INDEX_API_RANGE_QUERIES	0x10

struct index_api;

struct index {
  struct index *next;
  char descriptor_file[DB_MAX_FILENAME_LENGTH];
  relation_t *rel;
  attribute_t *attr;
  struct index_api *api;
  void *opaque_data;
  index_type_t type;
  uint8_t flags;
};

typedef struct index index_t;

struct index_iterator {
  index_t *index;
  attribute_value_t min_value;
  attribute_value_t max_value;
  tuple_id_t next_item_no;
  tuple_id_t found_items;
};
typedef struct index_iterator index_iterator_t;

struct index_api {
  index_type_t type;
  uint8_t flags;
  db_result_t (*create)(index_t *);
  db_result_t (*destroy)(index_t *);
  db_result_t (*load)(index_t *);
  db_result_t (*release)(index_t *);
  db_result_t (*insert)(index_t *, attribute_value_t *, tuple_id_t);
  db_result_t (*delete)(index_t *, attribute_value_t *);
  tuple_id_t (*get_next)(index_iterator_t *);
};

typedef struct index_api index_api_t;

extern index_api_t index_inline;
extern index_api_t index_maxheap;
extern index_api_t index_memhash;

void index_init(void);
db_result_t index_create(index_type_t, relation_t *, attribute_t *);
db_result_t index_destroy(index_t *);
db_result_t index_load(relation_t *, attribute_t *);
db_result_t index_release(index_t *);
db_result_t index_insert(index_t *, attribute_value_t *, tuple_id_t);
db_result_t index_delete(index_t *, attribute_value_t *);
db_result_t index_get_iterator(index_iterator_t *, index_t *, 
                               attribute_value_t *, attribute_value_t *);
tuple_id_t index_get_next(index_iterator_t *);
int index_exists(attribute_t *);

#endif /* !INDEX_H */
