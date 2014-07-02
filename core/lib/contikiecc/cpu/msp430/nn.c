/**
 * \defgroup msp430 MSP430 specific implementation
 * \ingroup nn
 * @{
 */

#include "nn.h"
#include <string.h>

#define TRUE 1
#define FALSE 0

#define MODINVOPT

#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define DIGIT_MSB(x) (NN_DIGIT)(((x) >> (NN_DIGIT_BITS - 1)) & 1)
#define DIGIT_2MSB(x) (NN_DIGIT)(((x) >> (NN_DIGIT_BITS - 2)) & 3)

#define NN_ASSIGN_DIGIT(a, b, digits) {NN_AssignZero (a, digits); a[0] = b;}
#define NN_EQUAL(a, b, digits) (! NN_Cmp (a, b, digits))
#define NN_EVEN(a, digits) (((digits) == 0) || ! (a[0] & 1))

/*---------------------------------------------------------------------------*/
/** 
 * \brief             Return b * c, where b and c are NN_DIGITs. 
 *                    The result is a NN_DOUBLE_DIGIT
 */
#define NN_DigitMult(b, c) (NN_DOUBLE_DIGIT)(b) * (c)


/*------------------------- Conversion functions -----------------------------*/
void 
NN_Decode(NN_DIGIT *a, NN_UINT digits, unsigned char *b, NN_UINT len)
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;
  
  for(i = 0, j = len - 1; i < digits && j >= 0; i++) {
    t = 0;
    for(u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8) {
      t |= ((NN_DIGIT)b[j]) << u;
    }
    a[i] = t;
  }
  
  for(; i < digits; i++) {
    a[i] = 0;
  }
}
/*---------------------------------------------------------------------------*/
void 
NN_Encode(unsigned char *a, NN_UINT len, NN_DIGIT *b, NN_UINT digits)
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;

  for(i = 0, j = len - 1; i < digits && j >= 0; i++) {
    t = b[i];
    for(u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8) {
      a[j] = (unsigned char)(t >> u);
    }
  }

  for(; j >= 0; j--) {
    a[j] = 0;
  }
}

/*------------------------------ ASSIGNMENTS ---------------------------------*/
void 
NN_Assign(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits)
{
  memcpy(a, b, digits*NN_DIGIT_LEN);
}

