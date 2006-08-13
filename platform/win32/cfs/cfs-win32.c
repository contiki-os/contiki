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
 * $Id: cfs-win32.c,v 1.1 2006/08/13 16:23:10 oliverschmidt Exp $
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "contiki.h"

#include "cfs/cfs.h"
#include "cfs/cfs-service.h"

static int  s_open(const char *n, int f);
static void s_close(int f);
static int  s_read(int f, char *b, unsigned int l);
static int  s_write(int f, char *b, unsigned int l);
static int  s_seek(int f, unsigned int offset);
static int  s_opendir(struct cfs_dir *p, const char *n);
static int  s_readdir(struct cfs_dir *p, struct cfs_dirent *e);
static int  s_closedir(struct cfs_dir *p);

SERVICE(cfs_win32_service, cfs_service,
{ s_open, s_close, s_read, s_write, s_seek,
    s_opendir, s_readdir, s_closedir });

struct cfs_win32_dir {
  HANDLE         handle;
  unsigned char* name;
  unsigned int   size;
};

PROCESS(cfs_win32_process, "CFS Win32 service");

PROCESS_THREAD(cfs_win32_process, ev, data)
{
  PROCESS_BEGIN();

  SERVICE_REGISTER(cfs_win32_service);

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_SERVICE_REMOVED ||
			   ev == PROCESS_EVENT_EXIT);

  SERVICE_REMOVE(cfs_win32_service);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
s_open(const char *n, int f)
{
  if(f == CFS_READ) {
    return _open(n, O_RDONLY);
  } else {
    return _open(n, O_CREAT|O_TRUNC|O_RDWR);
  }
}
/*---------------------------------------------------------------------------*/
static void
s_close(int f)
{
  _close(f);
}
/*---------------------------------------------------------------------------*/
static int
s_read(int f, char *b, unsigned int l)
{
  return _read(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_write(int f, char *b, unsigned int l)
{
  return _write(f, b, l);
}
/*---------------------------------------------------------------------------*/
static int
s_seek(int f, unsigned int o)
{
  return _lseek(f, o, SEEK_SET);
}
/*---------------------------------------------------------------------------*/
static int
s_opendir(struct cfs_dir *p, const char *n)
{
  struct cfs_win32_dir *dir = (struct cfs_win32_dir *)p;
  WIN32_FIND_DATA data;
  char dirname[MAX_PATH];

  if(n == NULL) {
    n = "";
  }
  if(*n == '/') {
    GetModuleFileName(NULL, dirname, sizeof(dirname));
    strcpy(strrchr(dirname, '\\'), "/*");
  }else{
    sprintf(dirname, "%s/*", n);
  }
  
  dir->handle = FindFirstFile(dirname, &data);
  if(dir->handle == INVALID_HANDLE_VALUE) {
    dir->name = NULL;
    return 1;
  }

  dir->name = _strdup(data.cFileName);
  dir->size = ((data.nFileSizeLow + 511) / 512) % 1000;

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
s_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  struct cfs_win32_dir *dir = (struct cfs_win32_dir *)p;
  WIN32_FIND_DATA data;
  
  if(dir->name == NULL) {
    return 1;
  }

  strncpy(e->name, dir->name, sizeof(e->name));
  e->size = dir->size;

  free(dir->name);
  if(FindNextFile(dir->handle, &data) == 0) {
    dir->name = NULL;
    return 0;
  }

  dir->name = _strdup(data.cFileName);
  dir->size = ((data.nFileSizeLow + 511) / 512) % 1000;

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
s_closedir(struct cfs_dir *p)
{
  struct cfs_win32_dir *dir = (struct cfs_win32_dir *)p;

  free(dir->name);
  FindClose(dir->handle);

  return 1;
}
/*---------------------------------------------------------------------------*/
