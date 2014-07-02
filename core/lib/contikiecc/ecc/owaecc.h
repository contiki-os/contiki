/**
 * \addtogroup ecc
 *
 * @{
 */

/**
 * \file
 * 			Interface to functions
 * \author
 * 			Andre Naz <andre.naz@ensimag.imag.fr>
 *
 */

#ifndef __OWAECC_H__
#define __OWAECC_H__

#include "contikiecc/ecc/ecc.h"

/**
 * \brief             Add a public key to the accumlator.
 * \param acc         Accumulator to which add pb_key.
 * \param pb_key      The public key to add.
 * \sa  ecc_init()
 *
 */
void
  owaecc_add_key(point_t * acc, point_t * pb_key);

/**
 * \brief             Check a key.
 * \param pb_key      The public key that is used to verify the 
 *                    to verify.
 * \param partial_acc Partial accumulator (witness in our case).
 * \param full_acc    Complete accumulator to compare to the result of 
 *                    (pb_key, partial_acc) computations.
 * \return            1 if the key is verified. 
 * \sa  ecc_init()
 */
uint8_t
owaecc_check_key(point_t * pb_key, point_t * partial_acc, point_t * full_acc);

#endif

/** @} */
