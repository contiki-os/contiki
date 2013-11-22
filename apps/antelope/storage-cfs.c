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
 *	Contiki File System (CFS) backend for the storage abstraction
 *      used by the database.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <stdio.h>
#include <string.h>

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/random.h"

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#include "db-options.h"
#include "storage.h"

struct attribute_record {
  char name[ATTRIBUTE_NAME_LENGTH];
  uint8_t domain;
  uint8_t element_size;
};

struct index_record {
  char attribute_name[ATTRIBUTE_NAME_LENGTH];
  char file_name[DB_MAX_FILENAME_LENGTH];
  uint8_t type;
};

#if DB_FEATURE_COFFEE
#define DB_COFFEE_CATALOG_SIZE RELATION_NAME_LENGTH +                     \
                               (DB_MAX_ATTRIBUTES_PER_RELATION *          \
                               sizeof(struct attribute_record))
#endif

#define ROW_XOR 0xf6U

static void
merge_strings(char *dest, char *prefix, char *suffix)
{
  strcpy(dest, prefix);
  strcat(dest, suffix);
}

char *
storage_generate_file(char *prefix, unsigned long size)
{
  static char filename[ATTRIBUTE_NAME_LENGTH + sizeof(".ffff")];
#if !DB_FEATURE_COFFEE
  int fd;
#endif

  snprintf(filename, sizeof(filename), "%s.%x", prefix,
           (unsigned)(random_rand() & 0xffff));

#if DB_FEATURE_COFFEE
  PRINTF("DB: Reserving %lu bytes in %s\n", size, filename);
  if(cfs_coffee_reserve(filename, size) < 0) {
    PRINTF("DB: Failed to reserve\n");
    return NULL;
  }
  return filename;
#else
  fd = cfs_open(filename, CFS_WRITE);
  cfs_close(fd);
  return fd < 0 ? NULL : filename;
#endif /* DB_FEATURE_COFFEE */
}

db_result_t
storage_load(relation_t *rel)
{
  PRINTF("DB: Opening the tuple file %s\n", rel->tuple_filename);
  rel->tuple_storage = cfs_open(rel->tuple_filename,
                                CFS_READ | CFS_WRITE | CFS_APPEND);
  if(rel->tuple_storage < 0) {
    PRINTF("DB: Failed to open the tuple file\n");
    return DB_STORAGE_ERROR;
  }

  return DB_OK;
}

void
storage_unload(relation_t *rel)
{
  if(RELATION_HAS_TUPLES(rel)) {
    PRINTF("DB: Unload tuple file %s\n", rel->tuple_filename);

    cfs_close(rel->tuple_storage);
    rel->tuple_storage = -1;
  }
}

db_result_t
storage_get_relation(relation_t *rel, char *name)
{
  int fd;
  int r;
  int i;
  struct attribute_record record;
  db_result_t result;

  fd = cfs_open(name, CFS_READ);
  if(fd < 0) {
    return DB_STORAGE_ERROR;
  }

  r = cfs_read(fd, rel->name, sizeof(rel->name));
  if(r != sizeof(rel->name)) {
    cfs_close(fd);
    PRINTF("DB: Failed to read name, got %d of %d bytes\n",
	   r, sizeof(rel->name));
    return DB_STORAGE_ERROR;
  }

  r = cfs_read(fd, rel->tuple_filename, sizeof(rel->tuple_filename));
  if(r != sizeof(rel->name)) {
    cfs_close(fd);
    PRINTF("DB: Failed to read tuple filename\n");
    return DB_STORAGE_ERROR;
  }

  rel->tuple_filename[sizeof(rel->tuple_filename) - 1] ^= ROW_XOR;

  /* Read attribute records. */
  result = DB_OK;
  for(i = 0;; i++) {
    r = cfs_read(fd, &record, sizeof(record));
    if(r == 0) {
      break;
    }
    if(r != sizeof(record)) {
      PRINTF("DB: Failed to read attribute record %d (r = %d)\n", i, r);
      result = DB_STORAGE_ERROR;
      break;
    }

    if(relation_attribute_add(rel, DB_MEMORY, record.name,
			      record.domain, record.element_size) == NULL) {
      PRINTF("DB: Failed to add the attribute %s\n", record.name);
      result = DB_STORAGE_ERROR;
      break;
    }
  }

  PRINTF("DB: Read %d attributes\n", i);

  cfs_close(fd);
  return result;
}

