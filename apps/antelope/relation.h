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

#ifndef RELATION_H
#define RELATION_H

#include <stdint.h>
#include <stdlib.h>

#include "lib/list.h"

#include "attribute.h"
#include "db-options.h"
#include "db-types.h"

typedef uint32_t tuple_id_t;
#define INVALID_TUPLE	(tuple_id_t)-1

typedef enum db_direction {
  DB_MEMORY = 0,
  DB_STORAGE = 1
} db_direction_t;

#define RELATION_HAS_TUPLES(rel) ((rel)->tuple_storage >= 0)

/*
 * A relation consists of a name, a set of domains, a set of indexes,
 * and a set of keys. Each relation must have a primary key.
 */
struct relation {
  struct relation *next;
  LIST_STRUCT(attributes);
  attribute_t *primary_key;
  size_t row_length;
  attribute_id_t attribute_count;
  tuple_id_t cardinality;
  tuple_id_t next_row;
  db_storage_id_t tuple_storage;
  db_direction_t dir;
  uint8_t references;
  char name[RELATION_NAME_LENGTH + 1];
  char tuple_filename[RELATION_NAME_LENGTH + 1];
};

typedef struct relation relation_t;

/* API for relations. */
db_result_t relation_init(void);
db_result_t relation_process_remove(void *);
db_result_t relation_process_select(void *);
db_result_t relation_process_join(void *);
relation_t *relation_load(char *);
db_result_t relation_release(relation_t *);
relation_t *relation_create(char *, db_direction_t);
db_result_t relation_rename(char *, char *);
attribute_t *relation_attribute_add(relation_t *, db_direction_t, char *,
				    domain_t, size_t);
attribute_t *relation_attribute_get(relation_t *, char *);
db_result_t relation_get_value(relation_t *, attribute_t *,
                               unsigned char *, attribute_value_t *);
db_result_t relation_attribute_remove(relation_t *, char *);
db_result_t relation_set_primary_key(relation_t *, char *);
db_result_t relation_remove(char *, int);
db_result_t relation_insert(relation_t *, attribute_value_t *);
db_result_t relation_select(void *, relation_t *, void *);
db_result_t relation_join(void *, void *);
tuple_id_t relation_cardinality(relation_t *);

#endif /* RELATION_H */
