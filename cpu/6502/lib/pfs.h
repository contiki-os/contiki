/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * Author: Greg King <gregdk@users.sf.net>
 *
 */

#ifndef PFS_H_
#define PFS_H_

#include <sys/types.h>

struct cfs_dir;
struct cfs_dirent;

int   __fastcall__ pfs_open(const char *name, int flags);
void  __fastcall__ pfs_close(int fd);
int   __fastcall__ pfs_read(int fd, void *buf, unsigned int len);
int   __fastcall__ pfs_write(int fd, const void *buf, unsigned int len);
off_t __fastcall__ pfs_seek(int fd, off_t offset, int whence);
int   __fastcall__ pfs_remove(const char *name);
int   __fastcall__ pfs_opendir(struct cfs_dir *dirp, const char *name);
void  __fastcall__ pfs_closedir(struct cfs_dir *dirp);
int   __fastcall__ pfs_readdir(struct cfs_dir *dirp, struct cfs_dirent *dirent);

#endif /* PFS_H_ */
