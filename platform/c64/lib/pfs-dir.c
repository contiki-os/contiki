/*
 * Copyright (c) 2016, Greg King
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions, and the following
 *    disclaimer, in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS"; AND, ANY EXPRESS
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
 * This file is a part of the Contiki operating system.
 */

/*
** Open a directory listing, so that it can be read by pfs_readdir().
**
** Read one file-name from a directory listing that was opened by pfs_opendir().
**
** 2012-05-30, Ullrich von Bassewitz
** 2016-04-22, Greg King
*/

#include "cfs.h"

typedef struct {
    int fd;                     /* File descriptor for a directory */
} DIR;

/*---------------------------------------------------------------------------*/
/*
** Read characters from the directory into the supplied buffer.
** Return true if the read was successful, and false otherwise.
*/
extern unsigned char __fastcall__ _pfs_dirread(struct cfs_dir *dir, void *buf,
                                               unsigned char count);

/*
** Read one byte from the directory into the supplied buffer.
** Return true if the read was successful, and false otherwise.
*/
extern unsigned char __fastcall__ _pfs_dirread1(struct cfs_dir *dir, void *buf);
/*---------------------------------------------------------------------------*/
int __fastcall__
pfs_opendir(struct cfs_dir *dirp, register const char *name)
{
  static int fd;
  static char buf[2 + 1] = "$";

  /* Set up the actual file name that is sent to the DOS.
  ** We accept "0:", "1:", "/", and "." as directory names.
  */
  if(name == NULL || name[0] == '\0' || (name[0] == '.' || name[0] == '/') && name[1] == '\0') {
    buf[1] = '\0';
  } else if((name[0] == '0' || name[0] == '1') && name[1] == ':' && name[2] == '\0') {
    buf[1] = name[0];
    buf[2] = '\0';
  } else {
    return -1;
  }

  /* Open the directory on a disk, for reading. */
  fd = pfs_open(buf, CFS_READ);
  if(fd >= 0) {
    ((DIR *)dirp)->fd = fd;

    /* Skip the load address. */
    if(_pfs_dirread(dirp, buf + 1, 2)) {
      return 0;
    } else {
      pfs_close(fd);
    }
  }

  return -1;
}
/*---------------------------------------------------------------------------*/
int __fastcall__
pfs_readdir(struct cfs_dir *dirp, register struct cfs_dirent *dirent)
{
  register unsigned char *b;
  register unsigned char i;
  register unsigned char count;
  unsigned char buffer[0x40];
  static unsigned char s;
  static unsigned char j;

  /* Skip the BASIC line-link. */
  if(!_pfs_dirread(dirp, buffer, 2)) {
    return -1;
  }

  /* Read the number of blocks. It's a two-byte number; but, the size is
  ** a four-byte number. Zero the size; then, put the block number in
  ** the first two bytes. It works because the 6502 CPU's numbers are
  ** little-endian.
  */
  dirent->size = 0;
  if(!_pfs_dirread(dirp, &dirent->size, 2)) {
    return -1;
  }

  /* Read the next file entry into a buffer. */
  for(count = 0, b = buffer; count < sizeof(buffer); ++b) {
    if(!_pfs_dirread1(dirp, b)) {
      return -1;
    }
    ++count;
    if(*b == '\0') {
      break;
    }
  }

  /* The end of the directory was reached if the buffer contains "blocks free."
  ** It is sufficient here to check for the leading 'b'. The buffer will have
  ** at least one byte if we come here.
  */
  if(buffer[0] == 'b') {
    return -1;
  }

  /* Parse the buffer for the file-name. */
  b = buffer;
  j = 0;
  s = 0;
  i = 0;
  while(i < count) {
    switch(s) {
    case 0:
      /* Search for the start of the file-name. */
      if(*b == '"') {
        s = 1;
      }
      break;
    case 1:
      /* Within the file-name. */
      if(*b == '"') {
        /* The end of the file-name was found. */
        dirent->name[j] = '\0';
        return 0;
      } else if(j < sizeof(dirent->name) - 1) {
        dirent->name[j] = *b;
        ++j;
      }
      break;
    }
    ++b;
    ++i;
  }

  /* The file-name is too long for the buffer. Return what could be read. */
  dirent->name[j] = '\0';
  return 0;
}