/*---------------------------------------------------------------------------*/
void 
NN_AssignZero(NN_DIGIT *a, NN_UINT digits)
{
  uint8_t i;

  for(i = 0; i < digits; i++) {
    a[i] = 0;
  }
}
/*---------------------------------------------------------------------------*/
void 
NN_Assign2Exp(NN_DIGIT *a, NN_UINT2 b, NN_UINT digits)
{
  NN_AssignZero(a, digits);

  if(b >= digits * NN_DIGIT_BITS) {
    return;
  }

  a[b / NN_DIGIT_BITS] = (NN_DIGIT)1 << (b % NN_DIGIT_BITS);
}
/*---------------------------------------------------------------------------*/
void 
NN_AssignDigit(NN_DIGIT * a, NN_DIGIT b, NN_UINT digits)
{
  NN_AssignZero(a, digits);
  a[0] = b;
}
/*-------------------------- ARITHMETIC OPERATIONS ---------------------------*/
NN_DIGIT 
NN_Add(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
{
  NN_DIGIT carry, ai;
  NN_UINT i;

  carry = 0;

  for(i = 0; i < digits; i++) {
    if((ai = b[i] + carry) < carry) {
      ai = c[i];
    } else if ((ai += c[i]) < c[i]) {
      carry = 1;
    } else {
      carry = 0;
    }
    a[i] = ai;
  }

  return carry;
}

/*---------------------------------------------------------------------------*/
NN_DIGIT 
NN_Sub(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
{
  NN_DIGIT ai, borrow;
  NN_UINT i;

  borrow = 0;

  for(i = 0; i < digits; i++) {
    if((ai = b[i] - borrow) > (MAX_NN_DIGIT - borrow)) {
      ai = MAX_NN_DIGIT - c[i];
    } else if((ai -= c[i]) > (MAX_NN_DIGIT - c[i])) {
        borrow = 1;
    } else {
        borrow = 0;
    }
    a[i] = ai;
  }

  return borrow;
}
/*---------------------------------------------------------------------------*/
void 
NN_Mult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
{
  NN_DIGIT t[2 * MAX_NN_DIGITS];
  unsigned int b_digits, c_digits, i;

  NN_AssignZero (t, 2 * digits);
  
  b_digits = NN_Digits (b, digits);
  c_digits = NN_Digits (c, digits);

  for (i = 0; i < b_digits; i++)
    t[i + c_digits] += NN_AddDigitMult (&t[i], &t[i], b[i], c, c_digits);
  
  NN_Assign (a, t, 2 * digits);

}
/*---------------------------------------------------------------------------*/
NN_DIGIT 
NN_LShift(NN_DIGIT *a, NN_DIGIT *b, NN_UINT c, NN_UINT digits)
{
  NN_DIGIT bi, carry;
  NN_UINT i, t;
  
  if(c >= NN_DIGIT_BITS) {
    return (0);
  }
  
  t = NN_DIGIT_BITS - c;

  carry = 0;

  for(i = 0; i < digits; i++) {
    bi = b[i];
    a[i] = (bi << c) | carry;
    carry = c ? (bi >> t) : 0;
  }
  
  return carry;
}
/*---------------------------------------------------------------------------*/
NN_DIGIT 
NN_RShift(NN_DIGIT *a, NN_DIGIT *b, NN_UINT c, NN_UINT digits)
{
  NN_DIGIT bi, carry;
  int i;
  NN_UINT t;
  
  if(c >= NN_DIGIT_BITS) {
    return (0);
  }
  
  t = NN_DIGIT_BITS - c;

  carry = 0;

  for(i = digits - 1; i >= 0; i--) {
    bi = b[i];
    a[i] = (bi >> c) | carry;
    carry = c ? (bi << t) : 0;
  }
  
  return carry;
}
/*---------------------------------------------------------------------------*/
void 
NN_Div(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT c_digits, NN_DIGIT *d, NN_UINT d_digits)
{
  NN_DIGIT ai, cc[2 * MAX_NN_DIGITS+1], dd[MAX_NN_DIGITS], t;

  int i;
  int dd_digits, shift;
  
  dd_digits = NN_Digits (d, d_digits);
  if(dd_digits == 0) {
    return;
  }
  
  /* Normalize operands. */
  shift = NN_DIGIT_BITS - NN_DigitBits (d[dd_digits-1]);
  NN_AssignZero (cc, dd_digits);
  cc[c_digits] = NN_LShift (cc, c, shift, c_digits);
  NN_LShift (dd, d, shift, dd_digits);
  t = dd[dd_digits - 1];

  if(a != NULL) {
    NN_AssignZero (a, c_digits);
  }

  for(i = c_digits - dd_digits; i >= 0; i--) {
    /* Underestimate quotient digit and subtract. */
    if (t == MAX_NN_DIGIT) {
      ai = cc[i + dd_digits];
    } else {
      NN_DigitDiv(&ai, &cc[i + dd_digits-1], t + 1);
    }
    cc[i + dd_digits] -= NN_SubDigitMult(&cc[i], &cc[i], ai, dd, dd_digits);

    /* Correct estimate. */
    while(cc[i + dd_digits] || (NN_Cmp (&cc[i], dd, dd_digits) >= 0)) {
      ai++;
      cc[i+dd_digits] -= NN_Sub (&cc[i], &cc[i], dd, dd_digits);
    }
    if(a != NULL) {
      a[i] = ai;
    }
  }    
    /* Restore result. */
  NN_AssignZero (b, d_digits);
  NN_RShift (b, cc, shift, dd_digits);
}
/*---------------------------------------------------------------------------*/
void 
NN_DigitDiv(NN_DIGIT *a, NN_DIGIT b[2], NN_DIGIT c)
{
  NN_DOUBLE_DIGIT t;
  t = (((NN_DOUBLE_DIGIT)b[1]) << NN_DIGIT_BITS) ^ ((NN_DOUBLE_DIGIT)b[0]);
  *a = t/c;
}
/*---------------------------------------------------------------------------*/
void 
NN_Sqr(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits)
{
  NN_DIGIT t[2 * MAX_NN_DIGITS];
  NN_UINT b_digits, i;

  NN_AssignZero (t, 2 * digits);
  
  b_digits = NN_Digits (b, digits);
    
  for (i = 0; i < b_digits; i++) {
    t[i + b_digits] += NN_AddDigitMult (&t[i], &t[i], b[i], b, b_digits);
  }
  
  NN_Assign (a, t, 2 * digits);
}

/*------------------------- NUMBER THEORY ------------------------------------*/
void 
NN_Mod(NN_DIGIT *a, NN_DIGIT *b, NN_UINT b_digits, NN_DIGIT *c, NN_UINT c_digits)
{  
  NN_Div(NULL, a, b, b_digits, c, c_digits);
}
/*---------------------------------------------------------------------------*/
void 
NN_ModAdd(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_UINT digits)
{
  NN_DIGIT tmp[MAX_NN_DIGITS];
  NN_DIGIT carry;
    
  carry = NN_Add(tmp, b, c, digits);
  if(carry) {
    NN_Sub(a, tmp, d, digits);
  } else if(NN_Cmp(tmp, d, digits) >= 0) {
      NN_Sub(a, tmp, d, digits);
  } else {
    NN_Assign(a, tmp, digits);
  }
    
}
/*---------------------------------------------------------------------------*/
void 
NN_ModSub(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_UINT digits)
{
  NN_DIGIT tmp[MAX_NN_DIGITS];
  NN_DIGIT borrow;
    
  borrow = NN_Sub(tmp, b, c, digits);
  if(borrow) {
    NN_Add(a, tmp, d, digits);
  } else {
    NN_Assign(a, tmp, digits);
  }
    
}
/*---------------------------------------------------------------------------*/
void 
NN_ModMult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, NN_UINT digits)
{
  NN_DIGIT t[2 * MAX_NN_DIGITS];
    
  //memset(t, 0, 2*MAX_NN_DIGITS*NN_DIGIT_LEN);
  t[2 * MAX_NN_DIGITS-1] = 0;
  t[2 * MAX_NN_DIGITS-2] = 0;
  NN_Mult(t, b, c, digits);
  NN_Mod(a, t, 2 * digits, d, digits);
}
/*---------------------------------------------------------------------------*/
void 
NN_ModExp(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT c_digits, NN_DIGIT *d, NN_UINT d_digits)
{
  NN_DIGIT b_power[3][MAX_NN_DIGITS], ci, t[MAX_NN_DIGITS];
  int i;
  uint8_t ci_bits, j, s;

  /* Store b, b^2 mod d, and b^3 mod d. */

  NN_Assign(b_power[0], b, d_digits);
  NN_ModMult(b_power[1], b_power[0], b, d, d_digits);
  NN_ModMult(b_power[2], b_power[1], b, d, d_digits);
  
  NN_ASSIGN_DIGIT(t, 1, d_digits);

  c_digits = NN_Digits(c, c_digits);

  for(i = c_digits - 1; i >= 0; i--) {
    ci = c[i];
    ci_bits = NN_DIGIT_BITS;
      
    /* Scan past leading zero bits of most significant digit. */
    if(i == (int)(c_digits - 1)) {
      while (! DIGIT_2MSB (ci)) {
        ci <<= 2;
        ci_bits -= 2;
      }
    }

    for(j = 0; j < ci_bits; j += 2, ci <<= 2) {
      /* Compute t = t^4 * b^s mod d, where s = two MSB's of ci. */
      NN_ModMult(t, t, t, d, d_digits);
      NN_ModMult(t, t, t, d, d_digits);
      if ((s = DIGIT_2MSB (ci)) != 0) {
        NN_ModMult (t, t, b_power[s-1], d, d_digits);
      }
    }
  }
  
  NN_Assign(a, t, d_digits);
}
/*---------------------------------------------------------------------------*/
void 
NN_ModInv(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
{
  NN_DIGIT q[MAX_NN_DIGITS], t1[MAX_NN_DIGITS], t3[MAX_NN_DIGITS],
          u1[MAX_NN_DIGITS], u3[MAX_NN_DIGITS], v1[MAX_NN_DIGITS],
          v3[MAX_NN_DIGITS], w[2 * MAX_NN_DIGITS];
  int u1Sign;

  /* Apply extended Euclidean algorithm, modified to avoid negative numbers. */
  NN_ASSIGN_DIGIT(u1, 1, digits);
  NN_AssignZero(v1, digits);
  NN_Assign(u3, b, digits);
  NN_Assign(v3, c, digits);
  u1Sign = 1;

  while (!NN_Zero(v3, digits)) {
    NN_Div (q, t3, u3, digits, v3, digits);
    NN_Mult (w, q, v1, digits);
    NN_Add (t1, u1, w, digits);
    NN_Assign (u1, v1, digits);
    NN_Assign (v1, t1, digits);
    NN_Assign (u3, v3, digits);
    NN_Assign (v3, t3, digits);
    u1Sign = -u1Sign;
  }
  
  /* Negate result if sign is negative. */
  if (u1Sign < 0) {
      NN_Sub (a, c, u1, digits);
  } else {
      NN_Assign (a, u1, digits);
  }

}
/*---------------------------------------------------------------------------*/
void 
NN_ModMultOpt(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_DIGIT * omega, NN_UINT digits)
{
  NN_DIGIT t1[2*MAX_NN_DIGITS];
  NN_DIGIT t2[2*MAX_NN_DIGITS];
  NN_DIGIT *pt1;
  NN_UINT len_t2, len_t1;

  //memset(t1, 0, 2*MAX_NN_DIGITS*NN_DIGIT_LEN);
  //memset(t2+KEYDIGITS*NN_DIGIT_LEN, 0, (2*MAX_NN_DIGITS-KEYDIGITS)*NN_DIGIT_LEN);
  t1[2*MAX_NN_DIGITS-1]=0;
  t1[2*MAX_NN_DIGITS-2]=0;
  t2[2*MAX_NN_DIGITS-1]=0;
  t2[2*MAX_NN_DIGITS-2]=0;

  NN_Mult(t1, b, c, KEYDIGITS);
     
  pt1 = &(t1[KEYDIGITS]);
  len_t2 = 2 * KEYDIGITS;
     
  /*
   * The "Curve-Specific Optimizations" algorithm in 
   * "Comparing Elliptic Curve Cryptography and RSA on 8-bit CPUs"
   */ 
  while(!NN_Zero(pt1, KEYDIGITS)) {
    memset(t2, 0, len_t2*NN_DIGIT_LEN);
    len_t2 -= KEYDIGITS;
    len_t1 = len_t2;
    len_t2 = omega_mul(t2, pt1, omega, len_t2);
    memset(pt1, 0, len_t1*NN_DIGIT_LEN);
    NN_Add(t1, t2, t1, MAX(KEYDIGITS,len_t2)+1);
  }

  while(NN_Cmp(t1, d, digits) > 0) {
    NN_Sub(t1, t1, d, digits);      
  }

  NN_Assign(a, t1, digits);
     
}
/*---------------------------------------------------------------------------*/
void 
NN_ModDivOpt(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, NN_UINT digits)
{
  NN_DIGIT A[MAX_NN_DIGITS], B[MAX_NN_DIGITS], U[MAX_NN_DIGITS], V_[MAX_NN_DIGITS];
  int tmp_even;

  NN_Assign(A, c, digits);
  NN_Assign(B, d, digits);
  NN_Assign(U, b, digits);
  NN_AssignZero(V_, digits);
    
  while((tmp_even = NN_Cmp(A, B, digits)) != 0) {
    if(NN_EVEN(A, digits)) {
      NN_RShift(A, A, 1, digits);
      if(NN_EVEN(U, digits)){
        NN_RShift(U, U, 1, digits);
      } else {
        NN_Add(U, U, d, digits);
        NN_RShift(U, U, 1, digits);
      }
    } else if(NN_EVEN(B, digits)) {
      NN_RShift(B, B, 1, digits);
      if(NN_EVEN(V_, digits)) {
        NN_RShift(V_, V_, 1, digits);
      } else {
        NN_Add(V_, V_, d, digits);
        NN_RShift(V_, V_, 1, digits);
      }
    } else if(tmp_even > 0) {
      NN_Sub(A, A, B, digits);
      NN_RShift(A, A, 1, digits);
      if(NN_Cmp(U, V_, digits) < 0) {
        NN_Add(U, U, d, digits);
      }
      NN_Sub(U, U, V_, digits);
      if(NN_EVEN(U, digits)) {
        NN_RShift(U, U, 1, digits);
      } else {
        NN_Add(U, U, d, digits);
        NN_RShift(U, U, 1, digits);
      }
    } else {
      NN_Sub(B, B, A, digits);
      NN_RShift(B, B, 1, digits);
      if(NN_Cmp(V_, U, digits) < 0) {
        NN_Add(V_, V_, d, digits);
      }
      NN_Sub(V_, V_, U, digits);
      if(NN_EVEN(V_, digits)) {
        NN_RShift(V_, V_, 1, digits);
      } else {
        NN_Add(V_, V_, d, digits);
        NN_RShift(V_, V_, 1, digits);
      }
    }
  }

  NN_Assign(a, U, digits);
}
/*---------------------------------------------------------------------------*/
void 
NN_ModSqrOpt(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * d, NN_DIGIT * omega, NN_UINT digits)
{
  NN_DIGIT t1[2*MAX_NN_DIGITS];
  NN_DIGIT t2[2*MAX_NN_DIGITS];
  NN_DIGIT *pt1;
  NN_UINT len_t1, len_t2;

  t1[2*MAX_NN_DIGITS-1]=0;
  t1[2*MAX_NN_DIGITS-2]=0;
  t2[2*MAX_NN_DIGITS-1]=0;
  t2[2*MAX_NN_DIGITS-2]=0;

  NN_Sqr(t1, b, KEYDIGITS);
     
  pt1 = &(t1[KEYDIGITS]);
  len_t2 = 2*KEYDIGITS;
  /*
   * The "Curve-Specific Optimizations" algorithm in 
   * "Comparing Elliptic Curve Cryptography and RSA on 8-bit CPUs"
   */ 
  while(!NN_Zero(pt1, KEYDIGITS)) {
    memset(t2, 0, len_t2*NN_DIGIT_LEN);
    len_t2 -= KEYDIGITS;
    len_t1 = len_t2;
    len_t2 = omega_mul(t2, pt1, omega, len_t2);
    memset(pt1, 0, len_t1*NN_DIGIT_LEN);
    NN_Add(t1, t2, t1, MAX(KEYDIGITS,len_t2)+1);
  }
     
  while(NN_Cmp(t1, d, digits) > 0) {
    NN_Sub(t1, t1, d, digits);
  }
  NN_Assign (a, t1, digits);

}
/*--------------------------- OTHER OPERATIONS -------------------------------*/
void 
NN_Gcd(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
{
  NN_DIGIT t[MAX_NN_DIGITS], u[MAX_NN_DIGITS], v[MAX_NN_DIGITS];

  NN_Assign(u, b, digits);
  NN_Assign(v, c, digits);

  while(!NN_Zero(v, digits)) {
    NN_Mod(t, u, digits, v, digits);
    NN_Assign(u, v, digits);
    NN_Assign(v, t, digits);
  }

  NN_Assign(a, u, digits);

}

/*---------------------------------------------------------------------------*/
int 
NN_Cmp(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits)
{
  int i;
  
  for(i = digits - 1; i >= 0; i--) { 
    if(a[i] > b[i]) {
      return 1;
    } else if (a[i] < b[i]) {
      return -1; 
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int 
NN_Equal(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits)
{
  return !NN_Cmp(a, b, digits);
}
/*---------------------------------------------------------------------------*/
int 
NN_Zero(NN_DIGIT *a, NN_UINT digits)
{
  NN_UINT i;
  
  for(i = 0; i < digits; i++) {
    if(a[i]) {
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
int 
NN_One(NN_DIGIT * a, NN_UINT digits)
{
  uint8_t i;
    
  for(i = 1; i < digits; i++) {
    if(a[i]) {
      return FALSE;
    }
    if(a[0] == 1) {
      return TRUE;
    }
  }
    
  return FALSE;
}
/*---------------------------------------------------------------------------*/
unsigned int 
NN_Bits(NN_DIGIT *a, NN_UINT digits)
{
  if((digits = NN_Digits(a, digits)) == 0) {
    return 0;
  }
  
  return ((digits - 1) * NN_DIGIT_BITS + NN_DigitBits (a[digits-1]));
}

/*---------------------------------------------------------------------------*/
unsigned int 
NN_Digits(NN_DIGIT *a, NN_UINT digits)
{
  int i;
  
  for(i = digits - 1; i >= 0; i--) {
    if(a[i]) {
      break;
    }
  }

  return i + 1;
}
/*---------------------------------------------------------------------------*/
unsigned int 
NN_DigitBits(NN_DIGIT a)
{
  unsigned int i;
  
  for(i = 0; i < NN_DIGIT_BITS; i++, a >>= 1) {
    if(a == 0) {
      break;
    }
  }  
  return i;
}
/*---------------------------------------------------------------------------*/
NN_DIGIT 
NN_AddDigitMult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_UINT digits)
{
  NN_DIGIT carry;
  unsigned int i;
  NN_DOUBLE_DIGIT t;

  /* Should copy b to a */
  if(c == 0) {
    return (0);
  }

  carry = 0;

  for(i = 0; i < digits; i++) {
    t = NN_DigitMult (c, d[i]);
    if ((a[i] = b[i] + carry) < carry) {
      carry = 1;
    } else {
      carry = 0;
    }
    if((a[i] += (t & MAX_NN_DIGIT)) < (t & MAX_NN_DIGIT)) {
      carry++;
    }
    carry += (NN_DIGIT)(t >> NN_DIGIT_BITS);
  }

  return carry;
}

/*---------------------------------------------------------------------------*/
NN_DIGIT 
NN_SubDigitMult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_UINT digits)
{
  NN_DIGIT borrow;
  unsigned int i;

  NN_DOUBLE_DIGIT t;


  if (c == 0) {
    return 0;
  }

  borrow = 0;

  for(i = 0; i < digits; i++) {
    t = NN_DigitMult (c, d[i]);
    if ((a[i] = b[i] - borrow) > (MAX_NN_DIGIT - borrow)) {
      borrow = 1;
    } else {
      borrow = 0;
    }
    if ((a[i] -= (t & MAX_NN_DIGIT)) > (MAX_NN_DIGIT - (t & MAX_NN_DIGIT))) {
      borrow++;
    }
    borrow += (NN_DIGIT)(t >> NN_DIGIT_BITS);
  }
    
  return borrow;
}
/*---------------------------------------------------------------------------*/
void 
NN_ModSmall(NN_DIGIT * b, NN_DIGIT * c, NN_UINT digits)
{
  while(NN_Cmp(b, c, digits) > 0) {
    NN_Sub(b, b, c, digits);
  }
}

/**
 * @}
 */ 
