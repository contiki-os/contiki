/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 * 
 */

#include "contiki-net.h"
#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-fsdata.h"

#include <string.h> /* for memcpy */

/* httpd-fsdata.c contains the web content.
 * It is generated using the PERL script /tools/makefsdata 
 */
#include "httpd-fsdata.c"

#if WEBSERVER_CONF_FILESTATS==1
uint16_t httpd_filecount[HTTPD_FS_NUMFILES];
#endif
/*-----------------------------------------------------------------------------------*/
void *
httpd_fs_get_root()
{
  return (void *)HTTPD_FS_ROOT;
}
/*-----------------------------------------------------------------------------------*/
uint16_t
httpd_fs_get_size()
{
  return HTTPD_FS_SIZE;
}
/*-----------------------------------------------------------------------------------*/
uint16_t
httpd_fs_open(const char *name, struct httpd_fs_file *file)
{
#if WEBSERVER_CONF_FILESTATS
  uint16_t i = 0;
#endif
  struct httpd_fsdata_file_noconst *f,fram;

  for(f = (struct httpd_fsdata_file_noconst *)HTTPD_FS_ROOT;
      f != NULL;
      f = (struct httpd_fsdata_file_noconst *)fram.next) {

    /*Get the linked list entry into ram from wherever it is */
    httpd_memcpy(&fram,f,sizeof(fram));

    /*Compare name passed in RAM with name in whatever flash the file is in */
	/*makefsdata no longer adds an extra zero byte at the end of the file */
    if(httpd_fs_strcmp((char *)name, fram.name) == 0) {
      if (file) {
        file->data = fram.data;
 //     file->len  = fram.len-1;
		file->len  = fram.len;
#if WEBSERVER_CONF_FILESTATS==2         //increment count in linked list field if it is in RAM
        f->count++;
      }
      return f->count;
    }
    ++i
#elif WEBSERVER_CONF_FILESTATS==1       //increment count in RAM array when linked list is in flash
        ++httpd_filecount[i];
      }
      return httpd_filecount[i];
    }
    ++i;
#else                              //no file statistics
      }
      return 1;
    }
#endif /* WEBSERVER_CONF_FILESTATS */
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
httpd_fs_init(void)
{
#if WEBSERVER_CONF_FILESTATS==1
  uint16_t i;
  for(i = 0; i < HTTPD_FS_NUMFILES; i++) {
    httpd_filecount[i] = 0;
  }
#endif
}
/*-----------------------------------------------------------------------------------*/
#if WEBSERVER_CONF_FILESTATS && 0
uint16_t
httpd_fs_count(char *name)
{
  struct httpd_fsdata_file_noconst *f;
  uint16_t i;

  i = 0;
  for(f = (struct httpd_fsdata_file_noconst *)HTTPD_FS_ROOT;
      f != NULL;
      f = (struct httpd_fsdata_file_noconst *)f->next) {

    if(httpd_fs_strcmp(name, f->name) == 0) {
      return httpd_filecount[i];
    }
    ++i;
  }
  return 0;
}
#endif
/*-----------------------------------------------------------------------------------*/
