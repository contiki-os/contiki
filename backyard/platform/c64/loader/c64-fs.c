/**
 * \defgroup c64fs C64 file system and disk functions.
 * @{
 *
 * The C64 file system functions are divided into two categories:
 * those that deal with C64 files and the C64 disk directory, and
 * those that allow direct block access to the disk. The former
 * functions can be used for accessing regular files, whereas the
 * latter functions are used e.g. to download D64 files onto 1541
 * disks.
 *
 * \note The C64 filesystem functions currently only work with the
 * 1541/1541-II/1571 and compatible drives, and not with the IDE64
 * hard disks or the 1581/FD2000 3.5" drives.
 *
 * 
 */

/**
 * \file
 * C64 file system operations interface for Contiki.
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
 * $Id: c64-fs.c,v 1.1 2007/05/23 23:11:28 oliverschmidt Exp $
 *
 */

#include "c64-dio.h"
#include "c64-dio-asm.h"
#include "c64-fs.h"

#include <string.h>
#include <stdio.h>

struct directory_entry {
  unsigned char type;
  unsigned char track, sect;
  unsigned char name[16];
  unsigned char reltrack, relsect, relreclen;
  unsigned char unused1, unused2, unused3, unused4;
  unsigned char tmptrack, tmpsect;
  unsigned char blockslo, blockshi;
};

unsigned char _c64_fs_dirbuf[256];
unsigned char _c64_fs_dirbuftrack = 0, _c64_fs_dirbufsect = 0;

unsigned char _c64_fs_filebuf[256];
unsigned char _c64_fs_filebuftrack = 0, _c64_fs_filebufsect = 0;

static struct c64_fs_dirent lastdirent;

static struct c64_fs_dir opendir;
static struct c64_fs_dirent opendirent;

/*-----------------------------------------------------------------------------------*/
/**
 * Open a file.
 *
 * The file description must be allocated by the caller and a pointer
 * to it is passed to this function.
 *
 * \param name A pointer to the name of the file to be opened.
 * \param f A pointer to the file descriptor struct.
 *
 * \retval 0 If the file was successfully opened.
 * \retval -1 If the file does not exist.
 */
