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
 *	Logic for relational databases.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <limits.h>
#include <string.h>

#include "lib/crc16.h"
#include "lib/list.h"
#include "lib/memb.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#include "db-options.h"
#include "index.h"
#include "lvm.h"
#include "relation.h"
#include "result.h"
#include "storage.h"
#include "aql.h"

/*
 * The source_dest_map structure is used for mapping the pointers to 
 * data in a source row and in the corresponding destination row. The 
 * structure is calculated just before processing a relational 
 * selection, and then used to improve the performance when processing 
 * each row.
*/
struct source_dest_map {
  attribute_t *from_attr;
  attribute_t *to_attr;
  unsigned from_offset;
  unsigned to_offset;
};

static struct source_dest_map attr_map[AQL_ATTRIBUTE_LIMIT];

#if DB_FEATURE_JOIN
/*
 * The source_map structure is used for mapping attributes to
 * their offsets in rows.
 */
struct source_map {
  attribute_t *attr;
  unsigned char *from_ptr;
};

static struct source_map source_map[AQL_ATTRIBUTE_LIMIT];
#endif /* DB_FEATURE_JOIN */

static unsigned char row[DB_MAX_ATTRIBUTES_PER_RELATION * DB_MAX_ELEMENT_SIZE];
static unsigned char extra_row[DB_MAX_ATTRIBUTES_PER_RELATION * DB_MAX_ELEMENT_SIZE];
static unsigned char result_row[AQL_ATTRIBUTE_LIMIT * DB_MAX_ELEMENT_SIZE];
static unsigned char * const left_row = row;
static unsigned char * const right_row = extra_row;
static unsigned char * const join_row = result_row;

LIST(relations);
MEMB(relations_memb, relation_t, DB_RELATION_POOL_SIZE);
MEMB(attributes_memb, attribute_t, DB_ATTRIBUTE_POOL_SIZE);

static relation_t *relation_find(char *);
static attribute_t *attribute_find(relation_t *, char *);
static int get_attribute_value_offset(relation_t *, attribute_t *);
static void attribute_free(relation_t *, attribute_t *);
static void purge_relations(void);
static void relation_clear(relation_t *);
static relation_t *relation_allocate(void);
static void relation_free(relation_t *);

static relation_t *
relation_find(char *name)
{
  relation_t *rel;

  for(rel = list_head(relations); rel != NULL; rel = rel->next) {
    if(strcmp(rel->name, name) == 0) {
      return rel;
    }
  }

  return NULL;
}

static attribute_t *
attribute_find(relation_t *rel, char *name)
{
  attribute_t *attr;

  for(attr = list_head(rel->attributes); attr != NULL; attr = attr->next) {
    if(strcmp(attr->name, name) == 0) {
      return attr;
    }
  }
    return NULL;
}

static int
get_attribute_value_offset(relation_t *rel, attribute_t *attr)
{
  attribute_t *ptr;
  int offset;

  for(offset = 0, ptr = list_head(rel->attributes);
      ptr != NULL;
      ptr = ptr->next) {
    if(ptr == attr) {
      return offset;
    }
    offset += ptr->element_size;
  }

  return -1;
}

static void
attribute_free(relation_t *rel, attribute_t *attr)
{
  if(attr->index != NULL) {
    index_release(attr->index);
  }
  memb_free(&attributes_memb, attr);
  rel->attribute_count--;
}

static void
purge_relations(void)
{
  relation_t *rel;
  relation_t *next;

  for(rel = list_head(relations); rel != NULL;) {
    next = rel->next;
    if(rel->references == 0) {
      relation_free(rel);
    }
    rel = next;
  }
}

static void
relation_clear(relation_t *rel)
{
  memset(rel, 0, sizeof(*rel));
  rel->tuple_storage = -1;
  rel->cardinality = INVALID_TUPLE;
  rel->dir = DB_STORAGE;
  LIST_STRUCT_INIT(rel, attributes);
}

static relation_t *
relation_allocate(void)
{
  relation_t *rel;

  rel = memb_alloc(&relations_memb);
  if(rel == NULL) {
    purge_relations();
    rel = memb_alloc(&relations_memb);
    if(rel == NULL) {
      PRINTF("DB: Failed to allocate a relation\n");
      return NULL;
    }
  }

  relation_clear(rel);
  return rel;
}

static void
relation_free(relation_t *rel)
{
  attribute_t *attr;

  while((attr = list_pop(rel->attributes)) != NULL) {
    attribute_free(rel, attr);
  }

  list_remove(relations, rel);
  memb_free(&relations_memb, rel);
}

