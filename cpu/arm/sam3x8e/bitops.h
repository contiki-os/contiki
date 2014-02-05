/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
#include "compiler.h"
#include "string.h"



#ifndef BITOPS_H_
#define BITOPS_H_

#ifndef __WORDSIZE
#define __WORDSIZE (sizeof(long) * 8)
#endif

#define BITS_PER_LONG __WORDSIZE

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE           8
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]
	
inline int ilog2(unsigned long v)
{
         int l = 0;
         while ((1UL << l) < v)
                 l++;
         return l;
}


/**
 * sign_extend32 - sign extend a 32-bit value using specified bit as sign-bit
 * @value: value to sign extend
 * @index: 0 based bit index (0<=index<32) to sign bit
 */
static inline S32 sign_extend32(U32 value, int index)
{
	U8 shift = 31 - index;
	return (S32)(value << shift) >> shift;
}


static inline void bitmap_zero(unsigned long *dst, int nbits)
{
	/*
	if (small_const_nbits(nbits))
		*dst = 0UL;
	else {
	*/	
	    int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
	    memset(dst, 0, len);
	//}
}

/*
static __inline__ int test_bit(int nr, const unsigned long* addr)
{
         return (*((volatile unsigned char *)addr +
                ((nr >> 3) ^ 3)) & (1UL << (nr & 7))) != 0;
}
*/
static __inline__ int
test_bit (int nr, const volatile void *addr)
{
         return 1 & (((const volatile U32 *) addr)[nr >> 5] >> (nr & 31));
}

static inline void set_bit(int nr, unsigned long *addr)
{
         addr[nr / BITS_PER_LONG] |= 1UL << (nr % BITS_PER_LONG);
}
#endif /* BITOPS_H_ */