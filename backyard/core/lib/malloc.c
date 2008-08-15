/* Copyright (c) 2002, 2004 Joerg Wunsch
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
 
 
/* $Id: malloc.c,v 1.1 2008/08/15 19:05:17 adamdunkels Exp $ */

#include <stdlib.h>

#ifdef MALLOC_TEST
#include "malloc.h"
char mymem[256];
#else
#include "contiki.h"
#include "lib/malloc.h"
#endif /* !MALLOC_TEST */

/*
 * Exported interface:
 *
 * When extending the data segment, the allocator will not try to go
 * beyond the current stack limit, decreased by __malloc_margin bytes.
 * Thus, all possible stack frames of interrupt routines that could
 * interrupt the current function, plus all further nested function
 * calls must not require more stack space, or they'll risk to collide
 * with the data segment.
 */
 
char *__malloc_heap_end;

char *__brkval;
struct __freelist *__flp;

#define handle nx

void *
malloc(size_t len)
{
	struct __freelist *fp1, *fp2;
	char *cp;
	size_t s;

	if (len <= 0)
	    return 0;

	len = (len + MALLOC_ROUNDUP) & ~MALLOC_ROUNDUP;

	/*
	 * First, walk the free list and try finding a chunk that
	 * would match exactly.  If we found one, we are done.  While
	 * walking, note down the size of the largest chunk we found
	 * that would still fit the request -- we need it for step 2.
	 *
	 */
	for (s = 0, fp1 = __flp, fp2 = 0;
	     fp1;
	     fp2 = fp1, fp1 = fp1->nx) {
		if (fp1->sz == len) {
			/*
			 * Found it.  Disconnect the chunk from the
			 * freelist, and return it.
			 */
			if (fp2)
				fp2->nx = fp1->nx;
			else
				__flp = fp1->nx;
			fp1->handle = NULL;
			return &fp1[1];
		}
		if (fp1->sz > len) {
			if (s == 0 || fp1->sz < s)
				s = fp1->sz;
		}
	}
	/*
	 * Step 2: If we found a chunk on the freelist that would fit
	 * (but was too large), look it up again and use it, since it
	 * is our closest match now.  Since the freelist entry needs
	 * to be split into two entries then, watch out that the
	 * difference between the requested size and the size of the
	 * chunk found is large enough for another freelist entry; if
	 * not, just enlarge the request size to what we have found,
	 * and use the entire chunk.
	 */
	if (s) {
		if (s - len < sizeof(struct __freelist))
			len = s;
		for (fp1 = __flp, fp2 = 0;
		     fp1;
		     fp2 = fp1, fp1 = fp1->nx) {
			if (fp1->sz == s) {
				if (len == s) {
					/*
					 * Use entire chunk; same as
					 * above.
					 */
					if (fp2)
						fp2->nx = fp1->nx;
					else
						__flp = fp1->nx;
					fp1->handle = NULL;
					return &fp1[1];
				}
				/*
				 * Split them up.  Note that we leave
				 * the first part as the new (smaller)
				 * freelist entry, and return the
				 * upper portion to the caller.  This
				 * saves us the work to fix up the
				 * freelist chain; we just need to
				 * fixup the size of the current
				 * entry, and note down the size of
				 * the new chunk before returning it
				 * to the caller.
				 */
				cp = (char *)fp1;
				s -= len;
				cp += s;
				fp2 = (struct __freelist *)cp;
				fp2->sz = len;
				fp1->sz = s - sizeof(struct __freelist);
				fp2->handle = NULL;
				return &fp2[1];
			}
		}
	}
	/*
	 * Step 3: If the request could not be satisfied from a
	 * freelist entry, just prepare a new chunk.  This means we
	 * need to obtain more memory first.  The largest address just
	 * not allocated so far is remembered in the brkval variable.
	 * Under Unix, the "break value" was the end of the data
	 * segment as dynamically requested from the operating system.
	 * Since we don't have an operating system, just make sure
	 * that we don't collide with the stack.
	 */
	if (__brkval == 0)
		__brkval = sbrk(len + sizeof(struct __freelist));
	else if (sbrk(len + sizeof(struct __freelist)) == (void *)-1)
		return 0;	/* There's no help, just fail. :-/ */

	__malloc_heap_end = sbrk(0);

	fp1 = (struct __freelist *)__brkval;
	__brkval += len + sizeof(struct __freelist);
	fp1->sz = len;
	fp1->handle = NULL;
	return &fp1[1];
}

void
free(void *p)
{
	struct __freelist *fp1, *fp2, *fpnew;
	char *cp1, *cp2, *cpnew;

	/* ISO C says free(NULL) must be a no-op */
	if (p == 0)
		return;

	cpnew = p;
	cpnew -= sizeof(struct __freelist);
	fpnew = (struct __freelist *)cpnew;
	fpnew->nx = 0;

	/*
	 * Trivial case first: if there's no freelist yet, our entry
	 * will be the only one on it.
	 */
	if (__flp == 0) {
		__flp = fpnew;
		return;
	}

	/*
	 * Now, find the position where our new entry belongs onto the
	 * freelist.  Try to aggregate the chunk with adjacent chunks
	 * if possible.
	 */
	for (fp1 = __flp, fp2 = 0;
	     fp1;
	     fp2 = fp1, fp1 = fp1->nx) {
		if (fp1 < fpnew)
			continue;
		cp1 = (char *)fp1;
		fpnew->nx = fp1;
		if (((char *)&fpnew[1]) + fpnew->sz == cp1) {
			/* upper chunk adjacent, assimilate it */
			fpnew->sz += fp1->sz + sizeof(struct __freelist);
			fpnew->nx = fp1->nx;
		}
		if (fp2 == 0) {
			/* new head of freelist */
			__flp = fpnew;
			return;
		}
		break;
	}
	/*
	 * Note that we get here either if we hit the "break" above,
	 * or if we fell off the end of the loop.  The latter means
	 * we've got a new topmost chunk.  Either way, try aggregating
	 * with the lower chunk if possible.
	 */
	fp2->nx = fpnew;
	cp2 = (char *)&fp2[1];
	if (cp2 + fp2->sz == cpnew) {
		/* lower junk adjacent, merge */
		fp2->sz += fpnew->sz + sizeof(struct __freelist);
		fp2->nx = fpnew->nx;
	}
}