db_result_t
relation_init(void)
{
  list_init(relations);
  memb_init(&relations_memb);
  memb_init(&attributes_memb);

  return DB_OK;
}

relation_t *
relation_load(char *name)
{
  relation_t *rel;

  rel = relation_find(name);
  if(rel != NULL) {
    rel->references++;
    goto end;
  }

  rel = relation_allocate();
  if(rel == NULL) {
    return NULL;
  }

  if(DB_ERROR(storage_get_relation(rel, name))) {
    memb_free(&relations_memb, rel);
    return NULL;
  }

  memcpy(rel->name, name, sizeof(rel->name));
  rel->name[sizeof(rel->name) - 1] = '\0';
  rel->references = 1;
  list_add(relations, rel);

end:
  if(rel->dir == DB_STORAGE && DB_ERROR(storage_load(rel))) {
    relation_release(rel);
    return NULL;
  }

  return rel;
}

db_result_t
relation_release(relation_t *rel)
{
  if(rel->references > 0) {
    rel->references--;
  }

  if(rel->references == 0) {
    storage_unload(rel);
  }

  return DB_OK;
}

relation_t *
relation_create(char *name, db_direction_t dir)
{
  relation_t old_rel;
  relation_t *rel;

  if(*name != '\0') {
    relation_clear(&old_rel);

    if(storage_get_relation(&old_rel, name) == DB_OK) {
      /* Reject a creation request if the relation already exists. */
      PRINTF("DB: Attempted to create a relation that already exists (%s)\n",
             name);
      return NULL;
    }

    rel = relation_allocate();
    if(rel == NULL) {
      return NULL;
    }

    rel->cardinality = 0;

    strncpy(rel->name, name, sizeof(rel->name) - 1);
    rel->name[sizeof(rel->name) - 1] = '\0';
    rel->dir = dir;

    if(dir == DB_STORAGE) {
      storage_drop_relation(rel, 1);

      if(storage_put_relation(rel) == DB_OK) {
        list_add(relations, rel);
        return rel;
      }
      memb_free(&relations_memb, rel);
    } else {
      list_add(relations, rel);
      return rel;
    }
  }

  return NULL;
}

#if DB_FEATURE_REMOVE
db_result_t
relation_rename(char *old_name, char *new_name)
{
  if(DB_ERROR(relation_remove(new_name, 0)) ||
     DB_ERROR(storage_rename_relation(old_name, new_name))) {
    return DB_STORAGE_ERROR;
  }

  return DB_OK;
}
#endif /* DB_FEATURE_REMOVE */

attribute_t *
relation_attribute_add(relation_t *rel, db_direction_t dir, char *name,
		       domain_t domain, size_t element_size)
{
  attribute_t *attribute;
  tuple_id_t cardinality;

  cardinality = relation_cardinality(rel);
  if(cardinality != INVALID_TUPLE && cardinality > 0) {
    PRINTF("DB: Attempt to create an attribute in a non-empty relation\n");
    return NULL;
  }

  if(element_size == 0 || element_size > DB_MAX_ELEMENT_SIZE) {
    PRINTF("DB: Unacceptable element size: %u\n", element_size);
    return NULL;
  }

  attribute = memb_alloc(&attributes_memb);
  if(attribute == NULL) {
    PRINTF("DB: Failed to allocate attribute \"%s\"!\n", name);
    return NULL;
  }

  strncpy(attribute->name, name, sizeof(attribute->name) - 1);
  attribute->name[sizeof(attribute->name) - 1] = '\0';
  attribute->domain = domain;
  attribute->element_size = element_size;
  attribute->aggregator = 0;
  attribute->index = NULL;
  attribute->flags = 0 /*ATTRIBUTE_FLAG_UNIQUE*/;

  rel->row_length += element_size;

  list_add(rel->attributes, attribute);
  rel->attribute_count++;

  if(dir == DB_STORAGE) {
    if(DB_ERROR(storage_put_attribute(rel, attribute))) {
       PRINTF("DB: Failed to store attribute %s\n", attribute->name);
       memb_free(&attributes_memb, attribute);
       return NULL;
    }
  } else {
    index_load(rel, attribute);
  }

  return attribute;
}

