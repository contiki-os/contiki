/*
 * Original file:
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Authors: Andreas Dröscher <contiki@anticat.ch>
 *          Hu Luo
 *          Hossein Shafagh <shafagh@inf.ethz.ch>
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
 * \defgroup cc2538-bignum cc2538 BigNum math function driver
 *
 * Driver for the cc2538 BigNum math functions of the PKC engine
 * @{
 *
 * \file
 * Header file for the cc2538 BigNum driver
 *
 * bignum_subtract_start  bignum_subtract_get_result (subtraction)
 * bignum_add_start       bignum_add_get_result      (addition)
 * bignum_mod_start       bignum_mod_get_result      (modulo)
 * bignum_exp_mod_start   bignum_exp_mod_get_result  (modular exponentiation operation)
 * bignum_inv_mod_start   bignum_inv_mod_get_result  (inverse modulo operation)
 * bignum_mul_start       bignum_mul_get_result      (multiplication)
 * bignum_divide_start    bignum_divide_get_result   (division)
 * bignum_cmp_start       bignum_cmp_get_result      (comparison)
 */
#ifndef BIGNUM_DRIVER_H_
#define BIGNUM_DRIVER_H_

#include "contiki.h"
#include "pka.h"

#include <stdint.h>

/** @} */
/*---------------------------------------------------------------------------*/
/** \brief Starts the big number modulus operation.
 *
 * \param number Pointer to the big number on which modulo operation
 *        needs to be carried out.
 * \param number_size Size of the big number \sa number in 32-bit word.
 * \param modulus Pointer to the divisor.
 * \param modulus_size Size of the divisor \sa modulus.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the modulo operation on the big num \sa number
 * using the divisor \sa modulus.  The PKA RAM location where the result
 * will be available is stored in \sa result_vector.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_mod_start(const uint32_t *number,
                         const uint8_t number_size,
                         const uint32_t *modulus,
                         const uint8_t modulus_size,
                         uint32_t *result_vector,
                         struct process *process);

/** \brief Gets the result of the big number modulus operation.
 *
 * \param buffer Pointer to buffer where the result needs to
 *        be stored.
 * \param buffer_size Size of the provided buffer in 32 bit size word.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKABigNumModStart().
 *
 * This function gets the result of the big number modulus operation which was
 * previously started using the function \sa PKABigNumModStart().
 *
 * \retval PKA_STATUS_SUCCESS if successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_BUF_UNDERFLOW if the \e size is less than the length
 *         of the result.
 */
uint8_t bignum_mod_get_result(uint32_t *buffer,
                              const uint8_t buffer_size,
                              const uint32_t result_vector);

/** \brief Starts the comparison of two big numbers.
 *
 * \param number1 Pointer to the first big number.
 * \param number2 Pointer to the second big number.
 * \param size Size of the big number in 32 bit size word.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the comparison of two big numbers pointed by
 * \e number1 and \e number2.
 * Note this function expects the size of the two big numbers equal.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_cmp_start(const uint32_t *number1,
                         const uint32_t *number2,
                         uint8_t size,
                         struct process *process);

/** \brief Gets the result of the comparison operation of two big numbers.
 *
 * This function provides the results of the comparison of two big numbers
 * which was started using the \sa PKABigNumCmpStart().
 *
 * \retval PKA_STATUS_OPERATION_INPRG if the operation is in progress.
 * \retval PKA_STATUS_SUCCESS if the two big numbers are equal.
 * \retval PKA_STATUS_A_GR_B if the first number is greater than the second.
 * \retval PKA_STATUS_A_LT_B if the first number is less than the second.
 */
uint8_t bignum_cmp_get_result(void);

/** \brief Starts the big number inverse modulo operation.
 *
 * \param number Pointer to the buffer containing the big number
 *        (dividend).
 * \param number_size Size of the \e number in 32 bit word.
 * \param modulus Pointer to the buffer containing the modulus.
 * \param modulus_size Size of the modulus in 32 bit word.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the the inverse modulo operation on \e number
 * using the divisor \e modulus.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_inv_mod_start(const uint32_t *number,
                             const uint8_t number_size,
                             const uint32_t *modulus,
                             const uint8_t modulus_size,
                             uint32_t *result_vector,
                             struct process *process);

/** \brief Gets the result of the big number inverse modulo operation.
 *
 * \param buffer Pointer to buffer where the result needs to be
 *        stored.
 * \param buffer_size Size of the provided buffer in 32 bit size
 *        word.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKABigNumInvModStart().
 *
 * This function gets the result of the big number inverse modulo operation
 * previously started using the function \sa PKABigNumInvModStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_BUF_UNDERFLOW if the length of the provided buffer is less
 *         then the result.
 */
