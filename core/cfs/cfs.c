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
 * $Id: cfs.c,v 1.2 2006/08/13 14:06:24 oliverschmidt Exp $
 */
#include "contiki.h"

#include "cfs/cfs.h"
#include "cfs/cfs-service.h"


static int  null_open(const char *n, int f)                       {return -1;}
static void null_close(int f)                                     {return;}
static int  null_read(int f, char *b, unsigned int l)             {return -1;}
static int  null_write(int f, char *b, unsigned int l)            {return -1;}
static int  null_seek(int fd, unsigned int l)                     {return -1;}
static int  null_opendir(struct cfs_dir *p, const char *n)        {return -1;}
static int  null_readdir(struct cfs_dir *p, struct cfs_dirent *e) {return -1;}
static int  null_closedir(struct cfs_dir *p)                      {return -1;}

SERVICE(cfs_nullservice, cfs_service, 
  {
    null_open,
    null_close,
    null_read,
    null_write,
    null_seek,
    null_opendir,
    null_readdir,
    null_closedir
  });

/*---------------------------------------------------------------------------*/
const struct cfs_service_interface *
cfs_find_service(void)
{
  struct service *s;

  s = service_find(cfs_service_name);
  
  if(s != NULL) {
    return s->interface;
  } else {
    return cfs_nullservice.interface;
  }
}
/*---------------------------------------------------------------------------*/
