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


#include <ieeefp.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <sys/config.h>
#include <sys/types.h>

#ifdef __IEEE_LITTLE_ENDIAN
#define IEEE_8087
#endif

#ifdef __IEEE_BIG_ENDIAN
#define IEEE_MC68k
#endif

#ifdef __Z8000__
#define Just_16
#endif

#ifdef DEBUG
#include "stdio.h"
#define Bug(x) {fprintf(stderr, "%s\n", x); exit(1);}
#endif

#ifdef Unsigned_Shifts
#define Sign_Extend(a,b) if (b < 0) a |= (__uint32_t)0xffff0000;
#else
#define Sign_Extend(a,b) /*no-op*/
#endif

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined.
#endif

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

/* The following definition of Storeinc is appropriate for MIPS processors.
 * An alternative that might be better on some machines is
 * #define Storeinc(a,b,c) (*a++ = b << 16 | c & 0xffff)
 */
#if defined (__IEEE_BYTES_LITTLE_ENDIAN) + defined (IEEE_8087) + defined (VAX)
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, \
((unsigned short *)a)[0] = (unsigned short)c, a++)
#else
#define Storeinc(a,b,c) (((unsigned short *)a)[0] = (unsigned short)b, \
((unsigned short *)a)[1] = (unsigned short)c, a++)
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

#ifndef IEEE_Arith
#define ROUND_BIASED
#endif

#ifdef RND_PRODQUOT
#define rounded_product(a,b) a = rnd_prod(a, b)
#define rounded_quotient(a,b) a = rnd_quot(a, b)
#ifdef KR_headers
extern double rnd_prod(), rnd_quot();
#else
extern double rnd_prod(double, double), rnd_quot(double, double);
#endif
#else
#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b
#endif

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 ((__uint32_t)0xffffffffL)

#ifndef Just_16
/* When Pack_32 is not defined, we store 16 bits per 32-bit long.
 * This makes some inner loops simpler and sometimes saves work
 * during multiplications, but it often seems to make things slightly
 * slower.  Hence the default is now to store 32 bits per long.
 */

#ifndef Pack_32
#define Pack_32
#endif
#endif


#ifdef __cplusplus
extern "C" double strtod(const char *s00, char **se);
extern "C" char *dtoa(double d, int mode, int ndigits,
			int *decpt, int *sign, char **rve);
#endif


typedef struct _Bigint _Bigint;

#if (defined (_SMALL_PRINTF) || defined(SMALL_SCANF) )
#define SMALL_LIB
#endif

#ifdef SMALL_LIB
 

 
 
#define small_Balloc	_small_Balloc
#define small_Bfree	_small_Bfree
#define small_multadd _small_multadd
#define small_s2b	_small_s2b
#define small_lo0bits _small_lo0bits
#define small_hi0bits _small_hi0bits
#define small_i2b	_small_i2b
#define small_mult	_small_multiply
#define small_pow5mult	_small_pow5mult
#define small_lshift	_small_lshift
#define small_cmp	__small_mcmp
#define small_diff	__small_mdiff
#define small_ulp 	_small_ulp
#define small_b2d	_small_b2d
#define small_d2b	_small_d2b
#define small_ratio	_small_ratio

#define small_tens __small_mprec_tens
#define small_bigtens __small_mprec_bigtens
#define small_tinytens __small_mprec_tinytens

