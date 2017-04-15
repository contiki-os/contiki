/*
 * Copyright (c) 2013, KTH, Royal Institute of Technology
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
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
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
 */

#include <sys/stat.h>
#include "debug_frmwrk.h"

#define CR     0x0D

int
_close(int file)
{
  return -1;
}
int
_fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}
int
_isatty(int file)
{
  return 1;
}
int
_lseek(int file, int ptr, int dir)
{
  return 0;
}
int
_open(const char *name, int flags, int mode)
{
  return -1;
}
int
_read(int file, char *ptr, int len)
{
  int todo;
  if(len == 0) {
    return 0;
  }

  for(todo = 0; todo < len; todo++) {
    *ptr++ = _DG;
  }
  return todo;
}
/*
 * sbrk -- changes heap size size. Get nbytes more
 * RAM. We just increment a pointer in what’s
 * left of memory on the board.
 */
char *heap_end = 0;
caddr_t
_sbrk(int incr)
{
  extern char __heap_start__; /* Defined by the linker */
  extern char __heap_end__; /* Defined by the linker */
  char *prev_heap_end;

  if(heap_end == 0) {
    heap_end = &__heap_start__;
  }
  prev_heap_end = heap_end;

  if(heap_end + incr > &__heap_end__) {
    /* Heap and stack collision */
    return (caddr_t)0;
  }
  heap_end += incr;
  return (caddr_t)prev_heap_end;
}
int
_write(int file, char *ptr, int len)
{
  int todo;
  char ch;

  for(todo = 0; todo < len; todo++) {
    ch = *ptr++;
    _DBC(ch);
    if(ch == '\n') {
      _DBC(CR);
    }
  }
  return len;
}
