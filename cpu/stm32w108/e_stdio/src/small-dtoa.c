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
 
 
#ifndef _SMALL_PRINTF

#define small_lo0bits 	lo0bits
#define small_hi0bits 	hi0bits
#define small_i2b			i2b
#define small_cmp			cmp
#define small_ulp 		ulp
#define small_b2d			b2d
#define small_d2b			d2b
#define small_ratio		ratio

#define small_tens 		tens
#define small_bigtens 	bigtens
#define small_tinytens 	tinytens 

#endif
 
 
 
 

#include <_ansi.h>
#include <stdlib.h>

#ifndef _SMALL_PRINTF
#include <reent.h>
#endif

#include <string.h>
#include "small-mprec.h"

static int
_DEFUN (quorem,
	(b, S),
	_Bigint * b _AND _Bigint * S)
{
  int n;
  __Long borrow, y;
  __ULong carry, q, ys;
  __ULong *bx, *bxe, *sx, *sxe;
#ifdef Pack_32
  __Long z;
  __ULong si, zs;
#endif

  n = S->_wds;
#ifdef DEBUG
  /*debug*/ if (b->_wds > n)
    /*debug*/ Bug ("oversize b in quorem");
#endif
  if (b->_wds < n)
    return 0;
  sx = S->_x;
  sxe = sx + --n;
  bx = b->_x;
  bxe = bx + n;
  q = *bxe / (*sxe + 1);	/* ensure q <= true quotient */
#ifdef DEBUG
  /*debug*/ if (q > 9)
    /*debug*/ Bug ("oversized quotient in quorem");
#endif
  if (q)
    {
      borrow = 0;
      carry = 0;
      do
	{
#ifdef Pack_32
	  si = *sx++;
	  ys = (si & 0xffff) * q + carry;
	  zs = (si >> 16) * q + (ys >> 16);
	  carry = zs >> 16;
	  y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
	  borrow = y >> 16;
	  Sign_Extend (borrow, y);
	  z = (*bx >> 16) - (zs & 0xffff) + borrow;
	  borrow = z >> 16;
	  Sign_Extend (borrow, z);
	  Storeinc (bx, z, y);
#else
	  ys = *sx++ * q + carry;
	  carry = ys >> 16;
	  y = *bx - (ys & 0xffff) + borrow;
	  borrow = y >> 16;
	  Sign_Extend (borrow, y);
	  *bx++ = y & 0xffff;
#endif
	}
      while (sx <= sxe);
      if (!*bxe)
	{
	  bx = b->_x;
	  while (--bxe > bx && !*bxe)
	    --n;
	  b->_wds = n;
	}
    }
  
  if (small_cmp (b, S) >= 0)
    {
      q++;
      borrow = 0;
      carry = 0;
      bx = b->_x;
      sx = S->_x;
      do
	{
#ifdef Pack_32
	  si = *sx++;
	  ys = (si & 0xffff) + carry;
	  zs = (si >> 16) + (ys >> 16);
	  carry = zs >> 16;
	  y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
	  borrow = y >> 16;
	  Sign_Extend (borrow, y);
	  z = (*bx >> 16) - (zs & 0xffff) + borrow;
	  borrow = z >> 16;
	  Sign_Extend (borrow, z);
	  Storeinc (bx, z, y);
#else
	  ys = *sx++ + carry;
	  carry = ys >> 16;
	  y = *bx - (ys & 0xffff) + borrow;
	  borrow = y >> 16;
	  Sign_Extend (borrow, y);
	  *bx++ = y & 0xffff;
#endif
	}
      while (sx <= sxe);
      bx = b->_x;
      bxe = bx + n;
      if (!*bxe)
	{
	  while (--bxe > bx && !*bxe)
	    --n;
	  b->_wds = n;
	}
    }
  return q;
}