db_result_t
storage_put_relation(relation_t *rel)
{
  int fd;
  int r;
  char *str;
  unsigned char *last_byte;

  PRINTF("DB: put_relation(%s)\n", rel->name);

  cfs_remove(rel->name);

#if DB_FEATURE_COFFEE
  cfs_coffee_reserve(rel->name, DB_COFFEE_CATALOG_SIZE);
#endif

  fd = cfs_open(rel->name, CFS_WRITE | CFS_READ);
  if(fd < 0) {
    return DB_STORAGE_ERROR;
  }

  r = cfs_write(fd, rel->name, sizeof(rel->name));
  if(r != sizeof(rel->name)) {
    cfs_close(fd);
    cfs_remove(rel->name);
    return DB_STORAGE_ERROR;
  }

  if(rel->tuple_filename[0] == '\0') {
    str = storage_generate_file("tuple", DB_COFFEE_RESERVE_SIZE);
    if(str == NULL) {
      cfs_close(fd);
      cfs_remove(rel->name);
      return DB_STORAGE_ERROR;
    }

    strncpy(rel->tuple_filename, str, sizeof(rel->tuple_filename) - 1);
    rel->tuple_filename[sizeof(rel->tuple_filename) - 1] = '\0';
  }

  /*
   * Encode the last byte to ensure that the filename is not
   * null-terminated. This will make the Coffee FS determine
   * the correct length when re-opening the file.
   */
  last_byte = (unsigned char *)&rel->tuple_filename[sizeof(rel->tuple_filename) - 1];
  *last_byte ^= ROW_XOR;

  r = cfs_write(fd, rel->tuple_filename, sizeof(rel->tuple_filename));

  *last_byte ^= ROW_XOR;

  if(r != sizeof(rel->tuple_filename)) {
    cfs_close(fd);
    cfs_remove(rel->tuple_filename);
    return DB_STORAGE_ERROR;
  }

  PRINTF("DB: Saved relation %s\n", rel->name);

  cfs_close(fd);
  return DB_OK;
}

db_result_t
storage_put_attribute(relation_t *rel, attribute_t *attr)
{
  int fd;
  struct attribute_record record;
  int r;

  PRINTF("DB: put_attribute(%s, %s)\n", rel->name, attr->name);

  fd = cfs_open(rel->name, CFS_WRITE | CFS_APPEND);
  if(fd < 0) {
    return DB_STORAGE_ERROR;
  }

  memset(&record.name, 0, sizeof(record.name));
  memcpy(record.name, attr->name, sizeof(record.name));
  record.domain = attr->domain;
  record.element_size = attr->element_size;
  r = cfs_write(fd, &record, sizeof(record));
  if(r != sizeof(record)) {
    cfs_close(fd);
    cfs_remove(rel->name);
    return DB_STORAGE_ERROR;
  }

  cfs_close(fd);
  return DB_OK;
}

db_result_t
storage_drop_relation(relation_t *rel, int remove_tuples)
{
  if(remove_tuples && RELATION_HAS_TUPLES(rel)) {
    cfs_remove(rel->tuple_filename);
  }
  return cfs_remove(rel->name) < 0 ? DB_STORAGE_ERROR : DB_OK;
}

