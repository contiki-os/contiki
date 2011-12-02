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
 *	The storage interface used by the database.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef STORAGE_H
#define STORAGE_H

#include "index.h"
#include "relation.h"

#define TABLE_NAME_SUFFIX       ".row"
#define TABLE_NAME_LENGTH       (RELATION_NAME_LENGTH + \
                                 sizeof(TABLE_NAME_SUFFIX) - 1)

#define INDEX_NAME_SUFFIX       ".idx"
#define INDEX_NAME_LENGTH       (RELATION_NAME_LENGTH + \
                                 sizeof(INDEX_NAME_SUFFIX) - 1)

typedef unsigned char * storage_row_t;

char *storage_generate_file(char *, unsigned long);

db_result_t storage_load(relation_t *);
void storage_unload(relation_t *);

db_result_t storage_get_relation(relation_t *, char *);
db_result_t storage_put_relation(relation_t *);
db_result_t storage_drop_relation(relation_t *, int);
db_result_t storage_rename_relation(char *, char *);

db_result_t storage_put_attribute(relation_t *, attribute_t *);
db_result_t storage_get_index(index_t *, relation_t *, attribute_t *);
db_result_t storage_put_index(index_t *);

db_result_t storage_get_row(relation_t *, tuple_id_t *, storage_row_t);
db_result_t storage_put_row(relation_t *, storage_row_t);
db_result_t storage_get_row_amount(relation_t *, tuple_id_t *);

db_storage_id_t storage_open(const char *);
void storage_close(db_storage_id_t);
db_result_t storage_read(db_storage_id_t, void *, unsigned long, unsigned);
db_result_t storage_write(db_storage_id_t, void *, unsigned long, unsigned);

#endif /* STORAGE_H */
