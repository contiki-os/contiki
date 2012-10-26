/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "cfs/cfs.h"
#include "dev/xmem.h"

struct filestate {
  int flag;
#define FLAG_FILE_CLOSED 0
#define FLAG_FILE_OPEN   1
  unsigned int fileptr;
  unsigned int filesize;
};

#ifdef CFS_XMEM_CONF_OFFSET
#define CFS_XMEM_OFFSET CFS_XMEM_CONF_OFFSET
#else
#define CFS_XMEM_OFFSET 0
#endif

/* Note the CFS_XMEM_CONF_SIZE must be a tuple of XMEM_ERASE_UNIT_SIZE */
#ifdef CFS_XMEM_CONF_SIZE
#define CFS_XMEM_SIZE CFS_XMEM_CONF_SIZE
#else
#define CFS_XMEM_SIZE XMEM_ERASE_UNIT_SIZE
#endif

static struct filestate file;

/*---------------------------------------------------------------------------*/
int
cfs_open(const char *n, int f)
{
  if(file.flag == FLAG_FILE_CLOSED) {
    file.flag = FLAG_FILE_OPEN;
    if(f & CFS_READ) {
      file.fileptr = 0;
    }
    if(f & CFS_WRITE){
      if(f & CFS_APPEND) {
	file.fileptr = file.filesize;
      } else {
	file.fileptr = 0;
	file.filesize = 0;
	xmem_erase(CFS_XMEM_SIZE, CFS_XMEM_OFFSET);
      }
    }
    return 1;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
void
cfs_close(int f)
{
  file.flag = FLAG_FILE_CLOSED;
}
/*---------------------------------------------------------------------------*/
int
cfs_read(int f, void *buf, unsigned int len)
{
  if(file.fileptr + len > CFS_XMEM_SIZE) {
    len = CFS_XMEM_SIZE - file.fileptr;
  }

  if(file.fileptr + len > file.filesize) {
    len = file.filesize - file.fileptr;
  }

  if(f == 1) {
    xmem_pread(buf, len, CFS_XMEM_OFFSET + file.fileptr);
    file.fileptr += len;
    return len;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
int
cfs_write(int f, const void *buf, unsigned int len)
{
  if(file.fileptr >= CFS_XMEM_SIZE) {
    return 0;
  }
  if(file.fileptr + len > CFS_XMEM_SIZE) {
    len = CFS_XMEM_SIZE - file.fileptr;
  }

  if(file.fileptr + len > file.filesize) {
    /* Extend the size of the file. */
    file.filesize = file.fileptr + len;
  }

  if(f == 1) {
    xmem_pwrite(buf, len, CFS_XMEM_OFFSET + file.fileptr);
    file.fileptr += len;
    return len;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
cfs_offset_t
cfs_seek(int f, cfs_offset_t o, int w)
{
  if(w == CFS_SEEK_SET && f == 1) {
    if(o > file.filesize) {
      o = file.filesize;
    }
    file.fileptr = o;
    return o;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
cfs_remove(const char *name)
{
  file.flag = FLAG_FILE_CLOSED;
  file.fileptr = 0;
  file.filesize = 0;
  xmem_erase(CFS_XMEM_SIZE, CFS_XMEM_OFFSET);
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *p, const char *n)
{
  return -1;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  return -1;
}
/*---------------------------------------------------------------------------*/
void
cfs_closedir(struct cfs_dir *p)
{
}
/*---------------------------------------------------------------------------*/
