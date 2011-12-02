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
 *	Result acquisition interface for AQL queries.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <string.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#include "result.h"
#include "storage.h"

/* db_get_value: Retrieve the value of the specified attribute in
   the current tuple. */
db_result_t
db_get_value(attribute_value_t *value, db_handle_t *handle, unsigned col)
{
  attribute_t *attr;
  unsigned char *buf;

  if(col >= handle->ncolumns) {
    PRINTF("DB: Requested value (%d) is out of bounds; max = (%d)\n",
	   col, handle->ncolumns);
    return DB_LIMIT_ERROR;
  }

  buf = handle->tuple;

  for(attr = list_head(handle->result_rel->attributes); attr != NULL; attr = attr->next) {
    if(attr->flags & ATTRIBUTE_FLAG_NO_STORE) {
      /* This attribute was used for processing only. */
      continue;
    }
    PRINTF("Found attribute %s in the result. The element size is %d\n",
	   attr->name, attr->element_size);
    if(col == 0) {
      break;
    }
    --col;
    buf += attr->element_size;
  }

  if(attr == NULL) {
    return DB_NAME_ERROR;
  }

  return db_phy_to_value(value, attr, buf);
}

/* db_phy_to_value: Convert a value from the physical storage 
   representation to the internal RAM representation. */
db_result_t
db_phy_to_value(attribute_value_t *value, attribute_t *attr, 
                 unsigned char *ptr)
{
  int int_value;
  long long_value;

  value->domain = attr->domain;

  switch(attr->domain) {
  case DOMAIN_STRING:
    ptr[attr->element_size - 1] = '\0';
    VALUE_STRING(value) = ptr;
    PRINTF("DB: %s = %s\n", attr->name, ptr);
    break;
  case DOMAIN_INT:
    int_value = (ptr[0] << 8) | ((unsigned)ptr[1] & 0xff);
    VALUE_INT(value) = int_value;
    PRINTF("DB: %s = %d\n", attr->name, int_value);
    break;
  case DOMAIN_LONG:
    long_value = (long)ptr[0] << 24 | (long)ptr[1] << 16 |
                 (long)ptr[2] << 8 | (long)ptr[3];
    VALUE_LONG(value) = long_value;
    PRINTF("DB: %s = %ld\n", attr->name, long_value);
    break;
  default:
    return DB_TYPE_ERROR;
  }

  return DB_OK;
}

/* db_value_to_phy: Convert a value from the internal RAM representation 
   to the physical storage representation. */
db_result_t
db_value_to_phy(unsigned char *ptr, attribute_t *attr,
                attribute_value_t *value)
{
  int int_value;
  long long_value;

  switch(attr->domain) {
  case DOMAIN_STRING:
    memcpy(ptr, VALUE_STRING(value), attr->element_size);
    ptr[attr->element_size - 1] = '\0';
    break;
  case DOMAIN_INT:
    int_value = VALUE_INT(value);
    ptr[0] = int_value >> 8;
    ptr[1] = int_value & 0xff;
    break;
  case DOMAIN_LONG:
    long_value = VALUE_LONG(value);
    ptr[0] = long_value >> 24;
    ptr[1] = long_value >> 16;
    ptr[2] = long_value >> 8;
    ptr[3] = long_value & 0xff;
    break;
  default:
    return DB_TYPE_ERROR;
  }

  return DB_OK;
}

/* db_value_to_long: Convert an attribute value
   to a value of the C long type. */
long
db_value_to_long(attribute_value_t *value)
{
  switch(value->domain) {
  case DOMAIN_INT:
    return (long)VALUE_INT(value);
  case DOMAIN_LONG:
    return (long)VALUE_LONG(value);
  default:
    return 0;
  }
}

/* db_free: Free all the resources that are referenced in a DB handle. */
db_result_t
db_free(db_handle_t *handle)
{
  if(handle->rel != NULL) {
    relation_release(handle->rel);
  }
  if(handle->result_rel != NULL) {
    relation_release(handle->result_rel);
  }
  if(handle->left_rel != NULL) {
    relation_release(handle->left_rel);
  }
  if(handle->right_rel != NULL) {
    relation_release(handle->right_rel);
  }

  handle->flags = 0;

  return DB_OK;
}
