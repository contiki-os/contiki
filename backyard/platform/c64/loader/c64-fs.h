/**
 * \addtogroup c64fs
 * @{
 */

/**
 * \file
 * Header file for the C64 filesystem functions.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 */

/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki desktop environment 
 *
 * $Id: c64-fs.h,v 1.1 2007/05/23 23:11:28 oliverschmidt Exp $
 *
 */
#ifndef __C64_FS_H__
#define __C64_FS_H__

#include "c64-dio.h"

/**
 * An opaque structure with no user visible elements that represents
 * an open file.
 */
struct c64_fs_file {
  unsigned char track, sect, ptr;
};

int c64_fs_open(const char *name, struct c64_fs_file *f);
void c64_fs_close(struct c64_fs_file *f);
int __fastcall__ c64_fs_read(struct c64_fs_file *f,
			     char *buf, int len);

int __fastcall__ c64_fs_write(struct c64_fs_file *f,
			      char *buf, int len);

/**
 * An opaque structure with no user visible elements that represents a
 * directory descriptor.
 */
struct c64_fs_dir {
  unsigned char track, sect, ptr;
};

/**
 * A C64 directory entry.
 */
struct c64_fs_dirent {
  char name[17];        /**< The name of the directory entry. */
  unsigned short size;  /**< The size of the directory entry in 256 byte blocks. */
  unsigned char track,  
    sect;
};


unsigned char c64_fs_opendir(struct c64_fs_dir *d);

void c64_fs_readdir_dirent(struct c64_fs_dir *d,
			   struct c64_fs_dirent *f);
unsigned char c64_fs_readdir_next(struct c64_fs_dir *d);

void c64_fs_closedir(struct c64_fs_dir *d);

/** @} */

#endif /* __C64_FS_H__ */
