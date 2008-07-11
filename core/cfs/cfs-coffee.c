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
 *	Coffee: A flash file system on memory-contrained sensor systems.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki-conf.h"
#include "cfs/cfs.h"
#include "cfs-coffee-arch.h"
#include "cfs/cfs-coffee.h"
#include "dev/watchdog.h"

#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define kb			* 1024UL
#define Mb			* (1024 kb)

#define COFFEE_FD_READ		0x1
#define COFFEE_FD_WRITE		0x2
#define COFFEE_FD_APPEND	0x4
#define COFFEE_FD_FREE		0x0

struct file_desc {
  uint32_t offset;
  uint32_t end;
  uint16_t file_page;
  uint8_t flags;
  uint16_t max_pages;
  uint16_t next_log_entry;
};

#define COFFEE_FLAG_ALLOCATED	0x1
#define COFFEE_FLAG_OBSOLETE	0x2
#define COFFEE_FLAG_MODIFIED	0x4
#define COFFEE_FLAG_LOG		0x8

#define LOG_CMD_MAGIC		0x7a

#define COFFEE_PAGE_ALLOCATED(hdr)	((hdr).flags & COFFEE_FLAG_ALLOCATED)
#define COFFEE_PAGE_FREE(hdr)		!COFFEE_PAGE_ALLOCATED(hdr)
#define COFFEE_PAGE_OBSOLETE(hdr)	((hdr).flags & COFFEE_FLAG_OBSOLETE)
#define COFFEE_PAGE_ACTIVE(hdr)		\
		(COFFEE_PAGE_ALLOCATED(hdr) && !COFFEE_PAGE_OBSOLETE(hdr))
#define COFFEE_PAGE_LOG(hdr)		((hdr).flags & COFFEE_FLAG_LOG)
#define COFFEE_PAGE_MODIFIED(hdr)	((hdr).flags & COFFEE_FLAG_MODIFIED)

#define FD_VALID(fd)				\
	((fd) < COFFEE_FD_SET_SIZE && coffee_fd_set[(fd)].flags != COFFEE_FD_FREE)
#define FD_READABLE(fd)		(coffee_fd_set[(fd)].flags & CFS_READ)
#define FD_WRITABLE(fd)		(coffee_fd_set[(fd)].flags & CFS_WRITE)
#define FD_APPENDABLE(fd)	(coffee_fd_set[(fd)].flags & CFS_APPEND)

#define READ_HEADER(hdr, page)			\
  COFFEE_READ((hdr), sizeof (*hdr), (page) * COFFEE_PAGE_SIZE)

#define WRITE_HEADER(hdr, page)			\
  COFFEE_WRITE((hdr), sizeof (*hdr), (page) * COFFEE_PAGE_SIZE)

struct dir_cache {
  char filename_start;
  int32_t page;
};

struct file_header {
  unsigned flags:4;
  unsigned max_pages:12;
  unsigned log_page:16;
  unsigned eof_locator:16;
  unsigned log_entries:16;
  unsigned log_entry_size:16;
  char name[COFFEE_NAME_LENGTH];
} __attribute__((packed));

/* This is needed because of a buggy compiler. */
struct log_param {
  uint32_t offset;
  const char *buf;
  uint16_t size;
};

#define ABS_OFFSET(file_page, file_offset)	\
	((file_page) * COFFEE_PAGE_SIZE + 	\
	sizeof (struct file_header) + (file_offset))

static struct dir_cache dir_cache[COFFEE_DIR_CACHE_ENTRIES];
static struct file_desc coffee_fd_set[COFFEE_FD_SET_SIZE];

