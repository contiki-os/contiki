/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: malloc_compact.c,v 1.1 2008/08/15 19:05:17 adamdunkels Exp $
 */

#include <stdlib.h>
#include <string.h>

#ifdef MALLOC_TEST
#include "malloc.h"
#else
#include "contiki.h"
#include "lib/malloc.h"
#endif /* !MALLOC_TEST */

#define handle nx

void
malloc_unlock(void **h)
{
    struct __freelist *t = *h;
    t = &t[-1];
    t->handle = (void *)h;
}

void
malloc_lock(void **h)
{
    struct __freelist *t = *h;
    t = &t[-1];
    t->handle = NULL;
}

static int
compact1(struct __freelist **flp)
{
    struct __freelist *fp = *flp, *fp2;
    struct __freelist *ap1, *ap2;
    struct __freelist *end = (struct __freelist *)__brkval;

    ap1 = (void *)(((char *)fp) + sizeof(struct __freelist) + fp->sz);
    if (ap1 >= end)
        return 0;
    if (ap1->handle == NULL)
        return 0;

    ap2 = (void *)(((char *)ap1) + sizeof(struct __freelist) + ap1->sz);
    fp2 = fp->nx;

    if (ap2 != fp2) {		/* 2 blocks become 2 */
        size_t s = fp->sz;

	fp->sz = ap1->sz;
	fp->handle = ap1->handle;
	memcpy(&fp[1], &ap1[1], ap1->sz);
	*(void **)(fp->handle) = &fp[1];

	fp = (void *)(((char *)fp) + sizeof(struct __freelist) + fp->sz);
	fp->sz = s;
	fp->nx = fp2;
	*flp = fp;
	return 1;
    }

    if (ap2 == fp2) {		/* 3 blocks become 2 */
        size_t s = fp->sz + fp2->sz + sizeof(struct __freelist);

	fp->sz = ap1->sz;
	fp->handle = ap1->handle;
	memcpy(&fp[1], &ap1[1], ap1->sz);
	*(void **)(fp->handle) = &fp[1];

	fp = (void *)(((char *)fp) + sizeof(struct __freelist) + fp->sz);
	fp->sz = s;
	fp->nx = fp2->nx;
	*flp = fp;
	return 1;
    }

    return 0;
}

void
malloc_compact(void)
{
    struct __freelist **flp;

    flp = &__flp;
    while (*flp != NULL) {
      while (compact1(flp))
	;
      flp = &((*flp)->nx);
    }
}

void
malloc_compact_one(void)
{
    struct __freelist **flp;

    flp = &__flp;
    while (*flp != NULL) {
      if (compact1(flp))
	return ;
      flp = &((*flp)->nx);
    }
}
