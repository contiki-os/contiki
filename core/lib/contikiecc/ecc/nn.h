/**
 * \defgroup nn Natural Number Arithmatic
 *
 * @{
 */

/**
 * \file
 * 				Header file for the Natural Number Arithmatic operations.
 * \author
 * 				Kasun Hewage <kasun.ch@gmail.com>, port to Contiki
 *				Vilhelm Jutvik <ville@imorgon.se>, bug fixes
 *
 */


#ifndef __NN_H__
#define __NN_H__

#include <stdint.h>
#include <contiki.h>

/**
 * A n-bit natural number are represented by an array of w-bit integers. The size
 * of array is n/w
 * 
 */ 

#if defined (SECP128R1) || defined (SECP128R2)
#define KEY_BIT_LEN 128
#else 
#if defined (SECP160K1) || defined (SECP160R1) || defined (SECP160R2) 
#define KEY_BIT_LEN 160
#else 
#if defined (SECP192K1) || defined (SECP192R1)
#define KEY_BIT_LEN 192
#else
#define KEY_BIT_LEN 128
#endif /* 192 */
#endif /* 160 */
#endif /* 128 */


/*---------------------------- 8-bit PROCESSOR -------------------------------*/

#ifdef EIGHT_BIT_PROCESSOR

/* Type definitions */
typedef uint8_t NN_DIGIT;
typedef uint16_t NN_DOUBLE_DIGIT;

/* Types for length */
typedef uint8_t NN_UINT;
typedef uint16_t NN_UINT2;

/* Length of digit in bits */
#define NN_DIGIT_BITS 8

/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS/8)

/* Maximum value of digit */
#define MAX_NN_DIGIT 0xff

/* Number of digits in key
 * used by optimized mod multiplication (ModMultOpt) and optimized mod square (ModSqrOpt)
 *
 */
#define KEYDIGITS (KEY_BIT_LEN/NN_DIGIT_BITS)

/* Maximum length in digits */
#define MAX_NN_DIGITS (KEYDIGITS+1)

/* 
 * Buffer size should be large enough to hold order of base point
 */
#define NUMWORDS MAX_NN_DIGITS

#define MOD_SQR_MASK1 0x8000
#define MOD_SQR_MASK2 0x0100

#endif /* EIGHT_BIT_PROCESSOR */

/*--------------------------- 16-bit PROCESSOR -------------------------------*/
#ifdef SIXTEEN_BIT_PROCESSOR

/* Type definitions */
typedef uint16_t NN_DIGIT;
typedef uint32_t NN_DOUBLE_DIGIT;

/* Types for length */
typedef uint8_t NN_UINT;
typedef uint16_t NN_UINT2;

/* Length of digit in bits */ 
#define NN_DIGIT_BITS 16

/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS/8)

/* Maximum value of digit */
#define MAX_NN_DIGIT 0xffff

/* Number of digits in key
 * used by optimized mod multiplication (ModMultOpt) and optimized mod square (ModSqrOpt)
 *
 */
#define KEYDIGITS (KEY_BIT_LEN/NN_DIGIT_BITS)

/* Maximum length in digits */
#define MAX_NN_DIGITS (KEYDIGITS+1)

/* Buffer size should be large enough to hold order of base point
 */
#define NUMWORDS MAX_NN_DIGITS

#define MOD_SQR_MASK1 0x80000000
#define MOD_SQR_MASK2 0x00010000

#endif /* SIXTEEN_BIT_PROCESSOR */

/*--------------------------- 32-bit PROCESSOR -------------------------------*/

#ifdef THIRTYTWO_BIT_PROCESSOR

/* Type definitions */
typedef uint32_t NN_DIGIT;
typedef uint64_t NN_DOUBLE_DIGIT;

/* Types for length */
typedef uint8_t NN_UINT;
typedef uint16_t NN_UINT2;

/* Length of digit in bits */
#define NN_DIGIT_BITS 32

/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS/8)

/* Maximum value of digit */
#define MAX_NN_DIGIT 0xffffffff

/* Number of digits in key
 * used by optimized mod multiplication (ModMultOpt) and optimized mod square (ModSqrOpt)
 *
 */
#define KEYDIGITS (KEY_BIT_LEN/NN_DIGIT_BITS)

/* Maximum length in digits */
#define MAX_NN_DIGITS (KEYDIGITS+1)

/* Buffer size should be large enough to hold order of base point
 */
