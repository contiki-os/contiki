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
 * $Id: cfs-ram.c,v 1.10 2009/02/27 14:25:38 nvt-se Exp $
 */

#include <string.h>

#include "cfs/cfs.h"

struct filestate {
  int flag;
#define FLAG_FILE_CLOSED 0
#define FLAG_FILE_OPEN   1
  int fileptr;
  int filesize;
};

#ifdef CFS_RAM_CONF_SIZE
#define CFS_RAM_SIZE CFS_RAM_CONF_SIZE
#else
#define CFS_RAM_SIZE 4096
#endif

static struct filestate file;
static char filemem[CFS_RAM_SIZE];

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
  if(file.fileptr + len > sizeof(filemem)) {
    len = sizeof(filemem) - file.fileptr;
  }
  
  if(file.fileptr + len > file.filesize) {
    len = file.filesize - file.fileptr;
  }

  if(f == 1) {
    memcpy(buf, &filemem[file.fileptr], len);
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
  if(file.fileptr >= sizeof(filemem)) {
    return 0;
  }
  if(file.fileptr + len > sizeof(filemem)) {
    len = sizeof(filemem) - file.fileptr;
  }

  if(file.fileptr + len > file.filesize) {
    /* Extend the size of the file. */
    file.filesize = file.fileptr + len;
  }
  
  if(f == 1) {
    memcpy(&filemem[file.fileptr], buf, len);
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
  return (cfs_offset_t)-1;
}
/*---------------------------------------------------------------------------*/
int
cfs_remove(const char *name)
{
  return -1;
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
