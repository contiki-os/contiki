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
 * \addtogroup cc2538-ecc
 * @{
 *
 * \file
 * Implementation of the cc2538 ECC driver
 */
#include "dev/ecc-driver.h"
#include "reg.h"
#include "dev/nvic.h"

#define ASSERT(IF) if(!(IF)) { return PKA_STATUS_INVALID_PARAM; }

/*---------------------------------------------------------------------------*/
uint8_t
ecc_mul_start(uint32_t *scalar, ec_point_t *ec_point,
              ecc_curve_info_t *curve, uint32_t *result_vector,
              struct process *process)
{

  uint8_t extraBuf;
  uint32_t offset;
  int i;

  /* Check for the arguments. */
  ASSERT(NULL != scalar);
  ASSERT(NULL != ec_point);
  ASSERT(NULL != ec_point->x);
  ASSERT(NULL != ec_point->y);
  ASSERT(NULL != curve);
  ASSERT(curve->size <= PKA_MAX_CURVE_SIZE);
  ASSERT(NULL != result_vector);

  offset = 0;

  /* Make sure no PKA operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Calculate the extra buffer requirement. */
  extraBuf = 2 + curve->size % 2;

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the scalar will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the scalar in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = *scalar++;
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + (curve->size % 2));

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the curve parameters will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Write curve parameter 'p' as 1st part of vector B immediately
   * following vector A at PKA RAM */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->prime[i];
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + extraBuf);

  /* Copy curve parameter 'a' in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->a[i];
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + extraBuf);

  /* Copy curve parameter 'b' in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->b[i];
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + extraBuf);

  /* Update the C ptr with the offset address of the PKA RAM location
   * where the x, y will be stored. */
  REG(PKA_CPTR) = offset >> 2;

  /* Write elliptic curve point.x co-ordinate value. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = ec_point->x[i];
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + extraBuf);

  /* Write elliptic curve point.y co-ordinate value. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = ec_point->y[i];
  }

  /* Determine the offset for the next data. */
  offset += 4 * (i + extraBuf);

  /* Update the result location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load D ptr with the result location in PKA RAM. */
  REG(PKA_DPTR) = offset >> 2;

  /* Load length registers. */
  REG(PKA_ALENGTH) = curve->size;
  REG(PKA_BLENGTH) = curve->size;

  /* set the PKA function to ECC-MULT and start the operation. */
  REG(PKA_FUNCTION) = 0x0000D000;

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    NVIC_ClearPendingIRQ(PKA_IRQn);
    NVIC_EnableIRQ(PKA_IRQn);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