uint8_t bignum_inv_mod_get_result(uint32_t *buffer,
                                  const uint8_t buffer_size,
                                  const uint32_t result_vector);

/** \brief Starts the big number multiplication.
 *
 * \param multiplicand Pointer to the buffer containing the big
 *        number multiplicand.
 * \param multiplicand_size Size of the multiplicand in 32-bit word.
 * \param multiplier Pointer to the buffer containing the big
 *        number multiplier.
 * \param multiplier_size Size of the multiplier in 32-bit word.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the multiplication of the two big numbers.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_mul_start(const uint32_t *multiplicand,
                         const uint8_t multiplicand_size,
                         const uint32_t *multiplier,
                         const uint8_t multiplier_size,
                         uint32_t *result_vector,
                         struct process *process);

/** \brief Gets the results of the big number multiplication.
 *
 * \param buffer Pointer to buffer where the result needs to be stored.
 * \param buffer_size Address of the variable containing the length of the
 *        buffer. After the operation, the actual length of the resultant is
 *        stored at this address.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKABigNumMultiplyStart().
 *
 * This function gets the result of the multiplication of two big numbers
 * operation previously started using the function \sa
 * PKABigNumMultiplyStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_FAILURE if the operation is not successful.
 * \retval PKA_STATUS_BUF_UNDERFLOW if the length of the provided buffer is less
 *         then the length of the result.
 */
uint8_t bignum_mul_get_result(uint32_t *buffer,
                              uint32_t *buffer_size,
                              const uint32_t result_vector);

/** \brief Starts the addition of two big number.
 *
 * \param number1 Pointer to the buffer containing the first big mumber.
 * \param number1_size Size of the first big number in 32-bit word.
 * \param number2 Pointer to the buffer containing the second big number.
 * \param number2_size Size of the second big number in 32-bit word.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the addition of the two big numbers.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_add_start(const uint32_t *number1,
                         const uint8_t number1_size,
                         const uint32_t *number2,
                         const uint8_t number2_size,
                         uint32_t *result_vector,
                         struct process *process);

/** \brief Gets the result of the addition operation on two big number.
 *
 * \param buffer Pointer to buffer where the result
 *        needs to be stored.
 * \param buffer_size Address of the variable containing the length of
 *        the buffer. After the operation the actual length of the
 *        resultant is stored at this address.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKABigNumAddStart().
 *
 * This function gets the result of the addition operation on two big numbers,
 * previously started using the function \sa PKABigNumAddStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_FAILURE if the operation is not successful.
 * \retval PKA_STATUS_BUF_UNDERFLOW if the length of the provided buffer is less
 *         then the length of the result.
 */
uint8_t bignum_add_get_result(uint32_t *buffer,
                              uint32_t *buffer_size,
                              const uint32_t result_vector);

/** \brief Starts the substract of two big number.
 *
 * \param number1 Pointer to the buffer containing the first big mumber.
 * \param number1_size Size of the first big number in 32-bit word.
 * \param number2 Pointer to the buffer containing the second big number.
 * \param number2_size Size of the second big number in 32-bit word.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the substraction of the two big numbers.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_subtract_start(const uint32_t *number1,
                              const uint8_t number1_size,
                              const uint32_t *number2,
                              const uint8_t number2_size,
                              uint32_t *result_vector,
                              struct process *process);

/** \brief Gets the result of big number subtract.
 *
 * \param buffer Pointer to store the result of subtraction.
 * \param buffer_size Address of the variable containing the length of the
 *        buffer. After the operation, the actual length of the resultant is
 *        stored at this address.
 * \param result_vector Address of the result location which
 *        was provided by the start function PKABigNumSubtractStart().
 *
 * This function gets the result of PKABigNumSubtractStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_FAILURE if the operation is not successful.
 */