attribute_t *
relation_attribute_get(relation_t *rel, char *name)
{
  attribute_t *attr;

  attr = attribute_find(rel, name);
  if(attr != NULL && !(attr->flags & ATTRIBUTE_FLAG_INVALID)) {
    return attr;
  }

  return NULL;
}

db_result_t
relation_attribute_remove(relation_t *rel, char *name)
{
  /* Not implemented completely. */
  return DB_IMPLEMENTATION_ERROR;
#if 0
  attribute_t *attr;

  if(rel->references > 1) {
    return DB_BUSY_ERROR;
  }

  attr = relation_attribute_get(rel, name);
  if(attr == NULL) {
    return DB_NAME_ERROR;
  }

  list_remove(rel->attributes, attr);
  attribute_free(rel, attr);
  return DB_OK;
#endif
}

db_result_t
relation_get_value(relation_t *rel, attribute_t *attr,
                  unsigned char *row_ptr, attribute_value_t *value)
{
  int offset;
  unsigned char *from_ptr;

  offset = get_attribute_value_offset(rel, attr);
  if(offset < 0) {
    return DB_IMPLEMENTATION_ERROR;
  }
  from_ptr = row_ptr + offset;

  return db_phy_to_value(value, attr, from_ptr);
}

db_result_t
relation_set_primary_key(relation_t *rel, char *name)
{
  attribute_t *attribute;

  attribute = relation_attribute_get(rel, name);
  if(attribute == NULL) {
    return DB_NAME_ERROR;
  }

  attribute->flags = ATTRIBUTE_FLAG_PRIMARY_KEY;
  PRINTF("DB: New primary key: %s\n", attribute->name);

  return DB_OK;
}

db_result_t
relation_remove(char *name, int remove_tuples)
{
  relation_t *rel;
  db_result_t result;

  rel = relation_load(name);
  if(rel == NULL) {
    /*
     * Attempt to remove an inexistent relation. To allow for this 
     * operation to be used for setting up repeatable tests and 
     * experiments, we do not signal an error.
     */
    return DB_OK;
  }

  if(rel->references > 1) {
    return DB_BUSY_ERROR;
  }

  result = storage_drop_relation(rel, remove_tuples);
  relation_free(rel);
  return result;
}

db_result_t
relation_insert(relation_t *rel, attribute_value_t *values)
{
  attribute_t *attr;
  unsigned char record[rel->row_length];
  unsigned char *ptr;
  attribute_value_t *value;
  db_result_t result;

  value = values;

  PRINTF("DB: Relation %s has a record size of %u bytes\n",
	 rel->name, (unsigned)rel->row_length);
  ptr = record;

  PRINTF("DB: Insert (");

  for(attr = list_head(rel->attributes); attr != NULL; attr = attr->next, value++) {
    /* Verify that the value is in the expected domain. An exception
       to this rule is that INT may be promoted to LONG. */
    if(attr->domain != value->domain &&
       !(attr->domain == DOMAIN_LONG && value->domain == DOMAIN_INT)) {
      PRINTF("DB: The value domain %d does not match the domain %d of attribute %s\n",
             value->domain, attr->domain, attr->name);
      return DB_RELATIONAL_ERROR;
    }

    /* Set the data area for removed attributes to 0. */
    if(attr->flags & ATTRIBUTE_FLAG_INVALID) {
      memset(ptr, 0, attr->element_size);
      ptr += attr->element_size;
      continue;
    }

    result = db_value_to_phy((unsigned char *)ptr, attr, value);
    if(DB_ERROR(result)) {
      return result;
    }

#if DEBUG
    switch(attr->domain) {
    case DOMAIN_INT:
      PRINTF("%s=%d", attr->name, VALUE_INT(value));
      break;
    case DOMAIN_LONG:
      PRINTF("%s=%ld", attr->name, VALUE_LONG(value));
      break;
    case DOMAIN_STRING:
      PRINTF("%s='%s", attr->name, VALUE_STRING(value));
      break;
    default:
      PRINTF(")\nDB: Unhandled attribute domain: %d\n", attr->domain);
      return DB_TYPE_ERROR;
    }

    if(attr->next != NULL) {
      PRINTF(", ");
    }
#endif /* DEBUG */

    ptr += attr->element_size;
    if(attr->index != NULL) {
      if(DB_ERROR(index_insert(attr->index, value, rel->next_row))) {
        return DB_INDEX_ERROR;
      }
    }
  }

  PRINTF(")\n");

  rel->cardinality++;
  rel->next_row++;
  return storage_put_row(rel, record);
}

