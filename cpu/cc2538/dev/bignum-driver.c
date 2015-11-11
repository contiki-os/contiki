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
 * \addtogroup cc2538-bignum
 * @{
 *
 * \file
 * Implementation of the cc2538 BigNum driver
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
#include "bignum-driver.h"

#include "stdio.h"

#include "reg.h"
#include "nvic.h"

#define ASSERT(IF) if(!(IF)) { return PKA_STATUS_INVALID_PARAM; }

/*---------------------------------------------------------------------------*/
uint8_t
bignum_mod_start(const uint32_t *number,
                 const uint8_t number_size,
                 const uint32_t *modulus,
                 const uint8_t modulus_size,
                 uint32_t *result_vector,
                 struct process *process)
{

  uint8_t extraBuf;
  uint32_t offset;
  int i;

  /* Check the arguments. */
  ASSERT(NULL != number);
  ASSERT(NULL != modulus);
  ASSERT(NULL != result_vector);

  /* make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* calculate the extra buffer requirement. */
  extraBuf = 2 + modulus_size % 2;

  offset = 0;

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the number will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the number in PKA RAM */
  for(i = 0; i < number_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number[i];
  }

  /* determine the offset for the next data input. */
  offset += 4 * (i + number_size % 2);

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the divisor will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the divisor in PKA RAM. */
  for(i = 0; i < modulus_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = modulus[i];
  }

  /* determine the offset for the next data. */
  offset += 4 * (i + extraBuf);

  /* Copy the result vector address location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load C ptr with the result location in PKA RAM */
  REG(PKA_CPTR) = offset >> 2;

  /* Load A length registers with Big number length in 32 bit words. */
  REG(PKA_ALENGTH) = number_size;

  /* Load B length registers  Divisor length in 32-bit words. */
  REG(PKA_BLENGTH) = modulus_size;

  /* Start the PKCP modulo operation by setting the PKA Function register. */
  REG(PKA_FUNCTION) = (PKA_FUNCTION_RUN | PKA_FUNCTION_MODULO);

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_mod_get_result(uint32_t *buffer,
                      const uint8_t buffer_size,
                      const uint32_t result_vector)
{

  uint32_t regMSWVal;
  uint32_t len;
  int i;

  /* Check the arguments. */
  ASSERT(NULL != buffer);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Get the MSW register value. */
  regMSWVal = REG(PKA_DIVMSW);

  /* Check to make sure that the result vector is not all zeroes. */
  if(regMSWVal & PKA_DIVMSW_RESULT_IS_ZERO) {
    return PKA_STATUS_RESULT_0;
  }

  /* Get the length of the result. */
  len = ((regMSWVal & PKA_DIVMSW_MSW_ADDRESS_M) + 1)
      - ((result_vector - PKA_RAM_BASE) >> 2);

  /* If the size of the buffer provided is less than the result length than
   * return error. */
  if(buffer_size < len) {
    return PKA_STATUS_BUF_UNDERFLOW;
  }
  /* copy the result from vector C into the pResult. */
  for(i = 0; i < len; i++) {
    buffer[i] = REG(result_vector + 4 * i);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_cmp_start(const uint32_t *number1,
                 const uint32_t *number2,
                 const uint8_t size,
                 struct process *process)
{

  uint32_t offset;
  int i;

  /* Check the arguments. */
  ASSERT(NULL != number1);
  ASSERT(NULL != number2);

  offset = 0;

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the first big number will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the first big number in PKA RAM. */
  for(i = 0; i < size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number1[i];
  }

  /* Determine the offset in PKA RAM for the next pointer. */
  offset += 4 * (i + size % 2);

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the second big number will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the second big number in PKA RAM. */
  for(i = 0; i < size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number2[i];
  }

  /* Load length registers in 32 bit word size. */
  REG(PKA_ALENGTH) = size;

  /* Set the PKA Function register for the compare operation
   * and start the operation. */
  REG(PKA_FUNCTION) = (PKA_FUNCTION_RUN | PKA_FUNCTION_COMPARE);

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_cmp_get_result(void)
{
  uint8_t status;

  /* verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    status = PKA_STATUS_OPERATION_INPRG;
    return status;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Check the compare register. */
  switch(REG(PKA_COMPARE)) {
  case PKA_COMPARE_A_EQUALS_B:
    status = PKA_STATUS_SUCCESS;
    break;

  case PKA_COMPARE_A_GREATER_THAN_B:
    status = PKA_STATUS_A_GR_B;
    break;

  case PKA_COMPARE_A_LESS_THAN_B:
    status = PKA_STATUS_A_LT_B;
    break;

  default:
    status = PKA_STATUS_FAILURE;
    break;
  }

  return status;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_inv_mod_start(const uint32_t *number,
                     const uint8_t number_size,
                     const uint32_t *modulus,
                     const uint8_t modulus_size,
                     uint32_t *result_vector,
                     struct process *process)
{

  uint32_t offset;
  int i;

  /* Check the arguments. */
  ASSERT(NULL != number);
  ASSERT(NULL != modulus);
  ASSERT(NULL != result_vector);

  offset = 0;

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the number will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the \e number number in PKA RAM. */
  for(i = 0; i < number_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number[i];
  }

  /* Determine the offset for next data. */
  offset += 4 * (i + number_size % 2);

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the modulus will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the \e modulus divisor in PKA RAM. */
  for(i = 0; i < modulus_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = modulus[i];
  }

  /* Determine the offset for result data. */
  offset += 4 * (i + modulus_size % 2);

  /* Copy the result vector address location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load D ptr with the result location in PKA RAM. */
  REG(PKA_DPTR) = offset >> 2;

  /* Load the respective length registers. */
  REG(PKA_ALENGTH) = number_size;
  REG(PKA_BLENGTH) = modulus_size;

  /* set the PKA function to InvMod operation and the start the operation. */
  REG(PKA_FUNCTION) = 0x0000F000;

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_inv_mod_get_result(uint32_t *buffer,
                          const uint8_t buffer_size,
                          const uint32_t result_vector)
{

  uint32_t regMSWVal;
  uint32_t len;
  int i;

  /* Check the arguments. */
  ASSERT(NULL != buffer);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* Verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Get the MSW register value. */
  regMSWVal = REG(PKA_MSW);

  /* Check to make sure that the result vector is not all zeroes. */
  if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
    return PKA_STATUS_RESULT_0;
  }

  /* Get the length of the result */
  len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
      - ((result_vector - PKA_RAM_BASE) >> 2);

  /* Check if the provided buffer length is adequate to store the result
   * data. */
  if(buffer_size < len) {
    return PKA_STATUS_BUF_UNDERFLOW;
  }

  /* Copy the result from vector C into the \e buffer. */
  for(i = 0; i < len; i++) {
    buffer[i] = REG(result_vector + 4 * i);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_mul_start(const uint32_t *multiplicand,
                 const uint8_t multiplicand_size,
                 const uint32_t *multiplier,
                 const uint8_t multiplier_size,
                 uint32_t *result_vector,
                 struct process *process)
{

  uint32_t offset;
  int i;

  /* Check for the arguments. */
  ASSERT(NULL != multiplicand);
  ASSERT(NULL != multiplier);
  ASSERT(NULL != result_vector);

  offset = 0;

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the multiplicand will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the multiplicand in PKA RAM. */
  for(i = 0; i < multiplicand_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = *multiplicand;
    multiplicand++;
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + (multiplicand_size % 2));

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the multiplier will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the multiplier in PKA RAM. */
  for(i = 0; i < multiplier_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = *multiplier;
    multiplier++;
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + (multiplier_size % 2));

  /* Copy the result vector address location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load C ptr with the result location in PKA RAM. */
  REG(PKA_CPTR) = offset >> 2;

  /* Load the respective length registers. */
  REG(PKA_ALENGTH) = multiplicand_size;
  REG(PKA_BLENGTH) = multiplier_size;

  /* Set the PKA function to the multiplication and start it. */
  REG(PKA_FUNCTION) = (PKA_FUNCTION_RUN | PKA_FUNCTION_MULTIPLY);

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_mul_get_result(uint32_t *buffer,
                      uint32_t *buffer_size,
                      const uint32_t result_vector)
{

  uint32_t regMSWVal;
  uint32_t len;
  int i;

  /* Check for arguments. */
  ASSERT(NULL != buffer);
  ASSERT(NULL != buffer_size);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* Verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Get the MSW register value. */
  regMSWVal = REG(PKA_MSW);

  /* Check to make sure that the result vector is not all zeroes. */
  if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
    return PKA_STATUS_RESULT_0;
  }

  /* Get the length of the result. */
  len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
      - ((result_vector - PKA_RAM_BASE) >> 2);

  /* Make sure that the length of the supplied result buffer is adequate
   * to store the resultant. */
  if(*buffer_size < len) {
    return PKA_STATUS_BUF_UNDERFLOW;
  }

  /* Copy the resultant length. */
  *buffer_size = len;

  /* Copy the result from vector C into the pResult. */
  for(i = 0; i < *buffer_size; i++) {
    buffer[i] = REG(result_vector + 4 * i);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_add_start(const uint32_t *number1,
                 const uint8_t number1_size,
                 const uint32_t *number2,
                 const uint8_t number2_size,
                 uint32_t *result_vector,
                 struct process *process)
{

  uint32_t offset;
  int i;

  /* Check for arguments. */
  ASSERT(NULL != number1);
  ASSERT(NULL != number2);
  ASSERT(NULL != result_vector);

  offset = 0;

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the big number 1 will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the big number 1 in PKA RAM. */
  for(i = 0; i < number1_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number1[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + (number1_size % 2));

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the big number 2 will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the big number 2 in PKA RAM. */
  for(i = 0; i < number2_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number2[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + (number2_size % 2));

  /* Copy the result vector address location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load C ptr with the result location in PKA RAM. */
  REG(PKA_CPTR) = offset >> 2;

  /* Load respective length registers. */
  REG(PKA_ALENGTH) = number1_size;
  REG(PKA_BLENGTH) = number2_size;

  /* Set the function for the add operation and start the operation. */
  REG(PKA_FUNCTION) = (PKA_FUNCTION_RUN | PKA_FUNCTION_ADD);

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_add_get_result(uint32_t *buffer,
                      uint32_t *buffer_size,
                      const uint32_t result_vector)
{

  uint32_t regMSWVal;
  uint32_t len;
  int i;

  /* Check for the arguments. */
  ASSERT(NULL != buffer);
  ASSERT(NULL != buffer_size);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* Verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Get the MSW register value. */
  regMSWVal = REG(PKA_MSW);

  /* Check to make sure that the result vector is not all zeroes. */
  if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
    return PKA_STATUS_RESULT_0;
  }

  /* Get the length of the result. */
  len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
      - ((result_vector - PKA_RAM_BASE) >> 2);

  /* Make sure that the supplied result buffer is adequate to store the
   * resultant data. */
  if(*buffer_size < len) {
    return PKA_STATUS_BUF_UNDERFLOW;
  }

  /* Copy the length. */
  *buffer_size = len;

  /* Copy the result from vector C into the provided buffer. */
  for(i = 0; i < *buffer_size; i++) {
    buffer[i] = REG(result_vector + 4 * i);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/* below functions are added by hu luo */
uint8_t
bignum_subtract_start(const uint32_t *number1,
                      const uint8_t number1_size,
                      const uint32_t *number2,
                      const uint8_t number2_size,
                      uint32_t *result_vector,
                      struct process *process)
{

  uint32_t offset;
  int i;

  /* Check for arguments. */
  ASSERT(NULL != number1);
  ASSERT(NULL != number2);
  ASSERT(NULL != result_vector);

  offset = 0;

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the big number 1 will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the big number 1 in PKA RAM. */
  for(i = 0; i < number1_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number1[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + (number1_size % 2));

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the big number 2 will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the big number 2 in PKA RAM. */
  for(i = 0; i < number2_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number2[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + (number2_size % 2));

  /* Copy the result vector address location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load C ptr with the result location in PKA RAM. */
  REG(PKA_CPTR) = offset >> 2;

  /* Load respective length registers. */
  REG(PKA_ALENGTH) = number1_size;
  REG(PKA_BLENGTH) = number2_size;

  /* Set the function for the add operation and start the operation. */
  REG(PKA_FUNCTION) = (PKA_FUNCTION_RUN | PKA_FUNCTION_SUBTRACT);

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_subtract_get_result(uint32_t *buffer,
                           uint32_t *buffer_size,
                           const uint32_t result_vector)
{

  uint32_t regMSWVal;
  uint32_t len;
  int i;

  /* Check for the arguments. */
  ASSERT(NULL != buffer);
  ASSERT(NULL != buffer_size);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* Verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Get the MSW register value. */
  regMSWVal = REG(PKA_MSW);

  /* Check to make sure that the result vector is not all zeroes. */
  if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
    return PKA_STATUS_RESULT_0;
  }

  /* Get the length of the result. */
  len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
      - ((result_vector - PKA_RAM_BASE) >> 2);

  /* Make sure that the supplied result buffer is adequate to store the
   * resultant data. */
  if(*buffer_size < len) {
    return PKA_STATUS_BUF_UNDERFLOW;
  }

  /* Copy the length. */
  *buffer_size = len;

  /* Copy the result from vector C into the provided buffer. */
  for(i = 0; i < *buffer_size; i++) {
    buffer[i] = REG(result_vector + 4 * i);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_exp_mod_start(const uint32_t *number,
                     const uint8_t number_size,
                     const uint32_t *modulus,
                     const uint8_t modulus_size,
                     const uint32_t *base,
                     const uint8_t base_size,
                     uint32_t *result_vector,
                     struct process *process)
{
  uint32_t offset;
  int i;

  /* Check for the arguments. */
  ASSERT(NULL != number);
  ASSERT(NULL != modulus);
  ASSERT(NULL != base);
  ASSERT(NULL != result_vector);
  ASSERT(modulus != base);

  offset = 0;

  /* Make sure no PKA operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the exponent will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the Exponent in PKA RAM. */
  for(i = 0; i < number_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = number[i];
  }

  /* Determine the offset for the next data(BPTR). */
  offset += 4 * (i + number_size % 2);
  /* Update the B ptr with the offset address of the PKA RAM location
   * where the divisor will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the Modulus in PKA RAM. */
  for(i = 0; i < modulus_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = modulus[i];
  }

  /* Determine the offset for the next data(CPTR). */
  offset += 4 * (i + modulus_size % 2 + 2);
  /* Update the C ptr with the offset address of the PKA RAM location
   * where the Base will be stored. */
  REG(PKA_CPTR) = offset >> 2;

  /* Write Base to the Vector C in PKA RAM */

  for(i = 0; i < base_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = base[i];
  }

  /* Determine the offset for the next data.
   * INFO D and B can share the same memory area!
   * offset += 4 * (i + extraBuf + 2); */

  /* Copy the result vector address location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load D ptr with the result location in PKA RAM */
  REG(PKA_DPTR) = offset >> 2;

  /* Load A length registers with Big number length in 32 bit words. */
  REG(PKA_ALENGTH) = number_size;

  /* Load B length registers  Divisor length in 32-bit words. */
  REG(PKA_BLENGTH) = modulus_size;
  /* REG(PKA_SHIFT) = 0x00000001;
   * Required for (EXPMod-variable): 0x0000A000
   * Start the PKCP modulo exponentiation operation(EXPMod-ACT2)
   * by setting the PKA Function register. */
  REG(PKA_FUNCTION) = 0x0000C000;

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_exp_mod_get_result(uint32_t *buffer,
                          const uint8_t buffer_size,
                          const uint32_t result_vector)
{

  uint32_t regMSWVal;
  uint32_t len;
  int i;

  /* Check the arguments. */
  ASSERT(NULL != buffer);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Get the MSW register value. */
  regMSWVal = REG(PKA_MSW);

  /* Check to make sure that the result vector is not all zeroes. */
  if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
    return PKA_STATUS_RESULT_0;
  }

  /* Get the length of the result */
  len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
      - ((result_vector - PKA_RAM_BASE) >> 2);
  /* If the size of the buffer provided is less than the result length than
   * return error. */
  if(buffer_size < len) {
    return PKA_STATUS_BUF_UNDERFLOW;
  }

  /* copy the result from vector C into the pResult. */
  for(i = 0; i < len; i++) {
    buffer[i] = REG(result_vector + 4 * i);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_divide_start(const uint32_t *dividend,
                    const uint8_t dividend_size,
                    const uint32_t *divisor,
                    const uint8_t divisor_size,
                    uint32_t *result_vector,
                    struct process *process)
{

  uint32_t offset;
  uint32_t spacing;
  int i;

  /* We use largest len for spacing */
  if(dividend_size > divisor_size) {
    spacing = dividend_size;
  } else {
    spacing = divisor_size;
  }
  spacing += 2 + spacing % 2;

  /* Check for the arguments. */
  ASSERT(NULL != dividend);
  ASSERT(NULL != divisor);
  ASSERT(NULL != result_vector);

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the multiplicand will be stored. */
  offset = 0;
  REG(PKA_APTR) = offset >> 2;

  /* Load the multiplicand in PKA RAM. */
  for(i = 0; i < dividend_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = *dividend;
    dividend++;
  }

  /* Determine the offset for the next data. */
  offset += 4 * spacing;

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the multiplier will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Load the multiplier in PKA RAM. */
  for(i = 0; i < divisor_size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = *divisor;
    divisor++;
  }

  /* Determine the offset for the reminder. */
  offset += 4 * spacing;

  /* Load C ptr with the result location in PKA RAM. */
  REG(PKA_CPTR) = offset >> 2;

  /* Determine the offset for the quotient. */
  offset += 4 * spacing;

  /* Copy the quotient vector address location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load D ptr with the result location in PKA RAM. */
  REG(PKA_DPTR) = offset >> 2;

  /* Load the respective length registers. */
  REG(PKA_ALENGTH) = dividend_size;
  REG(PKA_BLENGTH) = divisor_size;

  /* Set the PKA function to the multiplication and start it. */
  REG(PKA_FUNCTION) = (PKA_FUNCTION_RUN | PKA_FUNCTION_DIVIDE);

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    nvic_interrupt_unpend(NVIC_INT_PKA);
    nvic_interrupt_enable(NVIC_INT_PKA);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
bignum_divide_get_result(uint32_t *buffer,
                         uint32_t *buffer_size,
                         const uint32_t result_vector)
{

  uint32_t regMSWVal;
  uint32_t len;
  int i;

  /* Check for arguments. */
  ASSERT(NULL != buffer);
  ASSERT(NULL != buffer_size);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* Verify that the operation is complete. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  nvic_interrupt_disable(NVIC_INT_PKA);
  pka_register_process_notification(NULL);

  /* Get the MSW register value. */
  regMSWVal = REG(PKA_MSW);

  /* Check to make sure that the result vector is not all zeroes. */
  if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
    return PKA_STATUS_RESULT_0;
  }

  /* Get the length of the result. */
  len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
      - ((result_vector - PKA_RAM_BASE) >> 2);

  /* Make sure that the length of the supplied result buffer is adequate
   * to store the resultant. */
  if(*buffer_size < len) {
    return PKA_STATUS_BUF_UNDERFLOW;
  }

  /* Copy the resultant length. */
  *buffer_size = len;

  /* Copy the result from vector C into the pResult. */
  for(i = 0; i < *buffer_size; i++) {
    buffer[i] = REG(result_vector + 4 * i);
  }

  return PKA_STATUS_SUCCESS;
}
/** @} */

