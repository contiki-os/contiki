/*
 * Copyright (c) 2014, Lars Schmertmann <SmallLars@t-online.de>,
 * Jens Trillmann <jtrillma@informatik.uni-bremen.de>.
 * All rights reserved.
 *
 * Bases on an implementation from Chris K Cockrum <ckc@cockrum.net>.
 * http://cockrum.net/Implementation_of_ECC_on_an_8-bit_microcontroller.pdf
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "ecc.h"

#include <string.h>

/*---------------------------------------------------------------------------*/

const uint32_t ecc_prime_m[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xffffffff };
const uint32_t ecc_prime_r[8] = { 0x00000001, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffe, 0x00000000 };

/* private prototypes  ----------------------------------------------------- */

/* simple functions to work with 256 bit numbers */
static void ecc_setZero(uint32_t *a);
static void ecc_copy(uint32_t *dst, const uint32_t *src);
static uint32_t ecc_isX(const uint32_t *a, const uint32_t x);
static void ecc_rshift(uint32_t *a);
static uint32_t ecc_add(const uint32_t *x, const uint32_t *y, uint32_t *result);
static uint32_t ecc_sub(const uint32_t *x, const uint32_t *y, uint32_t *result);
static void ecc_mult(const uint32_t *x, const uint32_t *y, uint32_t *result, const uint32_t length);

