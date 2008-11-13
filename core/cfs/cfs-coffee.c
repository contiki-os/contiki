/*
 * Copyright (c) 2008, Swedish Institute of Computer Science
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
 *	Coffee: A flash file system for memory-contrained sensor systems.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki-conf.h"
#include "cfs/cfs.h"
#include "cfs-coffee-arch.h"
#include "cfs/cfs-coffee.h"
#include "dev/watchdog.h"

#include <limits.h>
#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if COFFEE_PAGES_PER_SECTOR & (COFFEE_PAGES_PER_SECTOR - 1)
#error COFFEE_PAGES_PER_SECTOR must be a power of two.
#error Change COFFEE_PAGES_PER_SECTOR in cfs-coffee-arch.h.
#endif

#define kb			* 1024UL
#define Mb			* (1024 kb)

#define COFFEE_FD_FREE		0x0
#define COFFEE_FD_READ		0x1
#define COFFEE_FD_WRITE		0x2
#define COFFEE_FD_APPEND	0x4
#define COFFEE_FD_MODIFIED	0x8

#define FD_VALID(fd)						\
		((fd) >= 0 && (fd) < COFFEE_FD_SET_SIZE && 	\
		coffee_fd_set[(fd)].flags != COFFEE_FD_FREE)
#define FD_READABLE(fd)		(coffee_fd_set[(fd)].flags & CFS_READ)
#define FD_WRITABLE(fd)		(coffee_fd_set[(fd)].flags & CFS_WRITE)
#define FD_APPENDABLE(fd)	(coffee_fd_set[(fd)].flags & CFS_APPEND)
#define FD_MODIFIED(fd)		(coffee_fd_set[(fd)].flags & COFFEE_FD_MODIFIED)

/* File header flags. */
#define COFFEE_FLAG_VALID	0x1	/* Completely written header. */
#define COFFEE_FLAG_ALLOCATED	0x2	/* Allocated file. */
#define COFFEE_FLAG_OBSOLETE	0x4	/* File marked for GC. */
#define COFFEE_FLAG_MODIFIED	0x8	/* Modified file, log exists. */
#define COFFEE_FLAG_LOG		0x10	/* Log file. */
#define COFFEE_FLAG_ISOLATED	0x20	/* Isolated page. */

#define COFFEE_PAGE_ALLOCATED(hdr)	((hdr).flags & COFFEE_FLAG_ALLOCATED)
#define COFFEE_PAGE_FREE(hdr)		!COFFEE_PAGE_ALLOCATED(hdr)
#define COFFEE_PAGE_LOG(hdr)		((hdr).flags & COFFEE_FLAG_LOG)
#define COFFEE_PAGE_MODIFIED(hdr)	((hdr).flags & COFFEE_FLAG_MODIFIED)
#define COFFEE_PAGE_ISOLATED(hdr)	((hdr).flags & COFFEE_FLAG_ISOLATED)
#define COFFEE_PAGE_OBSOLETE(hdr) 	((hdr).flags & COFFEE_FLAG_OBSOLETE)
#define COFFEE_PAGE_ACTIVE(hdr)		(COFFEE_PAGE_ALLOCATED(hdr) && \
					!COFFEE_PAGE_OBSOLETE(hdr)  && \
					!COFFEE_PAGE_ISOLATED(hdr))

#define COFFEE_PAGE_COUNT	(COFFEE_SIZE / COFFEE_PAGE_SIZE)
#define COFFEE_PAGES_PER_SECTOR	(COFFEE_SECTOR_SIZE / COFFEE_PAGE_SIZE)

struct file_desc {
  coffee_offset_t offset;
  coffee_offset_t end;
  coffee_page_t file_page;
  coffee_page_t max_pages;
  uint16_t next_log_record;
  uint8_t flags;
};

struct dir_cache {
  coffee_offset_t page;
  char filename_start;
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
  coffee_offset_t offset;
  const char *buf;
  uint16_t size;
};

