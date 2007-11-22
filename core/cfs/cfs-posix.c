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
 * $Id: cfs-posix.c,v 1.6 2007/11/22 11:37:34 oliverschmidt Exp $
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include "cfs/cfs.h"

struct cfs_posix_dir {
  DIR *dirp;
};

/*---------------------------------------------------------------------------*/
int
cfs_open(const char *n, int f)
{
  return open(n, f == CFS_READ? O_RDONLY: O_CREAT|O_RDWR);
}
/*---------------------------------------------------------------------------*/
void
cfs_close(int f)
{
  close(f);
}
/*---------------------------------------------------------------------------*/
int
cfs_read(int f, void *b, unsigned int l)
{
  return read(f, b, l);
}
/*---------------------------------------------------------------------------*/
int
cfs_write(int f, void *b, unsigned int l)
{
  return write(f, b, l);
}
/*---------------------------------------------------------------------------*/
unsigned int
cfs_seek(int f, unsigned int o)
{
  return lseek(f, o, SEEK_SET);
}
/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *p, const char *n)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;

  dir->dirp = opendir(n);
  return dir->dirp == NULL;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;
  struct dirent *res;

  if(dir->dirp == NULL) {
    return -1;
  }
  res = readdir(dir->dirp);
  if(res == NULL) {
    return -1;
  }
  strncpy(e->name, res->d_name, sizeof(e->name));
  e->size = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
void
cfs_closedir(struct cfs_dir *p)
{
  struct cfs_posix_dir *dir = (struct cfs_posix_dir *)p;

  if(dir->dirp != NULL) {
    closedir(dir->dirp);
  }
}
/*---------------------------------------------------------------------------*/
