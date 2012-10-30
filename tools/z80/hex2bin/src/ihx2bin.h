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
 *
 */

/*
 * A header file for ihx2bin
 * @author Takahide Matsutsuka <markn@markn.org>
 */

#ifndef __IHX2BIN_H__
#define __IHX2BIN_H__

#define DEF_MAX         1024
#define DEF_NAMELEN     256
#define DEF_VALUELEN    256

struct ConvertDefinition {
  char *name;
  char *value;
};

struct ConvertInfo {
  FILE* out;
  char* filename;
  unsigned char verbose;
  unsigned int defsize;
  struct ConvertDefinition *defs;
};

/* A default architecture-depend file name. */
#define DEFAULT_ARCH_FILENAME  "noname"

// int ihx2bin(FILE* dst, const char *src, unsigned char verbose);
int ihx2bin(struct ConvertInfo* info);

#endif /* __IHX2BIN_H__ */
