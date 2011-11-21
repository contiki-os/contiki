/*
 * Copyright (c) 2010, Kajtar Zsolt <soci@c64.rulez.org>.
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
 * Author: Kajtar Zsolt <soci@c64.rulez.org>
 *
 * $Id: urlconv.c,v 1.4 2010/09/29 09:35:56 oliverschmidt Exp $
 */

#include <string.h>

#include "cfs/cfs.h"
#include "http-strings.h"

#define ISO_number   0x23
#define ISO_percent  0x25
#define ISO_period   0x2e
#define ISO_slash    0x2f
#define ISO_question 0x3f

static char wwwroot[40];
static unsigned char wwwrootlen;

void
urlconv_init(void)
{
  int fd = cfs_open("wwwroot.cfg", CFS_READ);
  int rd = cfs_read(fd, wwwroot, sizeof(wwwroot));
  cfs_close(fd);
  if(rd != -1) wwwrootlen = rd;
}

/*---------------------------------------------------------------------------*/
/* URL to filename conversion
 *
 * prepends wwwroot prefix
 * normalizes path by removing "/./"
 * interprets "/../" and calculates path accordingly
 * resulting path is always absolute
 * replaces "%AB" notation with characters
 * strips "#fragment" and "?query" from end
 * replaces multiple slashes with a single one
 * rejects non-ASCII characters
 *
 * MAXLEN is including trailing zero!
 * input and output is ASCII
 */
void
urlconv_tofilename(char *dest, char *source, unsigned char maxlen)
{
  static unsigned char len;
  static unsigned char c, hex1;
  static unsigned char *from, *to;

  *dest = ISO_slash;
  strncpy(dest + 1, wwwroot, wwwrootlen);
  len = 0;
  from = source; to = dest + wwwrootlen;
  maxlen -= 2 + wwwrootlen;
  do {
    c = *(from++);
    switch(c) {
    case ISO_number:
    case ISO_question:
      c = 0;
      break;
    case ISO_percent:
      c = 0;
      hex1 = (*(from++) | 0x20) ^ 0x30;  // ascii only!
      if(hex1 > 0x50 && hex1 < 0x57)
        hex1 -= 0x47;
      else
        if(hex1 > 9)
          break;  // invalid hex
      c = (*(from++) | 0x20) ^ 0x30;  // ascii only!
      if(c > 0x50 && c < 0x57)
        c -= 0x47;
      else
        if(c > 9)
          break;  // invalid hex
      c |= hex1 << 4;
    }

    if(c < 0x20 || c > 0x7e)
      c = 0;  // non ascii?!
    if(len >= maxlen)
      c = 0;  // too long?

    if(c == ISO_slash || !c) {
      switch(*to) {
      case ISO_slash:
        continue;  // no repeated slash
      case ISO_period:
        switch(to[-1]) {
        case ISO_slash:  // handle "./" 
           --to; --len;
           continue;
        case ISO_period:
          if(to[-2] == ISO_slash) {
            to -= 2; len -= 2;
            if(len) {
              do {
                --to; --len;
              } while(*to != ISO_slash);
            }
            continue;
          }
        }
      }
    }
    if(c) {
      ++to; ++len; 
      *to = c;
    }
  } while(c);
  if(*to == ISO_slash && (len + sizeof(http_index_htm) - 3) < maxlen) {
    strcpy(to, http_index_htm);  // add index.htm
  } else {
    ++to;
    *to = 0;
  }
}
/*---------------------------------------------------------------------------*/
