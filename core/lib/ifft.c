/*
 * Copyright (c) 2008, Swedish Institute of Computer Science
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
 * -----------------------------------------------------------------
 * ifft - Integer FFT (fast fourier transform) library
 *
 *
 * Author  : Joakim Eriksson
 * Created : 2008-03-27
 * Updated : $Date: 2008/07/03 23:40:12 $
 *           $Revision: 1.3 $
 */
#include "lib/ifft.h"

/*---------------------------------------------------------------------------*/
/* constant table of sin values in 8/7 bits resolution */
/* NOTE: symmetry can be used to reduce this to 1/2 or 1/4 the size */
#define SIN_TAB_LEN 120
#define RESOLUTION 7
#define ABS(x) (x < 0 ? -x : x)

static const int8_t SIN_TAB[] = {
 0,6,13,20,26,33,39,45,52,58,63,69,75,80,
 85,90,95,99,103,107,110,114,116,119,121,
 123,125,126,127,127,127,127,127,126,125,
 123,121,119,116,114,110,107,103,99,95,90,
 85,80,75,69,63,58,52,45,39,33,26,20,13,6,
 0,-6,-13,-20,-26,-33,-39,-45,-52,-58,-63,
 -69,-75,-80,-85,-90,-95,-99,-103,-107,-110,
 -114,-116,-119,-121,-123,-125,-126,-127,-127,
 -127,-127,-127,-126,-125,-123,-121,-119,-116,
 -114,-110,-107,-103,-99,-95,-90,-85,-80,-75,
 -69,-63,-58,-52,-45,-39,-33,-26,-20,-13,-6
};


static uint16_t bitrev(uint16_t j, uint16_t nu)
{
  uint16_t k;
  k = 0;
  for (; nu > 0; nu--) {
    k  = (k << 1) + (j & 1);
    j = j >> 1;
  }
  return k;
}


/* Non interpolating sine... which takes an angle of 0 - 999 */
static int16_t sinI(uint16_t angleMilli)
{
  uint16_t pos;
  pos = (uint16_t) ((SIN_TAB_LEN * (uint32_t) angleMilli) / 1000);
  return SIN_TAB[pos % SIN_TAB_LEN];
}

static int16_t cosI(uint16_t angleMilli)
{
  return sinI(angleMilli + 250);
}

static uint16_t ilog2(uint16_t val)
{
  uint16_t log;
  log = 0;
  val = val >> 1; /* The 20 = 1 => log = 0 => val = 0 */
  while (val > 0) {
    val = val >> 1;
    log++;
  }
  return log;
}


/* ifft(xre[], n) - integer (fixpoint) version of Fast Fourier Transform
   An integer version of FFT that takes in-samples in an int16_t array
   and does an fft on n samples in the array.
   The result of the FFT is stored in the same array as the samples
   was stored. Them imaginary part of the result is stored in xim which
   needs to be of the same size as xre (e.g. n ints).

   Note: This fft is designed to be used with 8 bit values (e.g. not
   16 bit values). The reason for the int16_t array is for keeping some
   'room' for the calculations. It is also designed for doing fairly small
   FFT:s since to large sample arrays might cause it to overflow during
   calculations.
*/
void
ifft(int16_t xre[], int16_t xim[], uint16_t n)
{
  uint16_t nu;
  uint16_t n2;
  uint16_t nu1;
  int p, k, l, i;
  int32_t c, s, tr, ti;

  nu = ilog2(n);
  nu1 = nu - 1;
  n2 = n / 2;

  for (i = 0; i < n; i++)
    xim[i] = 0;

  for (l = 1; l <= nu; l++) {
    for (k = 0; k < n; k += n2) {
      for (i = 1; i <= n2; i++) {
	p = bitrev(k >> nu1, nu);
	c = cosI((1000 * p) / n);
	s = sinI((1000 * p) / n);

	tr = ((xre[k + n2] * c + xim[k + n2] * s) >> RESOLUTION);
	ti = ((xim[k + n2] * c - xre[k + n2] * s) >> RESOLUTION);

	xre[k + n2] = xre[k] - tr;
	xim[k + n2] = xim[k] - ti;
	xre[k] += tr;
	xim[k] += ti;
	k++;
      }
    }
    nu1--;
    n2 = n2 / 2;
  }

  for (k = 0; k < n; k++) {
    p = bitrev(k, nu);
    if (p > k) {
      n2 = xre[k];
      xre[k] = xre[p];
      xre[p] = n2;

      n2 = xim[k];
      xim[k] = xim[p];
      xim[p] = n2;
    }
  }

  /* This is a fast but not 100% correct magnitude calculation */
  /* Should be sqrt(xre[i]^2 + xim[i]^2) and normalized with div. by n */
  for (i = 0, n2 = n / 2; i < n2; i++) {
    xre[i] = (ABS(xre[i]) + ABS(xim[i]));
  }
}