#define NUMWORDS MAX_NN_DIGITS

/* the mask for ModSqrOpt */
#define MOD_SQR_MASK1 0x8000000000000000ll
#define MOD_SQR_MASK2 0x0000000100000000ll

#endif /* THIRTYTWO_BIT_PROCESSOR */

/************************* Conversion functions *******************************/

/**
 * \brief       Decodes character string b into a.
 *              Lengths: a[digits], b[len].
 *              Assumes b[i] = 0 for i < len - digits * NN_DIGIT_LEN. 
 *              (Otherwise most significant bytes are truncated.)
 */ 
void NN_Decode(NN_DIGIT *a, NN_UINT digits, unsigned char *b, NN_UINT len);
/**
 * \brief       Encodes a into character string b.
 *              Lengths: a[len], b[digits].
 *              Assumes NN_Bits (b, digits) <= 8 * len. 
 *              (Otherwise most significant digits are truncated.)
 */ 
void NN_Encode(unsigned char *a, NN_UINT len, NN_DIGIT *b, NN_UINT digits);


/************************** ASSIGNMENTS ***************************************/
/**
 * \brief       Assigns a = b.
 *              Lengths: a[digits], b[digits].
 */ 
void NN_Assign(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits);
/**
 * \brief       Assigns a = 0
 *              Lengths: a[digits].
 */ 
void NN_AssignZero(NN_DIGIT *a, NN_UINT digits);
/**
 * \brief       Assigns a = 2^b.
 *              Lengths: a[digits].
 *              Requires b < digits * NN_DIGIT_BITS.
 */ 
void NN_Assign2Exp(NN_DIGIT *a, NN_UINT2 b, NN_UINT digits);
/**
 * \brief       Assigns a = b, where b is a digit.
 */ 
void NN_AssignDigit(NN_DIGIT * a, NN_DIGIT b, NN_UINT digits);


/************************* ARITHMETIC OPERATIONS ******************************/

/**
 * \brief       Computes a = b + c. Returns carry.
 *              a, b ,c can be same
 *              Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Add(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits);
/**
 * \brief       Computes a = b - c. Returns borrow.
 *              a, b, c can be same
 *              Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Sub(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits);
/**
 * \brief       Computes a = b * c.
 *              a, b, c can be same
 *              Lengths: a[2*digits], b[digits], c[digits].
 *              Assumes digits < MAX_NN_DIGITS.
 */
void NN_Mult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits);
/**
 * \brief       Computes a = b * 2^c (i.e. shifts left c bits), returning carry.
 *              a, b can be same
 *              Lengths: a[digits], b[digits].
 *              Requires c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_LShift(NN_DIGIT *a, NN_DIGIT *b, NN_UINT c, NN_UINT digits);
/**
 * \brief       Computes a = b / 2^c (i.e. shifts right c bits), returning carry.
 *              a, b can be same
 *              Lengths: a[digits], b[digits].
 *              Requires c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_RShift(NN_DIGIT *a, NN_DIGIT *b, NN_UINT c, NN_UINT digits);
/**
 * \brief       Computes a = c div d and b = c mod d.
 *              a, c, d can be same
 *              b, c, d can be same
 *              Lengths: a[c_digits], b[d_digits], c[c_digits], d[d_digits].
 *              Assumes d > 0, cDigits < 2 * MAX_NN_DIGITS,
 *              dDigits < MAX_NN_DIGITS.
 */
void NN_Div(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT c_digits, NN_DIGIT *d, NN_UINT d_digits);
/**
 * \brief       Sets a = b / c, where a and c are digits.
 *              Lengths: b[2].
 *              Assumes b[1] < c and HIGH_HALF (c) > 0. For efficiency, 
 *              c should be normalized.
 */
void NN_DigitDiv(NN_DIGIT *a, NN_DIGIT b[2], NN_DIGIT c);
/**
 * \brief       Computes a = b^2
 *              Lengths: a[2*digits], b[digits]
 */ 
void NN_Sqr(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits);

/**************************** NUMBER THEORY ***********************************/

/**
 * \brief       Computes a = b mod c.
 *              Lengths: a[c_digits], b[b_digits], c[c_digits].
 *              Assumes c > 0, bDigits < 2 * MAX_NN_DIGITS, cDigits < MAX_NN_DIGITS.
 */