static void
aggregate(attribute_t *attr, attribute_value_t *value)
{
  long long_value;

  switch(value->domain) {
  case DOMAIN_INT:
    long_value = VALUE_INT(value);
    break;
  case DOMAIN_LONG:
    long_value = VALUE_LONG(value);
    break;
  default:
    return;
  }

  switch(attr->aggregator) {
  case AQL_COUNT:
    attr->aggregation_value++;
    break;
  case AQL_SUM:
    attr->aggregation_value += long_value;
    break;
  case AQL_MEAN:
    break;
  case AQL_MEDIAN:
    break;
  case AQL_MAX:
    if(long_value > attr->aggregation_value) {
      attr->aggregation_value = long_value;
    }
    break;
  case AQL_MIN:
    if(long_value < attr->aggregation_value) {
      attr->aggregation_value = long_value;
    }
    break;
  default:
    break;
  }
}

static db_result_t
generate_attribute_map(struct source_dest_map *attr_map, unsigned attribute_count,
                       relation_t *from_rel, relation_t *to_rel, 
                       unsigned char *from_row, unsigned char *to_row)
{
  attribute_t *from_attr;
  attribute_t *to_attr;
  unsigned size_sum;
  struct source_dest_map *attr_map_ptr;
  int offset;

  attr_map_ptr = attr_map;
  for(size_sum = 0, to_attr = list_head(to_rel->attributes);
      to_attr != NULL;
      to_attr = to_attr->next) {
    from_attr = attribute_find(from_rel, to_attr->name);
    if(from_attr == NULL) {
       PRINTF("DB: Invalid attribute in the result relation: %s\n",
		to_attr->name);
      return DB_NAME_ERROR;
    }

    attr_map_ptr->from_attr = from_attr;
    attr_map_ptr->to_attr = to_attr;
    offset = get_attribute_value_offset(from_rel, from_attr);
    if(offset < 0) {
      return DB_IMPLEMENTATION_ERROR;
    }
    attr_map_ptr->from_offset = offset;
    attr_map_ptr->to_offset = size_sum;

    size_sum += to_attr->element_size;
    attr_map_ptr++;
  }

  return DB_OK;
}

static void
select_index(db_handle_t *handle, lvm_instance_t *lvm_instance)
{
  index_t *index;
  attribute_t *attr;
  operand_value_t min;
  operand_value_t max;
  attribute_value_t av_min;
  attribute_value_t av_max;
  long range;
  unsigned long min_range;

  index = NULL;
  min_range = ULONG_MAX;

  /* Find all indexed and derived attributes, and select the index of 
     the attribute with the smallest range. */
  for(attr = list_head(handle->rel->attributes);
      attr != NULL;
      attr = attr->next) {
    if(attr->index != NULL &&
       !LVM_ERROR(lvm_get_derived_range(lvm_instance, attr->name, &min, &max))) {
      range = (unsigned long)max.l - (unsigned long)min.l;
      PRINTF("DB: The search range for attribute \"%s\" comprises %ld values\n",
             attr->name, range + 1);

      if(range <= min_range) {
        index = attr->index;
        av_min.domain = av_max.domain = DOMAIN_INT;
        VALUE_LONG(&av_min) = min.l;
        VALUE_LONG(&av_max) = max.l;
      }
    }
  }

  if(index != NULL) {
    /* We found a suitable index; get an iterator for it. */
    if(index_get_iterator(&handle->index_iterator, index, 
                          &av_min, &av_max) == DB_OK) {
      handle->flags |= DB_HANDLE_FLAG_SEARCH_INDEX;
    }
  }
}

static db_result_t
generate_selection_result(db_handle_t *handle, relation_t *rel, aql_adt_t *adt)
{
  relation_t *result_rel;
  unsigned attribute_count;
  attribute_t *attr;

  result_rel = handle->result_rel;

  handle->current_row = 0;
  handle->ncolumns = 0;
  handle->tuple_id = 0;
  for(attr = list_head(result_rel->attributes); attr != NULL; attr = attr->next) {
    if(attr->flags & ATTRIBUTE_FLAG_NO_STORE) {
      continue;
    }
    handle->ncolumns++;
  }
  handle->tuple = (tuple_t)result_row;

  attribute_count = result_rel->attribute_count;
  if(DB_ERROR(generate_attribute_map(attr_map, attribute_count, rel, result_rel, row, result_row))) {
    return DB_IMPLEMENTATION_ERROR;
  }

  if(adt->lvm_instance != NULL) {
    /* Try to establish acceptable ranges for the attribute values. */
    if(!LVM_ERROR(lvm_derive(adt->lvm_instance))) {
      select_index(handle, adt->lvm_instance);
    }
  }

  handle->flags |= DB_HANDLE_FLAG_PROCESSING;

  return DB_OK;
}

