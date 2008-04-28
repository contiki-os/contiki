#ifndef CFS_COFFEE_H
#define CFS_COFFEE_H

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
 *	Coffee main header.
 * \author
 * 	Nicolas Tsiftes <nvt@sics.se>
 */

#include "contiki-conf.h"
#include "cfs-coffee-arch.h"

#define kb			* 1024UL

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

#define CFS_PAGE_ALLOCATED(hdr)						\
  ((hdr).flags & COFFEE_FLAG_ALLOCATED)
#define CFS_PAGE_FREE(hdr)						\
  !CFS_PAGE_ALLOCATED(hdr)
#define CFS_PAGE_OBSOLETE(hdr)						\
  ((hdr).flags & COFFEE_FLAG_OBSOLETE)
#define CFS_PAGE_ACTIVE(hdr)						\
  (CFS_PAGE_ALLOCATED(hdr) && !CFS_PAGE_OBSOLETE(hdr))
#define CFS_PAGE_LOG(hdr)						\
  ((hdr).flags & COFFEE_FLAG_LOG)
#define CFS_PAGE_MODIFIED(hdr)						\
  ((hdr).flags & COFFEE_FLAG_MODIFIED)

#define FD_VALID(fd)							\
		((fd) < COFFEE_FD_SET_SIZE && fd_set[(fd)].flags != COFFEE_FD_FREE)
#define FD_READABLE(fd)							\
  (fd_set[(fd)].flags & CFS_READ)
#define FD_WRITABLE(fd)							\
  (fd_set[(fd)].flags & CFS_WRITE)
#define FD_APPENDABLE(fd)						\
  (fd_set[(fd)].flags & CFS_APPEND)

#define LOG_CMD_MAGIC		0x7a

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

#define ABS_OFFSET(file_page, file_offset)					\
		((file_page) * COFFEE_PAGE_SIZE + sizeof (struct file_header) + (file_offset))

int cfs_remove(const char *name);
int cfs_reserve(const char *name, uint32_t size);
int cfs_configure_log(const char *file, unsigned log_size,
	unsigned log_entry_size);

#endif /* !COFFEE_H */