/*---------------------------------------------------------------------------*/
static void
get_sector_status(uint16_t sector, uint16_t *active,
		  uint16_t *free, uint16_t *obsolete) {
  uint32_t offset;
  uint32_t end;
  struct file_header hdr;
  static uint16_t skip_pages;
  static int last_pages_are_active;
  
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
  
  if(last_pages_are_active) {
    *active = skip_pages;
  } else {
    *obsolete = skip_pages;
  }
  offset = sector * COFFEE_SECTOR_SIZE + skip_pages * COFFEE_PAGE_SIZE;
  end = (sector + 1) * COFFEE_SECTOR_SIZE;
 
  while(offset < end) {
    COFFEE_READ(&hdr, sizeof (hdr), offset);
    if(COFFEE_PAGE_ACTIVE(hdr)) {
      offset += hdr.max_pages * COFFEE_PAGE_SIZE;
      last_pages_are_active = 1;
      *active += hdr.max_pages;
    } else if(COFFEE_PAGE_OBSOLETE(hdr)) {
      last_pages_are_active = 0;
      offset += hdr.max_pages * COFFEE_PAGE_SIZE;
      *obsolete += hdr.max_pages;
    } else if(COFFEE_PAGE_FREE(hdr)) {
      ++*free;
      offset += COFFEE_PAGE_SIZE;
    }
  }
  
  skip_pages = *active + *free + *obsolete - COFFEE_PAGES_PER_SECTOR;
  if(skip_pages > 0) {
    if(last_pages_are_active) {
      *active = COFFEE_PAGES_PER_SECTOR - *free - *obsolete;
    } else {
      *obsolete = COFFEE_PAGES_PER_SECTOR - *free - *active;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
dir_cache_add(char c, int32_t page)
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
dir_cache_del(int32_t page)
{
  int i;
  
  for(i = 0; i < COFFEE_DIR_CACHE_ENTRIES; i++) {
    if(dir_cache[i].page == page) {
      dir_cache[i].page = -1;
      dir_cache[i].filename_start = '\0';
    } 
  }
}
/*---------------------------------------------------------------------------*/
static int32_t
dir_cache_find(const char *name)
{
  int i;
  struct file_header hdr;
  
  for(i = 0; i < COFFEE_DIR_CACHE_ENTRIES; i++) {
    if(*name == dir_cache[i].filename_start) {
      READ_HEADER(&hdr, dir_cache[i].page);
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
static int32_t
find_file(const char *name)
{
  struct file_header hdr;
  int32_t page;

  page = dir_cache_find(name);
  if(page >= 0) {
    return page;
  }
  
  page = 0;
  do {
    READ_HEADER(&hdr, page);
    if(COFFEE_PAGE_ACTIVE(hdr)) {
      if(strcmp(name, hdr.name) == 0) {
	dir_cache_add(name[0], page);
	return page;
      }
      page += hdr.max_pages;
    } else if(COFFEE_PAGE_OBSOLETE(hdr)) {
      page += hdr.max_pages;
    } else {
      /* It follows from the properties of the page allocation algorithm 
	 that if a free page is encountered, the rest of the sector is 
	 also free. */
      page = (page + COFFEE_PAGES_PER_SECTOR) & ~(COFFEE_PAGES_PER_SECTOR - 1);
    }
    watchdog_periodic();
  } while(page < COFFEE_PAGE_COUNT);
  
  return -1;
}
/*---------------------------------------------------------------------------*/
static int32_t
find_offset_in_file(int first_page)
{
  struct file_header hdr;
  unsigned char buf[COFFEE_PAGE_SIZE];
  int page;
  int i;
  
  READ_HEADER(&hdr, first_page);
  
  /* 
   * Move from the end of the file towards the beginning and look for
   * a byte that has been modified.
   *
   * One important implication of this is that if the last written bytes
   * are zeroes, then these are skipped from the calculation.
   * 
   */  
  for(page = first_page + hdr.max_pages - 1; page >= first_page; page--) {
    watchdog_periodic();
    COFFEE_READ(buf, sizeof (buf), page * COFFEE_PAGE_SIZE);
    for(i = COFFEE_PAGE_SIZE - 1; i >= 0; i--) {
      if(buf[i] != 0) {
	if(page == first_page) {
	  return i < sizeof (hdr) ? 0 : 1 + i - sizeof (hdr);
	} else {
	  return 1 + i + (page - first_page) * COFFEE_PAGE_SIZE - sizeof (hdr);
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
    READ_HEADER(&hdr, page);
    if(COFFEE_PAGE_FREE(hdr)) {
      if(start == -1) {
	start = page;
      } else {
	if(start + wanted <= page) {
	  return start;
	}
      }
      page++;
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
  uint16_t active_pages, free_pages, obsolete_pages;
  uint8_t sectors_in_row, longest_row;
  
  watchdog_stop();
  
  PRINTF("Coffee: Running the file system garbage collector...\n");
  
  sectors_in_row = longest_row = 0;
  /* 
   * The garbage collector erases as many sectors as possible. A sector is
   * erasable if there are only free or obsolete pages in it.
   */
  for(sector = 0; sector < COFFEE_SIZE / COFFEE_SECTOR_SIZE; sector++) {
    get_sector_status(sector, &active_pages, &free_pages, &obsolete_pages);
    PRINTF("Coffee: Sector %u has %u active, %u free, and %u obsolete pages.\n",
	sector, active_pages, free_pages, obsolete_pages);
    if(active_pages == 0 && obsolete_pages > 0) {
      COFFEE_ERASE(sector);
      PRINTF("Coffee: Erased sector %d!\n", sector);
      ++sectors_in_row;
    } else {
      if(sectors_in_row > longest_row) {
	longest_row = sectors_in_row;
      }
      sectors_in_row = 0;
    }
  }

  if(sectors_in_row > longest_row) {
    longest_row = sectors_in_row;
  }

  watchdog_start();
  
  return longest_row * (COFFEE_PAGES_PER_SECTOR);
}
/*---------------------------------------------------------------------------*/
static int
remove_by_page(uint16_t page, int remove_log, int close_fds)
{
  struct file_header hdr;
  int i;
  uint16_t log_page;
  
  if(page >= COFFEE_PAGE_COUNT) {
    return -1;
  }
  
  READ_HEADER(&hdr, page);
  if(!COFFEE_PAGE_ACTIVE(hdr)) {
    return -1;
  }
  
  dir_cache_del(page);
  hdr.flags |= COFFEE_FLAG_OBSOLETE;
  WRITE_HEADER(&hdr, page);
  if(remove_log && COFFEE_PAGE_MODIFIED(hdr)) {
    log_page = hdr.log_page;
    dir_cache_del(log_page);
    READ_HEADER(&hdr, log_page);
    hdr.flags |= COFFEE_FLAG_OBSOLETE;
    WRITE_HEADER(&hdr, log_page);
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
read_log_page(struct file_header *hdr, int16_t last_entry, struct log_param *lp)
{
  uint16_t page;
  int16_t match_index;
  int16_t i;
  uint16_t log_entry_size;
  uint16_t log_entries;
  unsigned long base;
  uint16_t entry_count;
  uint16_t search_entries;

  log_entries = hdr->log_entries == 0 ?
	COFFEE_LOG_SIZE / COFFEE_PAGE_SIZE : hdr->log_entries;
  log_entry_size = hdr->log_entry_size == 0 ?
	COFFEE_PAGE_SIZE : hdr->log_entry_size;

  page = lp->offset / log_entry_size;
  lp->offset %= log_entry_size;
  if(lp->size > log_entry_size - lp->offset) {
    lp->size = log_entry_size - lp->offset;
  }

  search_entries = last_entry < 0 ? log_entries : last_entry + 1;
  entry_count = search_entries > COFFEE_LOG_TABLE_LIMIT ?
      		COFFEE_LOG_TABLE_LIMIT : search_entries;
  
  {
    uint16_t indices[entry_count];
    uint16_t processed;
    uint16_t current_batch_size;

    base = ABS_OFFSET(hdr->log_page, sizeof (indices[0]) * search_entries);
    processed = 0;
    match_index = -1;
    while(processed < search_entries && match_index < 0) {
      if(entry_count + processed > search_entries) {
	current_batch_size = search_entries - processed;
      } else {
	current_batch_size = entry_count;
      }

      base -= current_batch_size * sizeof (indices[0]);
      COFFEE_READ(&indices, sizeof (indices[0]) * current_batch_size, base);
      
      for(i = current_batch_size - 1; i >= 0; i--) {
	if(indices[i] - 1 == page) {
	  match_index = search_entries - processed - (current_batch_size - i);
	  break;
	}
      }
      
      processed += current_batch_size;
    }
    if(match_index == -1) {
      return -1;
    }

    base = hdr->log_page * COFFEE_PAGE_SIZE;
    base += sizeof (struct file_header) + log_entries * sizeof (page);
    base += (unsigned long)match_index * log_entry_size;
    base += lp->offset;

    COFFEE_READ(lp->buf, lp->size, base);
  }

  return lp->size;
}
/*---------------------------------------------------------------------------*/
static unsigned char *
create_log_name(unsigned char *new, int max_size, unsigned char *old)
{
  unsigned char suffix[] = ".log";
  int len;

  len = strlen(old);
  if(len > max_size - sizeof (suffix)) {
    len = max_size - sizeof (suffix);
  }
  memcpy(new, old, len);
  memcpy(&new[len], suffix, sizeof (suffix));
  
  return new;
}
/*---------------------------------------------------------------------------*/
static int16_t
create_log(int16_t file_page, struct file_header *hdr)
{
  int16_t log_page;
  unsigned char log_name[sizeof (hdr->name)];
  uint16_t log_entry_size, log_entries;
  uint32_t size;
  
  log_entry_size = hdr->log_entry_size == 0 ?
	COFFEE_PAGE_SIZE : hdr->log_entry_size;
  log_entries = hdr->log_entries == 0 ?
	COFFEE_LOG_SIZE / log_entry_size : hdr->log_entries;
  
  size = log_entries * sizeof (uint16_t);
  size += log_entries * log_entry_size;
  log_page = cfs_coffee_reserve(create_log_name(log_name, sizeof (log_name), hdr->name),
      size);
  if(log_page < 0) {
    return -1;
  }
  hdr->flags |= COFFEE_FLAG_MODIFIED;
  hdr->log_page = log_page;
  WRITE_HEADER(hdr, file_page);
  READ_HEADER(hdr, log_page);
  hdr->flags |= COFFEE_FLAG_LOG;
  WRITE_HEADER(hdr, log_page);
  return log_page;
}
/*---------------------------------------------------------------------------*/
static int
flush_log(uint16_t file_page)
{
  int16_t log_page, new_file_page;
  struct file_header hdr, hdr2;
  int fd, n;
  char buf[64];
  int32_t offset;
  
  READ_HEADER(&hdr, file_page);
  log_page = hdr.log_page;
  fd = cfs_open(hdr.name, CFS_READ);
  if(fd < 0) {
    return -1;
  }
  
  new_file_page = cfs_coffee_reserve(hdr.name,
			hdr.max_pages * COFFEE_PAGE_SIZE);
  if(new_file_page < 0) {
    return -1;
  }

  offset = 0;
  do {
    watchdog_periodic();
    n = cfs_read(fd, buf, sizeof (buf));
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
  
  /* Copy the log configuration. */
  READ_HEADER(&hdr2, new_file_page);
  hdr2.log_entry_size = hdr.log_entry_size;
  hdr2.log_entries = hdr.log_entries;
  WRITE_HEADER(&hdr2, new_file_page);
  
  /* Point the file descriptors to the new file page. */
  for(n = 0; n < COFFEE_FD_SET_SIZE; n++) {
    if(coffee_fd_set[n].file_page == file_page) {
	coffee_fd_set[n].file_page = new_file_page;
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
  uint16_t page;
  int log_page;
  int16_t log_entry;
  uint16_t log_entry_size;
  uint16_t log_entries;
  int16_t i;
  unsigned long base;
  struct log_param lp_out;
  uint16_t entry_count;

  READ_HEADER(&hdr, fdp->file_page);

  log_entry_size = hdr.log_entry_size == 0 ?
	COFFEE_PAGE_SIZE : hdr.log_entry_size;
  log_entries = hdr.log_entries == 0 ?
	COFFEE_LOG_SIZE / COFFEE_PAGE_SIZE : hdr.log_entries;

  page = lp->offset / log_entry_size;
  lp->offset %= log_entry_size;
  if(lp->size > log_entry_size - lp->offset) {
    lp->size = log_entry_size - lp->offset;
  }

  log_page = 0;

  if(COFFEE_PAGE_MODIFIED(hdr)) {
    /* A log structure has already been created. */
    entry_count = log_entries > COFFEE_LOG_TABLE_LIMIT ?
          		COFFEE_LOG_TABLE_LIMIT : log_entries;
    log_page = hdr.log_page;

    if(fdp->next_log_entry == 0) {
      /* The next log entry is unknown. Search for it. */
      uint16_t indices[entry_count];
      uint16_t processed;
      uint16_t current_batch_size;

      log_entry = log_entries;
      for(processed = 0; processed < log_entries;) {
	current_batch_size = log_entries - processed >= entry_count ?
	    entry_count : log_entries - processed;
	COFFEE_READ(&indices, current_batch_size * sizeof (indices[0]),
	    ABS_OFFSET(log_page, processed * sizeof (indices[0])));
	for(i = 0; i < current_batch_size && indices[i] != 0; i++);
	log_entry = i;
	if(log_entry < current_batch_size) {
	  log_entry += processed;
	  break;
	}
	processed += current_batch_size;
      } 
    } else {
      log_entry = fdp->next_log_entry;
    }

    if(log_entry >= log_entries) {
      /* The log is full. The new file should be written out. */
      PRINTF("Coffee: The log for file %s is full\n", hdr.name);
      fdp->next_log_entry = 0;
      return flush_log(fdp->file_page);
    }
  } else {
    /* Create a log structure. */
    log_page = create_log(fdp->file_page, &hdr);
    if(log_page < 0) {
      return -1;
    }
    PRINTF("Coffee: Created a log structure for file %s at page %u\n",
    	hdr.name, log_page);
    hdr.log_page = log_page;
    log_entry = 0;
  }

  {
    unsigned char copy_buf[log_entry_size];
    
    lp_out.offset = page * log_entry_size;
    lp_out.buf = copy_buf;
    lp_out.size = log_entry_size;

    if(lp->offset > 0 &&
	read_log_page(&hdr, fdp->next_log_entry > 0 ? fdp->next_log_entry - 1 : -1, &lp_out) < 0) {
      COFFEE_READ(copy_buf, sizeof (copy_buf),
	  ABS_OFFSET(fdp->file_page, page * log_entry_size));
    }
 
    memcpy((char *) &copy_buf + lp->offset, lp->buf, lp->size);
    
    base = (unsigned long)log_page * COFFEE_PAGE_SIZE;
    base += sizeof (hdr);
    base += log_entries * sizeof (page);
    base += (unsigned long)log_entry * log_entry_size;

    COFFEE_WRITE(copy_buf, sizeof (copy_buf), base);
    ++page;
    COFFEE_WRITE(&page, sizeof (page),
	ABS_OFFSET(log_page, log_entry * sizeof (page)));
    fdp->next_log_entry = log_entry + 1;
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

  fd = get_available_fd();
  if(fd < 0) {
    return -1;
  }
  
  page = find_file(name);
  if(page < 0) {
    if((flags & (CFS_READ | CFS_WRITE)) == CFS_READ) {
      return -1;
    }
    if((page = cfs_coffee_reserve(name, COFFEE_DYN_SIZE)) < 0) {
	return -1;
    }
    coffee_fd_set[fd].max_pages = COFFEE_DYN_SIZE / COFFEE_PAGE_SIZE;
  } else {
    READ_HEADER(&hdr, page);
    coffee_fd_set[fd].max_pages = hdr.max_pages;
  }

  coffee_fd_set[fd].file_page = page;
  coffee_fd_set[fd].flags = flags;
  coffee_fd_set[fd].end = find_offset_in_file(page);
  coffee_fd_set[fd].offset = flags & CFS_APPEND ? coffee_fd_set[fd].end : 0;
  coffee_fd_set[fd].next_log_entry = 0;

  return fd;
}
/*---------------------------------------------------------------------------*/
void
cfs_close(int fd)
{
  if(FD_VALID(fd)) {
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

  READ_HEADER(&hdr, coffee_fd_set[fd].file_page);
  if(sizeof (hdr) + offset >= hdr.max_pages * COFFEE_PAGE_SIZE) {
    /* XXX: Try to extend the file here? */
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
  unsigned remains;
  int r;
  int32_t base, offset;
  struct log_param lp;
  
  if(!FD_VALID(fd) || !FD_READABLE(fd)) {
    return -1;
  }

  fdp = &coffee_fd_set[fd];
  if(fdp->offset + size > fdp->end) {
    size = fdp->end - fdp->offset;
  }

  READ_HEADER(&hdr, fdp->file_page);
  
  remains = size;
  base = fdp->offset;
  offset = 0;
  /*
   * Fill the buffer by copying from the log in first hand, or the 
   * ordinary file if the page has no log entry.
   */
  while(remains) {
    watchdog_periodic();
    r = -1;
    if(COFFEE_PAGE_MODIFIED(hdr)) {
      lp.offset = base + offset;
      lp.buf = (char *)buf + offset;
      lp.size = remains;
      r = read_log_page(&hdr,
		fdp->next_log_entry > 0 ? fdp->next_log_entry - 1 : -1, &lp);
      if(r >= 0) {
	offset += r;
	break;
      }
    }
    if(r < 0) {
      r = remains > COFFEE_PAGE_SIZE ? COFFEE_PAGE_SIZE : remains;
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

  if(!FD_VALID(fd) || !FD_WRITABLE(fd)) {
    return -1;
  }
  
  fdp = &coffee_fd_set[fd];

  if(size + fdp->offset + sizeof (struct file_header) >
     fdp->max_pages * COFFEE_PAGE_SIZE) {
    size = fdp->max_pages * COFFEE_PAGE_SIZE -
	   fdp->offset - sizeof (struct file_header);
  }
  
  if(fdp->offset < fdp->end) {
    lp.offset = fdp->offset;
    lp.buf = buf;
    lp.size = size;

    size = write_log_page(fdp, &lp);
    if(size == 0) {
      /* The log got flushed. Try again. */
      size = write_log_page(fdp, &lp);
    }
    if(size < 0) {
      return -1;
    }
  } else {
    COFFEE_WRITE(buf, size,
	ABS_OFFSET(fdp->file_page, fdp->offset));
  }
  fdp->offset += size;
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

  *(uint16_t *)dir->dummy_space = 0;
  
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *dir, struct cfs_dirent *entry)
{
  struct file_header hdr;
  uint16_t page;
  
  for(page = *(uint16_t *)dir->dummy_space; page < COFFEE_PAGE_COUNT;) {
    watchdog_periodic();
    READ_HEADER(&hdr, page);
    if(COFFEE_PAGE_ACTIVE(hdr) && !COFFEE_PAGE_LOG(hdr)) {
      memcpy(entry->name, hdr.name, sizeof (entry->name));
      entry->name[sizeof (entry->name) - 1] = '\0';
      entry->size = find_offset_in_file(page);
      page += hdr.max_pages;
       *(uint16_t *)dir->dummy_space = page;
      return 0;
    } else if (COFFEE_PAGE_FREE(hdr)) {
      page = (page + COFFEE_PAGES_PER_SECTOR) & ~(COFFEE_PAGES_PER_SECTOR - 1);
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
  struct file_header hdr;
  unsigned need_pages;
  int page;

  need_pages = (size + sizeof (hdr) + COFFEE_PAGE_SIZE - 1) / COFFEE_PAGE_SIZE;

  watchdog_stop();
  page = find_contiguous_pages(need_pages);
  watchdog_start();
  if(page < 0) {
    cfs_garbage_collect();
    watchdog_stop();
    page = find_contiguous_pages(need_pages);
    watchdog_start();
    if(page < 0) {
      return -1;
    }
  }
  
  memcpy(hdr.name, name, sizeof (hdr.name));
  hdr.name[sizeof (hdr.name) - 1] = '\0';
  hdr.max_pages = need_pages;
  hdr.flags = COFFEE_FLAG_ALLOCATED;
  hdr.log_page = 0;
  hdr.eof_locator = 0;
  hdr.log_entries = 0;
  hdr.log_entry_size = 0;
  WRITE_HEADER(&hdr, page);

  PRINTF("Coffee: Reserved %u pages starting from %u for file %s\n",
      need_pages, page, name);
  dir_cache_add(name[0], page);

  return page;
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_configure_log(const char *file, unsigned log_size, unsigned log_entry_size)
{
  int16_t page;
  struct file_header hdr;
 
  page = find_file(file);
  if(page < 0) {
    return -1;
  }
  
  READ_HEADER(&hdr, page);
  if(COFFEE_PAGE_MODIFIED(hdr)) {
    /* 
     * Too late to customize the log.
     * TODO: Flush the log and create a new log.
     * */
    return -1;
  }
  
  hdr.log_entries = log_size / log_entry_size;
  hdr.log_entry_size = log_entry_size;
  WRITE_HEADER(&hdr, page);
  
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_coffee_format(void)
{
  int nsectors, i;

  nsectors = COFFEE_SIZE / COFFEE_SECTOR_SIZE;
  /* Reject format requests when the configuration is incorrect. */
  if (nsectors < 1 || (COFFEE_START & (COFFEE_SECTOR_SIZE - 1))) {
    return -1;
  }

  PRINTF("Coffee: Formatting %d sectors", nsectors);

  watchdog_stop();
  for (i = 0; i < nsectors; i++) {
    COFFEE_ERASE(i);
    PRINTF(".");
  }
  watchdog_start();

  PRINTF("done!\n");

  return 0;
}