#if DB_FEATURE_REMOVE
db_result_t
relation_process_remove(void *handle_ptr)
{
  db_handle_t *handle;
  aql_adt_t *adt;
  db_result_t result;

  handle = (db_handle_t *)handle_ptr;
  adt = handle->adt;

  result = relation_process_select(handle_ptr);
  if(result == DB_FINISHED) {
    PRINTF("DB: Finished removing tuples. Overwriting relation %s with the result\n", 
	adt->relations[1]);
    relation_release(handle->rel);
    relation_rename(adt->relations[0], adt->relations[1]);
  }

  return result;
}
#endif

db_result_t
relation_process_select(void *handle_ptr)
{
  db_handle_t *handle;
  aql_adt_t *adt;
  db_result_t result;
  unsigned attribute_count;
  struct source_dest_map *attr_map_ptr, *attr_map_end;
  attribute_t *result_attr;
  unsigned char *from_ptr;
  unsigned char *to_ptr;
  operand_value_t operand_value;
  uint8_t intbuf[2];
  attribute_value_t value;
  lvm_status_t wanted_result;

  handle = (db_handle_t *)handle_ptr;
  adt = (aql_adt_t *)handle->adt;

  attribute_count = handle->result_rel->attribute_count;
  attr_map_end = attr_map + attribute_count;

  if(handle->flags & DB_HANDLE_FLAG_SEARCH_INDEX) {
    handle->tuple_id = index_get_next(&handle->index_iterator);
    if(handle->tuple_id == INVALID_TUPLE) {
      PRINTF("DB: An attribute value could not be found in the index\n");
      if(handle->index_iterator.next_item_no == 0) {
        return DB_INDEX_ERROR;
      }

      if(adt->flags & AQL_FLAG_AGGREGATE) {
        goto end_aggregation;
      }

      return DB_FINISHED;
    }
  }

  /* Put the tuples fulfilling the given condition into a new relation.
     The tuples may be projected. */
  result = storage_get_row(handle->rel, &handle->tuple_id, row);
  handle->tuple_id++;
  if(DB_ERROR(result)) {
    PRINTF("DB: Failed to get a row in relation %s!\n", handle->rel->name);
    return result;
  } else if(result == DB_FINISHED) {
    if(AQL_GET_FLAGS(adt) & AQL_FLAG_AGGREGATE) {
      goto end_aggregation;
    }
    return DB_FINISHED;
  }

  /* Process the attributes in the result relation. */
  for(attr_map_ptr = attr_map; attr_map_ptr < attr_map_end; attr_map_ptr++) {
    from_ptr = row + attr_map_ptr->from_offset;
    result_attr = attr_map_ptr->to_attr;

    /* Update the internal state of the PLE. */
    if(result_attr->domain == DOMAIN_INT) {
      operand_value.l = from_ptr[0] << 8 | from_ptr[1];
      lvm_set_variable_value(result_attr->name, operand_value);
    } else if(result_attr->domain == DOMAIN_LONG) {
      operand_value.l = (uint32_t)from_ptr[0] << 24 |
                        (uint32_t)from_ptr[1] << 16 |
                        (uint32_t)from_ptr[2] << 8 |
                        from_ptr[3];
      lvm_set_variable_value(result_attr->name, operand_value);
    }

    if(result_attr->flags & ATTRIBUTE_FLAG_NO_STORE) {
      /* The attribute is used just for the predicate,
         so do not copy the current value into the result. */
      continue;
    }

    if(!(AQL_GET_FLAGS(adt) & AQL_FLAG_AGGREGATE)) {
      /* No aggregators. Copy the original value into the resulting tuple. */
      memcpy(result_row + attr_map_ptr->to_offset, from_ptr,
             result_attr->element_size);
    }
  }

  wanted_result = TRUE;
  if(AQL_GET_FLAGS(adt) & AQL_FLAG_INVERSE_LOGIC) {
    wanted_result = FALSE;
  }

  /* Check whether the given predicate is true for this tuple. */
  if(adt->lvm_instance == NULL ||
     lvm_execute(adt->lvm_instance) == wanted_result) {
    if(AQL_GET_FLAGS(adt) & AQL_FLAG_AGGREGATE) {
      for(attr_map_ptr = attr_map; attr_map_ptr < attr_map_end; attr_map_ptr++) {
        from_ptr = row + attr_map_ptr->from_offset;
        result = db_phy_to_value(&value, attr_map_ptr->to_attr, from_ptr);
        if(DB_ERROR(result)) {
	  return result;
        }
        aggregate(attr_map_ptr->to_attr, &value);
      }
    } else {
      if(AQL_GET_FLAGS(adt) & AQL_FLAG_ASSIGN) {
        if(DB_ERROR(storage_put_row(handle->result_rel, result_row))) {
          PRINTF("DB: Failed to store a row in the result relation!\n");
          return DB_STORAGE_ERROR;
        }
      }
      handle->current_row++;
      return DB_GOT_ROW;
    }
  }

  return DB_OK;

end_aggregation:
  /* Generate aggregated result if requested. */
  for(attr_map_ptr = attr_map; attr_map_ptr < attr_map_end; attr_map_ptr++) {
    result_attr = attr_map_ptr->to_attr;
    to_ptr = result_row + attr_map_ptr->to_offset;

    intbuf[0] = result_attr->aggregation_value >> 8;
    intbuf[1] = result_attr->aggregation_value & 0xff;
    from_ptr = intbuf;
    memcpy(to_ptr, from_ptr, result_attr->element_size);
  }

  if(AQL_GET_FLAGS(adt) & AQL_FLAG_ASSIGN) {
    if(DB_ERROR(storage_put_row(handle->result_rel, result_row))) {
      PRINTF("DB: Failed to store a row in the result relation!\n");
      return DB_STORAGE_ERROR;
    }
  }

  handle->current_row = 1;
  AQL_GET_FLAGS(adt) &= ~AQL_FLAG_AGGREGATE; /* Stop the aggregation. */

  return DB_GOT_ROW;
}

