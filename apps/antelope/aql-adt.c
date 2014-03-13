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
 *      Utilities for building the internal representation of an AQL command.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <string.h>

#include "aql.h"

#define DEBUG   DEBUG_NONE
#include "net/ip/uip-debug.h"

static unsigned char char_buf[DB_MAX_CHAR_SIZE_PER_ROW];
static uint8_t next_free_offset;

static aql_attribute_t *
get_attribute(aql_adt_t *adt, char *name)
{
  int i;

  for(i = 0; i < AQL_ATTRIBUTE_COUNT(adt); i++) {
    if(strcmp(adt->attributes[i].name, name) == 0) {
      return &adt->attributes[i];
    }
  }
  return NULL;
}

static unsigned char *
save_char(unsigned char *ptr, size_t length)
{
  unsigned char *start_ptr;

  if(length + next_free_offset > DB_MAX_CHAR_SIZE_PER_ROW) {
    return NULL;
  }

  start_ptr = char_buf + next_free_offset;
  memcpy(start_ptr, ptr, length);
  next_free_offset += length;

  return start_ptr;
}

void
aql_clear(aql_adt_t *adt)
{
  char_buf[0] = 0;
  next_free_offset = 0;

  adt->optype = AQL_TYPE_NONE;
  adt->relation_count = 0;
  adt->attribute_count = 0;
  adt->value_count = 0;
  adt->flags = 0;
  memset(adt->aggregators, 0, sizeof(adt->aggregators));
}

db_result_t
aql_add_attribute(aql_adt_t *adt, char *name, domain_t domain,
                   unsigned element_size, int processed_only)
{
  aql_attribute_t *attr;

  if(adt->attribute_count == AQL_ATTRIBUTE_LIMIT) {
    return DB_LIMIT_ERROR;
  }

  if(processed_only && get_attribute(adt, name)) {
    /* No need to have multiple instances of attributes that are only 
       used for processing in the PLE. */
    return DB_OK;
  }

  attr = &adt->attributes[adt->attribute_count++];

  if(strlen(name) + 1 > sizeof(attr->name)) {
    return DB_LIMIT_ERROR;
  }

  strcpy(attr->name, name);
  attr->domain = domain;
  attr->element_size = element_size;
  attr->flags = processed_only ? ATTRIBUTE_FLAG_NO_STORE : 0;

  return DB_OK;
}

db_result_t
aql_add_value(aql_adt_t *adt, domain_t domain, void *value_ptr)
{
  attribute_value_t *value;

  if(adt->value_count == AQL_ATTRIBUTE_LIMIT) {
    return DB_LIMIT_ERROR;
  }

  value = &adt->values[adt->value_count++];
  value->domain = domain;

  switch(domain) {
  case DOMAIN_INT:
    VALUE_LONG(value) = *(long *)value_ptr;
    break;
  case DOMAIN_STRING:
    VALUE_STRING(value) = save_char(value_ptr, strlen(value_ptr) + 1);
    if(VALUE_STRING(value) != NULL) {
      break;
    }
  default:
    return DB_TYPE_ERROR;
  }

  return DB_OK;
}
