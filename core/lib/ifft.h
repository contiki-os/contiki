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
 *
 * Author  : Joakim Eriksson
 * Created : 2008-03-27
 * Updated : $Date: 2008/04/25 22:12:25 $
 *           $Revision: 1.2 $
 */

#ifndef IFFT_H
#define IFFT_H
#include "contiki-conf.h"

/* ifft(xre[], n) - integer (fixpoint) version of Fast Fourier Transform
   An integer version of FFT that takes in-samples in an int16_t array
   and does an fft on n samples in the array.
   The result of the FFT is stored in the same array as the samples
   was stored.

   Note: This fft is designed to be used with 8 bit values (e.g. not
   16 bit values). The reason for the int16_t array is for keeping some
   'room' for the calculations. It is also designed for doing fairly small
   FFT:s since to large sample arrays might cause it to overflow during
   calculations.
*/
void ifft(int16_t xre[], int16_t xim[], uint16_t n);

#endif /* IFFT_H */