db_result_t
relation_select(void *handle_ptr, relation_t *rel, void *adt_ptr)
{
  aql_adt_t *adt;
  db_handle_t *handle;
  char *name;
  db_direction_t dir;
  char *attribute_name;
  attribute_t *attr;
  int i;
  int normal_attributes;

  adt = (aql_adt_t *)adt_ptr;

  handle = (db_handle_t *)handle_ptr;
  handle->rel = rel;
  handle->adt = adt;

  if(AQL_GET_FLAGS(adt) & AQL_FLAG_ASSIGN) {
    name = adt->relations[0];
    dir = DB_STORAGE;
  } else {
    name = RESULT_RELATION;
    dir = DB_MEMORY;
  }
  relation_remove(name, 1);
  relation_create(name, dir);
  handle->result_rel = relation_load(name);

  if(handle->result_rel == NULL) {
    PRINTF("DB: Failed to load a relation for the query result\n");
    return DB_ALLOCATION_ERROR;
  }

  for(i = normal_attributes = 0; i < AQL_ATTRIBUTE_COUNT(adt); i++) {
    attribute_name = adt->attributes[i].name;

    attr = relation_attribute_get(rel, attribute_name);
    if(attr == NULL) {
      PRINTF("DB: Select for invalid attribute %s in relation %s!\n",
	     attribute_name, rel->name);
      return DB_NAME_ERROR;
    }

    PRINTF("DB: Found attribute %s in relation %s\n",
	attribute_name, rel->name);

    attr = relation_attribute_add(handle->result_rel, dir,
				  attribute_name, 
				  adt->aggregators[i] ? DOMAIN_INT : attr->domain,
				  attr->element_size);
    if(attr == NULL) {
      PRINTF("DB: Failed to add a result attribute\n");
      relation_release(handle->result_rel);
      return DB_ALLOCATION_ERROR;
    }

    attr->aggregator = adt->aggregators[i];
    switch(attr->aggregator) {
    case AQL_NONE:
      if(!(adt->attributes[i].flags & ATTRIBUTE_FLAG_NO_STORE)) {
        /* Only count attributes projected into the result set. */
        normal_attributes++;
      }
      break;
    case AQL_MAX:
      attr->aggregation_value = LONG_MIN;
      break;
    case AQL_MIN:
      attr->aggregation_value = LONG_MAX;
      break;
    default:
      attr->aggregation_value = 0;
      break;
    }

    attr->flags = adt->attributes[i].flags;
  }

  /* Preclude mixes of normal attributes and aggregated ones in 
     selection results. */
  if(normal_attributes > 0 &&
     handle->result_rel->attribute_count > normal_attributes) {
     return DB_RELATIONAL_ERROR;
  }

  return generate_selection_result(handle, rel, adt);
}

