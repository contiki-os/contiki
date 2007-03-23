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
 * $Id: cfs-xmem.c,v 1.3 2007/03/23 15:52:13 nifi Exp $
 */
#include "contiki.h"

#include "cfs/cfs.h"
#include "cfs/cfs-service.h"
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
static int
s_open(const char *n, int f)
{
  if(file.flag == FLAG_FILE_CLOSED) {
    file.flag = FLAG_FILE_OPEN;
    file.fileptr = 0;
    if((f & CFS_WRITE) && !(f & CFS_APPEND)) {
      file.filesize = 0;
      xmem_erase(CFS_XMEM_SIZE, CFS_XMEM_OFFSET);
    }
    return 1;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
static void
s_close(int f)
{
  file.flag = FLAG_FILE_CLOSED;
}
/*---------------------------------------------------------------------------*/
static int
s_read(int f, char *buf, unsigned int len)
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
static int
s_write(int f, char *buf, unsigned int len)
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
static int
s_seek(int f, unsigned int o)
{
  if(f == 1) {
    if(o > file.filesize) {
      o = file.filesize;
    }
    file.fileptr = o;
    return o;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
static int
s_opendir(struct cfs_dir *p, const char *n)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
s_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
s_closedir(struct cfs_dir *p)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
/*
 * Service registration code follows.
 */
SERVICE(cfs_xmem_service, cfs_service,
{ s_open, s_close, s_read, s_write, s_seek,
    s_opendir, s_readdir, s_closedir });

PROCESS(cfs_xmem_process, "CFS XMEM service");

PROCESS_THREAD(cfs_xmem_process, ev, data)
{
  PROCESS_BEGIN();

  file.fileptr = file.filesize = 0;

  SERVICE_REGISTER(cfs_xmem_service);

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_SERVICE_REMOVED ||
			   ev == PROCESS_EVENT_EXIT);

  SERVICE_REMOVE(cfs_xmem_service);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
cfs_xmem_init(void)
{
  process_start(&cfs_xmem_process, NULL);
}
/*---------------------------------------------------------------------------*/
