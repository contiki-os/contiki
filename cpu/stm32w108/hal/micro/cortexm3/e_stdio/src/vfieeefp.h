/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

/* Please send bug reports to
	David M. Gay
	AT&T Bell Laboratories, Room 2C-463
	600 Mountain Avenue
	Murray Hill, NJ 07974-2070
	U.S.A.
	dmg@research.att.com or research!dmg
 */

/* This header file is a modification of mprec.h that only contains floating
   point union code. */

#include <ieeefp.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <sys/config.h>

#ifdef __IEEE_LITTLE_ENDIAN
#define IEEE_8087
#endif

#ifdef __IEEE_BIG_ENDIAN
#define IEEE_MC68k
#endif

#ifdef __Z8000__
#define Just_16
#endif

#ifdef Unsigned_Shifts
#define Sign_Extend(a,b) if (b < 0) a |= (__uint32_t)0xffff0000;
#else
#define Sign_Extend(a,b) /*no-op*/
#endif

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined.
#endif

#ifdef WANT_IO_LONG_DBL
/* If we are going to examine or modify specific bits in a long double using
   the lword0 or lwordx macros, then we must wrap the long double inside
   a union.  This is necessary to avoid undefined behavior according to
   the ANSI C spec.  */

#ifdef IEEE_8087
#if LDBL_MANT_DIG == 24
struct ldieee
{
  unsigned manh:23;
  unsigned exp:8;
  unsigned sign:1;
};
#elif LDBL_MANT_DIG == 53
struct ldieee
{
  unsigned manl:20;
  unsigned manh:32;
  unsigned exp:11;
  unsigned sign:1;
};
#elif LDBL_MANT_DIG == 64
struct ldieee
{
  unsigned manl:32;
  unsigned manh:32;
  unsigned exp:15;
  unsigned sign:1;
};
#elif LDBL_MANT_DIG > 64
struct ldieee
{
  unsigned manl3:16;
  unsigned manl2:32;
  unsigned manl:32;
  unsigned manh:32;
  unsigned exp:15;
  unsigned sign:1;
};
#endif /* LDBL_MANT_DIG */
#else  /* !IEEE_8087 */
#if LDBL_MANT_DIG == 24
struct ldieee
{
  unsigned sign:1;
  unsigned exp:8;
  unsigned manh:23;
};
#elif LDBL_MANT_DIG == 53
struct ldieee
{
  unsigned sign:1;
  unsigned exp:11;
  unsigned manh:32;
  unsigned manl:20;
};
#elif LDBL_MANT_DIG == 64
struct ldieee
{
  unsigned sign:1;
  unsigned exp:15;
  unsigned manh:32;
  unsigned manl:32;
};
#elif LDBL_MANT_DIG > 64
struct ldieee
{
  unsigned sign:1;
  unsigned exp:15;
  unsigned manh:32;
  unsigned manl:32;
  unsigned manl2:32;
  unsigned manl3;16;
};
#endif /* LDBL_MANT_DIG */
#endif /* !IEEE_8087 */
#endif /* WANT_IO_LONG_DBL */

/* If we are going to examine or modify specific bits in a double using
   the word0 and/or word1 macros, then we must wrap the double inside
   a union.  This is necessary to avoid undefined behavior according to
   the ANSI C spec.  */
union double_union
{
  double d;
  __uint32_t i[2];
};

