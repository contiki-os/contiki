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
 * $Id: cfs-cooja.c,v 1.6 2007/11/25 22:44:06 fros4943 Exp $
 */
#include <string.h>
#include "lib/simEnvChange.h"
#include "cfs/cfs.h"

#define FLAG_FILE_CLOSED 0
#define FLAG_FILE_OPEN   1

struct filestate {
  char flag;
  int fileptr;
};

static struct filestate file;

const struct simInterface cfs_interface;

// COOJA variables
#define CFS_BUF_SIZE 60*1024
char simCFSData[CFS_BUF_SIZE] = { 0 };
char simCFSChanged = 0;
int simCFSRead = 0;
int simCFSWritten = 0;

/*---------------------------------------------------------------------------*/
int
cfs_open(const char *n, int f)
{
  if(file.flag == FLAG_FILE_CLOSED) {
    file.flag = FLAG_FILE_OPEN;
    file.fileptr = 0;
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
  if(f == FLAG_FILE_OPEN) {
	// TODO Should yield a few times?
	memcpy(buf, &simCFSData[0] + file.fileptr, len);
    file.fileptr += len;
	simCFSChanged = 1;
	simCFSRead += len;
    return len;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
int
cfs_write(int f, void *buf, unsigned int len)
{
  if(f == FLAG_FILE_OPEN) {
	// TODO Should yield a few times?
	memcpy(&simCFSData[0] + file.fileptr, buf, len);
    file.fileptr += len;
	simCFSChanged = 1;
	simCFSWritten += len;
    return len;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
unsigned int
cfs_seek(int f, unsigned int o)
{
  if(f == FLAG_FILE_OPEN) {
    file.fileptr = o;
    return o;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *p, const char *n)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
void
cfs_closedir(struct cfs_dir *p)
{
}
/*---------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
}
/*---------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
}
/*---------------------------------------------------------------------------*/
SIM_INTERFACE(cfs_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
/*---------------------------------------------------------------------------*/