#if DB_FEATURE_REMOVE
db_result_t
storage_rename_relation(char *old_name, char *new_name)
{
  db_result_t result;
  int old_fd;
  int new_fd;
  int r;
  char buf[64];

  result = DB_STORAGE_ERROR;
  old_fd = new_fd = -1;

  old_fd = cfs_open(old_name, CFS_READ);
  new_fd = cfs_open(new_name, CFS_WRITE);
  if(old_fd < 0 || new_fd < 0) {
    goto error;
  }

  for(;;) {
    r = cfs_read(old_fd, buf, sizeof(buf));
    if(r < 0) {
      goto error;
    } else if(r == 0) {
      break;
    }
    if(cfs_write(new_fd, buf, r) != r) {
      goto error;
    }
  };

  cfs_remove(old_name);
  result = DB_OK;

error:
  cfs_close(old_fd);
  cfs_close(new_fd);

  if(result != DB_OK) {
    cfs_remove(new_name);
  }
  return result;
}
#endif /* DB_FEATURE_REMOVE */

db_result_t
storage_get_index(index_t *index, relation_t *rel, attribute_t *attr)
{
  char filename[INDEX_NAME_LENGTH];
  int fd;
  int r;
  struct index_record record;
  db_result_t result;

  merge_strings(filename, rel->name, INDEX_NAME_SUFFIX);

  fd = cfs_open(filename, CFS_READ);
  if(fd < 0) {
    return DB_STORAGE_ERROR;
  }

  for(result = DB_STORAGE_ERROR;;) {
    r = cfs_read(fd, &record, sizeof(record));
    if(r < sizeof(record)) {
      break;
    }
    if(strcmp(attr->name, record.attribute_name) == 0) {
      PRINTF("DB: Found the index record for %s.%s: type %d, filename %s\n",
	rel->name, attr->name, record.type, record.file_name);
      index->type = record.type;
      memcpy(index->descriptor_file, record.file_name,
	     sizeof(index->descriptor_file));
      result = DB_OK;
    }
  }

  cfs_close(fd);

  return result;
}

db_result_t
storage_put_index(index_t *index)
{
  char filename[INDEX_NAME_LENGTH];
  int fd;
  int r;
  struct index_record record;
  db_result_t result;

  merge_strings(filename, index->rel->name, INDEX_NAME_SUFFIX);

  fd = cfs_open(filename, CFS_WRITE | CFS_APPEND);
  if(fd < 0) {
    return DB_STORAGE_ERROR;
  }

  strcpy(record.attribute_name, index->attr->name);
  memcpy(record.file_name, index->descriptor_file, sizeof(record.file_name));
  record.type = index->type;

  result = DB_OK;
  r = cfs_write(fd, &record, sizeof(record));
  if(r < sizeof(record)) {
    result = DB_STORAGE_ERROR;
  } else {
    PRINTF("DB: Wrote an index record for %s.%s, type %d\n",
      index->rel->name, index->attr->name, record.type);
  }

  cfs_close(fd);

  return result;
}

db_result_t
storage_get_row(relation_t *rel, tuple_id_t *tuple_id, storage_row_t row)
{
  int r;
  tuple_id_t nrows;

  if(DB_ERROR(storage_get_row_amount(rel, &nrows))) {
    return DB_STORAGE_ERROR;
  }

  if(*tuple_id >= nrows) {
    return DB_FINISHED;
  }

  if(cfs_seek(rel->tuple_storage, *tuple_id * rel->row_length, CFS_SEEK_SET) ==
              (cfs_offset_t)-1) {
    return DB_STORAGE_ERROR;
  }

  r = cfs_read(rel->tuple_storage, row, rel->row_length);
  if(r < 0) {
    PRINTF("DB: Reading failed on fd %d\n", rel->tuple_storage);
    return DB_STORAGE_ERROR;
  } else if(r == 0) {
    return DB_FINISHED;
  } else if(r < rel->row_length) {
    PRINTF("DB: Incomplete record: %d < %d\n", r, rel->row_length);
    return DB_STORAGE_ERROR;
  }

  row[rel->row_length - 1] ^= ROW_XOR;

  PRINTF("DB: Read %d bytes from relation %s\n", rel->row_length, rel->name);

  return DB_OK;
}

