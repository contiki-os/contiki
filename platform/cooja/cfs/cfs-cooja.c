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
 * $Id: cfs-cooja.c,v 1.12 2010/11/15 21:44:37 adamdunkels Exp $
 */
#include <string.h>
#include "lib/simEnvChange.h"
#include "cfs/cfs.h"

#define FLAG_FILE_CLOSED 0
#define FLAG_FILE_OPEN   1

struct filestate {
  char flag;
  int access;
  int fileptr;
  int endptr;
};

static struct filestate file;

const struct simInterface cfs_interface;

// COOJA variables
#define CFS_BUF_SIZE 4000 /* Configure CFS size here and in ContikiCFS.java */
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
    file.access = f;
		if(f & CFS_APPEND) {
			file.fileptr = file.endptr;
		} else {
	    file.fileptr = 0;
		}
    return 0;
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
	if(file.flag == FLAG_FILE_OPEN && file.access & CFS_READ) {
		if(file.fileptr + len >= file.endptr) {
			len = file.endptr - file.fileptr;
		}
		memcpy(buf, &simCFSData[file.fileptr], len);
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
cfs_write(int f, const void *buf, unsigned int len)
{
	if(file.flag == FLAG_FILE_OPEN && file.access & CFS_WRITE) {
		if(file.fileptr + len > CFS_BUF_SIZE) {
			len = CFS_BUF_SIZE - file.fileptr;
                        printf("cfs-cooja.c: warning: write truncated\n");
		}
		memcpy(&simCFSData[file.fileptr], buf, len);
  	file.fileptr += len;
		simCFSChanged = 1;
		simCFSWritten += len;
		if(file.fileptr > file.endptr) {
			file.endptr = file.fileptr;
		}
    return len;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
cfs_offset_t
cfs_seek(int f, cfs_offset_t o, int w)
{
  if(file.flag == FLAG_FILE_OPEN) {
  	if(w == CFS_SEEK_SET) {
    	file.fileptr = o;
  	} else if(w == CFS_SEEK_CUR) {
			file.fileptr += o;
  	} else if(w == CFS_SEEK_END) {
  		file.fileptr = file.endptr + o;
  	}
  	if(file.fileptr >= 0 && file.fileptr <= CFS_BUF_SIZE) {
  		if(file.fileptr > file.endptr) {
  			file.endptr = file.fileptr;
  		}
		  return file.fileptr;
  	}
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
int
cfs_remove(const char *name)
{
	memset(simCFSData, 0, sizeof(simCFSData));
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *p, const char *n)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  return 0;
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
