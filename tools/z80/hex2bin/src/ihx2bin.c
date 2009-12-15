/*
 * Copyright (c) 2003-2008, Takahide Matsutsuka.
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
 * $Id: ihx2bin.c,v 1.3 2009/12/15 07:13:14 matsutsuka Exp $
 *
 */

/*
 * Intel HEX format (extended) to binary format conversion utility.
 */

#include <stdio.h>
#include <string.h>
#include "ihx2bin.h"

#define TYPE_DATA  0
#define TYPE_END   1
#define TYPE_STRING  2
#define TYPE_BYTE  3
#define TYPE_WORD  4

#define MEMORY_SIZE 0x10000

typedef struct {
  unsigned int start;
  unsigned int end;
  char buffer[MEMORY_SIZE];
  // current line
  int type;
  unsigned int address;
  unsigned int length;
} Memory;

static
const char NAME_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";

/**
 * Convert a character to a value.
 * @param ch a character to convert
 * @return integer value represents the given character
 */
static
int aton(const unsigned char ch) {
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
static
int getByte(FILE *in) {
  int ch1, ch2;
  if (feof(in)) {
    printf("eof");
    return -1;
  }
  ch1 = fgetc(in);
  if (feof(in)) {
    printf("eof");
    return -1;
  }
  ch2 = fgetc(in);
  return 16 * aton(ch1) + aton(ch2);
}

/**
 * @return non-zero if error
 */
static
void replace(FILE* in, struct ConvertInfo *info, Memory *memory) {
  int i, j;
  char name[DEF_NAMELEN];
  int len = 0;
  // read name
  while (len < DEF_NAMELEN - 1) {
    char ch = fgetc(in);
    if (!strchr(NAME_CHARS, ch)) {
      break;
    }
    name[len] = ch;
    len++;
  }
  name[len] = 0;

  for (i = 0; i < info->defsize; i++) {
    if (!strcmp(name, info->defs[i].name)) {
      int tmp;
      char value[DEF_VALUELEN];
      memset(value, 0, DEF_VALUELEN);

      // replace!
      switch (memory->type) {
      case TYPE_STRING:
	strncpy(&memory->buffer[memory->address], info->defs[i].value, memory->length);
	strncpy(value, &memory->buffer[memory->address], memory->length);
	if (info->verbose) {
	  printf("[%s]->[%s], ", name, value);
	}
	break;
      case TYPE_BYTE:
	tmp = 0;
	for (j = 0; j < 2; j++) {
	  if (aton(info->defs[i].value[j])) {
	    tmp = tmp * 16 + aton(info->defs[i].value[j]);
	  }
	}
	memory->buffer[memory->address] = tmp;
	if (info->verbose) {
	  printf("[%s]->[%02x], ", name, tmp);
	}
	break;
      case TYPE_WORD:
	tmp = 0;
	for (j = 0; j < 2; j++) {
	  tmp = tmp * 16 + aton(info->defs[i].value[j]);
	}
	memory->buffer[memory->address + 1] = tmp;
	tmp = 0;
	for (j = 2; j < 4; j++) {
	  tmp = tmp * 16 + aton(info->defs[i].value[j]);
	}
	memory->buffer[memory->address] = tmp;
	if (info->verbose) {
	  printf("[%s]->[%02x%02x], ", name,
		 memory->buffer[memory->address + 1],
		 memory->buffer[memory->address]);
	}
	break;
      }
      break;
    }
  }
}



/**
 * Extract a 64kB memory map from given file.
 * IHEX format is as follows:
 * :A_B___C_D_....D_E_
 * A_  : size of this chunk
 * B___: address (big endian)
 * C_  : record type (00: notmal data, 01: end)
 *        extension: 02: char, 03: byte(hex), 04: word(hex, little-endian)
 * D_....D_: data
 * E_  : check sum
 * :0DCCCF00673008D620D607D63013C937C904
 * :00000001FF
 * @param inFilename file name to convert
 * @param start pointer to start address
 * @param end pointer to end address
 * @return 0 if noerror, otherwise if error
 */
static
int ihx2mem(struct ConvertInfo *info, Memory *memory) {
  FILE *in;
  memory->start = MEMORY_SIZE - 1;
  memory->end = 0;

  in = fopen(info->filename, "rb");
  if (in == NULL) {
    printf("cannot open input file\n");
    return 1;
  }

  while(1) {
    int tmp;

    // skip checksum and cr/lf
    while (!feof(in)) {
      if (fgetc(in) == ':') {
        break;
      }
    }
    if (feof(in)) {
      break;
    }

    // get length of this chunk
    if ((memory->length = getByte(in)) < 0) {
      break;
    }

    // make an address
    if ((tmp = getByte(in)) < 0) {
      break;
    }
    memory->address = tmp * 256;
    if ((tmp = getByte(in)) < 0) {
      break;
    }
    memory->address += tmp;    

    // process record type
    if ((memory->type = getByte(in)) < 0) {
      break;
    }

    if (memory->type != TYPE_END) {
      // modify start and end
      if (memory->start > memory->address) {
	memory->start = memory->address;
      }
      if (memory->end < (memory->address + memory->length)) {
	memory->end = memory->address + memory->length;
      }
    }

    if (memory->type == TYPE_DATA) {
      while (memory->length > 0) {
	memory->buffer[memory->address] = getByte(in);
	memory->address++;
	memory->length--;
      }
    } else if (memory->type == TYPE_STRING
	       || memory->type == TYPE_BYTE
	       || memory->type == TYPE_WORD) {
      replace(in, info, memory);
    }
  }

  fclose(in);
  return 0;
}

/**
 * @return written size
 */
int ihx2bin(struct ConvertInfo *info) {
  Memory memory;
  unsigned int i;

  memset(&memory, 0, sizeof(Memory));
  
  if (info->verbose) {
    printf("importing ihx: %s, ", info->filename);
  }

  if (ihx2mem(info, &memory)) {
    printf("cannot open input file: %s\n", info->filename);
    return 0;
  }

  if (info->verbose) {
    printf("(%04x:%04x)\n", memory.start, memory.end);
  }

  for (i = memory.start; i < memory.end; i++) {
    putc(memory.buffer[i], info->out);
  }

  return (memory.end - memory.start);
}
