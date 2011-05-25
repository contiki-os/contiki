/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * @(#)$Id: msp430def.h,v 1.5 2010/03/19 14:50:07 joxe Exp $
 */

#ifndef MSP430DEF_H
#define MSP430DEF_H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#ifndef uint8_t
typedef unsigned char   uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef   signed char    int8_t;
typedef          short  int16_t;
typedef          long   int32_t;
#endif
#endif /* !HAVE_STDINT_H */

/* These names are deprecated, use C99 names. */
typedef  uint8_t    u8_t;
typedef uint16_t   u16_t;
typedef uint32_t   u32_t;
typedef  int32_t   s32_t;

/* default DCOSYNCH Period is 30 seconds */
#ifdef DCOSYNCH_CONF_PERIOD
#define DCOSYNCH_PERIOD DCOSYNCH_CONF_PERIOD
#else
#define DCOSYNCH_PERIOD 30
#endif

void msp430_cpu_init(void);	/* Rename to cpu_init() later! */
void msp430_sync_dco(void);


#define cpu_init() msp430_cpu_init()

void   *sbrk(int);

typedef int spl_t;
void    splx_(spl_t);
spl_t   splhigh_(void);

#define splhigh() splhigh_()
#ifdef __IAR_SYSTEMS_ICC__
#define splx(sr) sr = __get_SR_register()
#else
#define splx(sr) __asm__ __volatile__("bis %0, r2" : : "r" (sr))
#endif

/* Workaround for bug in msp430-gcc compiler */
#if defined(__MSP430__) && defined(__GNUC__) && MSP430_MEMCPY_WORKAROUND
#ifndef memcpy
#include <string.h>

void *w_memcpy(void *out, const void *in, size_t n);
#define memcpy(dest, src, count) w_memcpy(dest, src, count)

/* #define memcpy(dest, src, count) do {                    \ */
/*   if(count == 2) {                                       \ */
/*     *((uint8_t *)dest) = *((uint8_t *)src);              \ */
/*     *((uint8_t *)dest + 1) = *((uint8_t *)src + 1);      \ */
/*   } else {                                               \ */
/*     memcpy(dest, src, count);                            \ */
/*   }                                                      \ */
/* } while(0) */

void *w_memset(void *out, int value, size_t n);
#define memset(dest, value, count) w_memset(dest, value, count)

/* #define memset(dest, value, count) do {                  \ */
/*   if(count == 2) {                                       \ */
/*     *((uint8_t *)dest) = (uint8_t)value;                 \ */
/*     *((uint8_t *)dest + 1) = (uint8_t)value;             \ */
/*   } else {                                               \ */
/*     memset(dest, value, count);                          \ */
/*   }                                                      \ */
/* } while(0) */
#endif /* memcpy */
#endif /* __GNUC__ &&  __MSP430__ && MSP430_MEMCPY_WORKAROUND */


/* Moved from the msp430.h file with other msp430 related defines */

#ifdef F_CPU
#define MSP430_CPU_SPEED F_CPU
#else
#define MSP430_CPU_SPEED 2457600UL
#endif

#define MSP430_REQUIRE_CPUON 0
#define MSP430_REQUIRE_LPM1 1
#define MSP430_REQUIRE_LPM2 2
#define MSP430_REQUIRE_LPM3 3

void msp430_add_lpm_req(int req);
void msp430_remove_lpm_req(int req);

#endif /* MSP430DEF_H */
