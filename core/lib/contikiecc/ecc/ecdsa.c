/**
 * \addtogroup ecdsa
 *
 * @{
 */

/**
 * \file
 * Source file for the Elliptic Curve Digital Signature Algorithm functions.
 * \author
 * Kasun Hewage <kasun.ch@gmail.com>
 *
 */
#include "contikiecc/ecc/ecc.h"
#include "contikiecc/ecc/ecc_sha1.h"
//#include "lib/rand.h"
#include <stdlib.h>

#include <string.h>

#define TRUE 1
#define FALSE 0

//enable shamir trick
//#define SHAMIR_TRICK

#ifdef SHAMIR_TRICK
/* The size of sliding window, S_W_BITS <= 8 */
#define S_W_BITS 2

/* 
 * basic mask used to generate mask array
 * For example: if S_W_BITS is 2, BASIC_MASK must be 0x03;
 *              if S_W_BITS is 4, BASIC_MASK must be 0x0f;
 *              if S_W_BITS is 8, BASIC_MASK must be 0xff;
 */ 
#define S_BASIC_MASK ((1 << S_W_BITS) - 1)

/* number of windows in one digit, NUM_MASKS = NN_DIGIT_BITS/W_BITS */
#define S_NUM_MASKS (NN_DIGIT_BITS/S_W_BITS)

/* number of points for precomputed points, NN_POINTS = 2^W_BITS - 1 */
#define S_NUM_POINTS ((1 << (2*S_W_BITS)) - 1)

#define S_MAX ((1 << S_W_BITS) - 1)

#endif /* SHAMIR_TRICK */



#ifdef SHAMIR_TRICK
static point_t pqBaseArray[S_NUM_POINTS];
static NN_DIGIT s_mask[S_NUM_MASKS];
static curve_params_t* param;
#else
/* precomputed array of public key(used in verification) for 
 * sliding window method. 
 */
static point_t qBaseArray[NUM_POINTS];
#endif /* SHAMIR_TRICK */

static NN_DIGIT order[NUMWORDS];
//static point_t * baseP;


  //generate random natural number a[length]
void 
gen_random(NN_DIGIT *a, uint8_t length)
{
/*
  a[5] = 0x00000000;
  a[4] = 0x7b012db7;
  a[3] = 0x681a3f28;
  a[2] = 0xb9185c8b;
  a[1] = 0x2ac5d528;
  a[0] = 0xdecd52da;
  uint8_t ri;
*/	
  int ri;
  for(ri=0; ri<length; ri++) { 
    random_init(100);
    a[ri] = ((uint32_t)rand() << 16)^((uint32_t)rand());
  }

} 

/*---------------------------------------------------------------------------*/
#ifdef SHAMIR_TRICK
/**
 * \brief             Shamir trick init.
 */ 