#if DB_FEATURE_JOIN
db_result_t
relation_process_join(void *handle_ptr)
{
  db_handle_t *handle;
  db_result_t result;
  relation_t *left_rel;
  relation_t *right_rel;
  relation_t *join_rel;
  unsigned char *join_next_attribute_ptr;
  size_t element_size;
  tuple_id_t right_tuple_id;
  attribute_value_t value;
  int i;

  handle = (db_handle_t *)handle_ptr;
  left_rel = handle->left_rel;
  right_rel = handle->right_rel;
  join_rel = handle->join_rel;

  if(!(handle->flags & DB_HANDLE_FLAG_INDEX_STEP)) {
    goto inner_loop;
  }

  /* Equi-join for indexed attributes only. In the outer loop, we iterate over
     each tuple in the left relation. */
  for(handle->tuple_id = 0;; handle->tuple_id++) {
    result = storage_get_row(left_rel, &handle->tuple_id, left_row);
    if(DB_ERROR(result)) {
      PRINTF("DB: Failed to get a row in left relation %s!\n", left_rel->name);
      return result;
    } else if(result == DB_FINISHED) {
      return DB_FINISHED;
    }

    if(DB_ERROR(relation_get_value(left_rel, handle->left_join_attr, left_row, &value))) {
      PRINTF("DB: Failed to get a value of the attribute \"%s\" to join on\n",
	handle->left_join_attr->name);
      return DB_IMPLEMENTATION_ERROR;
    }

    if(DB_ERROR(index_get_iterator(&handle->index_iterator, 
                                   handle->right_join_attr->index, 
                                   &value, &value))) { 
      PRINTF("DB: Failed to get an index iterator\n");
      return DB_INDEX_ERROR;
    }
    handle->flags &= ~DB_HANDLE_FLAG_INDEX_STEP;

    /* In the inner loop, we iterate over all rows with a matching value for the
       join attribute. The index component provides an iterator for this purpose. */
inner_loop:
    for(;;) {
      /* Get all rows matching the attribute value in the right relation. */
      right_tuple_id = index_get_next(&handle->index_iterator);
      if(right_tuple_id == INVALID_TUPLE) {
        /* Exclude this row from the left relation in the result,
           and step to the next value in the index iteration. */
        handle->flags |= DB_HANDLE_FLAG_INDEX_STEP;
        break;
      }

      result = storage_get_row(right_rel, &right_tuple_id, right_row);
      if(DB_ERROR(result)) {
        PRINTF("DB: Failed to get a row in right relation %s!\n", right_rel->name);
        return result;
      } else if(result == DB_FINISHED) {
	PRINTF("DB: The index refers to an invalid row: %lu\n",
	       (unsigned long)right_tuple_id);
        return DB_IMPLEMENTATION_ERROR;
      }

      /* Use the source attribute map to fill in the physical representation
	 of the resulting tuple. */
      join_next_attribute_ptr = join_row;

      for(i = 0; i < join_rel->attribute_count; i++) {
	element_size = source_map[i].attr->element_size;

	memcpy(join_next_attribute_ptr, source_map[i].from_ptr, element_size);
	join_next_attribute_ptr += element_size;
      }

      if(((aql_adt_t *)handle->adt)->flags & AQL_FLAG_ASSIGN) {
        if(DB_ERROR(storage_put_row(join_rel, join_row))) {
          return DB_STORAGE_ERROR;
        }
      }

      handle->current_row++;
      return DB_GOT_ROW;
    }
  }

  return DB_OK;
}

