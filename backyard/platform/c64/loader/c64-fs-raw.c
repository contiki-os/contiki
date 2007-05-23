/**
 * \addtogroup c64fs
 * @{
 */

/**
 * \file
 * "Raw" C64 file system access.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * This file provides functions that allow reading data from files
 * without updating the file descriptor pointer. The functions are not
 * automatically included in the core Contiki code and therefore
 * application programs that use tham must manually link with this
 * file.
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
 * $Id: c64-fs-raw.c,v 1.1 2007/05/23 23:11:28 oliverschmidt Exp $
 *
 */

#include "c64-fs-raw.h"

#include <string.h>

struct directory_entry {
  unsigned char type;
  unsigned char track, sect;
  unsigned char name[16];
  unsigned char reltrack, relsect, relreclen;
  unsigned char unused1, unused2, unused3, unused4;
  unsigned char tmptrack, tmpsect;
  unsigned char blockslo, blockshi;
};


extern unsigned char _c64_fs_dirbuf[256];
extern unsigned char _c64_fs_dirbuftrack, _c64_fs_dirbufsect;

extern unsigned char _c64_fs_filebuf[256];
extern unsigned char _c64_fs_filebuftrack, _c64_fs_filebufsect;

void _c64_fs_readdirbuf(unsigned char track, unsigned char sect);


/*-----------------------------------------------------------------------------------*/
/**
 * Read data from a file without updating the file descriptor pointer.
 *
 * This function reads data from an open file into a buffer than must
 * be allocated by the caller, but does not update the file
 * description pointer like the c64_fs_read() function does.
 *
 * \param f A pointer to a file descriptor structure that must have
 * been opened with c64_fs_open().
 *
 * \param buf A pointer to the buffer in which the data should be placed.
 *
 * \param len The maxiumum amount of bytes to read.
 *
 * \return The number of bytes that actually was read, or 0 if an end
 * of file was encountered.
 *
 */
/*-----------------------------------------------------------------------------------*/
int __fastcall__
c64_fs_read_raw(register struct c64_fs_file *f, char *buf, int len)
{
  int i;
  unsigned char fptr, ftrack, fsect;

  /* Check if current block is already in buffer, and if not read it
     from disk. */
  if(_c64_fs_filebuftrack != f->track ||
     _c64_fs_filebufsect != f->sect) {
    _c64_fs_filebuftrack = f->track;
    _c64_fs_filebufsect = f->sect;
    c64_dio_read_block(_c64_fs_filebuftrack,
		       _c64_fs_filebufsect, _c64_fs_filebuf);
  }

  if(_c64_fs_filebuf[0] == 0 &&
     f->ptr == _c64_fs_filebuf[1]) {
    return 0; /* EOF */
  }

  fptr = f->ptr;
  ftrack = f->track;
  fsect = f->sect;
  
  for(i = 0; i < len; ++i) {
    *buf = _c64_fs_filebuf[fptr];
    
    ++fptr;
    if(_c64_fs_filebuf[0] == 0) {
      if(fptr == _c64_fs_filebuf[1]) {
	/* End of file reached, we return the amount of bytes read so
	   far. */
	return i + 1;
      }
    } else if(fptr == 0) {

      /* Read new block into buffer and set buffer state
	 accordingly. */
      _c64_fs_filebuftrack = ftrack = _c64_fs_filebuf[0];
      _c64_fs_filebufsect = fsect = _c64_fs_filebuf[1];
      fptr = 2;
      c64_dio_read_block(_c64_fs_filebuftrack,
			 _c64_fs_filebufsect, _c64_fs_filebuf);
    }
    
    ++buf;
  }
  return i;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Move the file descriptior pointer forward in the file.
 *
 *
 * \param f A pointer to a file descriptor structure that must have
 * been opened with c64_fs_open().
 *
 * \param len The number of bytes the pointer should be moved forward.
 *
 * \return The number of bytes that the pointer actually was moved, or
 * 0 if an end of file was encountered.
 */
/*-----------------------------------------------------------------------------------*/
int
c64_fs_read_next(register struct c64_fs_file *f, int len)
{
  int i;

  /* Check if current block is already in buffer, and if not read it
     from disk. */
  if(_c64_fs_filebuftrack != f->track ||
     _c64_fs_filebufsect != f->sect) {
    _c64_fs_filebuftrack = f->track;
    _c64_fs_filebufsect = f->sect;
    c64_dio_read_block(_c64_fs_filebuftrack,
		       _c64_fs_filebufsect, _c64_fs_filebuf);
  }

  if(_c64_fs_filebuf[0] == 0 &&
     f->ptr == _c64_fs_filebuf[1]) {
    return 0; /* EOF */
  }

  for(i = 0; i < len; ++i) {
    
    ++f->ptr;
    if(_c64_fs_filebuf[0] == 0) {
      if(f->ptr == _c64_fs_filebuf[1]) {
	/* End of file reached, we return the amount of bytes read so
	   far. */
	return i + 1;
      }
    } else if(f->ptr == 0) {
      /* Read new block into buffer and set buffer state
	 accordingly. */
      _c64_fs_filebuftrack = f->track = _c64_fs_filebuf[0];
      _c64_fs_filebufsect = f->sect = _c64_fs_filebuf[1];
      f->ptr = 2;
      c64_dio_read_block(_c64_fs_filebuftrack,
			 _c64_fs_filebufsect, _c64_fs_filebuf);
    }    
  }
  return i;
}
/*-----------------------------------------------------------------------------------*/
/** @} */