/*-----------------------------------------------------------------------------------*/
int
c64_fs_open(const char *name, register struct c64_fs_file *f)
{
  /* First check if we already have the file cached. If so, we don't
     need to do an expensive directory lookup. */
  if(strncmp(lastdirent.name, name, 16) == 0) {
    f->track = lastdirent.track;
    f->sect = lastdirent.sect;
    f->ptr = 2;
    return 0;
  }

  /* Not in cache, so we walk through directory instead. */
  c64_fs_opendir(&opendir);

  do {
    c64_fs_readdir_dirent(&opendir, &opendirent);
    if(strncmp(opendirent.name, name, 16) == 0) {
      f->track = opendirent.track;
      f->sect = opendirent.sect;
      f->ptr = 2;
      return 0;
    }
  } while(c64_fs_readdir_next(&opendir) == 0);

  /* The file was not found in the directory. We flush the directory
     buffer cache now in order to prevent a nasty problem from
     happening: If the first directory block of an empty disk was
     cached, *all* subsequent file opens would return "file not
     found". */
  _c64_fs_dirbuftrack = 0; /* There are no disk blocks on track 0. */
  return -1;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Read data from an open file.
 *
 * This function reads data from an open file into a buffer than must
 * be allocated by the caller.
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
 */
/*-----------------------------------------------------------------------------------*/
#if !NOASM
#pragma optimize(push, off)
#endif /* !NOASM */
int __fastcall__
c64_fs_read(register struct c64_fs_file *f, char *buf, int len)
{
  static int i;

  /* Check if current block is already in buffer, and if not read it
     from disk. */

#if NOASM
  if(f->track != _c64_fs_filebuftrack ||
     _c64_fs_filebufsect != f->sect) {
    _c64_fs_filebuftrack = f->track;
    _c64_fs_filebufsect = f->sect;
    c64_dio_read_block(_c64_fs_filebuftrack, _c64_fs_filebufsect,
		       _c64_fs_filebuf);
  }
#else /* NOASM */
  asm("ldy #%b", offsetof(struct c64_fs_file, track));
  asm("lda (regbank+%b),y", 4);
  asm("cmp %v", _c64_fs_filebuftrack);
  asm("bne doblock");
  
  asm("ldy #%b", offsetof(struct c64_fs_file, sect));
  asm("lda (regbank+%b),y", 4);
  asm("cmp %v", _c64_fs_filebufsect);
  asm("bne doblock");

  asm("jmp noblock");

  asm("doblock:");
  
  asm("ldy #%b", offsetof(struct c64_fs_file, track));
  asm("lda (regbank+%b),y", 4);
  asm("sta %v", _c64_fs_filebuftrack);
  asm("sta %v", c64_dio_asm_track);
  
  asm("ldy #%b", offsetof(struct c64_fs_file, sect));
  asm("lda (regbank+%b),y", 4);
  asm("sta %v", _c64_fs_filebufsect);
  asm("sta %v", c64_dio_asm_sector);

  asm("lda #<(%v)", _c64_fs_filebuf);
  asm("sta %v", c64_dio_asm_ptr);
  asm("lda #>(%v)", _c64_fs_filebuf);
  asm("sta %v+1", c64_dio_asm_ptr);

  asm("jsr %v", c64_dio_asm_read_block);

  asm("noblock:");

#endif /* NOASM */

  if(_c64_fs_filebuf[0] == 0 &&
     f->ptr == _c64_fs_filebuf[1]) {
    return 0; /* EOF */
  }

  for(i = 0; i < len; ++i) {
#if NOASM    
    *buf = _c64_fs_filebuf[f->ptr];
    ++f->ptr;
#else /* NOASM */	
    asm("ldy #%o+1", buf);
    asm("jsr ldaxysp");
    asm("sta ptr2");
    asm("stx ptr2+1");

    asm("ldy #%b", offsetof(struct c64_fs_file, ptr));
    asm("lda (regbank+%b),y", 4);    
    asm("tax");

    asm("ldy #0");
    asm("lda %v,x", _c64_fs_filebuf);
    asm("sta (ptr2),y");

    asm("inx");
    asm("txa");
    asm("ldy #%b", offsetof(struct c64_fs_file, ptr));
    asm("sta (regbank+%b),y", 4);    
#endif /* NOASM */

    
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
    
    ++buf;
  }
  return i;
}
#if !NOASM    
#pragma optimize(pop)
#endif /* !NOASM */
/*-----------------------------------------------------------------------------------*/
/**
 * Close an open file.
 *
 * \param f A pointer to a file descriptor struct that previously has
 * been opened with c64_fs_open().
 */
/*-----------------------------------------------------------------------------------*/
void
c64_fs_close(struct c64_fs_file *f)
{
  
}
/*-----------------------------------------------------------------------------------*/
/**
 * \internal
 * Read a directory buffer into the _c64_fs_dirbuf buffer.
 *
 * This function is shared between this and  the c64-fs-raw module.
 *
 * \param track The track of the directory block.
 * \param sect The sector of the directory block.
 */
/*-----------------------------------------------------------------------------------*/
void
_c64_fs_readdirbuf(unsigned char track, unsigned char sect)
{
  if(_c64_fs_dirbuftrack == track &&
     _c64_fs_dirbufsect == sect) {
    /* Buffer already contains requested block, return. */
    return;
  }
  c64_dio_read_block(track, sect, _c64_fs_dirbuf);
  _c64_fs_dirbuftrack = track;
  _c64_fs_dirbufsect = sect;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Open the disk directory for reading.
 *
 * The caller must supply a pointer to a directory descriptor.
 *
 * \param d A pointer to a directory description that must be
 * allocated by the caller.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_fs_opendir(register struct c64_fs_dir *d)
{
  d->track = 18;
  d->sect = 1;
  d->ptr = 2;

  return 0;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Read the current directory entry.
 *
 * This function reads the directory entry to which the directory
 * descriptor currently points into a struct c64_fs_dirent supplied by
 * the caller.
 *
 * The function c64_fs_readdir_next() is used to move the directory
 * entry pointer forward in the directory.
 *
 * \param d A pointer to a directory descriptor previously opened with c64_fs_opendir().
 *
 * \param f A pointer to a directory entry that must have been
 * previously allocated by the caller.
 */
/*-----------------------------------------------------------------------------------*/
void
c64_fs_readdir_dirent(register struct c64_fs_dir *d,
		      register struct c64_fs_dirent *f)
{
  struct directory_entry *de;
  int i;
  register char *nameptr;
  
  _c64_fs_readdirbuf(d->track, d->sect);
  de = (struct directory_entry *)&_c64_fs_dirbuf[d->ptr];
  nameptr = de->name;
  for(i = 0; i < 16; ++i) {
    if(*nameptr == 0xa0) {
      *nameptr = 0;
      break;
    }
    ++nameptr;
  }
  strncpy(f->name, de->name, 16);
  f->track = de->track;
  f->sect = de->sect;
  f->size = de->blockslo + (de->blockshi >> 8);
  memcpy(&lastdirent, f, sizeof(struct c64_fs_dirent));
}
/*-----------------------------------------------------------------------------------*/
/**
 * Move the directory pointer forward.
 *
 * This function moves the directory entry pointer in the directory
 * descriptor forward so that it points to the next file.
 *
 * \param d A pointer to a directory descriptor previously opened with
 * c64_fs_opendir().
 *
 * \retval 1 If there are no more directory entried in the directory.
 * \retval 0 There were more directory entries and the pointer has
 * been moved to point to the next one.
 */
/*-----------------------------------------------------------------------------------*/
unsigned char
c64_fs_readdir_next(struct c64_fs_dir *d)
{
  struct directory_entry *de;
 again:
  _c64_fs_readdirbuf(d->track, d->sect);
  if(d->ptr == 226) {
    if(_c64_fs_dirbuf[0] == 0) {
      return 1;
    }
    d->track = _c64_fs_dirbuf[0];
    d->sect = _c64_fs_dirbuf[1];
    d->ptr = 2;
  } else {
    d->ptr += 32;
  }

  de = (struct directory_entry *)&_c64_fs_dirbuf[d->ptr];
  if(de->type == 0) {
    goto again;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
/**
 * Close a directory descriptor previously opened by c64_fs_opendir().
 *
 * \param d A poitner to a directory descriptor previously opened with
 * c64_fs_opendir().
 */
/*-----------------------------------------------------------------------------------*/
void
c64_fs_closedir(struct c64_fs_dir *d)
{
  
}
/*-----------------------------------------------------------------------------------*/
/** @} */
