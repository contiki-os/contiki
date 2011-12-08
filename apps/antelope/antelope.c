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
 *	Main functions for Antelope, a DBMS for sensor devices.
 *
 *      Antelope is described and evaluated in the paper A Database in
 *      Every Sensor, N. Tsiftes and A. Dunkels, in Proceedings of
 *      ACM SenSys 2011.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <stdio.h>

#include "antelope.h"

static db_output_function_t output = printf;

void
db_init(void)
{
  relation_init();
  index_init();
}

void
db_set_output_function(db_output_function_t f)
{
  output = f;
}

const char *
db_get_result_message(db_result_t code)
{
  switch(code) {
  case DB_FINISHED:
    return "Iteration finished";
  case DB_OK:
    return "Operation succeeded";
  case DB_LIMIT_ERROR:
    return "Limit reached";
  case DB_ALLOCATION_ERROR:
    return "Allocation error";
  case DB_STORAGE_ERROR:
    return "Storage error";
  case DB_PARSING_ERROR:
    return "Parsing error";
  case DB_NAME_ERROR:
    return "Invalid name";
  case DB_RELATIONAL_ERROR:
    return "Semantic error";
  case DB_TYPE_ERROR:
    return "Type error";
  case DB_IMPLEMENTATION_ERROR:
    return "Implementation error";
  case DB_INDEX_ERROR:
    return "Index error";
  case DB_BUSY_ERROR:
    return "Busy with processing";
  case DB_INCONSISTENCY_ERROR:
    return "Inconsistent handle";
  case DB_ARGUMENT_ERROR:
    return "Invalid argument";
  default:
    return "Unknown result code";
  };
}

db_result_t
db_print_header(db_handle_t *handle)
{
  int column;
  attribute_t *attr;

  output("[relation = %s, attributes = (", handle->result_rel->name);
  attr = list_head(handle->result_rel->attributes);
  for(column = 0; column < handle->ncolumns; column++) {
    if(attr == NULL) {
      return DB_IMPLEMENTATION_ERROR;
    } else if(attr->flags & ATTRIBUTE_FLAG_NO_STORE) {
      continue;
    }
    output("%s%s", column > 0 ? ", " : "", attr->name);
    attr = attr->next;
  }
  output(")]\n");
  return DB_OK;
}

db_result_t
db_print_tuple(db_handle_t *handle)
{
  int column;
  attribute_value_t value;
  db_result_t result;

  output("Row %lu:\t", (unsigned long)handle->current_row);

  for(column = 0; column < handle->ncolumns; column++) {
    result = db_get_value(&value, handle, column);
    if(DB_ERROR(result)) {
      output("Unable to get the value for row %lu, column %u: %s\n",
             (unsigned long)handle->current_row, column,
             db_get_result_message(result));
      break;
    }

    switch(value.domain) {
    case DOMAIN_STRING:
      output("\"%s\"\t", VALUE_STRING(&value));
      break;
    case DOMAIN_INT:
      output("%d\t", (int)VALUE_INT(&value));
      break;
    case DOMAIN_LONG:
      output("%ld\t", (long)VALUE_LONG(&value));
      break;
    default:
      output("\nUnrecognized domain: %d\n", value.domain);
      return DB_IMPLEMENTATION_ERROR;
    }
  }
  output("\n");

  return DB_OK;
}

int
db_processing(db_handle_t *handle)
{
  return handle->flags & DB_HANDLE_FLAG_PROCESSING;
}