#ifdef MALLOC_TEST

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void *handles[32];
size_t sizes[32];


void *
alloc(size_t s)
{
	void *p;

	if ((p = malloc(s)) == 0) {
	        malloc_compact();
		if ((p = malloc(s)) == 0)
			return 0;
	}
	memset(p, 0xd0, s);
	return p;
}

void
printfreelist(void)
{
	struct __freelist *fp1;
	int i;

	if (!__flp) {
		printf("no free list\n");
		return;
	}

	for (i = 0, fp1 = __flp; fp1; i++, fp1 = fp1->nx) {
		printf("free  %d @ %u: size %u, next ",
		       i, (char *)fp1 - mymem, fp1->sz);
		if (fp1->nx)
			printf("%u\n", (char *)fp1->nx - mymem);
		else
			printf("NULL\n");
	}
}

int
compare(const void *p1, const void *p2)
{
	return *((size_t *)p1) - *((size_t *)p2);
}

void
printalloc(void)
{
	int j, k;
	size_t i;
	size_t sum, sum2;
	void *sortedhandles[32];
	struct __freelist *fp;
	char *cp;

	for (i = j = k = sum = sum2 = 0;
	     i < sizeof handles / sizeof (void *);
	     i++)
		if (sizes[i]) {
			j++;
			sum += sizes[i];
			if (handles[i]) {
				k++;
				sum2 += sizes[i];
			}
		}
	printf("brkval: %d, %d request%s => sum %u bytes "
	       "(actually %d reqs => %u bytes)\n",
	       (char *)__brkval - mymem, j, j == 1? "": "s", sum, k, sum2);
	memcpy(sortedhandles, handles, sizeof sortedhandles);
	qsort(sortedhandles, 32, sizeof(void *), compare);
	for (i = j = 0; i < sizeof sortedhandles / sizeof (void *); i++)
		if ((cp = sortedhandles[i])) {
			cp -= sizeof(struct __freelist);
			fp = (struct __freelist *)cp;
			printf("alloc %d @ %u: %u bytes, handle %p\n",
			       j, (char *)fp - mymem, fp->sz, fp->handle);
			j++;
		}

}


void
printblk(void)
{
    struct __freelist *fp = __flp;
    struct __freelist *ap = (struct __freelist *)__malloc_heap_start;
    struct __freelist *end = (struct __freelist *)__brkval;
    int b = 0;
    int e = 0;

    if (fp == 0)
        fp = end;

 alloclist:
    while (ap < fp) {
        printf("Alloc %d @ %u: %u bytes, handle %p\n",
	       b, (char *)ap - mymem, ap->sz, ap->handle);
	ap = (void *)(((char *)ap) + sizeof(struct __freelist) + ap->sz);
	b++;
    }

    if (fp < end) {
        printf("Free  %d @ %u: size %u, next ",
	       e, (char *)fp - mymem, fp->sz);
	if (fp->nx)
	    printf("%u\n", (char *)fp->nx - mymem);
	else
	    printf("NULL\n");
	ap = (void *)(((char *)fp) + sizeof(struct __freelist) + fp->sz);
	fp = fp->nx;
	if (fp == 0)
	    fp = end;
	e++;
    }

    if (ap < end)
        goto alloclist;
}

int
main(void)
{
	int i, j, k, l, m, om, p, f;
	size_t s;

	srand(time(0) ^ getpid());

	for (k = 0; k < 100; k++) {
		memset(handles, 0, sizeof handles);
		memset(sizes, 0, sizeof sizes);

		j = rand() % 16 + 15;
		l = rand() % 80 + 7;

		for (i = s = 0; i < j && s < 256; i++) {
			sizes[i] = rand() % l + 1;
			s += sizes[i];
		}
		j = i;
		for (m = om = 1, p = 1, f = 0; m < 1000; m++) {
			for (i = s = 0; i < j; i++)
				if (handles[i])
					s++;
			if (s == (unsigned)j)
				break;

			if (m / om > 10) {
				p <<= 1;
				p |= 1;
			}

			for (i = 0; i < j; i++)
				if (rand() & p) {
					if (!handles[i]) {
					  if ((handles[i] = alloc(sizes[i])) == 0)
						f++;
					  else if (sizes[i] & 1)
					  	malloc_unlock(&handles[i]);
					}
				}
			for (i = 0; i < j; i++) {
				if (rand() & 1) {
					free(handles[i]);
					handles[i] = 0;
				}
			}
		}
		if (f)
			printf("%d alloc failure%s total\n",
			       f, f == 1? "": "s");
		printf("After alloc:\n");
		printalloc();
		//printfreelist();
		printblk();
		for (i = 0; i < j; i++)
			free(handles[i]);
		printf("After cleanup:\n");
		printfreelist();
		//printblk();
	}

	return 0;
}

#endif /* MALLOC_TEST */
