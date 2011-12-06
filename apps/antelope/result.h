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
 *	Declarations for the result acquisition API.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef RESULT_H
#define RESULT_H

#include "index.h"
#include "relation.h"
#include "storage.h"

#define RESULT_TUPLE_INVALID(tuple)	((tuple) == NULL)
#define RESULT_TUPLE_SIZE(handle)	(handle).rel->row_length

typedef unsigned char *tuple_t;

#define DB_HANDLE_FLAG_INDEX_STEP	0x01
#define DB_HANDLE_FLAG_SEARCH_INDEX	0x02
#define DB_HANDLE_FLAG_PROCESSING	0x04

struct db_handle {
  index_iterator_t index_iterator;
  tuple_id_t tuple_id;
  tuple_id_t current_row;
  relation_t *rel;
  relation_t *left_rel;
  relation_t *join_rel;
  relation_t *right_rel;
  relation_t *result_rel;
  attribute_t *left_join_attr;
  attribute_t *right_join_attr;
  tuple_t tuple;
  uint8_t flags;
  uint8_t ncolumns;
  void *adt;
};
typedef struct db_handle db_handle_t;

db_result_t db_get_value(attribute_value_t *value,
                         db_handle_t *handle, unsigned col);
db_result_t db_phy_to_value(attribute_value_t *value,
                            attribute_t *attr, unsigned char *ptr);
db_result_t db_value_to_phy(unsigned char *ptr,
                            attribute_t *attr, attribute_value_t *value);
long db_value_to_long(attribute_value_t *value);
db_result_t db_free(db_handle_t *handle);

#endif /* !RESULT_H */