void NN_Mod(NN_DIGIT *a, NN_DIGIT *b, NN_UINT b_digits, NN_DIGIT *c, NN_UINT c_digits);
/**
 * \brief       Computes a = (b + c) mod d.
 *              a, b, c can be same
 *              Assumption: b,c is in [0, d)
 */
void NN_ModAdd(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_UINT digits);
/**
 * \brief       Computes a = (b - c) mod d.
 *              Assume b and c are all smaller than d
 *              always return positive value
 */
void NN_ModSub(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_UINT digits);
/**
 * \brief       Computes a = b * c mod d.
 *              a, b, c can be same
 *              Lengths: a[digits], b[digits], c[digits], d[digits].
 *              Assumes d > 0, digits < MAX_NN_DIGITS.
 */
void NN_ModMult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, NN_UINT digits);
/** 
 * \brief       Computes a = b^c mod d.
 *              Lengths: a[d_digits], b[d_digits], c[c_digits], d[d_digits].
 *              Assumes d > 0, cDigits > 0, dDigits < MAX_NN_DIGITS.
 */
void NN_ModExp(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT c_digits, NN_DIGIT *d, NN_UINT d_digits);
/**
 * \brief       Compute a = 1/b mod c, assuming inverse exists.
 *              a, b, c can be same
 *              Lengths: a[digits], b[digits], c[digits].
 *              Assumes gcd (b, c) = 1, digits < MAX_NN_DIGITS.
   */
void NN_ModInv(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits);
/**
 * \brief       Computes a = b * c mod d
 *              d is generalized mersenne prime, d = 2^KEYBITS - omega
 *              (Integer multipication with curve-specific optimizations)
 */
void NN_ModMultOpt(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_DIGIT * omega, NN_UINT digits);
/**
 * \brief       Computes a= b/c mod d
 *              Algorithm in "From Euclid's GCD to Montgomery Multiplication 
 *              to the Great Divide"
 */
void NN_ModDivOpt(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, NN_UINT digits);
/**
 * \brief       Computes a = b^2 mod d 
 *              d is generalized mersenne prime, d = 2^KEYBITS - omega
 *              (Integer multipication with curve-specific optimizations)
 */ 
void NN_ModSqrOpt(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * d, NN_DIGIT * omega, NN_UINT digits);


/*************************** OTHER OPERATIONS ********************************/

/**
 * \brief       Returns sign of a - b.
 *              Lengths: a[digits], b[digits].
 */
int NN_Cmp(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits);
/**
 * \brief       Returns 1 iff a = b.
 *              Lengths: a[digits], b[digits].
 */ 
int NN_Equal(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits);
/**
 * \brief       Returns nonzero iff a is zero.
 *              Lengths: a[digits].
 */
int NN_Zero(NN_DIGIT *a, NN_UINT digits);
/**
 * \brief       Returns the significant length of a in bits.
 *              Lengths: a[digits].
 */
unsigned int NN_Bits(NN_DIGIT *a, NN_UINT digits);
/** 
 * \brief       returns 1 iff a = 1
 */
int NN_One(NN_DIGIT * a, NN_UINT digits);
/**
 * \brief       Returns the significant length of a in digits.
 *              Lengths: a[digits].
 */
unsigned int NN_Digits(NN_DIGIT *a, NN_UINT digits);
/**
 * \brief       Returns the significant length of a in bits, where a is a digit.
 */
unsigned int NN_DigitBits(NN_DIGIT a);
/**
 * \brief       Computes a = b + c*d, where c is a digit. Returns carry.
 *              a, b, c can be same
 *              Lengths: a[digits], b[digits], d[digits].
 */
NN_DIGIT NN_AddDigitMult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_UINT digits);
/**
 * \brief       Computes a = b - c*d, where c is a digit. Returns borrow.
 *              a, b, d can be same
 *              Lengths: a[digits], b[digits], d[digits].
 */
NN_DIGIT NN_SubDigitMult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_UINT digits);

/**
 * \brief       Computes b = b - c if b - c > 0
 *              Lengths: b[digits], c[digits].
 */
void NN_ModSmall(NN_DIGIT * b, NN_DIGIT * c, NN_UINT digits);
/**
 * \brief       Computes a = gcd(b, c).
 *              a, b, c can be same
 *              Lengths: a[digits], b[digits], c[digits].
 *              Assumes b > c, digits < MAX_NN_DIGITS.
 */
void NN_Gcd(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits);

NN_UINT omega_mul(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *omega, NN_UINT digits);


#endif /* __NN_H__ */

/** @} */
