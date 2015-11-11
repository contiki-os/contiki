/*
 * Original file:
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2014 Andreas Dröscher <contiki@anticat.ch>
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
 */
/**
 * \addtogroup cc2538-pka
 * @{
 *
 * \defgroup cc2538-ecc cc2538 ECC driver
 *
 * Driver for the cc2538 ECC mode of the PKC engine
 * @{
 *
 * \file
 * Header file for the cc2538 ECC driver
 */
#ifndef ECC_DRIVER_H_
#define ECC_DRIVER_H_

#include "contiki.h"
#include "pka.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name ECC structures
 * @{
 */
typedef struct {
  const char     *name;     /**< Name of the curve. */
  const uint8_t  size;      /**< Size of the curve in 32-bit word. */
  const uint32_t *prime;    /**< The prime that defines the field of the curve. */
  const uint32_t *n;        /**< Order of the curve. */
  const uint32_t *a;        /**< Co-efficient a of the equation. */
  const uint32_t *b;        /**< co-efficient b of the equation. */
  const uint32_t *x;        /**< x co-ordinate value of the generator point. */
  const uint32_t *y;        /**< y co-ordinate value of the generator point. */
} ecc_curve_info_t;

typedef struct {
  uint32_t       x[12];     /**< Pointer to value of the x co-ordinate. */
  uint32_t       y[12];     /**< Pointer to value of the y co-ordinate. */
} ec_point_t;

/** @} */
/*---------------------------------------------------------------------------*/
/** \name ECC functions
 *  \note Not all sequencer functions are implemented in this driver
 *        look at the CC2538 manual for a complete list.
 * @{
 */

/** \brief Starts ECC Multiplication.
 *
 * \param scalar Pointer to the buffer containing the scalar
 *        value to be multiplied.
 * \param ec_point Pointer to the structure containing the
 *        elliptic curve point to be multiplied.  The point should be
 *        on the given curve.
 * \param curve Pointer to the structure containing the curve
 *        info.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the Elliptical curve cryptography (ECC) point
 * multiplication operation on the EC point and the scalar value.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t ecc_mul_start(uint32_t *scalar,
                      ec_point_t *ec_point,
                      ecc_curve_info_t *curve,
                      uint32_t *result_vector,
                      struct process *process);

/** \brief Gets the result of ECC Multiplication
 *
 * \param ec_point Pointer to the structure where the resultant EC
 *        point will be stored. The callee is responsible to allocate the
 *        space for the ec point structure and the x and y co-ordinate as well.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKAECCMultiplyStart().
 *
 * This function gets the result of ecc point multiplication operation on the
 * ec point and the scalar value, previously started using the function
 * \sa PKAECCMultiplyStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_FAILURE if the operation is not successful.
 */
uint8_t ecc_mul_get_result(ec_point_t *ec_point,
                           uint32_t result_vector);

/** \brief Starts the ECC Multiplication with Generator point.
 *
 * \param scalar Pointer to the buffer containing the
 *        scalar value.
 * \param curve Pointer to the structure containing the curve
 *        info.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the ecc point multiplication operation of the
 * scalar value with the well known generator point of the given curve.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t ecc_mul_gen_pt_start(uint32_t *scalar,
                             ecc_curve_info_t *curve,
                             uint32_t *result_vector,
                             struct process *process);

/** \brief Gets the result of ECC Multiplication with Generator point.
 *
 * \param ec_point Pointer to the structure where the resultant EC
 *        point will be stored. The callee is responsible to allocate the
 *        space for the ec point structure and the x and y co-ordinate as well.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKAECCMultGenPtStart().
 *
 * This function gets the result of ecc point multiplication operation on the
 * scalar point and the known generator point on the curve, previously started
 * using the function \sa PKAECCMultGenPtStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_FAILURE if the operation is not successful.
 */
uint8_t ecc_mul_gen_pt_get_result(ec_point_t *ec_point,
                                  uint32_t result_vector);

/** \brief Starts the ECC Addition.
 *
 * \param ec_point1 Pointer to the structure containing the first
 *        ecc point.
 * \param ec_point2 Pointer to the structure containing the
 *        second ecc point.
 * \param curve Pointer to the structure containing the curve
 *        info.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the ecc point addition operation on the
 * two given ec points and generates the resultant ecc point.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t ecc_add_start(ec_point_t *ec_point1, ec_point_t *ec_point2,
                      ecc_curve_info_t *curve,
                      uint32_t *result_vector,
                      struct process *process);

/** \brief Gets the result of the ECC Addition
 *
 * \param ptOutEcPt Pointer to the structure where the resultant
 *        point will be stored. The callee is responsible to allocate memory,
 *        for the ec point structure including the memory for x and y
 *        co-ordinate values.
 * \param ui32ResultLoc Address of the result location which
 *        was provided by the function \sa PKAECCAddStart().
 *
 * This function gets the result of ecc point addition operation on the
 * on the two given ec points, previously started using the function \sa
 * PKAECCAddStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_FAILURE if the operation is not successful.
 */
uint8_t ecc_add_get_result(ec_point_t *ptOutEcPt, uint32_t ui32ResultLoc);

/** @} */

#endif /* ECC_DRIVER_H_ */

/**
 * @}
 * @}
 */
