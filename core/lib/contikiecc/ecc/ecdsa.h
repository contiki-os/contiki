/**
 * \defgroup ecdsa Elliptic Curve Digital Signature Algorithm 
 *
 * @{
 */

/**
 * \file
 * Header file for the Elliptic Curve Digital Signature Algorithm functions.
 * \author
 * Kasun Hewage <kasun.ch@gmail.com>
 *
 */

#ifndef __EDSA_H__
#define __EDSA_H__

#include "contikiecc/ecc/nn.h"
#include "contikiecc/ecc/ecc.h"

/**
 * \brief             Initialize the ECDSA using the public key that is to be 
 *                    used to verify the signature.
 * 
 * \param pb_key       A pointer to the public key. 
 *                    This public key may not be generated from the private 
 *                    key whose key is used to verify the signature.  
 */
void ecdsa_init(point_t * pb_key);

/**
 * \brief             Sign a message using the private key.
 * 
 * \param msg         A pointer to the message to be signed
 * \param len         Length of the message.
 * \param r
 * \param s           Signature of the message.
 * \param pr_key      The private key that is used to sign the message.
 */
void ecdsa_sign(uint8_t *msg, uint16_t len, NN_DIGIT *r, NN_DIGIT *s, NN_DIGIT * pr_key);

/**
 * \brief             Verify a message using public key.
 * \param msg         A pointer to the message to be verified.
 * \param len         Length of the message.
 * \param r
 * \param s           Signature of the message.
 * \param pb_key      The public key that is used to verify the signature. 
 *                    EDSA should be initialized by using this public key prior
 *                    to verify.
 * \return            1 if the signature is verified. 
 * \sa  ecdsa_init
 */ 
uint8_t ecdsa_verify(uint8_t *msg, uint16_t len, NN_DIGIT *r, NN_DIGIT *s, point_t * pb_key);


#endif /* __EDSA_H__ */

/** @} */
