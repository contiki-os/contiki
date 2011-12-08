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
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *	Definitions and declarations for AQL, the Antelope Query Language.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#ifndef AQL_H
#define AQL_H

#include "db-options.h"
#include "index.h"
#include "relation.h"
#include "result.h"

enum aql_status {
  OK = 2,
  SYNTAX_ERROR = 3,
  INVALID_TOKEN = 9,
  PLE_ERROR = 12
};
typedef enum aql_status aql_status_t;
#define AQL_ERROR(x)     ((x) >= 3)

enum token {
  END = 0,
  LEFT_PAREN = 1,
  RIGHT_PAREN = 2,
  COMMA = 3,
  EQUAL = 4,
  GT = 5,
  LT = 6,
  DOT = 7,
  ADD = 8,
  SUB = 9,
  MUL = 10,
  DIV = 11,
  COMMENT = 12,
  GEQ = 13,
  LEQ = 14,
  NOT_EQUAL = 15,
  ASSIGN = 16,
  OR = 17,
  IS = 18,
  ON = 19,
  IN = 20,
  AND = 21,
  NOT = 22,
  SUM = 23,
  MAX = 24,
  MIN = 25,
  INT = 26,
  INTO = 27,
  FROM = 28,
  MEAN = 29,
  JOIN = 30,
  LONG = 31,
  TYPE = 32,
  WHERE = 33,
  COUNT = 34,
  INDEX = 35,
  INSERT = 36,
  SELECT = 37,
  REMOVE = 38,
  CREATE = 39,
  MEDIAN = 40,
  DOMAIN = 41,
  STRING = 42,
  INLINE = 43,
  PROJECT = 44,
  MAXHEAP = 45,
  MEMHASH = 46,
  RELATION = 47,
  ATTRIBUTE = 48,

  INTEGER_VALUE = 251,
  FLOAT_VALUE = 252,
  STRING_VALUE = 253,
  IDENTIFIER = 254,
  NONE = 255
};

typedef enum token token_t;

typedef char value_t[DB_MAX_ELEMENT_SIZE];

struct lexer {
  const char *input;
  const char *prev_pos;
  token_t *token;
  value_t *value;
};

typedef struct lexer lexer_t;

enum aql_aggregator {
  AQL_NONE = 0,
  AQL_COUNT = 1,
  AQL_SUM = 2,
  AQL_MIN = 3,
  AQL_MAX = 4,
  AQL_MEAN = 5,
  AQL_MEDIAN = 6
};

typedef enum aql_aggregator aql_aggregator_t;

struct aql_attribute {
  domain_t domain;
  uint8_t element_size;
  uint8_t flags;
  char name[ATTRIBUTE_NAME_LENGTH + 1];
};
typedef struct aql_attribute aql_attribute_t;

struct aql_adt {
  char relations[AQL_RELATION_LIMIT][RELATION_NAME_LENGTH + 1];
  aql_attribute_t attributes[AQL_ATTRIBUTE_LIMIT];
  aql_aggregator_t aggregators[AQL_ATTRIBUTE_LIMIT];
  attribute_value_t values[AQL_ATTRIBUTE_LIMIT];
  index_type_t index_type;
  uint8_t relation_count;
  uint8_t attribute_count;
  uint8_t value_count;
  uint8_t optype;
  uint8_t flags;
  void *lvm_instance;
};
typedef struct aql_adt aql_adt_t;

#define AQL_TYPE_NONE           	0
#define AQL_TYPE_SELECT			1
#define AQL_TYPE_INSERT			2
#define AQL_TYPE_UPDATE			3
#define AQL_TYPE_DROP			4
#define AQL_TYPE_DELETE			5
#define AQL_TYPE_RENAME			6
#define AQL_TYPE_CREATE_ATTRIBUTE	7
#define AQL_TYPE_CREATE_INDEX		8
#define AQL_TYPE_CREATE_RELATION	9
#define AQL_TYPE_REMOVE_ATTRIBUTE	10
#define AQL_TYPE_REMOVE_INDEX		11
#define AQL_TYPE_REMOVE_RELATION	12
#define AQL_TYPE_REMOVE_TUPLES		13
#define AQL_TYPE_JOIN			14

#define AQL_FLAG_AGGREGATE		1
#define AQL_FLAG_ASSIGN			2
#define AQL_FLAG_INVERSE_LOGIC		4

#define AQL_CLEAR(adt)			aql_clear(adt)
#define AQL_SET_TYPE(adt, type)	(((adt))->optype = (type))
#define AQL_GET_TYPE(adt)		((adt)->optype)
#define AQL_SET_INDEX_TYPE(adt, type)	((adt)->index_type = (type))
#define AQL_GET_INDEX_TYPE(adt)	((adt)->index_type)

#define AQL_SET_FLAG(adt, flag)	(((adt)->flags) |= (flag))
#define AQL_GET_FLAGS(adt)		((adt)->flags)
#define AQL_ADD_RELATION(adt, rel)					\
  strcpy((adt)->relations[(adt)->relation_count++], (rel))
#define AQL_RELATION_COUNT(adt)	((adt)->relation_count)
#define AQL_ADD_ATTRIBUTE(adt, attr, dom, size)			\
    aql_add_attribute(adt, attr, dom, size, 0)
#define AQL_ADD_PROCESSING_ATTRIBUTE(adt, attr)			\
    aql_add_attribute((adt), (attr), DOMAIN_UNSPECIFIED, 0, 1)
#define AQL_ADD_AGGREGATE(adt, function, attr)				\
  do {									\
    (adt)->aggregators[(adt)->attribute_count] = (function);		\
    aql_add_attribute((adt), (attr), DOMAIN_UNSPECIFIED, 0, 0);	\
  } while(0)  
#define AQL_ATTRIBUTE_COUNT(adt)	((adt)->attribute_count)
#define AQL_SET_CONDITION(adt, cond)	((adt)->lvm_instance = (cond))
#define AQL_ADD_VALUE(adt, domain, value)				\
    aql_add_value((adt), (domain), (value))

int lexer_start(lexer_t *, char *, token_t *, value_t *);
int lexer_next(lexer_t *);
void lexer_rewind(lexer_t *);

void aql_clear(aql_adt_t *adt);
aql_status_t aql_parse(aql_adt_t *adt, char *query_string);
db_result_t aql_add_attribute(aql_adt_t *adt, char *name,
                               domain_t domain, unsigned element_size,
                               int processed_only);
db_result_t aql_add_value(aql_adt_t *adt, domain_t domain, void *value);
db_result_t db_query(db_handle_t *handle, const char *format, ...);
db_result_t db_process(db_handle_t *handle);

#endif /* !AQL_H */
