/*
 * Copyright (c) 2008, 2009, Swedish Institute of Computer Science
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
 *	Coffee: A flash file system for memory-constrained sensor systems.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include <limits.h>
#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "contiki-conf.h"
#include "cfs/cfs.h"
#include "cfs-coffee-arch.h"
#include "cfs/cfs-coffee.h"
#include "dev/watchdog.h"

#if COFFEE_PAGES_PER_SECTOR & (COFFEE_PAGES_PER_SECTOR - 1)
#error COFFEE_PAGES_PER_SECTOR must be a power of two.
#error Change COFFEE_PAGES_PER_SECTOR in cfs-coffee-arch.h.
#endif

#define COFFEE_FD_FREE		0x0
#define COFFEE_FD_READ		0x1
#define COFFEE_FD_WRITE		0x2
#define COFFEE_FD_APPEND	0x4

#define COFFEE_FILE_MODIFIED	0x1

#define INVALID_PAGE		((coffee_page_t)-1)
#define UNKNOWN_OFFSET		((cfs_offset_t)-1)

#define FD_VALID(fd)					\
	((fd) >= 0 && (fd) < COFFEE_FD_SET_SIZE && 	\
	coffee_fd_set[(fd)].flags != COFFEE_FD_FREE)
#define FD_READABLE(fd)		(coffee_fd_set[(fd)].flags & CFS_READ)
#define FD_WRITABLE(fd)		(coffee_fd_set[(fd)].flags & CFS_WRITE)
#define FD_APPENDABLE(fd)	(coffee_fd_set[(fd)].flags & CFS_APPEND)

#define FILE_MODIFIED(file)	((file)->flags & COFFEE_FILE_MODIFIED)

/* File header flags. */
#define HDR_FLAG_VALID		0x1	/* Completely written header. */
#define HDR_FLAG_ALLOCATED	0x2	/* Allocated file. */
#define HDR_FLAG_OBSOLETE	0x4	/* File marked for GC. */
#define HDR_FLAG_MODIFIED	0x8	/* Modified file, log exists. */
#define HDR_FLAG_LOG		0x10	/* Log file. */
#define HDR_FLAG_ISOLATED	0x20	/* Isolated page. */

#define CHECK_FLAG(hdr, flag)	((hdr).flags & (flag))
#define HDR_VALID(hdr)		CHECK_FLAG(hdr, HDR_FLAG_VALID)
#define HDR_ALLOCATED(hdr)	CHECK_FLAG(hdr, HDR_FLAG_ALLOCATED)
#define HDR_FREE(hdr)		!HDR_ALLOCATED(hdr)
#define HDR_LOG(hdr)		CHECK_FLAG(hdr, HDR_FLAG_LOG)
#define HDR_MODIFIED(hdr)	CHECK_FLAG(hdr, HDR_FLAG_MODIFIED)
#define HDR_ISOLATED(hdr)	CHECK_FLAG(hdr, HDR_FLAG_ISOLATED)
#define HDR_OBSOLETE(hdr) 	CHECK_FLAG(hdr, HDR_FLAG_OBSOLETE)
#define HDR_ACTIVE(hdr)		(HDR_ALLOCATED(hdr) && \
				!HDR_OBSOLETE(hdr)  && \
				!HDR_ISOLATED(hdr))

#define COFFEE_SECTOR_COUNT	(COFFEE_SIZE / COFFEE_SECTOR_SIZE)
#define COFFEE_PAGE_COUNT	\
	((coffee_page_t)(COFFEE_SIZE / COFFEE_PAGE_SIZE))
#define COFFEE_PAGES_PER_SECTOR	\
	((coffee_page_t)(COFFEE_SECTOR_SIZE / COFFEE_PAGE_SIZE))

struct sector_stats {
  coffee_page_t active;
  coffee_page_t obsolete;
  coffee_page_t free;
};

struct file {
  cfs_offset_t end;
  coffee_page_t page;
  coffee_page_t max_pages;
  int16_t next_log_record;
  uint8_t references;
  uint8_t flags;
};

struct file_desc {
  cfs_offset_t offset;
  struct file *file;
  uint8_t flags;
};

struct file_header {
  coffee_page_t log_page;
  uint16_t log_records;
  uint16_t log_record_size;
  coffee_page_t max_pages;
  uint8_t eof_hint;
  uint8_t flags;
  char name[COFFEE_NAME_LENGTH];
} __attribute__((packed));

/* This is needed because of a buggy compiler. */
struct log_param {
  cfs_offset_t offset;
  const char *buf;
  uint16_t size;
};

