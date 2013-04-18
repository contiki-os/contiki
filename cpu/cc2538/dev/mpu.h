/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-mpu cc2538 Memory Protection Unit
 *
 * Driver for the cc2538 Memory Protection Unit
 * @{
 *
 * \file
 * Header file for the ARM Memory Protection Unit
 */
#ifndef MPU_H_
#define MPU_H_

#define MPU_MPU_TYPE           0xE000ED90 /**< MPU Type */
#define MPU_MPU_CTRL           0xE000ED94 /**< MPU Control */
#define MPU_MPU_NUMBER         0xE000ED98 /**< MPU Region Number */
#define MPU_MPU_BASE           0xE000ED9C /**< MPU Region Base Address */
#define MPU_MPU_ATTR           0xE000EDA0 /**< MPU Region Attribute and Size */
#define MPU_MPU_BASE1          0xE000EDA4 /**< MPU Region Base Address Alias 1 */
#define MPU_MPU_ATTR1          0xE000EDA8 /**< MPU Region Attribute and Size Alias 1 */
#define MPU_MPU_BASE2          0xE000EDAC /**< MPU Region Base Address Alias 2 */
#define MPU_MPU_ATTR2          0xE000EDB0 /**< MPU Region Attribute and Size Alias 2*/
#define MPU_MPU_BASE3          0xE000EDB4 /**< MPU Region Base Address Alias 3 */
#define MPU_MPU_ATTR3          0xE000EDB8 /**< MPU Region Attribute and Size Alias 3*/
#define MPU_DBG_CTRL           0xE000EDF0 /**< Debug Control and Status Reg */
#define MPU_DBG_XFER           0xE000EDF4 /**< Debug Core Reg. Transfer Select */
#define MPU_DBG_DATA           0xE000EDF8 /**< Debug Core Register Data */
#define MPU_DBG_INT            0xE000EDFC /**< Debug Reset Interrupt Control */
#define MPU_SW_TRIG            0xE000EF00 /**< Software Trigger Interrupt */

#endif /* MPU_H_ */

/**
 * @}
 * @}
 */