#define ABS_OFFSET(file_page, file_offset)	\
	((file_page) * COFFEE_PAGE_SIZE + 	\
	sizeof(struct file_header) + (file_offset))

static struct dir_cache dir_cache[COFFEE_DIR_CACHE_ENTRIES];
static struct file_desc coffee_fd_set[COFFEE_FD_SET_SIZE];

/*---------------------------------------------------------------------------*/
static void
write_header(struct file_header *hdr, coffee_page_t page)
{
  COFFEE_WRITE(hdr, sizeof(*hdr), page * COFFEE_PAGE_SIZE);
}
/*---------------------------------------------------------------------------*/
static void
read_header(struct file_header *hdr, coffee_page_t page)
{
  COFFEE_READ(hdr, sizeof(*hdr), page * COFFEE_PAGE_SIZE);
}
/*---------------------------------------------------------------------------*/
static void
get_sector_status(uint16_t sector, coffee_page_t *active,
		  coffee_page_t *free, coffee_page_t *obsolete) {
  static coffee_page_t skip_pages;
  static int last_pages_are_active;
  coffee_offset_t offset, sector_start;
  coffee_offset_t end;
  struct file_header hdr;
  coffee_page_t jump;
  int i;
  
  *active = *free = *obsolete = 0;
  if(sector == 0) {
    skip_pages = 0;
    last_pages_are_active = 0;
  } else if(skip_pages > COFFEE_PAGES_PER_SECTOR) {
    skip_pages -= COFFEE_PAGES_PER_SECTOR;
    if(last_pages_are_active) {
      *active = COFFEE_PAGES_PER_SECTOR;
    } else {
      *obsolete = COFFEE_PAGES_PER_SECTOR;
    }
    return;
  }

  sector_start = sector * COFFEE_SECTOR_SIZE;

  if(last_pages_are_active) {
    *active = skip_pages;
  } else {
    *obsolete = skip_pages;

    /* Split an obsolete file starting in the previous sector and mark 
       the following pages as isolated. */
    offset = sector_start;
    for(i = 0; i < skip_pages; i++) {
      COFFEE_READ(&hdr, sizeof(hdr), offset);
      hdr.flags |= COFFEE_FLAG_ISOLATED;
      COFFEE_WRITE(&hdr, sizeof(hdr), offset);
      offset += COFFEE_PAGE_SIZE;
    }
    PRINTF("Coffee: Isolated %u pages starting in sector %d\n",
	(unsigned)skip_pages, (int)sector);
  }

  offset = sector_start + skip_pages * COFFEE_PAGE_SIZE;
  end = (sector + 1) * COFFEE_SECTOR_SIZE;
  jump = 0;
 
  while(offset < end) {
    COFFEE_READ(&hdr, sizeof(hdr), offset);
    last_pages_are_active = 0;
    if(COFFEE_PAGE_ACTIVE(hdr)) {
      last_pages_are_active = 1;
      jump = hdr.max_pages;
      *active += jump;
    } else if(COFFEE_PAGE_ISOLATED(hdr)) {
      jump = 1;
      *obsolete++;
    } else if(COFFEE_PAGE_OBSOLETE(hdr)) {
      jump = hdr.max_pages;
      *obsolete += jump;
    } else if(COFFEE_PAGE_FREE(hdr)) {
      *free = (end - offset) / COFFEE_PAGE_SIZE;
      break;
    }
    offset += jump * COFFEE_PAGE_SIZE;
  }
  
  skip_pages = *active + *obsolete - COFFEE_PAGES_PER_SECTOR;
  if(skip_pages > 0) {
    if(last_pages_are_active) {
      *active = COFFEE_PAGES_PER_SECTOR - *obsolete;
    } else {
      *obsolete = COFFEE_PAGES_PER_SECTOR - *active;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
dir_cache_add(char c, coffee_offset_t page)
{
  int i;
  
  for(i = 0; i < COFFEE_DIR_CACHE_ENTRIES; i++) {
    if(dir_cache[i].filename_start == '\0') {
      dir_cache[i].filename_start = c;
      dir_cache[i].page = page;
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
dir_cache_del(coffee_offset_t page)
{
  int i;
  
  for(i = 0; i < COFFEE_DIR_CACHE_ENTRIES; i++) {
    if(dir_cache[i].page == page) {
      dir_cache[i].filename_start = '\0';
    } 
  }
}
/*---------------------------------------------------------------------------*/
static coffee_offset_t
dir_cache_find(const char *name)
{
  int i;
  struct file_header hdr;

  for(i = 0; i < COFFEE_DIR_CACHE_ENTRIES; i++) {
    if(*name == dir_cache[i].filename_start) {
      read_header(&hdr, dir_cache[i].page);
      if(!COFFEE_PAGE_ACTIVE(hdr)) {
	dir_cache[i].filename_start = '\0';
      } else if(strcmp(hdr.name, name) == 0) {
	return dir_cache[i].page;
      }
    }
  }
  
  return -1;
}
/*---------------------------------------------------------------------------*/
static coffee_offset_t
find_file(const char *name)
{
  struct file_header hdr;
  coffee_page_t page;

  page = dir_cache_find(name);
  if(page >= 0) {
    return page;
  }
  
  page = 0;
  do {
    read_header(&hdr, page);
    if(COFFEE_PAGE_ACTIVE(hdr)) {
      if(dir_cache_find(name) == -1) {
	dir_cache_add(name[0], page);
      }
      if(strcmp(name, hdr.name) == 0) {
	return page;
      }
      page += hdr.max_pages;
    } else if(COFFEE_PAGE_ISOLATED(hdr)) {
      ++page;
    } else if(COFFEE_PAGE_OBSOLETE(hdr)) {
      page += hdr.max_pages;
    } else {
      /* It follows from the properties of the page allocation algorithm 
	 that if a free page is encountered, then the rest of the sector
	 is also free. */
      page = (page + COFFEE_PAGES_PER_SECTOR) & ~(COFFEE_PAGES_PER_SECTOR - 1);
    }
    watchdog_periodic();
  } while(page < COFFEE_PAGE_COUNT);
  
  return -1;
}
/*---------------------------------------------------------------------------*/
static coffee_offset_t
find_offset_in_file(int first_page)
{
  struct file_header hdr;
  unsigned char buf[COFFEE_PAGE_SIZE];
  int page;
  int i;
  int search_limit;
  coffee_offset_t range_start, range_end, part_size;

  read_header(&hdr, first_page);
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

  if(range_end > hdr.max_pages) {
    range_end = hdr.max_pages;
  }

  /*
   * Move from the end of the range towards the beginning and look for
   * a byte that has been modified.
   *
   * An important implication of this is that if the last written bytes
   * are zeroes, then these are skipped from the calculation.
   */
  
  for(page = first_page + range_end - 1; page >= first_page; page--) {
    watchdog_periodic();
    COFFEE_READ(buf, sizeof(buf), page * COFFEE_PAGE_SIZE);
    for(i = COFFEE_PAGE_SIZE - 1; i >= 0; i--) {
      if(buf[i] != 0) {
	if(page == first_page) {
	  return i < sizeof(hdr) ? 0 : 1 + i - sizeof(hdr);
	} else {
	  return 1 + i + (page - first_page) * COFFEE_PAGE_SIZE - sizeof(hdr);
	}
      }
    }
  }

  /* All bytes are writable. */
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
find_contiguous_pages(unsigned wanted)
{
  int page, start;
  struct file_header hdr;
  
  start = -1;
  for(page = 0; page < COFFEE_PAGE_COUNT;) {
    read_header(&hdr, page);
    if(COFFEE_PAGE_FREE(hdr)) {
      if(start == -1) {
	start = page;
      } else {
	if(start + wanted <= page) {
	  return start;
	}
      }
      /* Jump to the next sector. */
      page = (page + COFFEE_PAGES_PER_SECTOR) & ~(COFFEE_PAGES_PER_SECTOR - 1);
    } else if(COFFEE_PAGE_ISOLATED(hdr)) {
      ++page;
    } else {
      start = -1;
      page += hdr.max_pages;
    }
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int
cfs_garbage_collect(void)
{
  uint16_t sector;
  coffee_page_t active_pages, free_pages, obsolete_pages;
  int nerased;

  watchdog_stop();
  
  PRINTF("Coffee: Running the file system garbage collector...\n");
  /* 
   * The garbage collector erases as many sectors as possible. A sector is
   * erasable if there are only free or obsolete pages in it.
   */
  for(nerased = sector = 0; sector < COFFEE_SIZE / COFFEE_SECTOR_SIZE; sector++) {
    get_sector_status(sector, &active_pages, &free_pages, &obsolete_pages);
    PRINTF("Coffee: Sector %u has %u active, %u free, and %u obsolete pages.\n",
	sector, (unsigned)active_pages, (unsigned)free_pages, (unsigned)obsolete_pages);
    if(active_pages == 0 && obsolete_pages > 0) {
      COFFEE_ERASE(sector);
      nerased++;
      PRINTF("Coffee: Erased sector %d!\n", sector);
    }
  }

  watchdog_start();
  return nerased;
}
/*---------------------------------------------------------------------------*/
static int
remove_by_page(coffee_page_t page, int remove_log, int close_fds)
{
  struct file_header hdr;
  int i;
  coffee_page_t log_page;

  if(page >= COFFEE_PAGE_COUNT) {
    return -1;
  }
  
  read_header(&hdr, page);
  if(!COFFEE_PAGE_ACTIVE(hdr)) {
    return -1;
  }
  
  dir_cache_del(page);
  hdr.flags |= COFFEE_FLAG_OBSOLETE;
  write_header(&hdr, page);
  if(remove_log && COFFEE_PAGE_MODIFIED(hdr)) {
    log_page = hdr.log_page;
    dir_cache_del(log_page);
    read_header(&hdr, log_page);
    hdr.flags |= COFFEE_FLAG_OBSOLETE;
    write_header(&hdr, log_page);
  }
  
  /* Close all file descriptors that reference the remove file. */
  if(close_fds) {
    for(i = 0; i < COFFEE_FD_SET_SIZE; i++) {
      if(coffee_fd_set[i].file_page == page) {
	coffee_fd_set[i].flags = COFFEE_FD_FREE;
      }
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
reserve(const char *name, uint32_t size, int allow_duplicates)
{
  struct file_header hdr;
  unsigned need_pages;
  int page;

  if(!allow_duplicates && find_file(name) >= 0) {
    return -1;
  }

  need_pages = (size + sizeof(hdr) + COFFEE_PAGE_SIZE - 1) / COFFEE_PAGE_SIZE;

  watchdog_stop();
  page = find_contiguous_pages(need_pages);
  if(page < 0) {
    cfs_garbage_collect();
    page = find_contiguous_pages(need_pages);
    if(page < 0) {
      watchdog_start();
      return -1;
    }
  }

  memcpy(hdr.name, name, sizeof(hdr.name));
  hdr.name[sizeof(hdr.name) - 1] = '\0';
  hdr.max_pages = need_pages;
  hdr.flags = COFFEE_FLAG_ALLOCATED | COFFEE_FLAG_VALID;
  hdr.log_page = 0;
  hdr.eof_hint = 0;
  hdr.log_records = 0;
  hdr.log_record_size = 0;
  write_header(&hdr, page);

  PRINTF("Coffee: Reserved %u pages starting from %u for file %s\n",
      need_pages, page, name);
  dir_cache_add(name[0], page);

  watchdog_start();

  return page;
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
		  coffee_offset_t *offset, uint16_t *size)
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
get_record_index(coffee_page_t log_page, uint16_t record_count,
		 uint16_t search_records, uint16_t region)
{
  unsigned long base;
  uint16_t indices[record_count];
  uint16_t processed;
  uint16_t batch_size, i;
  int16_t match_index;

  base = ABS_OFFSET(log_page, sizeof(indices[0]) * search_records);
  processed = 0;
  match_index = -1;
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
  unsigned long base;
  uint16_t record_count;
  uint16_t search_records;

  adjust_log_config(hdr, &log_record_size, &log_records);
  region = modify_log_buffer(log_record_size, &lp->offset, &lp->size);

  search_records = last_record < 0 ? log_records : last_record + 1;
  record_count = search_records > COFFEE_LOG_TABLE_LIMIT ?
      		COFFEE_LOG_TABLE_LIMIT : search_records;
  
  match_index = get_record_index(hdr->log_page, record_count,
				 search_records, region);
  if(match_index < 0) {
    return -1;
  }

  base = hdr->log_page * COFFEE_PAGE_SIZE; 
  base += sizeof(struct file_header) + log_records * sizeof(region);
  base += (unsigned long)match_index * log_record_size;
  base += lp->offset;
  COFFEE_READ(lp->buf, lp->size, base);

  return lp->size;
}
/*---------------------------------------------------------------------------*/
static unsigned char *
create_log_name(unsigned char *new, int max_size, unsigned char *old)
{
  unsigned char suffix[] = ".log";
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
create_log(coffee_page_t file_page, struct file_header *hdr)
{
  coffee_page_t log_page;
  unsigned char log_name[sizeof(hdr->name)];
  uint16_t log_record_size, log_records;
  coffee_offset_t size;
  int i;

  adjust_log_config(hdr, &log_record_size, &log_records);  

  size = log_records * sizeof(uint16_t);	/* Log index size. */
  size += log_records * log_record_size;	/* Log data size. */

  log_page = reserve(create_log_name(log_name, sizeof(log_name), hdr->name),
	      size, 0);
  if(log_page < 0) {
    return -1;
  }

  hdr->flags |= COFFEE_FLAG_MODIFIED;
  hdr->log_page = log_page;
  write_header(hdr, file_page);
  read_header(hdr, log_page);
  hdr->flags |= COFFEE_FLAG_LOG;
  write_header(hdr, log_page);
  for(i = 0; i < COFFEE_FD_SET_SIZE; i++) {
    if(coffee_fd_set[i].file_page == file_page &&
       coffee_fd_set[i].flags != COFFEE_FD_FREE) {
      coffee_fd_set[i].flags |= COFFEE_FD_MODIFIED;
    }
  }
  return log_page;
}
/*---------------------------------------------------------------------------*/
static int
merge_log(coffee_page_t file_page)
{
  coffee_page_t log_page, new_file_page;
  struct file_header hdr, hdr2;
  int fd, n;
  coffee_offset_t offset;

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
  new_file_page = reserve(hdr.name,
			  hdr.max_pages * COFFEE_PAGE_SIZE - sizeof(hdr), 
			  1);
  if(new_file_page < 0) {
    cfs_close(fd);
    return -1;
  }

  offset = 0;
  do {
    char buf[hdr.log_record_size == 0 ? COFFEE_PAGE_SIZE : hdr.log_record_size];
    watchdog_periodic();
    n = cfs_read(fd, buf, sizeof(buf));
    if(n < 0) {
      remove_by_page(new_file_page, 0, 0);
      cfs_close(fd);
      return -1;
    } else if(n > 0) {
      COFFEE_WRITE(buf, n,
  	ABS_OFFSET(new_file_page, offset));
      offset += n;
    }
  } while(n != 0);
  
  if(remove_by_page(file_page, 1, 0) < 0) {
    remove_by_page(new_file_page, 0, 0);
    cfs_close(fd);
    return -1;
  }

  /* Copy the log configuration and the EOF hint. */
  read_header(&hdr2, new_file_page);
  hdr2.log_record_size = hdr.log_record_size;
  hdr2.log_records = hdr.log_records;
  hdr2.eof_hint = hdr.eof_hint;
  write_header(&hdr2, new_file_page);

  /* Point the file descriptors to the new file page. */
  for(n = 0; n < COFFEE_FD_SET_SIZE; n++) {
    if(coffee_fd_set[n].file_page == file_page) {
	coffee_fd_set[n].file_page = new_file_page;
	coffee_fd_set[n].flags &= ~COFFEE_FD_MODIFIED;
	coffee_fd_set[n].next_log_record = 0;
    }
  }

  cfs_close(fd);

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
write_log_page(struct file_desc *fdp, struct log_param *lp)
{
  struct file_header hdr;
  uint16_t region;
  coffee_page_t log_page;
  int16_t log_record;
  uint16_t log_record_size;
  uint16_t log_records;
  int16_t i;
  unsigned long base;
  struct log_param lp_out;
  uint16_t record_count;

  read_header(&hdr, fdp->file_page);

  adjust_log_config(&hdr, &log_record_size, &log_records);
  region = modify_log_buffer(log_record_size, &lp->offset, &lp->size);

  log_page = 0;
  if(COFFEE_PAGE_MODIFIED(hdr)) {
    /* A log structure has already been created. */
    record_count = log_records > COFFEE_LOG_TABLE_LIMIT ?
          		COFFEE_LOG_TABLE_LIMIT : log_records;
    log_page = hdr.log_page;

    if(fdp->next_log_record == 0) {
      /* The next log record is unknown. Search for it. */
      uint16_t indices[record_count];
      uint16_t processed;
      uint16_t batch_size;

      log_record = log_records;
      for(processed = 0; processed < log_records;) {
	batch_size = log_records - processed >= record_count ?
	    record_count : log_records - processed;
	COFFEE_READ(&indices, batch_size * sizeof(indices[0]),
	    ABS_OFFSET(log_page, processed * sizeof(indices[0])));
	for(i = 0; i < batch_size && indices[i] != 0; i++);
	log_record = i;
	if(log_record < batch_size) {
	  log_record += processed;
	  break;
	}
	processed += batch_size;
      } 
    } else {
      log_record = fdp->next_log_record;
    }

    if(log_record >= log_records) {
      /* The log is full; merge the log. */
      PRINTF("Coffee: Merging the file %s with its log\n", hdr.name);
      return merge_log(fdp->file_page);
    }
  } else {
    /* Create a log structure. */
    log_page = create_log(fdp->file_page, &hdr);
    if(log_page < 0) {
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
	read_log_page(&hdr, fdp->next_log_record - 1, &lp_out) < 0) {
      COFFEE_READ(copy_buf, sizeof(copy_buf),
	  ABS_OFFSET(fdp->file_page, region * log_record_size));
    }
 
    memcpy((char *) &copy_buf + lp->offset, lp->buf, lp->size);
    
    base = (unsigned long)log_page * COFFEE_PAGE_SIZE;
    base += sizeof(hdr);
    base += log_records * sizeof(region);
    base += (unsigned long)log_record * log_record_size;

    COFFEE_WRITE(copy_buf, sizeof(copy_buf), base);
    ++region;
    COFFEE_WRITE(&region, sizeof(region),
	ABS_OFFSET(log_page, log_record * sizeof(region)));
    fdp->next_log_record = log_record + 1;
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
  int page;
  struct file_header hdr;
  struct file_desc *fdp;

  fd = get_available_fd();
  if(fd < 0) {
    PRINTF("Coffee: Failed to allocate a new file descriptor!\n");
    return -1;
  }
  fdp = &coffee_fd_set[fd];
  
  page = find_file(name);
  if(page < 0) {
    if((flags & (CFS_READ | CFS_WRITE)) == CFS_READ) {
      return -1;
    }
    if((page = reserve(name, COFFEE_DYN_SIZE, 1)) < 0) {
	return -1;
    }
    fdp->max_pages = (COFFEE_DYN_SIZE + sizeof(hdr) + 
                      COFFEE_PAGE_SIZE - 1) / COFFEE_PAGE_SIZE;
  } else {
    read_header(&hdr, page);
    if(COFFEE_PAGE_MODIFIED(hdr)) {
      fdp->flags = COFFEE_FD_MODIFIED;
    } else
    fdp->max_pages = hdr.max_pages;
  }

  fdp->file_page = page;
  fdp->flags |= flags;
  fdp->end = find_offset_in_file(page);
  fdp->offset = flags & CFS_APPEND ? fdp->end : 0;
  fdp->next_log_record = 0;

  return fd;
}
/*---------------------------------------------------------------------------*/
void
cfs_close(int fd)
{
  struct file_header hdr;
  coffee_page_t current_page;
  int part_size, i;
  uint8_t eof_hint;

  if(FD_VALID(fd)) {
    read_header(&hdr, coffee_fd_set[fd].file_page);
    current_page = (coffee_fd_set[fd].end + sizeof(hdr)) / COFFEE_PAGE_SIZE;
    part_size = hdr.max_pages / (sizeof(hdr.eof_hint) * CHAR_BIT);
    if(part_size == 0) {
      part_size = 1;
    }
    for(i = eof_hint = 0; i < sizeof(eof_hint) * CHAR_BIT; i++) {
      eof_hint |= (current_page >= (i + 1) * part_size) << i;
    }

    if(eof_hint > hdr.eof_hint) {
      hdr.eof_hint |= eof_hint;
      write_header(&hdr, coffee_fd_set[fd].file_page);
    }

    coffee_fd_set[fd].flags = COFFEE_FD_FREE;
  }
}
/*---------------------------------------------------------------------------*/
unsigned
cfs_seek(int fd, unsigned offset)
{
  struct file_header hdr;

  if(!FD_VALID(fd)) {
    return -1;
  }
  read_header(&hdr, coffee_fd_set[fd].file_page);

  /* Check if the offset is within the file boundary. */
  if(sizeof(hdr) + offset >= hdr.max_pages * COFFEE_PAGE_SIZE ||
     sizeof(hdr) + offset < offset) {
    return -1;
  }

  if(coffee_fd_set[fd].end < offset) {
    coffee_fd_set[fd].end = offset;
  }
  
  return coffee_fd_set[fd].offset = offset;
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_remove(const char *name)
{
  int page;

  page = find_file(name);
  if(page < 0) {
    return -1;
  }

  return remove_by_page(page, 1, 1);
}
/*---------------------------------------------------------------------------*/
int
cfs_read(int fd, void *buf, unsigned size)
{
  struct file_header hdr;
  struct file_desc *fdp;
  unsigned remains, read_chunk;
  int r;
  coffee_offset_t base, offset;
  struct log_param lp;
  
  if(!(FD_VALID(fd) && FD_READABLE(fd))) {
    return -1;
  }

  fdp = &coffee_fd_set[fd];
  if(fdp->offset + size > fdp->end) {
    size = fdp->end - fdp->offset;
  }

  read_chunk = COFFEE_PAGE_SIZE;
  if(FD_MODIFIED(fd)) {
    read_header(&hdr, fdp->file_page);
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
    if(FD_MODIFIED(fd)) {
      lp.offset = base + offset;
      lp.buf = (char *)buf + offset;
      lp.size = remains;
      r = read_log_page(&hdr, fdp->next_log_record - 1, &lp);
    }
    /* Read from the original file if we cannot find the data in the log. */
    if(r < 0) {
      r = remains > read_chunk ? read_chunk : remains;
      COFFEE_READ((char *) buf + offset, r,
	ABS_OFFSET(fdp->file_page, base + offset));
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
  int i;
  struct log_param lp;
  coffee_offset_t remains;

  if(!(FD_VALID(fd) && FD_WRITABLE(fd))) {
    return -1;
  }
  
  fdp = &coffee_fd_set[fd];

  if(size + fdp->offset + sizeof(struct file_header) >
     fdp->max_pages * COFFEE_PAGE_SIZE) {
    size = fdp->max_pages * COFFEE_PAGE_SIZE -
	   fdp->offset - sizeof(struct file_header);
  }
  
  if(fdp->offset < fdp->end) {
    remains = size;
    while(remains) {
      lp.offset = fdp->offset;
      lp.buf = (char *)buf + size - remains;
      lp.size = remains;

      i = write_log_page(fdp, &lp);
      if(i == 0) {
        /* The file was merged with the log. Try again. */
        i = write_log_page(fdp, &lp);
      }
      if(i < 0) {
        return size - remains > 0 ? size - remains : -1;
      }
      remains -= i;
      fdp->offset += i;
    }
  } else {
    COFFEE_WRITE(buf, size,
	ABS_OFFSET(fdp->file_page, fdp->offset));
    fdp->offset += size;
  }

  if(fdp->offset > fdp->end) {
    /*
     * The file has been extended and the file descriptors
     * referencing this file must be updated.
     */
    for(i = 0; i < COFFEE_FD_SET_SIZE; i++) {
      if(coffee_fd_set[i].file_page == fdp->file_page) {
	coffee_fd_set[i].end = coffee_fd_set[i].offset;
      }
    }
  }

  return size;
}
/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *dir, const char *name)
{  
  /* We have only a root directory. */
  if(name[0] != '/' || name[1] != '\0') {
    return -1;
  }

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
    if(COFFEE_PAGE_FREE(hdr)) {
      page = (page + COFFEE_PAGES_PER_SECTOR) & ~(COFFEE_PAGES_PER_SECTOR - 1);
    } else if(COFFEE_PAGE_ISOLATED(hdr)) {
      ++page;
    } else if(COFFEE_PAGE_ACTIVE(hdr) && !COFFEE_PAGE_LOG(hdr)) {
	memcpy(record->name, hdr.name, sizeof(record->name));
	record->name[sizeof(record->name) - 1] = '\0';
	record->size = find_offset_in_file(page);
        page += hdr.max_pages;
	*(coffee_page_t *)dir->dummy_space = page;
	return 0;
    } else {
      page += hdr.max_pages;
    }
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
cfs_coffee_reserve(const char *name, uint32_t size)
{
  return reserve(name, size, 0);
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_configure_log(const char *file, unsigned log_size,
			 unsigned log_record_size)
{
  coffee_page_t page;
  struct file_header hdr;

  if(log_record_size == 0 || log_record_size > COFFEE_PAGE_SIZE ||
     log_size < log_record_size) {
    return -1;
  }

  page = find_file(file);
  if(page < 0) {
    return -1;
  }

  read_header(&hdr, page);
  if(COFFEE_PAGE_MODIFIED(hdr)) {
    /* Too late to customize the log. */
    return -1;
  }

  hdr.log_records = log_size / log_record_size;
  hdr.log_record_size = log_record_size;
  write_header(&hdr, page);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_format(void)
{
  int nsectors, i;

  nsectors = COFFEE_SIZE / COFFEE_SECTOR_SIZE;
  /* Reject format requests when the configuration is incorrect. */
  if(nsectors < 1 || (COFFEE_START & (COFFEE_SECTOR_SIZE - 1))) {
    return -1;
  }

  PRINTF("Coffee: Formatting %d sectors", nsectors);

  watchdog_stop();
  for(i = 0; i < nsectors; i++) {
    COFFEE_ERASE(i);
    PRINTF(".");
  }
  watchdog_start();

  /* All file descriptors have become invalid. */
  for(i = 0; i < COFFEE_FD_SET_SIZE; i++) {
    coffee_fd_set[i].flags = COFFEE_FD_FREE;
  }

  PRINTF("done!\n");

  return 0;
}