static struct protected_mem_t {
  struct file coffee_files[COFFEE_MAX_OPEN_FILES];
  struct file_desc coffee_fd_set[COFFEE_FD_SET_SIZE];
  coffee_page_t next_free;
} protected_mem;
static struct file *coffee_files = protected_mem.coffee_files;
static struct file_desc *coffee_fd_set = protected_mem.coffee_fd_set;
static coffee_page_t *next_free = &protected_mem.next_free;

/*---------------------------------------------------------------------------*/
static void
write_header(struct file_header *hdr, coffee_page_t page)
{
  hdr->flags |= HDR_FLAG_VALID;
  COFFEE_WRITE(hdr, sizeof(*hdr), page * COFFEE_PAGE_SIZE);
}
/*---------------------------------------------------------------------------*/
static void
read_header(struct file_header *hdr, coffee_page_t page)
{
  COFFEE_READ(hdr, sizeof(*hdr), page * COFFEE_PAGE_SIZE);
#if DEBUG
  if(HDR_ACTIVE(*hdr) && !HDR_VALID(*hdr)) {
    PRINTF("Invalid header at page %u!\n", (unsigned)page);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static cfs_offset_t
absolute_offset(coffee_page_t page, cfs_offset_t offset)
{
  return page * COFFEE_PAGE_SIZE + sizeof(struct file_header) + offset;
}
/*---------------------------------------------------------------------------*/
static void
get_sector_status(uint16_t sector, struct sector_stats *stats) {
  static coffee_page_t skip_pages;
  static char last_pages_are_active;
  struct file_header hdr;
  coffee_page_t active, obsolete, free;
  coffee_page_t sector_start, sector_end;
  coffee_page_t page;

  active = obsolete = free = 0;

  if(sector == 0) {
    skip_pages = 0;
    last_pages_are_active = 0;
  } else if(skip_pages > COFFEE_PAGES_PER_SECTOR) {
    skip_pages -= COFFEE_PAGES_PER_SECTOR;
    if(last_pages_are_active) {
      stats->active = COFFEE_PAGES_PER_SECTOR;
      stats->obsolete = 0;
    } else {
      stats->active = 0;
      stats->obsolete = COFFEE_PAGES_PER_SECTOR;
    }
    stats->free = 0;
    return;
  }

  sector_start = sector * COFFEE_SECTOR_SIZE / COFFEE_PAGE_SIZE;
  sector_end = sector_start + COFFEE_SECTOR_SIZE / COFFEE_PAGE_SIZE;

  if(last_pages_are_active) {
    active = skip_pages;
  } else {
    obsolete = skip_pages;

    /* Split an obsolete file starting in the previous sector and mark
       the following pages as isolated. */
    memset(&hdr, 0, sizeof(hdr));
    hdr.flags = HDR_FLAG_ALLOCATED | HDR_FLAG_ISOLATED;
    for(page = 0; page < skip_pages; page++) {
      write_header(&hdr, sector_start + page);
    }
    PRINTF("Coffee: Isolated %u pages starting in sector %d\n",
        (unsigned)skip_pages, (int)sector);
  }

  for(page = sector_start + skip_pages; page < sector_end;) {
    read_header(&hdr, page);
    last_pages_are_active = 0;
    if(HDR_ACTIVE(hdr)) {
      last_pages_are_active = 1;
      page += hdr.max_pages;
      active += hdr.max_pages;
    } else if(HDR_ISOLATED(hdr)) {
      page++;
      obsolete++;
    } else if(HDR_OBSOLETE(hdr)) {
      page += hdr.max_pages;
      obsolete += hdr.max_pages;
    } else {
      free = COFFEE_PAGES_PER_SECTOR - active - obsolete;
      break;
    }
  }

  skip_pages = active + obsolete + free - COFFEE_PAGES_PER_SECTOR;
  if(skip_pages > 0) {
    if(last_pages_are_active) {
      active = COFFEE_PAGES_PER_SECTOR - obsolete;
    } else {
      obsolete = COFFEE_PAGES_PER_SECTOR - active;
    }
  }

  stats->active = active;
  stats->obsolete = obsolete;
  stats->free = free;
}
/*---------------------------------------------------------------------------*/
static void
cfs_garbage_collect(void)
{
  uint16_t sector;
  struct sector_stats stats;
  coffee_page_t first_page;

  watchdog_stop();

  PRINTF("Coffee: Running the file system garbage collector...\n");
  /*
   * The garbage collector erases as many sectors as possible. A sector is
   * erasable if there are only free or obsolete pages in it.
   */
  for(sector = 0; sector < COFFEE_SECTOR_COUNT; sector++) {
    get_sector_status(sector, &stats);
    PRINTF("Coffee: Sector %u has %u active, %u free, and %u obsolete pages.\n",
        sector, (unsigned)stats.active, (unsigned)stats.free, (unsigned)stats.obsolete);
    if(stats.active == 0 && stats.obsolete > 0) {
      COFFEE_ERASE(sector);
      PRINTF("Coffee: Erased sector %d!\n", sector);
      first_page = sector * COFFEE_PAGES_PER_SECTOR;
      if(first_page < *next_free) {
        *next_free = first_page;
      }
    }
  }

  watchdog_start();
}
/*---------------------------------------------------------------------------*/
static coffee_page_t
next_file(coffee_page_t page, struct file_header *hdr)
{
  if(HDR_FREE(*hdr)) {
    return (page + COFFEE_PAGES_PER_SECTOR) & ~(COFFEE_PAGES_PER_SECTOR - 1);
  } else if(HDR_ISOLATED(*hdr)) {
    return page + 1;
  }
  return page + hdr->max_pages;    
}
/*---------------------------------------------------------------------------*/
static struct file *
load_file(const char *name, struct file_header *hdr, coffee_page_t start)
{
  int i, unreferenced, free;
  struct file *file;

  /*
   * We prefer to overwrite a free slot since unreferenced ones
   * contain usable data. Free slots are designated by the page
   * value INVALID_PAGE.
   */
  for(i = 0, unreferenced = free = -1; i < COFFEE_MAX_OPEN_FILES; i++) {
    if(coffee_files[i].page == INVALID_PAGE) {
      free = i;
      break;
    } else if(coffee_files[i].references == 0) {
      unreferenced = i;
    }
  }

  if(free != -1) {
     i = free;
  } else if(unreferenced != -1) {
    i = unreferenced;
  } else {
    return NULL;
  }

  file = &coffee_files[i];
  file->page = start;
  file->end = UNKNOWN_OFFSET;
  file->max_pages = hdr->max_pages;
  file->next_log_record = -1;

  return file;
}
/*---------------------------------------------------------------------------*/
static struct file *
find_file(const char *name)
{
  int i;
  struct file_header hdr;
  coffee_page_t page;

  /* First check if the file metadata is cached. */
  for(i = 0; i < COFFEE_MAX_OPEN_FILES; i++) {
    if(coffee_files[i].max_pages == 0) {
      continue;
    }

    read_header(&hdr, coffee_files[i].page);
    if(HDR_ACTIVE(hdr) && strcmp(name, hdr.name) == 0) {
      return &coffee_files[i];
    }
  }

  /* Scan the flash memory sequentially otherwise. */
  for(page = 0; page < COFFEE_PAGE_COUNT; page = next_file(page, &hdr)) {
    read_header(&hdr, page);
    if(HDR_ACTIVE(hdr) && strcmp(name, hdr.name) == 0) {
	return load_file(name, &hdr, page);
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
#if COFFEE_CONF_EOF_HINT
static void
refresh_eof_hint(struct file *file)
{
  struct file_header hdr;
  coffee_page_t current_page;
  int part_size, i;
  uint8_t eof_hint;

  read_header(&hdr, file->page);
  current_page = (file->end + sizeof(hdr)) / COFFEE_PAGE_SIZE;
  part_size = hdr.max_pages / (sizeof(hdr.eof_hint) * CHAR_BIT);
  if(part_size == 0) {
    part_size = 1;
  }
  for(i = eof_hint = 0; i < sizeof(eof_hint) * CHAR_BIT; i++) {
    eof_hint |= (current_page >= (i + 1) * part_size) << i;
  }

  if(eof_hint > hdr.eof_hint) {
    hdr.eof_hint |= eof_hint;
    write_header(&hdr, file->page);
  }
}
#endif /* COFFEE_CONF_EOF_HINT */
/*---------------------------------------------------------------------------*/
static cfs_offset_t
file_end(coffee_page_t start)
{
  struct file_header hdr;
  unsigned char buf[COFFEE_PAGE_SIZE];
  coffee_page_t page;
  int i;
  coffee_page_t range_start, range_end;
#if COFFEE_CONF_EOF_HINT
  coffee_page_t part_size;
  int search_limit;
#endif

  read_header(&hdr, start);
#if COFFEE_CONF_EOF_HINT
  search_limit = 0;
  for(i = 0; i < sizeof(hdr.eof_hint) * CHAR_BIT; i++) {
    if(hdr.eof_hint >> i) {
      search_limit = i + 1;
    }
  }
  part_size = hdr.max_pages / sizeof(hdr.eof_hint) / CHAR_BIT;
  if(part_size == 0) {
    part_size = 1;
  }
  range_start = part_size * search_limit;
  range_end = range_start + part_size;
  if(range_end + 1 > hdr.max_pages) {
    range_end = hdr.max_pages - 1;
  }
#else
  range_start = 0;
  range_end = hdr.max_pages - 1;
#endif /* COFFEE_CONF_EOF_HINT */

  /*
   * Move from the end of the range towards the beginning and look for
   * a byte that has been modified.
   *
   * An important implication of this is that if the last written bytes
   * are zeroes, then these are skipped from the calculation.
   */

  for(page = start + range_end; page >= start; page--) {
    watchdog_periodic();
    COFFEE_READ(buf, sizeof(buf), page * COFFEE_PAGE_SIZE);
    for(i = COFFEE_PAGE_SIZE - 1; i >= 0; i--) {
      if(buf[i] != 0) {
	if(page == start && i < sizeof(hdr)) {
	  return 0;
	}
	return 1 + i + ((page - start) * COFFEE_PAGE_SIZE) - sizeof(hdr);
      }
    }
  }

  /* All bytes are writable. */
  return 0;
}
/*---------------------------------------------------------------------------*/
static coffee_page_t
find_contiguous_pages(coffee_page_t amount)
{
  coffee_page_t page, start;
  struct file_header hdr;

  start = INVALID_PAGE;
  for(page = *next_free; page < COFFEE_PAGE_COUNT;) {
    read_header(&hdr, page);
    if(HDR_FREE(hdr)) {
      if(start == INVALID_PAGE) {
	start = page;
      }

      /* All remaining pages in this sector are free -- jump to the next sector. */
      page = next_file(page, &hdr);

      if(start + amount <= page) {
	*next_free = start + amount;
	return start;
      }
    } else {
      start = INVALID_PAGE;
      page = next_file(page, &hdr);
    }
  }
  return INVALID_PAGE;
}
/*---------------------------------------------------------------------------*/
static int
remove_by_page(coffee_page_t page, int remove_log, int close_fds)
{
  struct file_header hdr;
  int i;

  read_header(&hdr, page);
  if(!HDR_ACTIVE(hdr)) {
    return -1;
  }

  if(remove_log && HDR_MODIFIED(hdr)) {
    if (remove_by_page(hdr.log_page, 0, 0) < 0) {
      return -1;
    }
  }

  hdr.flags |= HDR_FLAG_OBSOLETE;
  write_header(&hdr, page);

  /* Close all file descriptors that reference the removed file. */
  if(close_fds) {
    for(i = 0; i < COFFEE_FD_SET_SIZE; i++) {
      if(coffee_fd_set[i].file != NULL && coffee_fd_set[i].file->page == page) {
	coffee_fd_set[i].flags = COFFEE_FD_FREE;
      }
    }
  }

  for(i = 0; i < COFFEE_MAX_OPEN_FILES; i++) {
    if(coffee_files[i].page == page) {
      coffee_files[i].page = INVALID_PAGE;
      coffee_files[i].references = 0;
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static coffee_page_t
page_count(cfs_offset_t size)
{
  return (size + sizeof(struct file_header) + COFFEE_PAGE_SIZE - 1) /
		COFFEE_PAGE_SIZE;
}
/*---------------------------------------------------------------------------*/
static struct file *
reserve(const char *name, coffee_page_t pages, int allow_duplicates)
{
  struct file_header hdr;
  coffee_page_t page;
  struct file *file;

  watchdog_stop();

  if(!allow_duplicates && find_file(name) != NULL) {
    watchdog_start();
    return NULL;
  }

  page = find_contiguous_pages(pages);
  if(page == INVALID_PAGE) {
    cfs_garbage_collect();
    page = find_contiguous_pages(pages);
    if(page == INVALID_PAGE) {
      watchdog_start();
      return NULL;
    }
  }

  memset(&hdr, 0, sizeof(hdr));
  memcpy(hdr.name, name, sizeof(hdr.name) - 1);
  hdr.name[sizeof(hdr.name) - 1] = '\0';
  hdr.max_pages = pages;
  hdr.flags = HDR_FLAG_ALLOCATED;
  write_header(&hdr, page);

  PRINTF("Coffee: Reserved %u pages starting from %u for file %s\n",
      pages, page, name);

  file = load_file(name, &hdr, page);
  file->end = 0;

  watchdog_start();

  return file;
}
/*---------------------------------------------------------------------------*/
static void
adjust_log_config(struct file_header *hdr,
		  uint16_t *log_record_size, uint16_t *log_records)
{
  *log_record_size = hdr->log_record_size == 0 ?
    COFFEE_PAGE_SIZE : hdr->log_record_size;
  *log_records = hdr->log_records == 0 ?
    COFFEE_LOG_SIZE / *log_record_size : hdr->log_records;
}
/*---------------------------------------------------------------------------*/
static uint16_t
modify_log_buffer(uint16_t log_record_size,
		  cfs_offset_t *offset, uint16_t *size)
{
  uint16_t region;

  region = *offset / log_record_size;
  *offset %= log_record_size;
  if(*size > log_record_size - *offset) {
    *size = log_record_size - *offset;
  }
  return region;
}
/*---------------------------------------------------------------------------*/
static int
get_record_index(coffee_page_t log_page, uint16_t search_records,
		 uint16_t region)
{
  cfs_offset_t base;
  uint16_t processed;
  uint16_t batch_size;
  int16_t match_index, i;
  uint16_t record_count;

  base = absolute_offset(log_page, sizeof(uint16_t) * search_records);
  record_count = search_records > COFFEE_LOG_TABLE_LIMIT ?
      		COFFEE_LOG_TABLE_LIMIT : search_records;
  processed = 0;
  match_index = -1;

  {
  uint16_t indices[record_count];

  while(processed < search_records && match_index < 0) {
    if(record_count + processed > search_records) {
      batch_size = search_records - processed;
    } else {
      batch_size = record_count;
    }

    base -= batch_size * sizeof(indices[0]);
    COFFEE_READ(&indices, sizeof(indices[0]) * batch_size, base);

    for(i = batch_size - 1; i >= 0; i--) {
      if(indices[i] - 1 == region) {
	match_index = search_records - processed - (batch_size - i);
	break;
      }
    }

    processed += batch_size;
  }
  }

  return match_index;
}
/*---------------------------------------------------------------------------*/
static int
read_log_page(struct file_header *hdr, int16_t last_record, struct log_param *lp)
{
  uint16_t region;
  int16_t match_index;
  uint16_t log_record_size;
  uint16_t log_records;
  cfs_offset_t base;
  uint16_t search_records;

  adjust_log_config(hdr, &log_record_size, &log_records);
  region = modify_log_buffer(log_record_size, &lp->offset, &lp->size);

  search_records = last_record < 0 ? log_records : last_record + 1;
  match_index = get_record_index(hdr->log_page, search_records, region);
  if(match_index < 0) {
    return -1;
  }

  base = absolute_offset(hdr->log_page, log_records * sizeof(region));
  base += (cfs_offset_t)match_index * log_record_size;
  base += lp->offset;
  COFFEE_READ(lp->buf, lp->size, base);

  return lp->size;
}
/*---------------------------------------------------------------------------*/
static unsigned char *
create_log_name(unsigned char *new, int max_size, unsigned char *old)
{
  const unsigned char suffix[] = ".log";
  int len;

  len = strlen(old);
  if(len > max_size - sizeof(suffix)) {
    len = max_size - sizeof(suffix);
  }
  memcpy(new, old, len);
  memcpy(&new[len], suffix, sizeof(suffix));

  return new;
}
/*---------------------------------------------------------------------------*/
static coffee_page_t
create_log(struct file *file, struct file_header *hdr)
{
  coffee_page_t log_page;
  unsigned char log_name[sizeof(hdr->name)];
  uint16_t log_record_size, log_records;
  cfs_offset_t size;
  struct file *log_file;

  adjust_log_config(hdr, &log_record_size, &log_records);

  size = log_records * sizeof(uint16_t);	/* Log index size. */
  size += log_records * log_record_size;	/* Log data size. */

  log_file = reserve(create_log_name(log_name, sizeof(log_name), hdr->name),
	      page_count(size), 0);
  if(file == NULL) {
    return INVALID_PAGE;
  }
  log_page = log_file->page;

  hdr->flags |= HDR_FLAG_MODIFIED;
  hdr->log_page = log_page;
  write_header(hdr, file->page);
  read_header(hdr, log_page);
  hdr->flags |= HDR_FLAG_LOG;
  write_header(hdr, log_page);

  file->flags |= COFFEE_FILE_MODIFIED;
  return log_page;
}
/*---------------------------------------------------------------------------*/
static int
merge_log(coffee_page_t file_page, int extend)
{
  coffee_page_t log_page;
  struct file_header hdr, hdr2;
  int fd, n;
  cfs_offset_t offset;
  coffee_page_t max_pages;
  struct file *new_file;
  int i;

  read_header(&hdr, file_page);
  log_page = hdr.log_page;

  fd = cfs_open(hdr.name, CFS_READ);
  if(fd < 0) {
    return -1;
  }

  /*
   * The reservation function adds extra space for the header, which has
   * already been calculated with in the previous reservation.
   */
  max_pages = hdr.max_pages << extend;
  new_file = reserve(hdr.name, max_pages, 1);
  if(new_file == NULL) {
    cfs_close(fd);
    return -1;
  }

  offset = 0;
  do {
    char buf[hdr.log_record_size == 0 ? COFFEE_PAGE_SIZE : hdr.log_record_size];
    watchdog_periodic();
    n = cfs_read(fd, buf, sizeof(buf));
    if(n < 0) {
      remove_by_page(new_file->page, 0, 0);
      cfs_close(fd);
      return -1;
    } else if(n > 0) {
      COFFEE_WRITE(buf, n,
  	absolute_offset(new_file->page, offset));
      offset += n;
    }
  } while(n != 0);

  for(i = 0; i < COFFEE_FD_SET_SIZE; i++) {
    if(coffee_fd_set[i].flags != COFFEE_FD_FREE && 
       coffee_fd_set[i].file->page == file_page) {
      coffee_fd_set[i].file = new_file;
      new_file->references++;
    }
  }

  if(remove_by_page(file_page, 1, 0) < 0) {
    remove_by_page(new_file->page, 0, 0);
    cfs_close(fd);
    return -1;
  }

  /* Copy the log configuration and the EOF hint. */
  read_header(&hdr2, new_file->page);
  hdr2.log_record_size = hdr.log_record_size;
  hdr2.log_records = hdr.log_records;
  hdr2.eof_hint = extend ? 0 : hdr.eof_hint;
  write_header(&hdr2, new_file->page);

  if(extend) {
    new_file->end = offset;
#if COFFEE_CONF_EOF_HINT
    refresh_eof_hint(new_file);
#endif
  }

  cfs_close(fd);

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
find_next_record(struct file *file, coffee_page_t log_page,
		int log_records)
{
  int log_record, i, preferred_batch_size;

  preferred_batch_size = log_records > COFFEE_LOG_TABLE_LIMIT ?
			 COFFEE_LOG_TABLE_LIMIT : log_records;

  if(file->next_log_record == 0) {
    /* The next log record is unknown. Search for it. */
    uint16_t indices[preferred_batch_size];
    uint16_t processed;
    uint16_t batch_size;

    log_record = log_records;
    for(processed = 0; processed < log_records; processed += batch_size) {
      batch_size = log_records - processed >= preferred_batch_size ?
	preferred_batch_size : log_records - processed;

      COFFEE_READ(&indices, batch_size * sizeof(indices[0]),
		  absolute_offset(log_page, processed * sizeof(indices[0])));
      for(i = 0; i < batch_size && indices[i] != 0; i++);
      log_record = i;

      if(log_record < batch_size) {
	log_record += processed;
	break;
      }
    }
  } else {
    log_record = file->next_log_record;
  }

  return log_record;
}
/*---------------------------------------------------------------------------*/
static int
write_log_page(struct file *file, struct log_param *lp)
{
  struct file_header hdr;
  uint16_t region;
  coffee_page_t log_page;
  int16_t log_record;
  uint16_t log_record_size;
  uint16_t log_records;
  cfs_offset_t table_base, record_base;
  struct log_param lp_out;

  read_header(&hdr, file->page);

  adjust_log_config(&hdr, &log_record_size, &log_records);
  region = modify_log_buffer(log_record_size, &lp->offset, &lp->size);

  log_page = 0;
  if(HDR_MODIFIED(hdr)) {
    /* A log structure has already been created. */
    log_page = hdr.log_page;
    log_record = find_next_record(file, log_page, log_records);
    if(log_record >= log_records) {
      /* The log is full; merge the log. */
      PRINTF("Coffee: Merging the file %s with its log\n", hdr.name);
#if COFFEE_CONF_EOF_HINT
      refresh_eof_hint(file);
#endif
      return merge_log(file->page, 0);
    }
  } else {
    /* Create a log structure. */
    log_page = create_log(file, &hdr);
    if(log_page == INVALID_PAGE) {
      return -1;
    }
    PRINTF("Coffee: Created a log structure for file %s at page %u\n",
    	hdr.name, (unsigned)log_page);
    hdr.log_page = log_page;
    log_record = 0;
  }

  {
    unsigned char copy_buf[log_record_size];

    lp_out.offset = region * log_record_size;
    lp_out.buf = copy_buf;
    lp_out.size = log_record_size;

    if((lp->offset > 0 || lp->size != log_record_size) &&
	read_log_page(&hdr, file->next_log_record - 1, &lp_out) < 0) {
      COFFEE_READ(copy_buf, sizeof(copy_buf),
	  absolute_offset(file->page, region * log_record_size));
    }

    memcpy((char *) &copy_buf + lp->offset, lp->buf, lp->size);

    table_base = absolute_offset(log_page, 0);
    ++region;
    COFFEE_WRITE(&region, sizeof(region),
		 table_base + log_record * sizeof(region));

    record_base = table_base + log_records * sizeof(region);
    COFFEE_WRITE(copy_buf, sizeof(copy_buf),
		 record_base + log_record * log_record_size);
    file->next_log_record = log_record + 1;
  }

  return lp->size;
}
/*---------------------------------------------------------------------------*/
static int
get_available_fd(void)
{
  int i;

  for(i = 0; i < COFFEE_FD_SET_SIZE; i++) {
    if(coffee_fd_set[i].flags == COFFEE_FD_FREE) {
      return i;
    }
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
cfs_open(const char *name, int flags)
{
  int fd;
  struct file_desc *fdp;

  fd = get_available_fd();
  if(fd < 0) {
    PRINTF("Coffee: Failed to allocate a new file descriptor!\n");
    return -1;
  }

  fdp = &coffee_fd_set[fd];
  fdp->flags = 0;

  fdp->file = find_file(name);
  if(fdp->file == NULL) {
    if((flags & (CFS_READ | CFS_WRITE)) == CFS_READ) {
      return -1;
    }
    fdp->file = reserve(name, page_count(COFFEE_DYN_SIZE), 1);
    if(fdp->file == NULL) {
      return -1;
    }
    fdp->file->end = 0;
  } else if(fdp->file->end == UNKNOWN_OFFSET) {
    fdp->file->end = file_end(fdp->file->page);
  }

  fdp->flags |= flags;
  fdp->offset = flags & CFS_APPEND ? fdp->file->end : 0;
  fdp->file->references++;

  return fd;
}
/*---------------------------------------------------------------------------*/
void
cfs_close(int fd)
{
  if(FD_VALID(fd)) {
#if COFFEE_CONF_EOF_HINT
    refresh_eof_hint(coffee_fd_set[fd].file);
#endif
    coffee_fd_set[fd].flags = COFFEE_FD_FREE;
    coffee_fd_set[fd].file->references--;
  }
}
/*---------------------------------------------------------------------------*/
cfs_offset_t
cfs_seek(int fd, cfs_offset_t offset, int whence)
{
  struct file_header hdr;
  struct file_desc *fdp;

  if(!FD_VALID(fd)) {
    return -1;
  }
  fdp = &coffee_fd_set[fd];
  read_header(&hdr, fdp->file->page);

  /* Check if the offset is within the file boundary. */
  if(sizeof(hdr) + offset >= hdr.max_pages * COFFEE_PAGE_SIZE ||
     sizeof(hdr) + offset < offset) {
    return -1;
  }

  if(whence == CFS_SEEK_SET) {
    fdp->offset = offset;
  } else if(whence == CFS_SEEK_END) {
    fdp->offset = fdp->file->end + offset;
  } else if(whence == CFS_SEEK_CUR) {
    fdp->offset += offset;
  } else {
    return (cfs_offset_t)-1;
  }

  if(fdp->file->end < fdp->offset) {
    fdp->file->end = fdp->offset;
  }

  return fdp->offset;
}
/*---------------------------------------------------------------------------*/
int
cfs_remove(const char *name)
{
  struct file *file;

  /*
   * Coffee removes files by marking them as obsolete. The space
   * is not guaranteed to be reclaimed immediately, but must be
   * sweeped by the garbage collector. The garbage collector is
   * called once a file reservation request cannot be granted.
   */
  file = find_file(name);
  if(file == NULL) {
    return -1;
  }

  return remove_by_page(file->page, 1, 1);
}
/*---------------------------------------------------------------------------*/
int
cfs_read(int fd, void *buf, unsigned size)
{
  struct file_header hdr;
  struct file_desc *fdp;
  struct file *file;
  unsigned remains, read_chunk;
  int r;
  cfs_offset_t base, offset;
  struct log_param lp;

  if(!(FD_VALID(fd) && FD_READABLE(fd))) {
    return -1;
  }

  fdp = &coffee_fd_set[fd];
  file = fdp->file;
  if(fdp->offset + size > file->end) {
    size = file->end - fdp->offset;
  }

  read_chunk = COFFEE_PAGE_SIZE;
  if(FILE_MODIFIED(file)) {
    read_header(&hdr, file->page);
    if(hdr.log_record_size > 0) {
      read_chunk = hdr.log_record_size;
    }
  }

  remains = size;
  base = fdp->offset;
  offset = 0;
  /*
   * Fill the buffer by copying from the log in first hand, or the
   * ordinary file if the page has no log record.
   */
  while(remains) {
    watchdog_periodic();
    r = -1;
    if(FILE_MODIFIED(file)) {
      lp.offset = base + offset;
      lp.buf = (char *)buf + offset;
      lp.size = remains;
      r = read_log_page(&hdr, file->next_log_record - 1, &lp);
    }
    /* Read from the original file if we cannot find the data in the log. */
    if(r < 0) {
      r = remains > read_chunk ? read_chunk : remains;
      COFFEE_READ((char *) buf + offset, r,
	absolute_offset(file->page, base + offset));
    }
    remains -= r;
    offset += r;
  }
  fdp->offset += offset;
  return offset;
}
/*---------------------------------------------------------------------------*/
int
cfs_write(int fd, const void *buf, unsigned size)
{
  struct file_desc *fdp;
  struct file *file;
  int i;
  struct log_param lp;
  cfs_offset_t remains;

  if(!(FD_VALID(fd) && FD_WRITABLE(fd))) {
    return -1;
  }

  fdp = &coffee_fd_set[fd];
  file = fdp->file;

  /* Attempt to extend the file if we try to write past the end. */
  while(size + fdp->offset + sizeof(struct file_header) >
     (file->max_pages * COFFEE_PAGE_SIZE)) {
#if COFFEE_CONF_EOF_HINT
    refresh_eof_hint(file);
#endif
    if(merge_log(file->page, 1) < 0) {
      return -1;
    }
    file = fdp->file;
    PRINTF("Extended the file at page %u\n", (unsigned)file->page);
  }

  if(fdp->offset < file->end) {
    remains = size;
    while(remains) {
      lp.offset = fdp->offset;
      lp.buf = (char *)buf + size - remains;
      lp.size = remains;
      i = write_log_page(file, &lp);
      if(i == 0) {
	file = fdp->file;
        /* The file was merged with the log. Try again. */
	continue;
      }
      if(i < 0) {
        return size - remains > 0 ? size - remains : -1;
      }
      remains -= i;
      fdp->offset += i;
    }
  } else {
    COFFEE_WRITE(buf, size, absolute_offset(file->page, fdp->offset));
    fdp->offset += size;
  }

  if(fdp->offset > file->end) {
    file->end = fdp->offset;
  }

  return size;
}
/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *dir, const char *name)
{
  /*
   * Coffee is only guaranteed to support "/" and ".", but it does not 
   * currently enforce this.
   */
    *(coffee_page_t *)dir->dummy_space = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *dir, struct cfs_dirent *record)
{
  struct file_header hdr;
  coffee_page_t page;

  for(page = *(coffee_page_t *)dir->dummy_space; page < COFFEE_PAGE_COUNT;) {
    watchdog_periodic();
    read_header(&hdr, page);
    if(HDR_ACTIVE(hdr) && !HDR_LOG(hdr)) {
      memcpy(record->name, hdr.name, sizeof(record->name));
      record->name[sizeof(record->name) - 1] = '\0';
      record->size = file_end(page);
      *(coffee_page_t *)dir->dummy_space = next_file(page, &hdr);
      return 0;
    }
    page = next_file(page, &hdr);
  }

  return -1;
}
/*---------------------------------------------------------------------------*/
void
cfs_closedir(struct cfs_dir *dir)
{
  return;
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_reserve(const char *name, cfs_offset_t size)
{
  return reserve(name, page_count(size), 0) == NULL ? -1 : 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_configure_log(const char *filename, unsigned log_size,
			 unsigned log_record_size)
{
  struct file *file;
  struct file_header hdr;

  if(log_record_size == 0 || log_record_size > COFFEE_PAGE_SIZE ||
     log_size < log_record_size) {
    return -1;
  }

  file = find_file(filename);
  if(file == NULL) {
    return -1;
  }

  read_header(&hdr, file->page);
  if(HDR_MODIFIED(hdr)) {
    /* Too late to customize the log. */
    return -1;
  }

  hdr.log_records = log_size / log_record_size;
  hdr.log_record_size = log_record_size;
  write_header(&hdr, file->page);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_format(void)
{
  int nsectors, i;

  nsectors = COFFEE_SECTOR_COUNT;
  /* Reject format requests when the configuration is incorrect. */
  if(nsectors < 1 || (COFFEE_START & (COFFEE_SECTOR_SIZE - 1))) {
    return -1;
  }

  PRINTF("Coffee: Formatting %d sectors", nsectors);

  *next_free = 0;

  watchdog_stop();
  for(i = 0; i < nsectors; i++) {
    COFFEE_ERASE(i);
    PRINTF(".");
  }
  watchdog_start();

  /* Formatting invalidates the file information. */
  memset(&protected_mem, 0, sizeof(protected_mem));

  PRINTF("done!\n");

  return 0;
}
/*---------------------------------------------------------------------------*/
void *
cfs_coffee_get_protected_mem(unsigned *size)
{
  *size = sizeof(protected_mem);
  return &protected_mem;
}