/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *	1. Rather than iterating, we use a simple numeric overestimate
 *	   to determine k = floor(log10(d)).  We scale relevant
 *	   quantities using O(log2(k)) rather than O(k) multiplications.
 *	2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *	   try to generate digits strictly left to right.  Instead, we
 *	   compute with fewer bits and propagate the carry if necessary
 *	   when rounding the final digit up.  This is often faster.
 *	3. Under the assumption that input will be rounded nearest,
 *	   mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *	   That is, we allow equality in stopping tests when the
 *	   round-nearest rule will give the same floating-point value
 *	   as would satisfaction of the stopping test with strict
 *	   inequality.
 *	4. We remove common factors of powers of 2 from relevant
 *	   quantities.
 *	5. When converting floating-point integers less than 1e16,
 *	   we use floating-point arithmetic rather than resorting
 *	   to multiple-precision integers.
 *	6. When asked to produce fewer than 15 digits, we first try
 *	   to get by with floating-point arithmetic; we resort to
 *	   multiple-precision integer arithmetic only if we cannot
 *	   guarantee that the floating-point calculation has given
 *	   the correctly rounded result.  For k requested digits and
 *	   "uniformly" distributed input, the probability is
 *	   something like 10^(k-15) that we must resort to the long
 *	   calculation.
 */
 
 
 /* Scanf and printf call both the small-mprec.c file if small_printf 
  * has not been specfied optimizations concerning small-mprec.c and
  * call of balloc will be performed anyway for printf. 
  */
 
#ifdef SMALL_SCANF
#ifndef  _SMALL_PRINTF 
#define  _SMALL_PRINTF 
#endif
#endif
 