/* ecc_fieldModP-Helper */
__attribute__((always_inline)) static void ecc_form_s1(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_s2(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_s3(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_s4(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d1(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d2(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d3(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d4(uint32_t *dst, const uint32_t *src);

/* field functions for 256 bit numbers */
static void ecc_fieldAdd(const uint32_t *x, const uint32_t *y, uint32_t *result);
static void ecc_fieldSub(const uint32_t *x, const uint32_t *y, uint32_t *result);
static void ecc_fieldModP(uint32_t *A, const uint32_t *B);
static uint32_t ecc_fieldAddAndDivide(const uint32_t *x, uint32_t *result);
static void ecc_fieldInv(const uint32_t *A, uint32_t *B);

/* elliptic curve functions */
static void ecc_ec_add(const uint32_t *px, const uint32_t *py, const uint32_t *qx, const uint32_t *qy, uint32_t *Sx, uint32_t *Sy);
static void ecc_ec_double(const uint32_t *px, const uint32_t *py, uint32_t *Dx, uint32_t *Dy);

/* public functions -------------------------------------------------------- */

signed int
ecc_compare(const uint32_t *a, const uint32_t *b)
{
  uint32_t i = 8;
  while(i--) {
    if(a[i] > b[i]) {
      return 1;
    }
    if(a[i] < b[i]) {
      return -1;
    }
  }
  return 0;
}
void
ecc_ec_mult(uint32_t *resultx, uint32_t *resulty, const uint32_t *px, const uint32_t *py, const uint32_t *secret)
{
  uint32_t Qx[8];
  uint32_t Qy[8];

  ecc_setZero(resultx);
  ecc_setZero(resulty);

  uint32_t i = 256;
  while(i--) {
    ecc_ec_double(resultx, resulty, Qx, Qy);
    if(((secret[i / 32]) >> (i & 0x1F)) & 0x01) {
      ecc_ec_add(Qx, Qy, px, py, resultx, resulty);
    } else {
      ecc_copy(resultx, Qx);
      ecc_copy(resulty, Qy);
    }
  }
}
/* private functions ------------------------------------------------------- */

static void
ecc_setZero(uint32_t *a)
{
  asm volatile (
    "mov r1, $0 \n\t"
    "mov r2, r1 \n\t"
    "mov r3, r2 \n\t"
    "mov r4, r3 \n\t"
    "stm %[a]!, {r1-r4} \n\t"
    "stm %[a]!, {r1-r4} \n\t"
    : /* out */
    : /* in */
    [a] "l" (a)
    : /* clobber list */
    "r1", "r2", "r3", "r4", "memory"
    );
}
/*
 * copy one array to another
 */
static void
ecc_copy(uint32_t *dst, const uint32_t *src)
{
  asm volatile (
    "ldm %[s]!, {r2-r5} \n\t"
    "stm %[d]!, {r2-r5} \n\t"
    "ldm %[s]!, {r2-r5} \n\t"
    "stm %[d]!, {r2-r5} \n\t"
    : /* out */
    : /* in */
    [d] "l" (dst),
    [s] "l" (src)
    : /* clobber list */
    "r2", "r3", "r4", "r5", "memory"
    );
}
static uint32_t
ecc_isX(const uint32_t *a, const uint32_t x)
{
  if(a[0] != x) {
    return 0;
  }

  uint32_t n = 8;
  while(--n)
    if(a[n]) {
      return 0;
    }

  return 1;
}
static void
ecc_rshift(uint32_t *a)
{
  uint32_t index = 32;
  uint32_t carry = 0;

  asm volatile (
    "0: \n\t"
    "sub %[i], %[i], #4 \n\t"             /* index -= 4 */
    "mov r4, %[c] \n\t"                   /* result = carry */
    "ldr r3, [%[a],%[i]] \n\t"            /* value = a[index] */
    "lsl %[c], r3, #31 \n\t"              /* carry = value << 31 */
    "lsr r3, r3, #1 \n\t"                 /* value >>= 1 */
    "orr r4, r4, r3 \n\t"                 /* result |= value */
    "str r4, [%[a],%[i]] \n\t"            /* a[index] = result */
    "cmp %[i], $0 \n\t"                   /* index == 0 */
    "bne 0b \n\t"                         /* != ? next loop */
    : /* out */
    : /* in */
    [a] "r" (a),
    [i] "r" (index),
    [c] "r" (carry)
    : /* clobber list */
    "r3", "r4", "memory"
    );
}
static uint32_t
ecc_add(const uint32_t *x, const uint32_t *y, uint32_t *result)
{
  uint32_t carry;

  asm volatile (
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "add r4, r4, r6 \n\t"
    "adc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "adc r4, r4, r6 \n\t"
    "adc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "adc r4, r4, r6 \n\t"
    "adc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "adc r4, r4, r6 \n\t"
    "adc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "bcc 0f \n\t"
    "mov %[c], #1 \n\t"
    "b 1f \n\t"
    "0: \n\t"
    "mov %[c], $0 \n\t"
    "1: \n\t"
    : /* out */
    [c] "=l" (carry)
    : /* in */
    [x] "l" (x),
    [y] "l" (y),
    [r] "l" (result)
    : /* clobber list */
    "r4", "r5", "r6", "r7", "memory"
    );

  return carry;
}
static uint32_t
ecc_sub(const uint32_t *x, const uint32_t *y, uint32_t *result)
{
  uint32_t carry;

  asm volatile (
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "sub r4, r4, r6 \n\t"
    "sbc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "sbc r4, r4, r6 \n\t"
    "sbc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "sbc r4, r4, r6 \n\t"
    "sbc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "ldm %[x]!, {r4,r5} \n\t"
    "ldm %[y]!, {r6,r7} \n\t"
    "sbc r4, r4, r6 \n\t"
    "sbc r5, r5, r7 \n\t"
    "stm %[r]!, {r4,r5} \n\t"
    "bcs 0f \n\t"
    "mov %[c], #1 \n\t"
    "b 1f \n\t"
    "0: \n\t"
    "mov %[c], $0 \n\t"
    "1: \n\t"
    : /* out */
    [c] "=l" (carry)
    : /* in */
    [x] "l" (x),
    [y] "l" (y),
    [r] "l" (result)
    : /* clobber list */
    "r4", "r5", "r6", "r7", "memory"
    );

  return carry;
}
static void
ecc_mult(const uint32_t *x, const uint32_t *y, uint32_t *result, const uint32_t length)
{
  if(length == 1) {
    /* Version 1: 56 Byte bigger as ASM-Version */
    /* uint64_t *r = (uint64_t *) result; */
    /* *r = (uint64_t) x[0] * (uint64_t) y[0]; */

    /* Version 2: Size is like Version 1 but its much more slower */
    /* uint32_t carry; */
    /* uint32_t AB[length*2]; */
    /* uint32_t C[length*2]; */
    /* AB[0] = (x[0]&0x0000FFFF) * (y[0]&0x0000FFFF); */
    /* AB[1] = (x[0]>>16) * (y[0]>>16); */
    /* C[0] = (x[0]>>16) * (y[0]&0x0000FFFF); */
    /* C[1] = (x[0]&0x0000FFFF) * (y[0]>>16); */
    /* carry = ecc_add(&C[0], &C[1], C, 1); */
    /* C[1] = carry << 16 | C[0] >> 16; */
    /* C[0] = C[0] << 16; */
    /* ecc_add(AB, C, result, 2); */

    /* Version 3: 56 Byte lesser as Version 1 but same speed */
    asm volatile (
      "ldrh r5, [%[x], $0] \n\t"                /* r5 = (x[0] & 0x0000FFFF) */
      "ldrh r3, [%[y], $0] \n\t"                /* r3 = (y[0] & 0x0000FFFF) */
      "mul r5, r3 \n\t"                         /* r5 *= r3                 r5 = AB[0] */
      "ldrh r6, [%[x], #2] \n\t"                /* r6 = (x[0] >> 16) */
      "mul r3, r6 \n\t"                         /* r3 *= r6                 r3 = C[0] */
      "ldrh r4, [%[y], #2] \n\t"                /* r4 = (y[0] >> 16) */
      "mul r6, r4 \n\t"                         /* r6 *= r4                 r6 = AB[1] */
      /* %[y] is not longer needed - its called ry now */
      "ldrh %[y], [%[x], $0] \n\t"              /* ry = (x[0] & 0x0000FFFF) */
      "mul r4, %[y] \n\t"                       /* r4 *= ry                 r4 = C[1] */
      "add %[y], r3, r4 \n\t"                   /* ry = r3 + r4             ry = C[0] + C[1] */
      /* C[1] (r4) is not longer needed */
      "mov r4, $0 \n\t"                         /* r4 = 0 */
      "bcc 0f \n\t"                             /* jump falls carry clear */
      "mov r4, #1 \n\t"                         /* r4 = 1 */
      "lsl r4, r4, #16 \n\t"                    /* r4 <<= 16 */
      "0: \n\t"                                 /*                          r4 = 0x000c0000 = (carry << 16) */
      "lsr r3, %[y], #16 \n\t"                  /* r3 = (ry >> 16) */
      "orr r4, r4, r3 \n\t"                     /* r4 |= r3                 r4 = 0x000c'ryh' = (r4 | ry >> 16) */
      "lsl r3, %[y], #16 \n\t"                  /* r3 = (ry << 16)          r3 = 0x'ryl'0000 = (ry << 16) */
      "add r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[r]!, {r3, r4} \n\t"
      :   /* out */
      :   /* in */
      [x] "l" (x),
      [y] "l" (y),
      [r] "l" (result)
      :   /* clobber list */
      "r3", "r4", "r5", "r6", "memory"
      );
  } else {
    uint32_t carry;
    uint32_t C[length * 2];
    ecc_mult(x, y, result, length / 2);
    ecc_mult(x + (length / 2), y + (length / 2), result + length, length / 2);
    ecc_mult(x, y + (length / 2), C, length / 2);
    ecc_mult(x + (length / 2), y, C + length, length / 2);
    if(length == 8) {
      carry = ecc_add(C, C + length, C);
    } else {
      asm volatile (
        "cmp %[l], #2 \n\t"
        "beq .add2 \n\t"
        /* ASM for: ecc_add(C, C + 4, C, 4); */
        "mov %[l], %[a] \n\t"
        "ldm %[a]!, {r3-r6} \n\t"
        "ldm %[a]!, {r5,r6} \n\t"
        "sub %[a], %[a], #16 \n\t"
        "add r3, r3, r5 \n\t"
        "adc r4, r4, r6 \n\t"
        "stm %[l]!, {r3,r4} \n\t"
        "ldm %[a]!, {r3-r6} \n\t"
        "ldm %[a]!, {r5,r6} \n\t"
        "adc r3, r3, r5 \n\t"
        "adc r4, r4, r6 \n\t"
        "stm %[l]!, {r3,r4} \n\t"
        "b 0f \n\t"
        ".add2: \n\t"
        /* ASM for: ecc_add(C, C + 2, C, 2); */
        "ldm %[a]!, {r3-r6} \n\t"
        "sub %[a], %[a], #16 \n\t"
        "add r3, r3, r5 \n\t"
        "adc r4, r4, r6 \n\t"
        "stm %[a]!, {r3,r4} \n\t"
        "0: \n\t"
        "bcc 1f \n\t"
        "mov %[c], #1 \n\t"
        "b 2f \n\t"
        "1: \n\t"
        "mov %[c], $0 \n\t"
        "2: \n\t"
        :     /* out */
        [c] "=l" (carry)
        :     /* in */
        [a] "l" (C),
        [l] "l" (length)
        :     /* clobber list */
        "r3", "r4", "r5", "r6", "memory"
        );
    } C[length] = carry;
    asm volatile (
      "cmp %[l], #2 \n\t"
      "beq .add3 \n\t"
      "cmp %[l], #4 \n\t"
      "beq .add6 \n\t"
      ".add12: \n\t"
      /* ASM for: ecc_add(result + 4, C, result + 4, 12); */
      "add %[r], %[r], #16 \n\t"
      "mov %[l], %[r] \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5,r6} \n\t"
      "add r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5,r6} \n\t"
      "adc r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5,r6} \n\t"
      "adc r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5,r6} \n\t"
      "adc r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5} \n\t"
      "mov r6, $0 \n\t"
      "adc r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "adc r3, r3, r6 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "b 0f \n\t"
      ".add6: \n\t"
      /* ASM for: ecc_add(result + 2, C, result + 2, 6); */
      "add %[r], %[r], #8 \n\t"
      "mov %[l], %[r] \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5,r6} \n\t"
      "add r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5,r6} \n\t"
      "adc r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5} \n\t"
      "mov r6, $0 \n\t"
      "adc r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "stm %[l]!, {r3,r4} \n\t"
      "b 0f \n\t"
      ".add3: \n\t"
      /* ASM for: ecc_add(result + 1, C, result + 1, 3); */
      "add %[r], %[r], #4 \n\t"
      "mov %[l], %[r] \n\t"
      "ldm %[r]!, {r3,r4} \n\t"
      "ldm %[c]!, {r5,r6} \n\t"
      "add r3, r3, r5 \n\t"
      "adc r4, r4, r6 \n\t"
      "ldr r5, [%[r], $0] \n\t"
      "ldr r6, [%[c], $0] \n\t"
      "adc r5, r5, r6 \n\t"
      "stm %[l]!, {r3-r5} \n\t"
      "0: \n\t"
      :   /* out */
      :   /* in */
      [r] "l" (result),
      [c] "l" (C),
      [l] "l" (length)
      :   /* clobber list */
      "r3", "r4", "r5", "r6", "memory"
      );
  }
}
/*---------------------------------------------------------------------------*/

__attribute__((always_inline)) static void
ecc_form_s1(uint32_t *dst, const uint32_t *src)
{
  /* 0, 0, 0, src[11], src[12], src[13], src[14], src[15] */
  asm volatile (
    "mov r2, $0 \n\t"
    "mov r3, r2 \n\t"
    "mov r4, r3 \n\t"
    "stm %[d]!, {r2-r4} \n\t"
    "add %[s], #44 \n\t"
    "ldm %[s]!, {r2-r6} \n\t"
    "stm %[d]!, {r2-r6} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "r6", "memory"
    );
}
__attribute__((always_inline)) static void
ecc_form_s2(uint32_t *dst, const uint32_t *src)
{
  /* 0, 0, 0, src[12], src[13], src[14], src[15], 0 */
  asm volatile (
    "mov r2, $0 \n\t"
    "mov r3, r2 \n\t"
    "mov r4, r3 \n\t"
    "stm %[d]!, {r2-r4} \n\t"
    "add %[s], #48 \n\t"
    "ldm %[s]!, {r2-r5} \n\t"
    "stm %[d]!, {r2-r5} \n\t"
    "mov r2, $0 \n\t"
    "stm %[d]!, {r2} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "memory"
    );
}
__attribute__((always_inline)) static void
ecc_form_s3(uint32_t *dst, const uint32_t *src)
{
  /* src[8], src[9], src[10], 0, 0, 0, src[14], src[15] */
  asm volatile (
    "add %[s], #32 \n\t"
    "ldm %[s]!, {r2-r4} \n\t"
    "mov r5, $0 \n\t"
    "stm %[d]!, {r2-r5} \n\t"
    "mov r2, r5 \n\t"
    "mov r3, r2 \n\t"
    "add %[s], #12 \n\t"
    "ldm %[s]!, {r4,r5} \n\t"
    "stm %[d]!, {r2-r5} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "memory"
    );
}
__attribute__((always_inline)) static void
ecc_form_s4(uint32_t *dst, const uint32_t *src)
{
  /* src[9], src[10], src[11], src[13], src[14], src[15], src[13], src[8] */
  asm volatile (
    "add %[s], #32 \n\t"
    "ldm %[s]!, {r2-r5} \n\t"
    "stm %[d]!, {r3-r5} \n\t"
    "add %[s], #4 \n\t"
    "ldm %[s]!, {r3-r5} \n\t"
    "stm %[d]!, {r3-r5} \n\t"
    "mov r4, r2 \n\t"
    "stm %[d]!, {r3,r4} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "memory"
    );
}
__attribute__((always_inline)) static void
ecc_form_d1(uint32_t *dst, const uint32_t *src)
{
  /* src[11], src[12], src[13], 0, 0, 0, src[8], src[10] */
  asm volatile (
    "add %[s], #32 \n\t"
    "ldm %[s]!, {r2-r7} \n\t"
    "stm %[d]!, {r5-r7} \n\t"
    "mov r3, $0 \n\t"
    "mov r5, r3 \n\t"
    "mov r6, r5 \n\t"
    "stm %[d]!, {r3,r5,r6} \n\t"
    "stm %[d]!, {r2,r4} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "r6", "r7", "memory"
    );
}
__attribute__((always_inline)) static void
ecc_form_d2(uint32_t *dst, const uint32_t *src)
{
  /* src[12], src[13], src[14], src[15], 0, 0, src[9], src[11] */
  asm volatile (
    "add %[s], #48 \n\t"
    "ldm %[s]!, {r2-r5} \n\t"
    "stm %[d]!, {r2-r5} \n\t"
    "sub %[s], #28 \n\t"
    "ldm %[s]!, {r4-r6} \n\t"
    "mov r2, $0 \n\t"
    "mov r3, r2 \n\t"
    "stm %[d]!, {r2-r4,r6} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "r6", "memory"
    );
}
__attribute__((always_inline)) static void
ecc_form_d3(uint32_t *dst, const uint32_t *src)
{
  /* src[13], src[14], src[15], src[8], src[9], src[10], 0, src[12] */
  asm volatile (
    "add %[s], #52 \n\t"
    "ldm %[s]!, {r2-r4} \n\t"
    "stm %[d]!, {r2-r4} \n\t"
    "sub %[s], #32 \n\t"
    "ldm %[s]!, {r2-r6} \n\t"
    "mov r5, $0 \n\t"
    "stm %[d]!, {r2-r6} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "r6", "memory"
    );
}
__attribute__((always_inline)) static void
ecc_form_d4(uint32_t *dst, const uint32_t *src)
{
  /* src[14], src[15], 0, src[9], src[10], src[11], 0, src[13] */
  asm volatile (
    "add %[s], #56 \n\t"
    "ldm %[s]!, {r2,r3} \n\t"
    "mov r4, $0 \n\t"
    "stm %[d]!, {r2-r4} \n\t"
    "sub %[s], #28 \n\t"
    "ldm %[s]!, {r2-r6} \n\t"
    "mov r5, $0 \n\t"
    "stm %[d]!, {r2-r6} \n\t"
    : /* out */
    [d] "+l" (dst),
    [s] "+l" (src)
    : /* in */
    : /* clobber list */
    "r2", "r3", "r4", "r5", "r6", "memory"
    );
}
/*---------------------------------------------------------------------------*/

static void
ecc_fieldAdd(const uint32_t *x, const uint32_t *y, uint32_t *result)
{
  if(ecc_add(x, y, result)) {    /* add prime if carry is still set! */
    ecc_add(result, ecc_prime_r, result);
  }
}
static void
ecc_fieldSub(const uint32_t *x, const uint32_t *y, uint32_t *result)
{
  if(ecc_sub(x, y, result)) {    /* add modulus if carry is set */
    ecc_add(result, ecc_prime_m, result);
  }
}
/* TODO: maximum: */
/* fffffffe00000002fffffffe0000000100000001fffffffe00000001fffffffe00000001fffffffefffffffffffffffffffffffe000000000000000000000001_16 */
static void
ecc_fieldModP(uint32_t *A, const uint32_t *B)
{
  uint32_t BX_o_DX[8];

  ecc_copy(A, B);                 /* A = T */

  ecc_form_s1(BX_o_DX, B);        /* Form S1 */
  ecc_fieldAdd(A, BX_o_DX, A);    /* A = T + S1 */
  ecc_fieldAdd(A, BX_o_DX, A);    /* A = T + S1 + S1 */

  ecc_form_s2(BX_o_DX, B);        /* Form S2 */
  ecc_fieldAdd(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 */
  ecc_fieldAdd(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 + S2 */

  ecc_form_s3(BX_o_DX, B);        /* Form S3 */
  ecc_fieldAdd(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 + S2 + S3 */

  ecc_form_s4(BX_o_DX, B);        /* Form S4 */
  ecc_fieldAdd(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 + S2 + S3 + S4 */

  ecc_form_d1(BX_o_DX, B);        /* Form D1 */
  ecc_fieldSub(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 */

  ecc_form_d2(BX_o_DX, B);        /* Form D2 */
  ecc_fieldSub(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 - D2 */

  ecc_form_d3(BX_o_DX, B);        /* Form D3 */
  ecc_fieldSub(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 - D2 - D3 */

  ecc_form_d4(BX_o_DX, B);        /* Form D4 */
  ecc_fieldSub(A, BX_o_DX, A);    /* A = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 - D2 - D3 - D4 */

  if(ecc_compare(A, ecc_prime_m) >= 0) {
    ecc_fieldSub(A, ecc_prime_m, A);
  }
}
static uint32_t
ecc_fieldAddAndDivide(const uint32_t *x, uint32_t *result)
{
  uint32_t n = ecc_add(x, ecc_prime_m, result);
  ecc_rshift(result);
  if(n) {    /* add prime if carry is still set! */
    result[7] |= 0x80000000;    /* add the carry */
    if(ecc_compare(result, ecc_prime_m) == 1) {
      uint32_t tempas[8];
      ecc_setZero(tempas);
      ecc_add(result, ecc_prime_r, tempas);
      ecc_copy(result, tempas);
    }
  }
  return 0;
}
/*
 * Inverse A and output to B
 */
static void
ecc_fieldInv(const uint32_t *A, uint32_t *B)
{
  uint32_t u[8], v[8], x1[8];
  uint32_t tempm[8];
  ecc_setZero(tempm);
  ecc_setZero(u);
  ecc_setZero(v);

  ecc_copy(u, A);
  ecc_copy(v, ecc_prime_m);
  ecc_setZero(x1);
  ecc_setZero(B);
  x1[0] = 1;
  /* While u !=1 and v !=1 */
  while((ecc_isX(u, 1) || ecc_isX(v, 1)) == 0) {
    while(!(u[0] & 1)) {                        /* While u is even */
      ecc_rshift(u);                            /* divide by 2 */
      if(!(x1[0] & 1)) {                        /*ifx1iseven*/
        ecc_rshift(x1);                         /* Divide by 2 */
      } else {
        ecc_fieldAddAndDivide(x1, tempm);        /* tempm=(x1+p)/2 */
        ecc_copy(x1, tempm);                    /* x1=tempm */
      }
    }
    while(!(v[0] & 1)) {                        /* While v is even */
      ecc_rshift(v);                            /* divide by 2 */
      if(!(B[0] & 1)) {                         /*if x2 is even*/
        ecc_rshift(B);                          /* Divide by 2 */
      } else {
        ecc_fieldAddAndDivide(B, tempm);        /* tempm=(x2+p)/2 */
        ecc_copy(B, tempm);                     /* x2=tempm */
      }
    }
    /* tempm=u-v */
    if(ecc_sub(u, v, tempm)) {                  /* If u == 0 */
      ecc_sub(v, u, tempm);                     /* tempm=v-u */
      ecc_copy(v, tempm);                       /* v=v-u */
      ecc_fieldSub(B, x1, tempm);               /* tempm=x2-x1 */
      ecc_copy(B, tempm);                       /* x2=x2-x1 */
    } else {                                    /* If u > 0 */
      ecc_copy(u, tempm);                       /* u=u-v */
      ecc_fieldSub(x1, B, tempm);               /* tempm=x1-x2 */
      ecc_copy(x1, tempm);                      /* x1=x1-x2 */
    }
  }
  if(ecc_isX(u, 1)) {
    ecc_copy(B, x1);
  }
}
static void
ecc_ec_add(const uint32_t *px, const uint32_t *py, const uint32_t *qx, const uint32_t *qy, uint32_t *Sx, uint32_t *Sy)
{
  uint32_t tempC[8];
  uint32_t tempD[16];

  if(ecc_isX(px, 0) && ecc_isX(py, 0)) {
    ecc_copy(Sx, qx);
    ecc_copy(Sy, qy);
    return;
  } else if(ecc_isX(qx, 0) && ecc_isX(qy, 0)) {
    ecc_copy(Sx, px);
    ecc_copy(Sy, py);
    return;
  }

  if(!ecc_compare(px, qx)) {
    if(ecc_compare(py, qy)) {
      ecc_setZero(Sx);
      ecc_setZero(Sy);
      return;
    } else {
      ecc_ec_double(px, py, Sx, Sy);
      return;
    }
  }

  ecc_fieldSub(py, qy, Sx);
  ecc_fieldSub(px, qx, Sy);
  ecc_fieldInv(Sy, Sy);
  ecc_mult(Sx, Sy, tempD, 8);
  ecc_fieldModP(tempC, tempD);         /* tempC = lambda */

  ecc_mult(tempC, tempC, tempD, 8);    /* Sx = lambda^2 */
  ecc_fieldModP(Sx, tempD);
  ecc_fieldSub(Sx, px, Sy);            /* lambda^2 - Px */
  ecc_fieldSub(Sy, qx, Sx);            /* lambda^2 - Px - Qx */

  ecc_fieldSub(qx, Sx, Sy);
  ecc_mult(tempC, Sy, tempD, 8);
  ecc_fieldModP(tempC, tempD);
  ecc_fieldSub(tempC, qy, Sy);
}
static void
ecc_ec_double(const uint32_t *px, const uint32_t *py, uint32_t *Dx, uint32_t *Dy)
{
  uint32_t tempB[8];
  uint32_t tempC[8];
  uint32_t tempD[16];

  if(ecc_isX(px, 0) && ecc_isX(py, 0)) {
    ecc_copy(Dx, px);
    ecc_copy(Dy, py);
    return;
  }

  ecc_mult(px, px, tempD, 8);
  ecc_fieldModP(Dy, tempD);
  ecc_setZero(tempB);
  tempB[0] = 0x00000001;
  ecc_fieldSub(Dy, tempB, tempC);     /* tempC = (qx^2-1) */
  tempB[0] = 0x00000003;
  ecc_mult(tempC, tempB, tempD, 8);
  ecc_fieldModP(Dy, tempD);           /* Dy = 3*(qx^2-1) */
  ecc_fieldAdd(py, py, tempB);        /* tempB = 2*qy */
  ecc_fieldInv(tempB, tempC);         /* tempC = 1/(2*qy) */
  ecc_mult(Dy, tempC, tempD, 8);      /* tempB = lambda = (3*(qx^2-1))/(2*qy) */
  ecc_fieldModP(tempB, tempD);

  ecc_mult(tempB, tempB, tempD, 8);   /* tempC = lambda^2 */
  ecc_fieldModP(tempC, tempD);
  ecc_fieldSub(tempC, px, Dy);        /* lambda^2 - Px */
  ecc_fieldSub(Dy, px, Dx);           /* lambda^2 - Px - Qx */

  ecc_fieldSub(px, Dx, Dy);           /* Dy = qx-dx */
  ecc_mult(tempB, Dy, tempD, 8);      /* tempC = lambda * (qx-dx) */
  ecc_fieldModP(tempC, tempD);
  ecc_fieldSub(tempC, py, Dy);        /* Dy = lambda * (qx-dx) - px */
}
