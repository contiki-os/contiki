/**
 * \defgroup ecc Elliptic Curve Point Arithmetic
 *
 * @{
 */

/**
 * \file
 * Header file for the Elliptic Curve point arithmetic functions.
 * \author
 * Kasun Hewage <kasun.ch@gmail.com>, port to Contiki
 *
 */


#ifndef __ECC_H__
#define __ECC_H__

#include <lib/random.h>
#include "contikiecc/ecc/nn.h"

/**
 * The size of sliding window, must be power of 2 (change this if you
 * want to use other window size, for example: 2 or 4)
 */
#ifdef CONF_W_BITS
#define W_BITS CONF_W_BITS
#else
#define W_BITS 4
#endif

/**
 * Basic mask used to generate mask array (you need to change this if
 * you want to change window size)
 * For example: if W_BITS is 2, BASIC_MASK must be 0x03;
 *              if W_BITS is 4, BASIC_MASK must be 0x0f
 *              if W_BITS is 8, BASIC_MASK must be 0xff
 */
//#define BASIC_MASK 0x0f
#define BASIC_MASK ((1 << W_BITS) - 1)

/**
 * Number of windows in one digit, NUM_MASKS = NN_DIGIT_BITS/W_BITS
 */
#define NUM_MASKS (NN_DIGIT_BITS/W_BITS)

/**
 * Number of points for precomputed points, NN_POINTS = 2^W_BITS - 1
 */
#define NUM_POINTS ((1 << W_BITS) - 1)

/**
 * The data structure define the elliptic curve.
 */
typedef struct ecurve {
    /** curve's coefficients */
    NN_DIGIT a[NUMWORDS];
    NN_DIGIT b[NUMWORDS];

    /** whether a is -3 */
    char a_minus3;

    /** whether a is zero */
    char a_zero;

} ecurve_t;

/**
 * The data structure that defines a point of the elliptic curve.
 */ 
typedef struct point {
    /** point's X coordinate. */
    NN_DIGIT x[NUMWORDS];
    /** point's X coordinate. */
    NN_DIGIT y[NUMWORDS];
} point_t;

/**
 * All the parameters needed for elliptic curve operation.
 */
typedef struct curve_params {
    /** prime modulus */
    NN_DIGIT p[NUMWORDS];

    /** Omega, p = 2^m -omega */
    NN_DIGIT omega[NUMWORDS];

    /** curve over which ECC will be performed. */
    ecurve_t E;

    /** The base point, a point on E of order r */
    point_t G;

    /** A positive, prime integer dividing the number of points on E */
    NN_DIGIT r[NUMWORDS];

    // a positive prime integer, s.t. k = #E/r
//    NN_DIGIT k[NUMWORDS];
} curve_params_t;

/**
 * \brief             Initialize parameters and basepoint array for 
 *                    sliding window method. This function should be called first
 *                    before using other functions.
 */ 
void ecc_init();

/**
 * \brief             Provide order of curve for the modules which need to know
 */
void ecc_get_order(NN_DIGIT * order);
    
/**
 * \brief             Point addition, P0 = P1 + P2
 */
void ecc_add(point_t * P0, point_t * P1, point_t * P2);

/**
 * \brief             Point addition, (P0,Z0) = (P1,Z1) + (P2,Z2) 
 *                    using projective coordinates system.
 *                    P0, P1, P2 can be same pointer.
 */
void ecc_add_proj(point_t * P0, NN_DIGIT *Z0, point_t * P1, NN_DIGIT * Z1, point_t * P2, NN_DIGIT * Z2);

/**
 * \brief             Point doubleing, (P0,Z0) = 2*(P1,Z1) 
 *                    using projective coordinates system. 
 *                    P0 and P1 can be same pointer
 */
void ecc_dbl_proj(point_t * P0, NN_DIGIT *Z0, point_t * P1, NN_DIGIT * Z1);
    
/**
 * \brief             Scalar point multiplication P0 = n * P1
 *                    P0 and P1 can not be same pointer
 */
void ecc_mul(point_t * P0, point_t * P1, NN_DIGIT * n);
    
/**
 * \brief             Precompute the points for sliding window method
 */ 
void ecc_win_precompute(point_t * baseP, point_t * pointArray);

/**    
 * \brief             Scalr point multiplication using slide window method
 *                    P0 = n * Point, this Point may not be the base point of curve
 *                    pointArray is constructed by call win_precompute(Point, pointArray)
 */
void ecc_win_mul(point_t * P0, NN_DIGIT * n, point_t * pointArray);

/**
 * \brief             m repeated point doublings (Algorithm 3.23 in "Guide to ECC")
 */ 
void ecc_m_dbl_projective(point_t * P0, NN_DIGIT *Z0, uint8_t m);

/**
 * \brief             Scalar point multiplication on basepoint, P0 = n * basepoint
 *                    using window method.
 *                    pointArray is array of basepoint, 
 *                    pointArray[0] = basepoint, pointArray[1] = 2*basepoint ...
 */
void ecc_win_mul_base(point_t * P0, NN_DIGIT * n);

/**
  * \brief            Generate a private key at address prvKey of IKE_DH_SCALAR_CONTIKIECC_LEN bytes length (192 bits key length)
  */
void ecc_gen_private_key(NN_DIGIT * prvKey);

void ecc_gen_public_key(point_t *PublicKey, NN_DIGIT *PrivateKey);

/**
 * \brief             Get base point
 */
point_t * ecc_get_base_p();

/**
 * \brief             Get the parameters of specific curve.
 */
void get_curve_param(curve_params_t *para);

/*---------------------------------------------------------------------------*/
/**
 * \brief             P0 = P1
 */
void p_copy(point_t * P0, point_t * P1);

/*---------------------------------------------------------------------------*/
/**
 * \brief             Test whether points P1 and P2 are equal
 */
int p_equal(point_t * P1, point_t * P2);

#endif /* __ECC_H__ */

/** @} */
