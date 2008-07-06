/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 * $Id: cfs-win32-dir.c,v 1.1 2008/07/06 10:24:38 oliverschmidt Exp $
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "contiki.h"

#include "cfs/cfs.h"

struct cfs_win32_dir {
  HANDLE       handle;
  char*        name;
  unsigned int size;
};

/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *p, const char *n)
{
  struct cfs_win32_dir *dir = (struct cfs_win32_dir *)p;
  WIN32_FIND_DATA data;
  char dirname[MAX_PATH];

  if(*n == '/') {
    GetModuleFileName(NULL, dirname, sizeof(dirname));
    strcpy(strrchr(dirname, '\\'), "/*");
  }else{
    sprintf(dirname, "%s/*", n);
  }
  
  dir->handle = FindFirstFile(dirname, &data);
  if(dir->handle == INVALID_HANDLE_VALUE) {
    dir->name = NULL;
    return -1;
  }
  dir->name = strdup(data.cFileName);
  dir->size = ((data.nFileSizeLow + 511) / 512) % 1000;	
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  struct cfs_win32_dir *dir = (struct cfs_win32_dir *)p;
  WIN32_FIND_DATA data;
  
  if(dir->name == NULL) {
    return -1;
  }

  strncpy(e->name, dir->name, sizeof(e->name));
  e->size = dir->size;

  free(dir->name);
  if(FindNextFile(dir->handle, &data) == 0) {
    dir->name = NULL;
    return 0;
  }
  dir->name = strdup(data.cFileName);
  dir->size = ((data.nFileSizeLow + 511) / 512) % 1000;
  return 0;
}
/*---------------------------------------------------------------------------*/
void
cfs_closedir(struct cfs_dir *p)
{
  struct cfs_win32_dir *dir = (struct cfs_win32_dir *)p;

  free(dir->name);
  FindClose(dir->handle);
}
/*---------------------------------------------------------------------------*/
