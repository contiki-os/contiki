/*
 * Copyright (c) 2003-2008, Takahide Matsutsuka.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The end-user documentation included with the redistribution, if
 *    any, must include the following acknowlegement:
 *       "This product includes software developed by Takahide Matsutsuka."
 *    Alternately, this acknowlegement may appear in the software itself,
 *    if and wherever such third-party acknowlegements normally appear.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Intel HEX format to PC-6001 CAS format conversion utility.
 */

#include <stdio.h>
#include "ihx2bin.h"

/**
 * Convert a character to a value.
 * @param ch a character to convert
 * @return integer value represents the given character
 */
static int aton(const unsigned char ch) {
  int n;
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  }

  return 0;
}

/**
 * Convert a byte characters from a given file and returns a byte.
 * @param in file
 * @return -1 if EOF
 */
static int getByte(FILE *in) {
  int ch1, ch2;
  ch1 = getc(in);
  if (ch1 == EOF) {
    return -1;
  }
  ch2 = getc(in);
  if (ch2 == EOF) {
    return -1;
  }
  return 16 * aton(ch1) + aton(ch2);
}

/**
 * Extract a 64kB memory map from given file.
 * IHEX format is as follows:
 * :A_B___C_D_....D_E_
 * A_  : size of this chunk
 * B___: address (big endian)
 * C_  : record type (00: notmal data, 01: end)
 * D_....D_: data
 * E_  : check sum
 * :0DCCCF00673008D620D607D63013C937C904
 * :00000001FF
 * @param inFilename file name to convert
 * @param start pointer to start address
 * @param end pointer to end address
 * @return 0 if noerror, otherwise if error
 */
static int ihx2mem(const char *inFilename, unsigned char *buffer, unsigned int *start, unsigned int *end) {
  FILE *in;
  *start = 0xffff;
  *end = 0;

  in = fopen(inFilename, "rb");
  if (in == NULL) {
    printf("cannot open input file\n");
    return 1;
  }

  while(1) {
    int ch;
    int length;
    unsigned int address;
    int tmp;

    // skip checksum and cr/lf
    while ((ch = getc(in)) != ':') {
      if (ch == EOF) {
        break;
      }
    }
    if (ch == EOF) {
      break;
    }

    // get length of this chunk
    length = getByte(in);
    if (length <= 0) {
      // TODO: end of bytes, retrieve variables
      break;
    }

    // make an address
    tmp = getByte(in);
    if (tmp < 0) {
      break;
    }
    address = tmp * 256;
    tmp = getByte(in);
    if (tmp < 0) {
      break;
    }
    address += tmp;    
    if (*start > address) {
      *start = address;
    }

    if (*end < (address + length)) {
      *end = address + length;
    }

    // ignore record type
    if (getByte(in) < 0) {
      break;
    }

    while (length > 0) {
      buffer[address] = getByte(in);
      address++;
      length--;
    }
  }

  fclose(in);
  return 0;
}

/**
 * @return written size
 */
int ihx2bin(FILE* dst, const char *src, unsigned char verbose) {
  unsigned int start, end;
  unsigned char buffer[65536];
  unsigned int i;

  memset(buffer, 0, 65536);
  
  if (ihx2mem(src, buffer, &start, &end)) {
    printf("cannot open input file: %s\n", src);
    return 0;
  }

  if (verbose) {
    printf("importing ihx : %s (%04x:%04x)\n", src, start, end);
  }
  for (i = start; i < end; i++) {
    putc(buffer[i], dst);
  }

  return (end - start);
}