struct _reent ;
double 		_EXFUN(small_ulp,(double x));
double		_EXFUN(small_b2d,(_Bigint *a , int *e));
_Bigint *	_EXFUN(small_multadd,(struct _reent *p, _Bigint *, int, int,_Bigint tab[]));
_Bigint *	_EXFUN(small_s2b,(struct _reent *, const char*, int, int, __ULong,_Bigint tab[]));
_Bigint	*	_EXFUN(small_i2b,(struct _reent *,int,_Bigint tab[]));
_Bigint *	_EXFUN(small_mult, (struct _reent *, _Bigint *, _Bigint *,_Bigint tab[]));
_Bigint *	_EXFUN(small_pow5mult, (struct _reent *, _Bigint *, int k,_Bigint tab[]));
int 		_EXFUN(small_hi0bits,(__ULong));
int 		_EXFUN(small_lo0bits,(__ULong *));
_Bigint *        _EXFUN(small_d2b,(struct _reent *p, double d, int *e, int *bits,_Bigint tab[]));
_Bigint *        _EXFUN(small_lshift,(struct _reent *p, _Bigint *b, int k,_Bigint tab[]));
_Bigint *        _EXFUN(small_diff,(struct _reent *p, _Bigint *a, _Bigint *b,_Bigint tab[]));
int             _EXFUN(small_cmp,(_Bigint *a, _Bigint *b));

double		_EXFUN(small_ratio,(_Bigint *a, _Bigint *b));
#define Bcopy(x,y) memcpy((char *)&x->_sign, (char *)&y->_sign, y->_wds*sizeof(__Long) + 2*sizeof(int))

#if defined(_DOUBLE_IS_32BITS) && defined(__v800)
#define n_bigtens 2
#else
#define n_bigtens 5
#endif

extern _CONST double small_tinytens[];
extern _CONST double small_bigtens[];
extern _CONST double small_tens[];


double _EXFUN(_small_mprec_log10,(int));


#else // NO SMALL_LIB



#define Balloc	_Balloc
#define Bfree	_Bfree
#define multadd _multadd
#define s2b	_s2b
#define lo0bits _lo0bits
#define hi0bits _hi0bits
#define i2b	_i2b
#define mult	_multiply
#define pow5mult	_pow5mult
#define lshift	_lshift
#define cmp	__mcmp
#define diff	__mdiff
#define ulp 	_ulp
#define b2d	_b2d
#define d2b	_d2b
#define ratio	_ratio

#define tens __mprec_tens
#define bigtens __mprec_bigtens
#define tinytens __mprec_tinytens

struct _reent ;
double 		_EXFUN(ulp,(double x));
double		_EXFUN(b2d,(_Bigint *a , int *e));
_Bigint *	_EXFUN(Balloc,(struct _reent *p, int k));
void 		_EXFUN(Bfree,(struct _reent *p, _Bigint *v));
_Bigint *	_EXFUN(multadd,(struct _reent *p, _Bigint *, int, int));
_Bigint *	_EXFUN(s2b,(struct _reent *, const char*, int, int, __ULong));
_Bigint	*	_EXFUN(i2b,(struct _reent *,int));
_Bigint *	_EXFUN(mult, (struct _reent *, _Bigint *, _Bigint *));
_Bigint *	_EXFUN(pow5mult, (struct _reent *, _Bigint *, int k));
int 		_EXFUN(hi0bits,(__ULong));
int 		_EXFUN(lo0bits,(__ULong *));
_Bigint *        _EXFUN(d2b,(struct _reent *p, double d, int *e, int *bits));
_Bigint *        _EXFUN(lshift,(struct _reent *p, _Bigint *b, int k));
_Bigint *        _EXFUN(diff,(struct _reent *p, _Bigint *a, _Bigint *b));
int             _EXFUN(cmp,(_Bigint *a, _Bigint *b));

double		_EXFUN(ratio,(_Bigint *a, _Bigint *b));
#define Bcopy(x,y) memcpy((char *)&x->_sign, (char *)&y->_sign, y->_wds*sizeof(__Long) + 2*sizeof(int))

#if defined(_DOUBLE_IS_32BITS) && defined(__v800)
#define n_bigtens 2
#else
#define n_bigtens 5
#endif

extern _CONST double tinytens[];
extern _CONST double bigtens[];
extern _CONST double tens[];


double _EXFUN(_mprec_log10,(int));

#endif
