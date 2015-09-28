/*
 * Copyright (c) 2015, Lars Schmertmann <SmallLars@t-online.de>,
 * Jens Trillmann <jtrillma@informatik.uni-bremen.de>.
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

#define X 0
#define Y 8
#define Z 16

const uint32_t ecc_prime_m[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xffffffff };
const uint32_t ecc_prime_r[8] = { 0x00000001, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffe, 0x00000000 };

/*---------------------------------------------------------------------------*/

#define DEBUG 0
#define SELF_TEST 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTHEX(...) print_hex(__VA_ARGS__)
static void
print_hex(const char *name, const uint32_t *d, uint32_t l)
{
  printf("%s:", name);
  int i;
  for(i = l - 1; i >= 0; --i) {
    printf(" %08X", d[i]);
  }
  printf("\n");
}
#else
#define PRINTF(...)
#define PRINTHEX(...)
#endif

#if SELF_TEST
#include <stdio.h>
static void selfTest();
#endif

/* private prototypes  ----------------------------------------------------- */

/* simple functions to work with 256 bit numbers */
static void ecc_setZero(uint32_t *a);
static void ecc_copy(uint32_t *dst, const uint32_t *src);
static uint32_t ecc_isX(const uint32_t *a, const uint32_t x);
static void ecc_rshift(uint32_t *a);
static void ecc_replace(uint32_t bit, uint32_t *dst, uint32_t *src);
static uint32_t ecc_add(uint32_t *result, const uint32_t *a, const uint32_t *b);
static uint32_t ecc_sub(uint32_t *result, const uint32_t *a, const uint32_t *b);
static void ecc_mult(uint32_t *result, const uint32_t *x, const uint32_t *y, const uint32_t length);