static db_result_t
generate_join_result(db_handle_t *handle)
{
  relation_t *left_rel;
  relation_t *right_rel;
  relation_t *join_rel;
  attribute_t *attr;
  attribute_t *result_attr;
  struct source_map *source_pair;
  int i;
  int offset;
  unsigned char *from_ptr;

  handle->tuple = (tuple_t)join_row;
  handle->tuple_id = 0;

  left_rel = handle->left_rel;
  right_rel = handle->right_rel;
  join_rel = handle->join_rel;

  /* Generate a map over the source attributes for each
     attribute in the join relation. */
  for(i = 0, result_attr = list_head(join_rel->attributes);
      result_attr != NULL; 
      result_attr = result_attr->next, i++) {
    source_pair = &source_map[i];
    attr = attribute_find(left_rel, result_attr->name);
    if(attr != NULL) {
      offset = get_attribute_value_offset(left_rel, attr);
      from_ptr = left_row + offset;
    } else if((attr = attribute_find(right_rel, result_attr->name)) != NULL) {
      offset = get_attribute_value_offset(right_rel, attr);
      from_ptr = right_row + offset;
    } else {
      PRINTF("DB: The attribute %s could not be found\n", result_attr->name);
      return DB_NAME_ERROR;
    }

    if(offset < 0) {
      PRINTF("DB: Unable to retrieve attribute values for the JOIN result\n");
      return DB_IMPLEMENTATION_ERROR;
    }
    
    source_pair->attr = attr;
    source_pair->from_ptr = from_ptr;
  }

  handle->flags |= DB_HANDLE_FLAG_PROCESSING;

  return DB_OK;
}

db_result_t
relation_join(void *query_result, void *adt_ptr)
{
  aql_adt_t *adt;
  db_handle_t *handle;
  relation_t *left_rel;
  relation_t *right_rel;
  relation_t *join_rel;
  char *name;
  db_direction_t dir;
  int i;
  char *attribute_name;
  attribute_t *attr;

  adt = (aql_adt_t *)adt_ptr;

  handle = (db_handle_t *)query_result;
  handle->current_row = 0;
  handle->ncolumns = 0;
  handle->adt = adt;
  handle->flags = DB_HANDLE_FLAG_INDEX_STEP;

  if(AQL_GET_FLAGS(adt) & AQL_FLAG_ASSIGN) {
    name = adt->relations[0];
    dir = DB_STORAGE;
  } else {
    name = RESULT_RELATION;
    dir = DB_MEMORY;
  }
  relation_remove(name, 1);
  relation_create(name, dir);
  join_rel = relation_load(name);
  handle->result_rel = join_rel;

  if(join_rel == NULL) {
    PRINTF("DB: Failed to create a join relation!\n");
    return DB_ALLOCATION_ERROR;
  }

  handle->join_rel = handle->result_rel = join_rel;
  left_rel = handle->left_rel;
  right_rel = handle->right_rel;

  handle->left_join_attr = relation_attribute_get(left_rel, adt->attributes[0].name);
  handle->right_join_attr = relation_attribute_get(right_rel, adt->attributes[0].name);
  if(handle->left_join_attr == NULL || handle->right_join_attr == NULL) {
    PRINTF("DB: The attribute (\"%s\") to join on does not exist in both relations\n",
	adt->attributes[0].name);
    return DB_RELATIONAL_ERROR;
  }

  if(!index_exists(handle->right_join_attr)) {
    PRINTF("DB: The attribute to join on is not indexed\n");
    return DB_INDEX_ERROR;
  }

  /*
   * Define the resulting relation. We start from 1 when counting attributes
   * because the first attribute is only the one to join, and is not included
   * by default in the projected attributes.
   */
  for(i = 1; i < AQL_ATTRIBUTE_COUNT(adt); i++) {
    attribute_name = adt->attributes[i].name;
    attr = relation_attribute_get(left_rel, attribute_name);
    if(attr == NULL) {
      attr = relation_attribute_get(right_rel, attribute_name);
      if(attr == NULL) {
	PRINTF("DB: The projection attribute \"%s\" does not exist in any of the relations to join\n",
		attribute_name);
        return DB_RELATIONAL_ERROR;
      }
    }

    if(relation_attribute_add(join_rel, dir, attr->name, attr->domain, 
                              attr->element_size) == NULL) {
      PRINTF("DB: Failed to add an attribute to the join relation\n");
      return DB_ALLOCATION_ERROR;
    }

    handle->ncolumns++;
  }

  return generate_join_result(handle);
}
#endif /* DB_FEATURE_JOIN */

tuple_id_t
relation_cardinality(relation_t *rel)
{
  tuple_id_t tuple_id;


  if(rel->cardinality != INVALID_TUPLE) {
    return rel->cardinality;
  }

  if(!RELATION_HAS_TUPLES(rel)) {
    return 0;
  }

  if(DB_ERROR(storage_get_row_amount(rel, &tuple_id))) {
    return INVALID_TUPLE;
  }

  rel->cardinality = tuple_id;

  PRINTF("DB: Relation %s has cardinality %lu\n", rel->name,
	(unsigned long)tuple_id);

  return tuple_id;
}