#ifdef IEEE_8087
#define word0(x) (x.i[1])
#define word1(x) (x.i[0])
#else
#define word0(x) (x.i[0])
#define word1(x) (x.i[1])
#endif

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#if defined(IEEE_8087) + defined(IEEE_MC68k)
#if defined (_DOUBLE_IS_32BITS) 
#define Exp_shift   23
#define Exp_shift1  23
#define Exp_msk1    ((__uint32_t)0x00800000L)
#define Exp_msk11   ((__uint32_t)0x00800000L)
#define Exp_mask    ((__uint32_t)0x7f800000L)
#define P    	    24
#define Bias 	    127
#if 0
#define IEEE_Arith  /* it is, but the code doesn't handle IEEE singles yet */
#endif
#define Emin        (-126)
#define Exp_1       ((__uint32_t)0x3f800000L)
#define Exp_11      ((__uint32_t)0x3f800000L)
#define Ebits 	    8
#define Frac_mask   ((__uint32_t)0x007fffffL)
#define Frac_mask1  ((__uint32_t)0x007fffffL)
#define Ten_pmax    10
#define Sign_bit    ((__uint32_t)0x80000000L)
#define Ten_pmax    10
#define Bletch	    2
#define Bndry_mask  ((__uint32_t)0x007fffffL)
#define Bndry_mask1 ((__uint32_t)0x007fffffL)
#define LSB 1
#define Sign_bit    ((__uint32_t)0x80000000L)
#define Log2P 	    1
#define Tiny0 	    0
#define Tiny1 	    1
#define Quick_max   5
#define Int_max     6
#define Infinite(x) (word0(x) == ((__uint32_t)0x7f800000L))
#undef word0
#undef word1

#define word0(x) (x.i[0])
#define word1(x) 0
#else

#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    ((__uint32_t)0x100000L)
#define Exp_msk11   ((__uint32_t)0x100000L)
#define Exp_mask  ((__uint32_t)0x7ff00000L)
#define P 53
#define Bias 1023
#define IEEE_Arith
#define Emin (-1022)
#define Exp_1  ((__uint32_t)0x3ff00000L)
#define Exp_11 ((__uint32_t)0x3ff00000L)
#define Ebits 11
#define Frac_mask  ((__uint32_t)0xfffffL)
#define Frac_mask1 ((__uint32_t)0xfffffL)
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  ((__uint32_t)0xfffffL)
#define Bndry_mask1 ((__uint32_t)0xfffffL)
#define LSB 1
#define Sign_bit ((__uint32_t)0x80000000L)
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define Infinite(x) (word0(x) == ((__uint32_t)0x7ff00000L)) /* sufficient test for here */
#endif

#else
#undef  Sudden_Underflow
#define Sudden_Underflow
#ifdef IBM
#define Exp_shift  24
#define Exp_shift1 24
#define Exp_msk1   ((__uint32_t)0x1000000L)
#define Exp_msk11  ((__uint32_t)0x1000000L)
#define Exp_mask  ((__uint32_t)0x7f000000L)
#define P 14
#define Bias 65
#define Exp_1  ((__uint32_t)0x41000000L)
#define Exp_11 ((__uint32_t)0x41000000L)
#define Ebits 8	/* exponent has 7 bits, but 8 is the right value in b2d */
#define Frac_mask  ((__uint32_t)0xffffffL)
#define Frac_mask1 ((__uint32_t)0xffffffL)
#define Bletch 4
#define Ten_pmax 22
#define Bndry_mask  ((__uint32_t)0xefffffL)
#define Bndry_mask1 ((__uint32_t)0xffffffL)
#define LSB 1
#define Sign_bit ((__uint32_t)0x80000000L)
#define Log2P 4
#define Tiny0 ((__uint32_t)0x100000L)
#define Tiny1 0
#define Quick_max 14
#define Int_max 15
#else /* VAX */
#define Exp_shift  23
#define Exp_shift1 7
#define Exp_msk1    0x80
#define Exp_msk11   ((__uint32_t)0x800000L)
#define Exp_mask  ((__uint32_t)0x7f80L)
#define P 56
#define Bias 129
#define Exp_1  ((__uint32_t)0x40800000L)
#define Exp_11 ((__uint32_t)0x4080L)
#define Ebits 8
#define Frac_mask  ((__uint32_t)0x7fffffL)
#define Frac_mask1 ((__uint32_t)0xffff007fL)
#define Ten_pmax 24
#define Bletch 2
#define Bndry_mask  ((__uint32_t)0xffff007fL)
#define Bndry_mask1 ((__uint32_t)0xffff007fL)
#define LSB ((__uint32_t)0x10000L)
#define Sign_bit ((__uint32_t)0x8000L)
#define Log2P 1
#define Tiny0 0x80
#define Tiny1 0
#define Quick_max 15
#define Int_max 15
#endif
#endif


