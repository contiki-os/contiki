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
 * $Id: cfs-service.h,v 1.1 2006/06/17 22:41:15 adamdunkels Exp $
 */
#ifndef __CFS_SERVICE_H__
#define __CFS_SERVICE_H__

#include "contiki.h"
#include "cfs/cfs.h"

struct cfs_dir {
  unsigned char dummy_space[32];
};

struct cfs_dirent {
  unsigned char name[32];
  unsigned int size;
};

struct cfs_service_interface {
  int  (* open)(const char *name, int flags);
  void (* close)(int fd);
  int  (* read)(int fd, char *buf, unsigned int len);
  int  (* write)(int fd, char *buf, unsigned int len);
  int  (* seek)(int fd, unsigned int offset);

  int  (* opendir)(struct cfs_dir *dir, const char *name);
  int  (* readdir)(struct cfs_dir *dir, struct cfs_dirent *dirent);
  int  (* closedir)(struct cfs_dir *dir);
};

#define cfs_service_name "Filesystem"
SERVICE_INTERFACE(cfs_service, 
{
  int  (* open)(const char *name, int flags);
  void (* close)(int fd);
  int  (* read)(int fd, char *buf, unsigned int len);
  int  (* write)(int fd, char *buf, unsigned int len);
  int  (* seek)(int fd, unsigned int offset);
  
  int  (* opendir)(struct cfs_dir *dir, const char *name);
  int  (* readdir)(struct cfs_dir *dir, struct cfs_dirent *dirent);
  int  (* closedir)(struct cfs_dir *dir);
});

#endif /* __CFS_SERVICE_H__ */