db_result_t
storage_put_row(relation_t *rel, storage_row_t row)
{
  cfs_offset_t end;
  unsigned remaining;
  int r;
  unsigned char *last_byte;
#if DB_FEATURE_INTEGRITY
  int missing_bytes;
  char buf[rel->row_length];
#endif

  end = cfs_seek(rel->tuple_storage, 0, CFS_SEEK_END);
  if(end == (cfs_offset_t)-1) {
    return DB_STORAGE_ERROR;
  }

#if DB_FEATURE_INTEGRITY
  missing_bytes = end % rel->row_length;
  if(missing_bytes > 0) {
    memset(buf, 0xff, sizeof(buf));
    r = cfs_write(rel->tuple_storage, buf, sizeof(buf));
    if(r != missing_bytes) {
      return DB_STORAGE_ERROR;
    }
  }
#endif

  /* Ensure that last written byte is separated from 0, to make file
     lengths correct in Coffee. */
  last_byte = row + rel->row_length - 1;
  *last_byte ^= ROW_XOR;

  remaining = rel->row_length;
  do {
    r = cfs_write(rel->tuple_storage, row, remaining);
    if(r < 0) {
      PRINTF("DB: Failed to store %u bytes\n", remaining);
      *last_byte ^= ROW_XOR;
      return DB_STORAGE_ERROR;
    }
    row += r;
    remaining -= r;
  } while(remaining > 0);

  PRINTF("DB: Stored a of %d bytes\n", rel->row_length);

  *last_byte ^= ROW_XOR;

  return DB_OK;
}

db_result_t
storage_get_row_amount(relation_t *rel, tuple_id_t *amount)
{
  cfs_offset_t offset;

  if(rel->row_length == 0) {
    *amount = 0;
  } else {
    offset = cfs_seek(rel->tuple_storage, 0, CFS_SEEK_END);
    if(offset == (cfs_offset_t)-1) {
      return DB_STORAGE_ERROR;
    }

    *amount = (tuple_id_t)(offset / rel->row_length);
  }

  return DB_OK;
}

db_storage_id_t
storage_open(const char *filename)
{
  int fd;

  fd = cfs_open(filename, CFS_WRITE | CFS_READ);
#if DB_FEATURE_COFFEE
  if(fd >= 0) {
    cfs_coffee_set_io_semantics(fd, CFS_COFFEE_IO_FLASH_AWARE);
  }
#endif
  return fd;
}

void
storage_close(db_storage_id_t fd)
{
  cfs_close(fd);
}

db_result_t
storage_read(db_storage_id_t fd,
	     void *buffer, unsigned long offset, unsigned length)
{
  char *ptr;
  int r;

  /* Extend the file if necessary, so that previously unwritten bytes
     will be read in as zeroes. */
  if(cfs_seek(fd, offset + length, CFS_SEEK_SET) == (cfs_offset_t)-1) {
    return DB_STORAGE_ERROR;
  }

  if(cfs_seek(fd, offset, CFS_SEEK_SET) == (cfs_offset_t)-1) {
    return DB_STORAGE_ERROR;
  }

  ptr = buffer;
  while(length > 0) {
    r = cfs_read(fd, ptr, length);
    if(r <= 0) {
      return DB_STORAGE_ERROR;
    }
    ptr += r;
    length -= r;
  }

  return DB_OK;
}

db_result_t
storage_write(db_storage_id_t fd,
	      void *buffer, unsigned long offset, unsigned length)
{
  char *ptr;
  int r;

  if(cfs_seek(fd, offset, CFS_SEEK_SET) == (cfs_offset_t)-1) {
    return DB_STORAGE_ERROR;
  }

  ptr = buffer;
  while(length > 0) {
    r = cfs_write(fd, ptr, length);
    if(r <= 0) {
      return DB_STORAGE_ERROR;
    }
    ptr += r;
    length -= r;
  }

  return DB_OK;
}