ecc_mul_get_result(ec_point_t *ec_point,
                   uint32_t result_vector)
{
  int i;
  uint32_t addr;
  uint32_t regMSWVal;
  uint32_t len;

  /* Check for the arguments. */
  ASSERT(NULL != ec_point);
  ASSERT(NULL != ec_point->x);
  ASSERT(NULL != ec_point->y);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* Verify that the operation is completed. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  NVIC_DisableIRQ(PKA_IRQn);
  pka_register_process_notification(NULL);

  if(REG(PKA_SHIFT) == 0x00000000) {
    /* Get the MSW register value. */
    regMSWVal = REG(PKA_MSW);

    /* Check to make sure that the result vector is not all zeroes. */
    if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
      return PKA_STATUS_RESULT_0;
    }

    /* Get the length of the result */
    len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
        - ((result_vector - PKA_RAM_BASE) >> 2);

    addr = result_vector;

    /* copy the x co-ordinate value of the result from vector D into
     * the \e ec_point. */
    for(i = 0; i < len; i++) {
      ec_point->x[i] = REG(addr + 4 * i);
    }

    addr += 4 * (i + 2 + len % 2);

    /* copy the y co-ordinate value of the result from vector D into
     * the \e ec_point. */
    for(i = 0; i < len; i++) {
      ec_point->y[i] = REG(addr + 4 * i);
    }

    return PKA_STATUS_SUCCESS;
  } else {
    return PKA_STATUS_FAILURE;
  }
}
/*---------------------------------------------------------------------------*/
uint8_t
ecc_mul_gen_pt_start(uint32_t *scalar, ecc_curve_info_t *curve,
                     uint32_t *result_vector, struct process *process)
{
  uint8_t extraBuf;
  uint32_t offset;
  int i;

  /* check for the arguments. */
  ASSERT(NULL != scalar);
  ASSERT(NULL != curve);
  ASSERT(curve->size <= PKA_MAX_CURVE_SIZE);
  ASSERT(NULL != result_vector);

  offset = 0;

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Calculate the extra buffer requirement. */
  extraBuf = 2 + curve->size % 2;

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the scalar will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the scalar in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = *scalar++;
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + (curve->size % 2));

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the curve parameters will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Write curve parameter 'p' as 1st part of vector B. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->prime[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Write curve parameter 'a' in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->a[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* write curve parameter 'b' in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->b[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Update the C ptr with the offset address of the PKA RAM location
   * where the x, y will be stored. */
  REG(PKA_CPTR) = offset >> 2;

  /* Write x co-ordinate value of the Generator point in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->x[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Write y co-ordinate value of the Generator point in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->y[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Update the result location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load D ptr with the result location in PKA RAM. */
  REG(PKA_DPTR) = offset >> 2;

  /* Load length registers. */
  REG(PKA_ALENGTH) = curve->size;
  REG(PKA_BLENGTH) = curve->size;

  /* Set the PKA function to ECC-MULT and start the operation. */
  REG(PKA_FUNCTION) = 0x0000D000;

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    NVIC_ClearPendingIRQ(PKA_IRQn);
    NVIC_EnableIRQ(PKA_IRQn);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
ecc_mul_gen_pt_get_result(ec_point_t *ec_point,
                          uint32_t result_vector)
{

  int i;
  uint32_t regMSWVal;
  uint32_t addr;
  uint32_t len;

  /* Check for the arguments. */
  ASSERT(NULL != ec_point);
  ASSERT(NULL != ec_point->x);
  ASSERT(NULL != ec_point->y);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  /* Verify that the operation is completed. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  NVIC_DisableIRQ(PKA_IRQn);
  pka_register_process_notification(NULL);

  if(REG(PKA_SHIFT) == 0x00000000) {
    /* Get the MSW register value. */
    regMSWVal = REG(PKA_MSW);

    /* Check to make sure that the result vector is not all zeroes. */
    if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
      return PKA_STATUS_RESULT_0;
    }

    /* Get the length of the result. */
    len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
        - ((result_vector - PKA_RAM_BASE) >> 2);

    addr = result_vector;

    /* Copy the x co-ordinate value of the result from vector D into the
     * EC point. */
    for(i = 0; i < len; i++) {
      ec_point->x[i] = REG(addr + 4 * i);
    }

    addr += 4 * (i + 2 + len % 2);

    /* Copy the y co-ordinate value of the result from vector D into the
     * EC point. */
    for(i = 0; i < len; i++) {
      ec_point->y[i] = REG(addr + 4 * i);
    }

    return PKA_STATUS_SUCCESS;
  } else {
    return PKA_STATUS_FAILURE;
  }
}
/*---------------------------------------------------------------------------*/
uint8_t
ecc_add_start(ec_point_t *ec_point1, ec_point_t *ec_point2,
              ecc_curve_info_t *curve, uint32_t *result_vector,
              struct process *process)
{

  uint8_t extraBuf;
  uint32_t offset;
  int i;

  /* Check for the arguments. */
  ASSERT(NULL != ec_point1);
  ASSERT(NULL != ec_point1->x);
  ASSERT(NULL != ec_point1->y);
  ASSERT(NULL != ec_point2);
  ASSERT(NULL != ec_point2->x);
  ASSERT(NULL != ec_point2->y);
  ASSERT(NULL != curve);
  ASSERT(NULL != result_vector);

  offset = 0;

  /* Make sure no operation is in progress. */
  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Calculate the extra buffer requirement. */
  extraBuf = 2 + curve->size % 2;

  /* Update the A ptr with the offset address of the PKA RAM location
   * where the first ecPt will be stored. */
  REG(PKA_APTR) = offset >> 2;

  /* Load the x co-ordinate value of the first EC point in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = ec_point1->x[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Load the y co-ordinate value of the first EC point in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = ec_point1->y[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Update the B ptr with the offset address of the PKA RAM location
   * where the curve parameters will be stored. */
  REG(PKA_BPTR) = offset >> 2;

  /* Write curve parameter 'p' as 1st part of vector B */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->prime[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Write curve parameter 'a'. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = (uint32_t)curve->a[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Update the C ptr with the offset address of the PKA RAM location
   * where the ecPt2 will be stored. */
  REG(PKA_CPTR) = offset >> 2;

  /* Load the x co-ordinate value of the second EC point in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = ec_point2->x[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Load the y co-ordinate value of the second EC point in PKA RAM. */
  for(i = 0; i < curve->size; i++) {
    REG(PKA_RAM_BASE + offset + 4 * i) = ec_point2->y[i];
  }

  /* Determine the offset in PKA RAM for the next data. */
  offset += 4 * (i + extraBuf);

  /* Copy the result vector location. */
  *result_vector = PKA_RAM_BASE + offset;

  /* Load D ptr with the result location in PKA RAM. */
  REG(PKA_DPTR) = offset >> 2;

  /* Load length registers. */
  REG(PKA_BLENGTH) = curve->size;

  /* Set the PKA Function to ECC-ADD and start the operation. */
  REG(PKA_FUNCTION) = 0x0000B000;

  /* Enable Interrupt */
  if(process != NULL) {
    pka_register_process_notification(process);
    NVIC_ClearPendingIRQ(PKA_IRQn);
    NVIC_EnableIRQ(PKA_IRQn);
  }

  return PKA_STATUS_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
ecc_add_get_result(ec_point_t *ec_point, uint32_t result_vector)
{
  uint32_t regMSWVal;
  uint32_t addr;
  int i;
  uint32_t len;

  /* Check for the arguments. */
  ASSERT(NULL != ec_point);
  ASSERT(NULL != ec_point->x);
  ASSERT(NULL != ec_point->y);
  ASSERT(result_vector > PKA_RAM_BASE);
  ASSERT(result_vector < (PKA_RAM_BASE + PKA_RAM_SIZE));

  if((REG(PKA_FUNCTION) & PKA_FUNCTION_RUN) != 0) {
    return PKA_STATUS_OPERATION_INPRG;
  }

  /* Disable Interrupt */
  NVIC_DisableIRQ(PKA_IRQn);
  pka_register_process_notification(NULL);

  if(REG(PKA_SHIFT) == 0x00000000) {
    /* Get the MSW register value. */
    regMSWVal = REG(PKA_MSW);

    /* Check to make sure that the result vector is not all zeroes. */
    if(regMSWVal & PKA_MSW_RESULT_IS_ZERO) {
      return PKA_STATUS_RESULT_0;
    }

    /* Get the length of the result. */
    len = ((regMSWVal & PKA_MSW_MSW_ADDRESS_M) + 1)
        - ((result_vector - PKA_RAM_BASE) >> 2);

    addr = result_vector;

    /* Copy the x co-ordinate value of result from vector D into the
     * the output EC Point. */
    for(i = 0; i < len; i++) {
      ec_point->x[i] = REG(addr + 4 * i);
    }

    addr += 4 * (i + 2 + len % 2);

    /* Copy the y co-ordinate value of result from vector D into the
     * the output EC Point. */
    for(i = 0; i < len; i++) {
      ec_point->y[i] = REG(addr + 4 * i);
    }

    return PKA_STATUS_SUCCESS;
  } else {
    return PKA_STATUS_FAILURE;
  }
}
/** @} */