char *
_DEFUN (_dtoa_r,
	(ptr, _d, mode, ndigits, decpt, sign, rve),
	struct _reent *ptr _AND
	double _d _AND
	int mode _AND
	int ndigits _AND
	int *decpt _AND
	int *sign _AND
	char **rve)
{
  /*	Arguments ndigits, decpt, sign are similar to those
	of ecvt and fcvt; trailing zeros are suppressed from
	the returned string.  If not null, *rve is set to point
	to the end of the return value.  If d is +-Infinity or NaN,
	then *decpt is set to 9999.

	mode:
		0 ==> shortest string that yields d when read in
			and rounded to nearest.
		1 ==> like 0, but with Steele & White stopping rule;
			e.g. with IEEE P754 arithmetic , mode 0 gives
			1e23 whereas mode 1 gives 9.999999999999999e22.
		2 ==> max(1,ndigits) significant digits.  This gives a
			return value similar to that of ecvt, except
			that trailing zeros are suppressed.
		3 ==> through ndigits past the decimal point.  This
			gives a return value similar to that from fcvt,
			except that trailing zeros are suppressed, and
			ndigits can be negative.
		4-9 should give the same return values as 2-3, i.e.,
			4 <= mode <= 9 ==> same return as mode
			2 + (mode & 1).  These modes are mainly for
			debugging; often they run slower but sometimes
			faster than modes 2-3.
		4,5,8,9 ==> left-to-right digit generation.
		6-9 ==> don't try fast floating-point estimate
			(if applicable).

		Values of mode other than 0-9 are treated as mode 0.

		Sufficient space is allocated to the return value
		to hold the suppressed trailing zeros.
	*/

  int bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1, j, j1, k, k0,
    k_check, leftright, m2, m5, s2, s5, spec_case, try_quick;
  union double_union d, d2, eps;
  __Long L;
#ifndef Sudden_Underflow
  int denorm;
  __ULong x;
#endif
  _Bigint *b, *b1, *delta, *mlo = NULL, *mhi, *S;
  double ds;
  #ifndef _SMALL_PRINTF
  char *s, *s0;
  
  #else //Declarations for SMALL_PRINTF
  
  /*
   *  SIZE have been chosen regarding size allocated by default printf it seems that most of time 32 is sufficient except 
   *  for lshift that allocated 40
   *  Nevertheless for some examples greater buffer size can be usefull. 
   */
  
  #define BUF_LSHIFT_SIZE 40// Size of each buffer for variables of _Bigint type
  #define BUF_SIZE  32
  #define S0_SIZE  32 // Size of the buffer result that will be provided by _dtoa_r
  
  /*  
   *	For the SMALL_PRINTF implementation for floating points numbers :  
  	*  - To avoid the call of allocator we defined a buffer for each variable : instead of taking the adress 
  	*  provided by Balloc variables are initialized to the beginning of the array.
  	*	- For some variables many buffers have been declared, in fact for each call of small_lshift we used a 
  	*  buffer that has not been used at the moment 
   *  - This buffers are used in the call of function declared in small-mprec.h 
   *  To have more informations look at small-mprec.c 
   */
  
_Bigint tab_b[BUF_LSHIFT_SIZE],tab_b1[BUF_SIZE],tab_delta[BUF_SIZE],tab_mlo[BUF_SIZE],tab_mhi[BUF_LSHIFT_SIZE],tab_S[BUF_LSHIFT_SIZE];
_Bigint tab_blshift[BUF_LSHIFT_SIZE],tab_Slshift[BUF_LSHIFT_SIZE],tab_mhilshift[BUF_LSHIFT_SIZE],tab_mlolshift[BUF_LSHIFT_SIZE];
  char tab_s0[S0_SIZE];
  char *s, *s0;
  #endif  //Declarations for SMALL_PRINTF

   d.d = _d;
#ifndef _SMALL_PRINTF
  _REENT_CHECK_MP(ptr);
  if (_REENT_MP_RESULT(ptr))
    {
      _REENT_MP_RESULT(ptr)->_k = _REENT_MP_RESULT_K(ptr);
      _REENT_MP_RESULT(ptr)->_maxwds = 1 << _REENT_MP_RESULT_K(ptr);     
      Bfree (ptr, _REENT_MP_RESULT(ptr));
      _REENT_MP_RESULT(ptr) = 0;
    }
 #endif
  if (word0 (d) & Sign_bit)
    {
      /* set sign for everything, including 0's and NaNs */
      *sign = 1;
      word0 (d) &= ~Sign_bit;	/* clear sign bit */
    }
  else
    *sign = 0;

#if defined(IEEE_Arith) + defined(VAX)
#ifdef IEEE_Arith
  if ((word0 (d) & Exp_mask) == Exp_mask)
#else
  if (word0 (d) == 0x8000)
#endif
    {
      /* Infinity or NaN */
      *decpt = 9999;
      s =
#ifdef IEEE_Arith
	!word1 (d) && !(word0 (d) & 0xfffff) ? "Infinity" :
#endif
	"NaN";
      if (rve)
	*rve =
#ifdef IEEE_Arith
	  s[3] ? s + 8 :
#endif
	  s + 3;


      return s;
    }

#ifdef IBM
  d.d += 0;			/* normalize */
#endif
  if (!d.d)
    {
      *decpt = 1;
      s = "0";
      if (rve)
         *rve = s + 1;
          
      return s;
    }
   #ifdef _SMALL_PRINTF
   b = small_d2b (ptr, d.d, &be, &bbits,&tab_b[0]);
   #else
   b = small_d2b (ptr, d.d, &be, &bbits);
   #endif
  
#ifdef Sudden_Underflow
  i = (int) (word0 (d) >> Exp_shift1 & (Exp_mask >> Exp_shift1));
#else
  if ((i = (int) (word0 (d) >> Exp_shift1 & (Exp_mask >> Exp_shift1))) != 0)
    {
#endif
      d2.d = d.d;
      word0 (d2) &= Frac_mask1;
      word0 (d2) |= Exp_11;
#ifdef IBM
      if (j = 11 - hi0bits (word0 (d2) & Frac_mask))
	d2.d /= 1 << j;
#endif

      /* log(x)	~=~ log(1.5) + (x-1.5)/1.5
		 * log10(x)	 =  log(x) / log(10)
		 *		~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
		 * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
		 *
		 * This suggests computing an approximation k to log10(d) by
		 *
		 * k = (i - Bias)*0.301029995663981
		 *	+ ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
		 *
		 * We want k to be too large rather than too small.
		 * The error in the first-order Taylor series approximation
		 * is in our favor, so we just round up the constant enough
		 * to compensate for any error in the multiplication of
		 * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
		 * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
		 * adding 1e-13 to the constant term more than suffices.
		 * Hence we adjust the constant term to 0.1760912590558.
		 * (We could get a more accurate k by invoking log10,
		 *  but this is probably not worthwhile.)
		 */

      i -= Bias;
#ifdef IBM
      i <<= 2;
      i += j;
#endif
#ifndef Sudden_Underflow
      denorm = 0;
    }
  else
    {
      /* d is denormalized */

      i = bbits + be + (Bias + (P - 1) - 1);
      x = (i > 32) ? (word0 (d) << (64 - i)) | (word1 (d) >> (i - 32))
       : (word1 (d) << (32 - i));
      d2.d = x;
      word0 (d2) -= 31 * Exp_msk1;	/* adjust exponent */
      i -= (Bias + (P - 1) - 1) + 1;
      denorm = 1;
    }
#endif
  ds = (d2.d - 1.5) * 0.289529654602168 + 0.1760912590558 + i * 0.301029995663981;
  k = (int) ds;
  if (ds < 0. && ds != k)
    k--;			/* want k = floor(ds) */
  k_check = 1;
  if (k >= 0 && k <= Ten_pmax)
    {
      if (d.d < small_tens[k])
	k--;
      k_check = 0;
    }
  j = bbits - i - 1;
  if (j >= 0)
    {
      b2 = 0;
      s2 = j;
    }
  else
    {
      b2 = -j;
      s2 = 0;
    }
  if (k >= 0)
    {
      b5 = 0;
      s5 = k;
      s2 += k;
    }
  else
    {
      b2 -= k;
      b5 = -k;
      s5 = 0;
    }
  if (mode < 0 || mode > 9)
    mode = 0;
  try_quick = 1;
  if (mode > 5)
    {
      mode -= 4;
      try_quick = 0;
    }
  leftright = 1;
  ilim = ilim1 = -1;
  switch (mode)
    {
    case 0:
    case 1:
      i = 18;
      ndigits = 0;
      break;
    case 2:
      leftright = 0;
      /* no break */
    case 4:
      if (ndigits <= 0)
	ndigits = 1;
      ilim = ilim1 = i = ndigits;
      break;
    case 3:
      leftright = 0;
      /* no break */
    case 5:
      i = ndigits + k + 1;
      ilim = i;
      ilim1 = i - 1;
      if (i <= 0)
	i = 1;
    }
  j = sizeof (__ULong);
  #ifndef _SMALL_PRINTF
  for (_REENT_MP_RESULT_K(ptr) = 0; sizeof (_Bigint) - sizeof (__ULong) + j <= i;
       j <<= 1)
  _REENT_MP_RESULT_K(ptr)++;  
  _REENT_MP_RESULT(ptr) = Balloc (ptr, _REENT_MP_RESULT_K(ptr));
  s = s0 = (char *) _REENT_MP_RESULT(ptr);
  #else
  s = s0 = &tab_s0[0];
  #endif
  
  if (ilim >= 0 && ilim <= Quick_max && try_quick)
    {
      /* Try to get by with floating-point arithmetic. */

      i = 0;
      d2.d = d.d;
      k0 = k;
      ilim0 = ilim;
      ieps = 2;			/* conservative */
      if (k > 0)
	{
	  ds = small_tens[k & 0xf];
	  j = k >> 4;
	  if (j & Bletch)
	    {
	      /* prevent overflows */
	      j &= Bletch - 1;
	      d.d /= small_bigtens[n_bigtens - 1];
	      ieps++;
	    }
	  for (; j; j >>= 1, i++)
	    if (j & 1)
	      {
		ieps++;
		ds *= small_bigtens[i];
	      }
	  d.d /= ds;
	}
      else if ((j1 = -k) != 0)
	{
	  d.d *= small_tens[j1 & 0xf];
	  for (j = j1 >> 4; j; j >>= 1, i++)
	    if (j & 1)
	      {
		ieps++;
		d.d *= small_bigtens[i];
	      }
	}
      if (k_check && d.d < 1. && ilim > 0)
	{
	  if (ilim1 <= 0)
	    goto fast_failed;
	  ilim = ilim1;
	  k--;
	  d.d *= 10.;
	  ieps++;
	}
      eps.d = ieps * d.d + 7.;
      word0 (eps) -= (P - 1) * Exp_msk1;
      if (ilim == 0)
	{
	  S = mhi = 0;
	  d.d -= 5.;
	  if (d.d > eps.d)
	    goto one_digit;
	  if (d.d < -eps.d)
	    goto no_digits;
	  goto fast_failed;
	}
#ifndef No_leftright
      if (leftright)
	{
	  /* Use Steele & White method of only
	   * generating digits needed.
	   */
	  eps.d = 0.5 / small_tens[ilim - 1] - eps.d;
	  for (i = 0;;)
	    {
	      L = d.d;
	      d.d -= L;
	      
	      *s++ = '0' + (int) L;
	      
	      if (d.d < eps.d)
		goto ret1;
	      if (1. - d.d < eps.d)
		goto bump_up;
	      if (++i >= ilim)
		break;
	      eps.d *= 10.;
	      d.d *= 10.;
	    }
	}
      else
	{
#endif
	  /* Generate ilim digits, then fix them up. */
	  eps.d *= small_tens[ilim - 1];
	  for (i = 1;; i++, d.d *= 10.)
	    {
	      L = d.d;
	      d.d -= L;	      
	      *s++ = '0' + (int) L;	      
	      if (i == ilim)
		{
		  if (d.d > 0.5 + eps.d)
		    goto bump_up;
		  else if (d.d < 0.5 - eps.d)
		    {		      
		      while (*--s == '0');
		      s++;		    
		      goto ret1;
		    }
		  break;
		}
	    }
#ifndef No_leftright
	}
#endif
    fast_failed:
      s = s0;
      d.d = d2.d;
      k = k0;
      ilim = ilim0;
    }

  /* Do we have a "small" integer? */

  if (be >= 0 && k <= Int_max)
    {
      /* Yes. */
      ds = small_tens[k];
      if (ndigits < 0 && ilim <= 0)
	{
	  S = mhi = 0;
	  if (ilim < 0 || d.d <= 5 * ds)
	    goto no_digits;
	  goto one_digit;
	}
      for (i = 1;; i++)
	{
	  L = d.d / ds;
	  d.d -= L * ds;
#ifdef Check_FLT_ROUNDS
	  /* If FLT_ROUNDS == 2, L will usually be high by 1 */
	  if (d.d < 0)
	    {
	      L--;
	      d.d += ds;
	    }
#endif
	 
	      *s++ = '0' + (int) L;
	
	  if (i == ilim)
	    {
	      d.d += d.d;
             if ((d.d > ds) || ((d.d == ds) && (L & 1)))
		{
		bump_up:
		 
		 
		 
		  while (*--s == '9')
		    if (s == s0)
		      {
			k++;
			*s = '0';
			break;
		      }
		  ++*s++;
	
		}
	      break;
	    }
	  if (!(d.d *= 10.))
	    break;
	}
      goto ret1;
    }

  m2 = b2;
  m5 = b5;
  mhi = mlo = 0;
  if (leftright)
    {
      if (mode < 2)
	{
	  i =
#ifndef Sudden_Underflow
	    denorm ? be + (Bias + (P - 1) - 1 + 1) :
#endif
#ifdef IBM
	    1 + 4 * P - 3 - bbits + ((bbits + be - 1) & 3);
#else
	    1 + P - bbits;
#endif
	}
      else
	{
	  j = ilim - 1;
	  if (m5 >= j)
	    m5 -= j;
	  else
	    {
	      s5 += j -= m5;
	      b5 += j;
	      m5 = 0;
	    }
	  if ((i = ilim) < 0)
	    {
	      m2 -= i;
	      i = 0;
	    }
	}
      b2 += i;
      s2 += i;
		
	   #ifdef _SMALL_PRINTF 
      mhi = small_i2b (ptr, 1,&tab_mhi[0]);
      #else
      mhi=i2b(ptr,1);
      #endif
    }
  if (m2 > 0 && s2 > 0)
    {
      i = m2 < s2 ? m2 : s2;
      b2 -= i;
      m2 -= i;
      s2 -= i;
    }
  if (b5 > 0)
    {
      if (leftright)
	{
	  if (m5 > 0)
	    {
	      #ifdef _SMALL_PRINTF
	      if (mhi == &tab_mhi[0]){
	      mhi = small_pow5mult (ptr, mhi, m5,&tab_mhilshift[0]);
	      }
	      else{
	      mhi = small_pow5mult (ptr, mhi, m5,&tab_mhi[0]);
	      }
	      
	      b1 = small_mult (ptr, mhi, b,&tab_b1[0]);
	      #else 
	       	mhi = pow5mult (ptr, mhi, m5);
	   		b1 = mult (ptr, mhi, b);	      
	      	Bfree (ptr, b);
	      #endif
	    }
         if ((j = b5 - m5) != 0)
       #ifdef _SMALL_PRINTF
	    if( b == &tab_b[0]){
           b = small_pow5mult (ptr, b, b5,&tab_blshift[0]);
           }
       else{
           b = small_pow5mult (ptr, b, b5,&tab_b[0]);
           }
	    #else
	    b = pow5mult (ptr, b, j);
	    #endif
	    
	 

	}
      else
        #ifdef _SMALL_PRINTF  
           if( b == &tab_b[0]){
           b = small_pow5mult (ptr, b, b5,&tab_blshift[0]);
           }
           else{
           b = small_pow5mult (ptr, b, b5,&tab_b[0]);
           }
		     
        #else
	        b = pow5mult (ptr, b, b5);
	     #endif

    }
  #ifdef _SMALL_PRINTF  
  S = small_i2b (ptr, 1,&tab_S[0]);
  #else
  S = small_i2b (ptr, 1);
  #endif
  if (s5 > 0)
    #ifdef _SMALL_PRINTF
    if (S == &tab_S[0]){
    S = small_pow5mult (ptr, S, s5,&tab_Slshift[0]);
    }
    else{
    S = small_pow5mult (ptr, S, s5,&tab_S[0]);
    }
    
    #else
    S = pow5mult (ptr, S, s5);
    #endif

  /* Check for special case that d is a normalized power of 2. */

  spec_case = 0;
  if (mode < 2)
    {
      if (!word1 (d) && !(word0 (d) & Bndry_mask)
#ifndef Sudden_Underflow
	  && word0 (d) & Exp_mask
#endif
	)
	{
	  /* The special case */
	  b2 += Log2P;
	  s2 += Log2P;
	  spec_case = 1;
	}
    }

  /* Arrange for convenient computation of quotients:
   * shift left if necessary so divisor has 4 leading 0 bits.
   *
   * Perhaps we should just compute leading 28 bits of S once
   * and for all and pass them and a shift to quorem, so it
   * can do shifts and ors to compute the numerator for q.
   */

#ifdef Pack_32
  if ((i = ((s5 ? 32 -small_hi0bits (S->_x[S->_wds - 1]) : 1) + s2) & 0x1f) != 0)
    i = 32 - i;
#else
  if ((i = ((s5 ? 32 - small_hi0bits (S->_x[S->_wds - 1]) : 1) + s2) & 0xf) != 0)
    i = 16 - i;
#endif
  if (i > 4)
    {
      i -= 4;
      b2 += i;
      m2 += i;
      s2 += i;
    }
  else if (i < 4)
    {
      i += 28;
      b2 += i;
      m2 += i;
      s2 += i;
    }
  if (b2 > 0)
    #ifdef _SMALL_PRINTF
      if (b==&tab_b[0]){
    		b = small_lshift (ptr, b, b2,&tab_blshift[0]);
    	}
    	else {
    	   b = small_lshift (ptr, b, b2,&tab_b[0]);
    	}
     #else
         b = lshift (ptr, b, b2);
     #endif
  if (s2 > 0)
    #ifdef _SMALL_PRINTF 
      if ( S == tab_S) { 
    	S = small_lshift (ptr, S, s2,&tab_Slshift[0]);  
    	}
    	else {
    	S = small_lshift (ptr, S, s2,&tab_S[0]); 
    	}
    #else
      S = lshift (ptr, S, s2);
    #endif
  if (k_check)
    {
      if (small_cmp (b, S) < 0)
	{
	  k--;	  /* we botched the k estimate */
	  #ifdef _SMALL_PRINTF	   
	    if (b == &tab_b[0] ){
	    
	    b = small_multadd (ptr, b, 10, 0,&tab_blshift[0]);	
	    }
	    else{
	    b = small_multadd (ptr, b, 10, 0,&tab_b[0]);	
	    }
	  #else
	    b = multadd (ptr, b, 10, 0);		  
	  #endif

	
	  if (leftright)
	    #ifdef _SMALL_PRINTF
	    if (mhi == &tab_mhi[0] ){
	    
	    mhi = small_multadd (ptr, mhi, 10, 0,&tab_mhilshift[0]);	
	    }
	    else{
	    mhi = small_multadd (ptr, mhi, 10, 0,&tab_mhi[0]);	
	    }	    
	    #else
	       mhi = multadd (ptr, mhi, 10, 0);
	    #endif
	  ilim = ilim1;
	}
    }
  if (ilim <= 0 && mode > 2)
    {
      #ifdef _SMALL_PRINTF
      _Bigint * tab;
      if ( S == &tab_S[0] ){
      tab = tab_Slshift;
      }
      else {
      tab = tab_S;
      }
      
      if (ilim < 0 || small_cmp (b, S = small_multadd (ptr, S, 5, 0,&tab[0])) <= 0)
	{
	   #else
	   if (ilim < 0 || small_cmp (b, S = multadd (ptr, S, 5, 0)) <= 0)
	{
	   #endif
	  /* no digits, fcvt style */
	no_digits:
	  k = -1 - ndigits;
	  goto ret;
	}
    one_digit: 
      *s++ = '1'; 
      k++;
      goto ret;
    }
  if (leftright)
    {
      if (m2 > 0)
   #ifdef _SMALL_PRINTF   
   if (mhi == &tab_mhi[0]){
     mhi = small_lshift (ptr, mhi, m2,&tab_mhilshift[0]);
    }
   else {
     mhi = small_lshift (ptr, mhi, m2,&tab_mhi[0]);
   }
   
   #else
     mhi = lshift (ptr, mhi, m2);
   #endif
      /* Compute mlo -- check for special case
       * that d is a normalized power of 2.
       */

      mlo = mhi;
      if (spec_case)
	{
	  #ifndef _SMALL_PRINTF
	  
	  mhi = Balloc (ptr, mhi->_k);
	  
	  #else
	  int sauv_k =mhi->_k;
	  mhi =&tab_mhi[0];
	  mhi->_k = sauv_k;
	  mhi->_maxwds = (1<<sauv_k);
	  mhi->_sign = mhi->_wds =0 ;	  
	  #endif
	  Bcopy (mhi, mlo); 
	  
	  #ifdef _SMALL_PRINTF
	  if( mhi == &tab_mhi[0]){
	  	 mhi = small_lshift (ptr, mhi, Log2P,&tab_mhilshift[0]);
	  }	
	  else {
	   mhi = small_lshift (ptr, mhi, Log2P,&tab_mhi[0]);
	   }
	  #else
	    mhi = lshift (ptr, mhi, Log2P);
	  #endif
	}

      for (i = 1;; i++)
	{
	  dig = quorem (b, S) + '0';
	  /* Do we yet have the shortest decimal string
	   * that will round to d?
	   */
	  j = small_cmp (b, mlo);
	  #ifdef _SMALL_PRINTF
	  delta = small_diff (ptr, S, mhi,&tab_delta[0]);
	  #else
	  delta = diff (ptr, S, mhi);
	  #endif
	  j1 = delta->_sign ? 1 : small_cmp (b, delta);
	  #ifndef _SMALL_PRINTF
	  Bfree (ptr, delta);
	  #endif
#ifndef ROUND_BIASED
	  if (j1 == 0 && !mode && !(word1 (d) & 1))
	    {
	      if (dig == '9')
		goto round_9_up;
	      if (j > 0)
		dig++;
		   
	      *s++ = dig;
	     
	      goto ret;
	    }
#endif
         if ((j < 0) || ((j == 0) && !mode
#ifndef ROUND_BIASED
	      && !(word1 (d) & 1)
#endif
           ))
	    {
	      if (j1 > 0)
		{
		  
		  #ifdef _SMALL_PRINTF
		  if (b == &tab_b[0]){
		  		b = small_lshift (ptr, b, 1,&tab_blshift[0]);		
		  }
		  else {
		   	b = small_lshift (ptr, b, 1,&tab_b[0]);		
		  }
		  #else
		  b = lshift (ptr, b, 1);
		  #endif

		  j1 = small_cmp (b, S);
                 if (((j1 > 0) || ((j1 == 0) && (dig & 1)))
		      && dig++ == '9')
		    goto round_9_up;
		}
	      
	      *s++ = dig;
	      
	      goto ret;
	    }
	  if (j1 > 0)
	    {
	      if (dig == '9')
		{		/* possible if i == 1 */
		round_9_up:
		 
	      *s++ = '9';
	     
		  goto roundoff;
		}
	      
	      *s++ = dig+1;
	      
	      goto ret;
	    }
	 
	      *s++ = dig;
	      
	  if (i == ilim)
	    break;
	  #ifdef _SMALL_PRINTF	
	     if (b == tab_b ){
	      b = small_multadd (ptr, b, 10, 0,&tab_blshift[0]);
	     }
	     else{
	      b = small_multadd (ptr, b, 10, 0,&tab_b[0]);
	     }   
        
	  #else
	     b = multadd (ptr, b, 10, 0);
	  #endif
	  
	  
	  if (mlo == mhi)
	    #ifdef _SMALL_PRINTF 	
	     if ( mhi = &tab_mhi[0] ) {   
	     		mlo = mhi = small_multadd (ptr, mhi, 10, 0,&tab_mhilshift[0]);
	     }
	     else{
	        mlo = mhi = small_multadd (ptr, mhi, 10, 0,&tab_mhi[0]);
	     }
	     
	    #else
	    mlo = mhi = multadd (ptr, mhi, 10, 0);
	    #endif
	  else
	    {
	     
	     #ifdef _SMALL_PRINTF	  
	      if ( mlo = &tab_mhi[0] ) {   
	     		mlo  = small_multadd (ptr, mlo, 10, 0,&tab_mlolshift[0]);
	     }
	     else{
	        mlo = small_multadd (ptr, mlo, 10, 0,&tab_mlo[0]);
	     }
	    	if ( mhi = &tab_mhi[0] ) {   
	     		mhi = small_multadd (ptr, mhi, 10, 0,&tab_mhilshift[0]);
	     }
	     else{
	        mhi = small_multadd (ptr, mhi, 10, 0,&tab_mhi[0]);
	     }
	     #else
	      mlo = multadd (ptr, mlo, 10, 0);
	      mhi = multadd (ptr, mhi, 10, 0);
	     #endif
	    }
	}
    }
  else
    for (i = 1;; i++)
      {
      
	     *s++ = dig =quorem (b, S) + '0';
	      
	
	if (i >= ilim)
	  break;
   #ifdef _SMALL_PRINTF  
    if ( b == &tab_b[0] ) {   
	  b = small_multadd (ptr, b, 10, 0,&tab_blshift[0]);	 
	  }
	else {
	 b = small_multadd (ptr, b, 10, 0,&tab_b[0]);	 
	} 
	#else
	  b = multadd (ptr, b, 10, 0);
	#endif
      }

  /* Round off last digit */
  #ifdef _SMALL_PRINTF
   if (b == &tab_b[0]) {
  		b = small_lshift (ptr, b, 1,&tab_blshift[0]);
  	}
  	else {
  	  b = small_lshift (ptr, b, 1,&tab_b[0]);
  	}
  #else
      b = lshift (ptr, b, 1);
  #endif
  
  j = small_cmp (b, S);
  if ((j > 0) || ((j == 0) && (dig & 1)))
    {
    roundoff:
     
   
      while (*--s == '9')
	if (s == s0)
	  {
	    k++;
	    *s++ = '1';
	    goto ret;
	  }
      ++*s++;
    }
  else
    {
      while (*--s == '0');
      s++;
    }
  
    
  
ret:
  #ifndef _SMALL_PRINTF
  Bfree (ptr, S);
  
  if (mhi)
    {
      if (mlo && mlo != mhi)
	Bfree (ptr, mlo);
      Bfree (ptr, mhi);
    }
  #endif
ret1:
  #ifndef _SMALL_PRINTF
  Bfree (ptr, b); 
  #endif  
  *s = 0;
  #endif
  *decpt = k + 1;
  if (rve)
    *rve = s;
  return s0;
}

