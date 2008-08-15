/* Copyright (c) 2004, Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/* $Id: malloc.h,v 1.1 2008/08/15 19:05:17 adamdunkels Exp $ */

#ifndef MALLOC_H
#define MALLOC_H

#if !defined(__DOXYGEN__)

struct __freelist {
	size_t sz;
	struct __freelist *nx;
};

#endif

/*
 * Only unlocked memory can be subject to compaction.
 *
 * Malloc and realloc return locked memory.
 */
void malloc_unlock(void **h);
void malloc_lock(void **h);
void malloc_compact(void);
void malloc_compact_one(void);

void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

extern char *__brkval;		/* first location not yet allocated */
extern struct __freelist *__flp; /* freelist pointer (head of freelist) */
extern char *__malloc_heap_end;

#define MALLOC_ROUNDUP (sizeof(int) - 1)

#ifdef MALLOC_TEST
/*
 * When compiling malloc.c/realloc.c natively on a host machine, it will
 * include a main() that performs a regression test.  This is meant as
 * a debugging aid, where a normal source-level debugger will help to
 * verify that the various allocator structures have the desired
 * appearance at each stage.
 *
 */

extern void *mymalloc(size_t);
extern void myfree(void *);
extern void *myrealloc(void *, size_t);

#define malloc mymalloc
#define free myfree
#define realloc myrealloc

#define __malloc_heap_start &mymem[0]
#define __malloc_heap_end &mymem[256]
extern char mymem[];    

#endif /* MALLOC_TEST */

#endif /* MALLOC_H */
