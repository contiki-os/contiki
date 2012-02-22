/*
 * Copyright (c) 2007, Takahide Matsutsuka.
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
 * $Id: mef.c,v 1.1 2007/11/28 06:13:24 matsutsuka Exp $
 *
 */
 
/*
 * \file
 *  mef.c
 *  The Micro Executable Format
 * \author
 *  Takahide Matsutsuka <markn@markn.org>
 */

#ifdef WITH_LOADER_ARCH
#include "contiki.h"
#include "loader/mef.h"

struct Area areas[MEF_AREA_MAX];

void
mef_load(unsigned char* offset)
{
  unsigned char* start = offset;
  unsigned char areasize = load_byte();
  uint16_t relocsize;
  unsigned int i, j;
  uint16_t checksum = 0;
  unsigned char* buf;
  struct Relocation reloc;
  
  for (i = 0; i < areasize; i++) {
    buf = (unsigned char *) &areas[i];
    for (j = 0; j < sizeof(struct Area); j++) {
      *buf++ = load_byte();
    }
  }
  
  for (i = 0; i < areasize; i++) {
    for (j = 0; j < areas[i].size; j++) {
      *offset = load_byte();
      checksum += *offset;
      offset++;
    }
    if (areas[i].checksum != checksum) {
      // Checksum error!
    }
  }
  
  // relocation information
  relocsize = load_byte();
  relocsize = (load_byte() << 8) + relocsize;
  for (i = 0; i < relocsize; i++) {
    buf = (unsigned char *) &reloc;
    for (j = 0; j < sizeof(struct Relocation); j++) {
      *buf++ = load_byte();
    }
    mef_reloc(start, &reloc);
  }
}

void
mef_reloc(unsigned char* offset, struct Relocation *reloc)
{
  if (reloc->mode & MEF_RELOC_ABSOLUTE) {
    return;
  }
  offset += reloc->address;
  if (reloc->mode & MEF_RELOC_MSB_BYTE) {
    *offset = (unsigned char) ((reloc->data + (uint16_t) offset) >> 8);
  } else if (reloc->mode & MEF_RELOC_LSB_BYTE) {
    *offset = (unsigned char) ((reloc->data + (uint16_t) offset) & 0xff);
  } else { /* word */
    *offset++ = (unsigned char) ((reloc->data + (uint16_t) offset) & 0xff);
    *offset = (unsigned char) ((reloc->data + (uint16_t) offset) >> 8);
  }
}


#endif /* WITH_LOADER_ARCH */