static void 
shamir_init(point_t * pKey, point_t * pointArray)
{
  uint16_t i;
  uint16_t j;

  baseP = ecc_get_base_p();

  /* max = 2^w-1
   * [0] = Q
   * [1] = 2Q
   * ...
   * [max-1] = max*Q
   */
  NN_Assign(pointArray[0].x, pKey->x, NUMWORDS);
  NN_Assign(pointArray[0].y, pKey->y, NUMWORDS);
    
  for(i=1; i<S_MAX; i++) {
    ecc_add(&(pointArray[i]), &(pointArray[i-1]), pKey);
  }

  /*
   * [max] = P
   * [max+1] = P+Q
   * ...
   * [max+max] = P+max*Q
   * [max+max+1] = 2P
   * [max+max+2] = 2P+Q
   * ...
   * [max+2*max+1] = 2P+max*Q
   * ...
   */
  for(i = 0; i < S_MAX; i++) {
    for(j = 0; j<S_MAX+1; j++) {
      if(i == 0 && j==0) {
        NN_Assign(pointArray[S_MAX].x, baseP->x, NUMWORDS);
	      NN_Assign(pointArray[S_MAX].y, baseP->y, NUMWORDS);
      } else if(j==0) {
        ecc_add(&(pointArray[S_MAX+(S_MAX+1)*i]), 
                &(pointArray[S_MAX+(S_MAX+1)*(i-1)]), 
                baseP);
      }else{
        ecc_add(&(pointArray[S_MAX+(S_MAX+1)*i+j]), 
                &(pointArray[S_MAX+(S_MAX+1)*i]), 
                &(pointArray[j-1]));
      }
    }
  }
    
  for(i = 0; i < S_NUM_MASKS; i++) {
    s_mask[i] = S_BASIC_MASK << (S_W_BITS*i);    
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief             Shamir trick
 * 
 */ 
static void 
shamir(point_t * P0, NN_DIGIT * u1, NN_DIGIT * u2)
{
  int16_t i, tmp, tmp2;
  int8_t j, k;
  NN_DIGIT windex;
  NN_DIGIT Z0[NUMWORDS];
  NN_DIGIT Z1[NUMWORDS];

  /* clear point */
  NN_AssignZero(P0->x, NUMWORDS);
  NN_AssignZero(P0->y, NUMWORDS);
    
  /* convert to Jprojective coordinate */
  NN_AssignZero(Z0, NUMWORDS);
  NN_AssignZero(Z1, NUMWORDS);
  Z1[0] = 0x01;	
    
  tmp = NN_Digits(u1, NUMWORDS);
  tmp2 = NN_Digits(u2, NUMWORDS);
  if(tmp2 > tmp) {
    tmp = tmp2;
  }

  for(i = tmp - 1; i >= 0; i--) { 
    for(j = NN_DIGIT_BITS/S_W_BITS - 1; j >= 0; j--) {
      for(k = 0; k < S_W_BITS; k++) {
        ecc_dbl_proj(P0, Z0, P0, Z0);
      }
	
      if(j != 0) {
        windex = ((s_mask[j] & u1[i]) >> ((j-1)*S_W_BITS)) | 
                 ((s_mask[j] & u2[i]) >> (j*S_W_BITS));
      } else {
        windex = ((s_mask[j] & u1[i]) << S_W_BITS) | 
                 (s_mask[j] & u2[i]);
      }
      if(windex) {
        ecc_add_proj(P0, Z0, P0, Z0, &(pqBaseArray[windex-1]), Z1);
      }
    }
      
  }

       
  /* convert back to affine coordinate */
  if(NN_One(Z0, NUMWORDS) == FALSE) {
    NN_ModInv(Z1, Z0, param->p, NUMWORDS);
    NN_ModMultOpt(Z0, Z1, Z1, param->p, param->omega, NUMWORDS);
    NN_ModMultOpt(P0->x, P0->x, Z0, param->p, param->omega, NUMWORDS);
    NN_ModMultOpt(Z0, Z0, Z1, param->p, param->omega, NUMWORDS);
    NN_ModMultOpt(P0->y, P0->y, Z0, param->p, param->omega, NUMWORDS);
  }

}
#endif /* SHAMIR_TRICK */
/*---------------------------------------------------------------------------*/
void 
ecdsa_init(point_t * pb_key)
{
#ifdef SHAMIR_TRICK
  param = ecc_get_param();
  shamir_init(pb_key, pqBaseArray);
#else
  /* precompute the array of public key for sliding window method */
  ecc_win_precompute(pb_key, qBaseArray);
#endif /* SHAMIR_TRICK */
  /* we need to know param->r */
  ecc_get_order(order);
}

/*---------------------------------------------------------------------------*/
void 
ecdsa_sign(uint8_t *msg, uint16_t len, NN_DIGIT *r, NN_DIGIT *s, NN_DIGIT *d)
{

  char done = FALSE;
  NN_DIGIT k[NUMWORDS];
  NN_DIGIT k_inv[NUMWORDS];
  NN_DIGIT tmp[NUMWORDS];
  NN_DIGIT digest[NUMWORDS];
  point_t P;
  uint8_t sha1sum[20];
  NN_DIGIT sha1tmp[20/NN_DIGIT_LEN];
  SHA1Context ctx;
		
  while(!done) {
    gen_random(k, NUMWORDS);
    NN_Mod(k, k, NUMWORDS, order, NUMWORDS);

    if((NN_Zero(k, NUMWORDS)) == 1) {
      continue;
    }
    ecc_win_mul_base(&P, k);
    NN_Mod(r, P.x, NUMWORDS, order, NUMWORDS);	
      
    if((NN_Zero(r, NUMWORDS)) == 1) {
	    continue;
    }
    NN_ModInv(k_inv, k, order, NUMWORDS);
      //memset(sha1sum, 0, 20);
    sha1_reset(&ctx);
    sha1_update(&ctx, msg, len);
    sha1_digest(&ctx, sha1sum);

    NN_Decode(sha1tmp, 20/NN_DIGIT_LEN, sha1sum, 20);
    NN_Mod(digest, sha1tmp, 20/NN_DIGIT_LEN, order, NUMWORDS);
    NN_ModMult(k, d, r, order, NUMWORDS);
    NN_ModAdd(tmp, digest, k, order, NUMWORDS);
    NN_ModMult(s, k_inv, tmp, order, NUMWORDS);
    if((NN_Zero(s, NUMWORDS)) != 1) {			
	    done = TRUE;
    }
    
  }
		
}
/*---------------------------------------------------------------------------*/	
uint8_t 
ecdsa_verify(uint8_t *msg, uint16_t len, NN_DIGIT *r, NN_DIGIT *s, point_t *Q)
{
  uint8_t sha1sum[20];
  NN_DIGIT sha1tmp[20/NN_DIGIT_LEN];
  SHA1Context ctx;
  NN_DIGIT w[NUMWORDS];
  NN_DIGIT u1[NUMWORDS];
  NN_DIGIT u2[NUMWORDS];
  NN_DIGIT digest[NUMWORDS];
#ifndef SHAMIR_TRICK
  point_t u1P, u2Q;
#endif
  point_t final;

  /* r and s shoud be in [1, p-1] */
  if((NN_Cmp(r, order, NUMWORDS)) >= 0) {
    return 3;
  }
  if((NN_Zero(r, NUMWORDS)) == 1) {
    return 4;
  }
  if((NN_Cmp(s, order, NUMWORDS)) >= 0) {
    return 5;
  }
  if((NN_Zero(s, NUMWORDS)) == 1) {
    return 6;
  }
		
  /* w = s^-1 mod p */
  NN_ModInv(w, s, order, NUMWORDS);
  /* sha1(msg) */
  memset(sha1sum, 0, 20);
  // calculate hash, e
  sha1_reset(&ctx);
  sha1_update(&ctx, msg, len);
  sha1_digest(&ctx, sha1sum);

  NN_Decode(sha1tmp, 20/NN_DIGIT_LEN, sha1sum, 20);
  NN_Mod(digest, sha1tmp, 20/NN_DIGIT_LEN, order, NUMWORDS);
  /* u1 = ew mod p */
  NN_ModMult(u1, digest, w, order, NUMWORDS);
  /* u2 = rw mod p */
  NN_ModMult(u2, r, w, order, NUMWORDS);

  /* u1P+u2Q */
#ifdef SHAMIR_TRICK
  shamir(&final, u1, u2);
#else
  ecc_win_mul_base(&u1P, u1);
  ecc_win_mul(&u2Q, u2, qBaseArray);
  ecc_add(&final, &u1P, &u2Q);
#endif

  NN_Mod(w, final.x, NUMWORDS, order, NUMWORDS);

  if((NN_Cmp(w, r, NUMWORDS)) == 0) {
    return 1;
  } else {
    return 2;
  }
}

/**
 * @}
 */ 
