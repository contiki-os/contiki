/**
 * \addtogroup c64fs
 * @{
 */

/**
 * \file
 * Implementation of C64 file writes.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * The functions in this file are not included in the core Contiki
 * code, but must be explicitly linked by an application that that
 * wishes to be able to write to files.
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
 * $Id: c64-fs-write.c,v 1.1 2007/05/23 23:11:28 oliverschmidt Exp $
 *
 */

#include "c64-dio.h"
#include "c64-fs.h"
#include <string.h>

/* An *UGLY* implementation of c64_fs_write that only can be used to
   overwrite a single block file. */

extern unsigned char _c64_fs_filebuf[256];
extern unsigned char _c64_fs_filebuftrack,
  _c64_fs_filebufsect;


/*-----------------------------------------------------------------------------------*/
/**
 * Write data to an open file.
 *
 * \note This function currently does not support writing to other than a single block file (cannot be more than 254 bytes long).
 *
 * \param f A pointer to a file descriptor previously opened with c64_fs_open().
 *
 * \param buf A pointer to a buffer with data that should be written
 * to the file.
 *
 * \param len The length of the data that should be written.
 *
 * \return The number of bytes actually written.
 *
 */
/*-----------------------------------------------------------------------------------*/
int __fastcall__
c64_fs_write(register struct c64_fs_file *f, char *buf, int len)
{
  int i;

  if(len <= 0) {
    return 0;
  }

  /* Check if current block is already in buffer, and if not read it
     from disk. */
  if(_c64_fs_filebuftrack != f->track ||
     _c64_fs_filebufsect != f->sect) {
    _c64_fs_filebuftrack = f->track;
    _c64_fs_filebufsect = f->sect;
    c64_dio_read_block(_c64_fs_filebuftrack,
		       _c64_fs_filebufsect,
		       _c64_fs_filebuf);
  }

  i = 256 - f->ptr;
  if(len < i) {
    i = len;
  }

  memcpy(&_c64_fs_filebuf[f->ptr], buf, i);

  f->ptr += i;
  if(_c64_fs_filebuf[0] == 0 &&
     f->ptr > _c64_fs_filebuf[1]) {
    _c64_fs_filebuf[1] = f->ptr;    
  }

  c64_dio_write_block(_c64_fs_filebuftrack,
		      _c64_fs_filebufsect,
		      _c64_fs_filebuf);
  
  return i;
}
/*-----------------------------------------------------------------------------------*/
/** @} */