/* ecc_field_ModP-Helper */
__attribute__((always_inline)) static void ecc_form_s1(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_s2(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_s3(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_s4(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d1(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d2(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d3(uint32_t *dst, const uint32_t *src);
__attribute__((always_inline)) static void ecc_form_d4(uint32_t *dst, const uint32_t *src);

/* field functions for 256 bit numbers */
static void ecc_field_Add(uint32_t *result, const uint32_t *x, const uint32_t *y);
static void ecc_field_Sub(uint32_t *result, const uint32_t *x, const uint32_t *y);
static void ecc_field_ModP(uint32_t *result, const uint32_t *T);
static void ecc_field_Mult(uint32_t *result, const uint32_t *A, const uint32_t *B);
static void ecc_field_Inv(uint32_t *result, const uint32_t *A);

/* new projective stuff */
static void ecc_projective_double(uint32_t *val);
static void ecc_projective_add(uint32_t *result, const uint32_t *val_1, const uint32_t *x_2, const uint32_t *y_2, const uint32_t *z_2);

/* public functions -------------------------------------------------------- */

int32_t
ecc_compare(const uint32_t *a, const uint32_t *b)
{
  int32_t r = 0;
  uint32_t i = 8;
  while(i--) {
    uint32_t neq = (a[i] != b[i]);
    int32_t greater = (a[i] > b[i] ? 1 : -1);
    r ^= ((-(!r && neq)) & (r ^ greater));
  }
  return r;
}
void
ecc_ec_mult(uint32_t *resultx, uint32_t *resulty, const uint32_t *px, const uint32_t *py, const uint32_t *secret)
{
#if SELF_TEST
  selfTest();
#endif

  PRINTHEX("PX", px, 8);
  PRINTHEX("PY", py, 8);
  PRINTHEX("SC", secret, 8);

  uint32_t Q[24];
  ecc_setZero(Q + X);
  ecc_setZero(Q + Y);
  ecc_setZero(Q + Z);
  Q[Z] = 0x00000001;

  uint32_t pz[8];
  ecc_setZero(pz);
  pz[0] = 0x00000001;

  uint32_t temp[24];

  int i;
  for(i = 255; i >= 0; --i) {
    ecc_projective_double(Q);
/*    PRINTHEX("QX", Q+X, 8); */
/*    PRINTHEX("QY", Q+Y, 8); */
/*    PRINTHEX("QZ", Q+Z, 8); */
    ecc_projective_add(temp, Q, px, py, pz);
/*    PRINTHEX("QX", temp+X, 8); */
/*    PRINTHEX("QY", temp+Y, 8); */
/*    PRINTHEX("QZ", temp+Z, 8); */
    int current_bit = (secret[i / 32] >> (i % 32)) & 0x1; /* ((secret[i / 32]) & ((uint32_t)1 << (i % 32))); */
    ecc_replace(current_bit, Q, temp);
/*    PRINTHEX("QX", Q+X, 8); */
/*    PRINTHEX("QY", Q+Y, 8); */
/*    PRINTHEX("QZ", Q+Z, 8); */
  }
/*  PRINTHEX("QX", Q+X, 8); */
/*  PRINTHEX("QY", Q+Y, 8); */
/*  PRINTHEX("QZ", Q+Z, 8); */
  ecc_field_Inv(temp, Q + Z);
  ecc_field_Mult(resultx, Q + X, temp);
  ecc_field_Mult(resulty, Q + Y, temp);
  PRINTHEX("RX", resultx, 8);
  PRINTHEX("RY", resulty, 8);
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
  uint32_t r = (a[0] == x);
  uint32_t n = 8;
  while(--n) {
    r &= (a[n] == 0);
  }
  return r;
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
static void
ecc_replace(uint32_t bit, uint32_t *dst, uint32_t *src)
{
  bit = -bit;
  int i;
  for(i = 0; i < 24; i++) {
    dst[i] ^= (bit & (dst[i] ^ src[i]));
  }
}
static uint32_t
ecc_add(uint32_t *result, const uint32_t *a, const uint32_t *b)
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
    [x] "l" (a),
    [y] "l" (b),
    [r] "l" (result)
    : /* clobber list */
    "r4", "r5", "r6", "r7", "memory"
    );

  return carry;
}
static uint32_t
ecc_sub(uint32_t *result, const uint32_t *a, const uint32_t *b)
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
    [x] "l" (a),
    [y] "l" (b),
    [r] "l" (result)
    : /* clobber list */
    "r4", "r5", "r6", "r7", "memory"
    );

  return carry;
}
static void
ecc_mult(uint32_t *result, const uint32_t *x, const uint32_t *y, const uint32_t length)
{
  if(length == 1) {
    /* Version 1: 56 Byte bigger as ASM-Version */
    /* uint64_t *r = (uint64_t *) result; */
    /* *r = (uint64_t) x[0] * (uint64_t) y[0]; */

    /* Version 2: 56 Byte lesser as Version 1 but same speed */
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
      "bcc 0f \n\t"                             /* jump if carry clear */
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
    ecc_mult(result, x, y, length / 2);
    ecc_mult(result + length, x + (length / 2), y + (length / 2), length / 2);
    ecc_mult(C, x, y + (length / 2), length / 2);
    ecc_mult(C + length, x + (length / 2), y, length / 2);
    if(length == 8) {
      carry = ecc_add(C, C, C + length);
    } else {
      asm volatile (
        "cmp %[l], #2 \n\t"
        "beq .add2 \n\t"
        /* ASM for: ecc_add(C, C, C + 4, 4); */
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
        /* ASM for: ecc_add(C, C, C + 2, 2); */
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
      /* ASM for: ecc_add(result + 4, result + 4, C, 12); */
      /*   RRRRRRRRRRRRRRRR */
      /* +     CCCCCCCCC000 */
      /* = RRRRRRRRRRRRRRRR */
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
      /* ASM for: ecc_add(result + 2, result + 2, C, 6); */
      /*   RRRRRRRR */
      /* +   CCCCC0 */
      /* = RRRRRRRR */
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
      /* ASM for: ecc_add(result + 1, result + 1, C, 3); */
      /*   RRRR */
      /* +  CCC */
      /* = RRRR */
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
ecc_field_Add(uint32_t *result, const uint32_t *x, const uint32_t *y)
{
  uint32_t temp[8];
  uint32_t carry = -ecc_add(result, x, y);
  ecc_add(temp, result, ecc_prime_r);

  int i;
  for(i = 0; i < 8; i++) {
    result[i] ^= (carry & (result[i] ^ temp[i]));
  }
}
static void
ecc_field_Sub(uint32_t *result, const uint32_t *x, const uint32_t *y)
{
  uint32_t temp[8];
  uint32_t carry = -ecc_sub(result, x, y);
  ecc_add(temp, result, ecc_prime_m);

  int i;
  for(i = 0; i < 8; i++) {
    result[i] ^= (carry & (result[i] ^ temp[i]));
  }
}
static void
ecc_field_ModP(uint32_t *result, const uint32_t *T)
{
  uint32_t SX_o_DX[8];
  ecc_copy(result, T);                      /* result = T */

  ecc_form_s1(SX_o_DX, T);                  /* Form S1 */
  ecc_field_Add(result, result, SX_o_DX);   /* result = T + S1 */
  ecc_field_Add(result, result, SX_o_DX);   /* result = T + S1 + S1 */

  ecc_form_s2(SX_o_DX, T);                  /* Form S2 */
  ecc_field_Add(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 */
  ecc_field_Add(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 + S2 */

  ecc_form_s3(SX_o_DX, T);                  /* Form S3 */
  ecc_field_Add(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 + S2 + S3 */

  ecc_form_s4(SX_o_DX, T);                  /* Form S4 */
  ecc_field_Add(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 + S2 + S3 + S4 */

  ecc_form_d1(SX_o_DX, T);                  /* Form D1 */
  ecc_field_Sub(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 */

  ecc_form_d2(SX_o_DX, T);                  /* Form D2 */
  ecc_field_Sub(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 - D2 */

  ecc_form_d3(SX_o_DX, T);                  /* Form D3 */
  ecc_field_Sub(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 - D2 - D3 */

  ecc_form_d4(SX_o_DX, T);                  /* Form D4 */
  ecc_field_Sub(result, result, SX_o_DX);   /* result = T + S1 + S1 + S2 + S2 + S3 + S4 - D1 - D2 - D3 - D4 */

  if(ecc_compare(result, ecc_prime_m) >= 0) {
    ecc_field_Sub(result, result, ecc_prime_m);
  }
}
static void
ecc_field_Mult(uint32_t *result, const uint32_t *A, const uint32_t *B)
{
  uint32_t product[16];
  ecc_mult(product, A, B, 8);
  ecc_field_ModP(result, product);
}
static void
ecc_field_Inv(uint32_t *result, const uint32_t *A)
{
  PRINTHEX("Input", A, 8);

  ecc_setZero(result);
  result[0] = 0x00000001;
  int i;
  for(i = 255; i >= 0; --i) {
    ecc_field_Mult(result, result, result);
    if(((ecc_prime_m[i / 32] >> (i % 32)) & 0x1) == 1 && i != 1) {
      ecc_field_Mult(result, result, A);
    }
  }

  PRINTHEX("Result", result, 8);
}
/*---------------------------------------------------------------------------*/

static void
ecc_projective_double(uint32_t *val)
{
  /* Algorithm taken from https://hyperelliptic.org/EFD/g1p/auto-shortw-projective-3.html#doubling-dbl-2007-bl-2 */
  /* w = 3*(X1-Z1)*(X1+Z1) */
  /* s = 2*Y1*Z1 */
  /* ss = s^2 */
  /* sss = s*ss */
  /* R = Y1*s */
  /* RR = R^2 */
  /* B = 2*X1*R */
  /* h = w^2-2*B */
  /* X3 = h*s */
  /* Y3 = w*(B-h)-2*RR */
  /* Z3 = sss */

  uint32_t temp[24];
  uint32_t w[8];
  uint32_t s[8];
  uint32_t B[8];
  uint32_t h[8];

  uint8_t is_zero = ecc_isX(val + X, 0) & ecc_isX(val + Y, 0) & ecc_isX(val + Z, 1);

  ecc_field_Sub(temp + X, val + X, val + Z);
  ecc_field_Add(temp + Y, val + X, val + Z);
  ecc_field_Mult(temp + Z, temp + X, temp + Y);
  ecc_field_Add(temp + X, temp + Z, temp + Z);
  ecc_field_Add(w, temp + Z, temp + X);
  ecc_field_Mult(temp + X, val + Y, val + Z);
  ecc_field_Add(s, temp + X, temp + X);
  ecc_field_Mult(temp + X, s, s);
  ecc_field_Mult(val + Z, s, temp + X);
  ecc_field_Mult(temp + X, val + Y, s); /* temp = R */
  ecc_field_Mult(temp + Z, temp + X, temp + X); /* temp3 = RR */
  ecc_field_Mult(temp + Y, val + X, temp + X); /* temp2 = R*x */
  ecc_field_Add(B, temp + Y, temp + Y); /* B = 2*R*x */
  ecc_field_Mult(temp + X, w, w);
  ecc_field_Add(temp + Y, B, B);
  ecc_field_Sub(h, temp + X, temp + Y);
  ecc_field_Mult(val + X, h, s);
  ecc_field_Sub(temp + X, B, h);
  ecc_field_Mult(temp + Y, w, temp + X);
  ecc_field_Add(temp + Z, temp + Z, temp + Z); /* temp3 = 2*RR */
  ecc_field_Sub(val + Y, temp + Y, temp + Z);
  /* finished, now swap the result if necessary */

  ecc_setZero(temp + X);
  ecc_setZero(temp + Y);
  ecc_setZero(temp + Z);
  (temp + Z)[0] = 0x00000001;

  ecc_replace(is_zero, val, temp);
}
static void
ecc_projective_add(uint32_t *result, const uint32_t *val_1, const uint32_t *x_2, const uint32_t *y_2, const uint32_t *z_2)
{
/* algorithm taken from https://hyperelliptic.org/EFD/g1p/auto-shortw-projective-3.html#addition-add-1998-cmo-2 */
/*                            X       Z     X Y       U            Y  */
/*                            1       1     2 2 U     UU    V      1V */
/*                            Z R     Z     ZVZ R    UZZ    VR     ZY */
/*                      VX    2RAY    2Z    1V1UA    UZV    V2A    2Z */
/*    Y1Z2 = Y1*Z2                                                 |  */
/*    X2Z1 = X2*Z1                          |                      |  */
/*    X1Z2 = X1*Z2            |             |                      |  */
/*    V    = X2Z1-X1Z2  |     x             x                      |  */
/*    VV   = V^2        x     |              |                     |  */
/*    R    = VV*X1Z2    |     x|             x                     |  */
/*    VVV  = V*VV       x      |             x              |      |  */
/*    Y2Z1 = Y2*Z1      |      |              |             |      |  */
/*    U    = Y2Z1-Y1Z2  |      |              x|            |      x  */
/*    UU   = U^2        |      |               x     |      |      |  */
/*    Z1Z2 = Z1*Z2      |      |      |        |     |      |      |  */
/*    UUZZ = UU*Z1Z2    |      |      x        |     x|     |      |  */
/*    UZV  = UUZZ-VVV   |      |      |        |      x|    x      |  */
/*    Z    = VVV*Z1Z2   |      |      x|       |       |    x      |  */
/*    VYZ  = VVV*Y1Z2   |      |       |       |       |    x      x| */
/*    R2   = 2*R        |      x       |       |       |     |      | */
/*    A    = UZV-2R     |      |       |       |       x     x|     | */
/*    X    = V*A        x|     |       |       |              x     | */
/*    RA   = R-A         |     x|      |       |              x     | */
/*    URA  = U*RA        |      x      |       x|                   | */
/*    Y    = URA-VYZ     |       |     |        x                   x */

  uint32_t temp[32];
  #define X1   val_1 + X
  #define Y1   val_1 + Y
  #define Z1   val_1 + Z
  #define X2   x_2
  #define Y2   y_2
  #define Z2   z_2
  #define V    result + X
  #define X1Z2 result + Y
  #define R    result + Y
  #define RA   result + Y
  #define Z1Z2 result + Z
  #define X2Z1 temp + X
  #define VV   temp + X
  #define Y2Z1 temp + X
  #define U    temp + X
  #define URA  temp + X
  #define UU   temp + Y
  #define UUZZ temp + Y
  #define UZV  temp + Y
  #define VVV  temp + Z
  #define R2   temp + Z
  #define A    temp + Z
  #define Y1Z2 temp + 24
  #define VYZ  temp + 24

  uint8_t is_input1_zero = ecc_isX(val_1 + X, 0) & ecc_isX(val_1 + Y, 0) & ecc_isX(val_1 + Z, 1);
  uint8_t is_input2_zero = ecc_isX(x_2, 0) & ecc_isX(y_2, 0) & ecc_isX(z_2, 1);

  ecc_copy(temp + X, x_2);
  ecc_copy(temp + Y, y_2);
  ecc_copy(temp + Z, z_2);
  ecc_replace(is_input1_zero, result, temp);

  ecc_copy(temp + X, val_1 + X);
  ecc_copy(temp + Y, val_1 + Y);
  ecc_copy(temp + Z, val_1 + Z);
  ecc_replace(is_input2_zero, result, temp);

  /* invalidate the result pointer */
  result = (uint32_t *)((uintptr_t)result ^ (-(is_input2_zero | is_input1_zero) & ((uintptr_t)result ^ (uintptr_t)temp)));

  ecc_field_Mult(Y1Z2, Y1, Z2);
  ecc_field_Mult(X2Z1, X2, Z1);
  ecc_field_Mult(X1Z2, X1, Z2);
  ecc_field_Sub(V, X2Z1, X1Z2);
  ecc_field_Mult(VV, V, V);
  ecc_field_Mult(R, VV, X1Z2);
  ecc_field_Mult(VVV, V, VV);
  ecc_field_Mult(Y2Z1, Y2, Z1);
  ecc_field_Sub(U, Y2Z1, Y1Z2);
  ecc_field_Mult(UU, U, U);
  ecc_field_Mult(Z1Z2, Z1, Z2);
  ecc_field_Mult(UUZZ, UU, Z1Z2);
  ecc_field_Sub(UZV, UUZZ, VVV);
  ecc_field_Mult(result + Z, VVV, Z1Z2);
  ecc_field_Mult(VYZ, VVV, Y1Z2);
  ecc_field_Add(R2, R, R);
  ecc_field_Sub(A, UZV, R2);
  ecc_field_Mult(result + X, V, A);
  ecc_field_Sub(RA, R, A);
  ecc_field_Mult(URA, U, RA);
  ecc_field_Sub(result + Y, URA, VYZ);
}
/*---------------------------------------------------------------------------*/

#if SELF_TEST
static void
assertTrue(uint32_t value, const char *msg)
{
  if(!value) {
    printf("%s\n", msg);
  }
}
static void
assertFalse(uint32_t value, const char *msg)
{
  if(value) {
    printf("%s\n", msg);
  }
}
static void
assertSame(uint32_t *val_1, uint32_t *val_2, const char *msg)
{
  if(ecc_compare(val_1, val_2)) {
    printf("%s\n", msg);
  }
}
static void
selfTest()
{
  uint32_t num_000[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
  uint32_t num_001[8] = { 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
  uint32_t num_002[8] = { 0x00000002, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
  uint32_t num_004[8] = { 0x00000004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
  uint32_t num_max[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
  uint32_t primeMinusOne[8] = { 0xfffffffe, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0xffffffff };

  uint32_t result[8];

  /* ecc_compare */
  assertFalse(ecc_compare(num_001, num_001), "ecc_compare 1: Wrong result!");
  assertTrue(ecc_compare(num_000, num_001) == -1, "ecc_compare 2: Wrong result!");
  assertTrue(ecc_compare(num_001, num_000) == 1, "ecc_compare 3: Wrong result!");

  /* ecc_isX */
  assertTrue(ecc_isX(num_000, 0), "ecc_isX 1: Wrong result!");
  assertTrue(ecc_isX(num_001, 1), "ecc_isX 2: Wrong result!");
  assertTrue(ecc_isX(num_002, 2), "ecc_isX 3: Wrong result!");
  assertTrue(ecc_isX(num_004, 4), "ecc_isX 4: Wrong result!");
  assertFalse(ecc_isX(num_000, 1), "ecc_isX 5: Wrong result!");
  assertFalse(ecc_isX(num_000, 2), "ecc_isX 6: Wrong result!");
  assertFalse(ecc_isX(num_000, 4), "ecc_isX 7: Wrong result!");
  assertFalse(ecc_isX(num_001, 0), "ecc_isX 8: Wrong result!");
  assertFalse(ecc_isX(num_001, 2), "ecc_isX 9: Wrong result!");
  assertFalse(ecc_isX(num_001, 4), "ecc_isX 10: Wrong result!");
  assertFalse(ecc_isX(num_002, 0), "ecc_isX 11: Wrong result!");
  assertFalse(ecc_isX(num_002, 1), "ecc_isX 12: Wrong result!");
  assertFalse(ecc_isX(num_002, 4), "ecc_isX 13: Wrong result!");
  assertFalse(ecc_isX(num_004, 0), "ecc_isX 14: Wrong result!");
  assertFalse(ecc_isX(num_004, 1), "ecc_isX 15: Wrong result!");
  assertFalse(ecc_isX(num_004, 2), "ecc_isX 16: Wrong result!");

  /* ecc_add */
  assertFalse(ecc_add(result, num_001, num_002), "ecc_add 1: Unexpected carrybit!");
  assertFalse(ecc_add(result, result, num_001), "ecc_add 2: Unexpected carrybit!");
  assertSame(result, num_004, "ecc_add 3: Wrong result!");
  assertTrue(ecc_add(result, num_max, num_002), "ecc_add 4: Carrybit missing!");
  assertSame(result, num_001, "ecc_add 5: Wrong result!");

  /* ecc_sub */
  assertFalse(ecc_sub(result, num_004, num_002), "ecc_sub 1: Unexpected carrybit!");
  assertFalse(ecc_sub(result, result, num_001), "ecc_sub 2: Unexpected carrybit!");
  assertFalse(ecc_sub(result, result, num_001), "ecc_sub 3: Unexpected carrybit!");
  assertSame(result, num_000, "ecc_sub 4: Wrong result!");
  assertTrue(ecc_sub(result, num_000, num_001), "ecc_sub 5: Carrybit missing!");
  assertSame(result, num_max, "ecc_sub 6: Wrong result!");

  /* ecc_field_Sub */
  ecc_field_Sub(result, num_001, num_000);
  assertSame(num_001, result, "ecc_field_Sub 1: Wrong result!");
  ecc_field_Sub(result, num_001, num_001);
  assertSame(num_000, result, "ecc_field_Sub 2: Wrong result!");
  ecc_field_Sub(result, num_000, num_001);
  assertSame(primeMinusOne, result, "ecc_field_Sub 3: Wrong result!");

  printf("Tests completed!\n");
}
#endif