uint8_t bignum_subtract_get_result(uint32_t *buffer,
                                   uint32_t *buffer_size,
                                   const uint32_t result_vector);

/** \brief Starts the big number moduluar Exponentiation operation.
 *
 * \param number Pointer to the Exponent on which moduluar Exponentiation operation
 *        needs to be carried out.
 * \param number_size Size of the the Exponent number number in 32-bit word.
 * \param modulus Pointer to the divisor.
 * \param modulus_size Size of the divisor modulus.
 * \param base Pointer to the Base.
 * \param base_size Size of the divisor base.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the moduluar Exponentiation operation on the base num base
 * using the Exponent number and the Modulus num modulus.  The PKA RAM location where the result
 * will be available is stored in \sa result_vector.
 * IMPORTANT = Modulus and Based should have buffers of the same length!
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_exp_mod_start(const uint32_t *number,
                             const uint8_t number_size,
                             const uint32_t *modulus,
                             const uint8_t modulus_size,
                             const uint32_t *base,
                             const uint8_t base_size,
                             uint32_t *result_vector,
                             struct process *process);

/** \brief Gets the result of the big number modulus operation result.
 *
 * \param buffer Pointer to buffer where the result needs to
 *        be stored.
 * \param buffer_size Size of the provided buffer in 32 bit size word.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKABigNumExpModStart().
 *
 * This function gets the result of the big number modulus operation which was
 * previously started using the function \sa PKABigNumExpModStart().
 *
 * \retval PKA_STATUS_SUCCESS if successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_BUF_UNDERFLOW if the \e size is less than the length
 *         of the result.
 *
 * \note
 * - 0 < number_size <= Max_Len
 * - 1 < modulus_size <=Max_Len
 * - modulus must be odd and modulus > 232
 * - base < modulus
 */
uint8_t bignum_exp_mod_get_result(uint32_t *buffer,
                                  const uint8_t buffer_size,
                                  const uint32_t result_vector);

/** \brief Starts the big number Divide.
 *
 * \param dividend Pointer to the buffer containing the big
 *        number dividend.
 * \param dividend_size Size of the dividend in 32-bit word.
 * \param divisor Pointer to the buffer containing the big
 *        number divisor.
 * \param divisor_size Size of the divisor in 32-bit word.
 * \param result_vector Pointer to the result vector location
 *        which will be set by this function.
 * \param process Process to be polled upon completion of the
 *        operation, or \c NULL
 *
 * This function starts the divide of the two big numbers.
 *
 * \retval PKA_STATUS_SUCCESS if successful in starting the operation.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy doing
 *         some other operation.
 */
uint8_t bignum_divide_start(const uint32_t *dividend,
                            const uint8_t dividend_size,
                            const uint32_t *divisor,
                            const uint8_t divisor_size,
                            uint32_t *result_vector,
                            struct process *process);

/** \brief Gets the results of the big number Divide.
 *
 * \param buffer Pointer to buffer where the result needs to be stored.
 * \param buffer_size Address of the variable containing the length of the
 *        buffer.  After the operation, the actual length of the resultant is
 *        stored at this address.
 * \param result_vector Address of the result location which
 *        was provided by the start function \sa PKABigNumMultiplyStart().
 *
 * This function gets the result of the Divide of two big numbers
 * operation previously started using the function \sa
 * PKABigNumDivideStart().
 *
 * \retval PKA_STATUS_SUCCESS if the operation is successful.
 * \retval PKA_STATUS_OPERATION_INPRG if the PKA hw module is busy performing
 *         the operation.
 * \retval PKA_STATUS_RESULT_0 if the result is all zeroes.
 * \retval PKA_STATUS_FAILURE if the operation is not successful.
 * \retval PKA_STATUS_BUF_UNDERFLOW if the length of the provided buffer is less
 *         then the length of the result.
 */
uint8_t bignum_divide_get_result(uint32_t *buffer,
                                 uint32_t *buffer_size,
                                 const uint32_t result_vector);

/** @} */

#endif /* BIGNUM_DRIVER_H_ */

/**
 * @}
 * @}
 */
