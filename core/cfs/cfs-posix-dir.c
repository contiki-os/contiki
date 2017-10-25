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

#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define CFS_IMPL 1
#include "cfs/cfs.h"

struct cfs_posix_dir {
  DIR *dirp;
};

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
#if defined(__APPLE2__) || defined(__CBM__)
  e->size = res->d_blocks;
#else /* __APPLE2__ || __CBM__ */
  e->size = 0;
#endif /* __APPLE2__ || __CBM__ */
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
