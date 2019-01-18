/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * $Id: cfs-init.c,v 1.1 2007/05/23 23:11:29 oliverschmidt Exp $
 */
#include "contiki.h"

#include "cfs/cfs.h"
#include "cfs/cfs-service.h"

#include <cbm.h>
#include <string.h>

static int  s_open(const char *n, int f);
static void s_close(int f);
static int  s_read(int f, char *b, unsigned int l);
static int  s_write(int f, char *b, unsigned int l) {return -1;}
static int  s_opendir(struct cfs_dir *p, const char *n) {return -1;}
static int  s_readdir(struct cfs_dir *p, struct cfs_dirent *e) {return -1;}
static int  s_closedir(struct cfs_dir *p) {return -1;}

SERVICE(cfs_posix_service, cfs_service,
{ s_open, s_close, s_read, s_write, s_opendir, s_readdir, s_closedir });

PROCESS(cfs_posix_process, "CFS POSIX service");
PROCESS_THREAD(cfs_posix_process, ev, data) {
  PROCESS_BEGIN();

  SERVICE_REGISTER(cfs_posix_service);

  while(ev != PROCESS_EVENT_SERVICE_REMOVED &&
	ev != PROCESS_EVENT_EXIT) {
    PROCESS_WAIT_EVENT();
  }

  SERVICE_REMOVE(cfs_posix_service);
  
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
#include <stdio.h>
static int
s_open(const char *n, int f)
{
  if(cbm_open(2, _curunit, CBM_READ, n) == 0) {
    /*    printf("Opening with _curunit %d\n", _curunit);*/
    return 2;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static void
s_close(int f)
{
  cbm_close(f);
}
/*---------------------------------------------------------------------------*/
static int
s_read(int f, char *b, unsigned int l)
{
  return cbm_read(f, b, l);
}
/*---------------------------------------------------------------------------*/
