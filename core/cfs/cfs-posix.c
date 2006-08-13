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
 * $Id: cfs-posix.c,v 1.2 2006/08/13 14:05:20 oliverschmidt Exp $
 */
#include "contiki.h"

#include "cfs/cfs.h"
#include "cfs/cfs-service.h"

#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
#undef BYTE_ORDER
#undef HTONS

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

static int  s_open(const char *n, int f);
static void s_close(int f);
static int  s_read(int f, char *b, unsigned int l);
static int  s_write(int f, char *b, unsigned int l);
static int  s_seek(int f, unsigned int offset);
static int  s_opendir(struct cfs_dir *p, const char *n);
static int  s_readdir(struct cfs_dir *p, struct cfs_dirent *e);
static int  s_closedir(struct cfs_dir *p);

SERVICE(cfs_posix_service, cfs_service,
{ s_open, s_close, s_read, s_write, s_seek,
    s_opendir, s_readdir, s_closedir });

struct cfs_posix_dir {
  DIR *dirp;
};

PROCESS(cfs_posix_process, "CFS POSIX service");

PROCESS_THREAD(cfs_posix_process, ev, data)
{
  PROCESS_BEGIN();

  SERVICE_REGISTER(cfs_posix_service);

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_SERVICE_REMOVED ||
			   ev == PROCESS_EVENT_EXIT);

  SERVICE_REMOVE(cfs_posix_service);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
s_open(const char *n, int f)
{
  char filename[255];

  sprintf(filename, "cfs-root/%s", n);

  if(f == CFS_READ) {
    return open(filename, O_RDONLY);
  } else {
    return open(filename, O_CREAT|O_RDWR);
  }
}
/*---------------------------------------------------------------------------*/
static void
s_close(int f)
{
  close(f);
}
/*---------------------------------------------------------------------------*/
static int
s_read(int f, char *b, unsigned int l)
{
  return read(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_write(int f, char *b, unsigned int l)
{
  return write(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_seek(int f, unsigned int o)
{
  return lseek(f, o, SEEK_SET);
}
/*---------------------------------------------------------------------------*/
static int
s_opendir(struct cfs_dir *p, const char *n)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;
  char dirname[255];

  if(n == NULL) {
    n = "";
  }
  sprintf(dirname, "cfs-root/%s", n);
  
  dir->dirp = opendir(dirname);

  if(dir->dirp == NULL) {
    printf("cfs-posix: could not open directory '%s'\n", dirname);
  }
  return dir->dirp == NULL;
}
/*---------------------------------------------------------------------------*/
static int
s_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;
  struct dirent *res;
  int ret;

  if(dir->dirp == NULL) {
    return 1;
  }
  res = readdir(dir->dirp);
  if(res == NULL) {
    return 1;
  }
  strncpy(e->name, res->d_name, sizeof(e->name));
  e->size = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
s_closedir(struct cfs_dir *p)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;
  if(dir->dirp != NULL) {
    closedir(dir->dirp);
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
