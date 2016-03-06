/*
 * Copyright (c) 2015, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
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
 * \defgroup cc2538-devices cc2538 family of devices
 *
 * Definitions for the cc2538 family of devices
 * @{
 *
 * \file
 * Header file for the cc2538 devices definitions
 */
#ifndef CC2538_DEV_H_
#define CC2538_DEV_H_

#include "contiki-conf.h"
#include "sys/cc.h"
/*----------------------------------------------------------------------------*/
/** \name Bit-fields for the CC2538 devices features
 * @{
 */
#define CC2538_DEV_ID_M                 0x0000000F /**< ID mask */
#define CC2538_DEV_ID_S                 0          /**< ID shift */
#define CC2538_DEV_FLASH_SIZE_KB_M      0x0000FFF0 /**< kiB flash size mask */
#define CC2538_DEV_FLASH_SIZE_KB_S      4          /**< kiB flash size shift */
#define CC2538_DEV_SRAM_SIZE_KB_M       0x00FF0000 /**< kiB SRAM size mask */
#define CC2538_DEV_SRAM_SIZE_KB_S       16         /**< kiB SRAM size shift */
#define CC2538_DEV_AES_SHA_M            0x01000000 /**< Security HW AES/SHA */
#define CC2538_DEV_ECC_RSA_M            0x02000000 /**< Security HW ECC/RSA */
/** @} */
/*----------------------------------------------------------------------------*/
/** \name Macro defining a CC2538 device from its features
 * @{
 */
#define CC2538_DEV_DEF(id, flash_size_kb, sram_size_kb, aes_sha, ecc_rsa) \
  ((id) << CC2538_DEV_ID_S | (flash_size_kb) << CC2538_DEV_FLASH_SIZE_KB_S | \
   (sram_size_kb) << CC2538_DEV_SRAM_SIZE_KB_S | \
   ((aes_sha) ? CC2538_DEV_AES_SHA_M : 0) | \
   ((ecc_rsa) ? CC2538_DEV_ECC_RSA_M : 0))
/** @} */
/*----------------------------------------------------------------------------*/
/** \name Available CC2538 devices
 * @{
 */
#define CC2538_DEV_CC2538SF53   CC2538_DEV_DEF(0, 512, 32, 1, 1)
#define CC2538_DEV_CC2538SF23   CC2538_DEV_DEF(1, 256, 32, 1, 1)
#define CC2538_DEV_CC2538NF53   CC2538_DEV_DEF(2, 512, 32, 1, 0)
#define CC2538_DEV_CC2538NF23   CC2538_DEV_DEF(3, 256, 32, 1, 0)
#define CC2538_DEV_CC2538NF11   CC2538_DEV_DEF(4, 128, 16, 1, 0)
/** @} */
/*----------------------------------------------------------------------------*/
/** \name CC2538 device used by Contiki
 * @{
 */
#ifdef CC2538_DEV_CONF
#define CC2538_DEV      CC2538_DEV_CONF
#else
#define CC2538_DEV      CC2538_DEV_CC2538SF53
#endif
/** @} */
/*----------------------------------------------------------------------------*/
/** \name Features of the CC2538 device used by Contiki
 * @{
 */
/** Flash address */
#define CC2538_DEV_FLASH_ADDR   0x00200000
/** Flash size in bytes */
#define CC2538_DEV_FLASH_SIZE   (((CC2538_DEV & CC2538_DEV_FLASH_SIZE_KB_M) >> \
                                  CC2538_DEV_FLASH_SIZE_KB_S) << 10)
/** SRAM (non-retention + low-leakage) address */
#define CC2538_DEV_SRAM_ADDR    (CC2538_DEV_RLSRAM_SIZE ? \
                                 CC2538_DEV_RLSRAM_ADDR : \
                                 CC2538_DEV_LLSRAM_ADDR)
/** SRAM (non-retention + low-leakage) size in bytes */
#define CC2538_DEV_SRAM_SIZE    (((CC2538_DEV & CC2538_DEV_SRAM_SIZE_KB_M) >> \
                                  CC2538_DEV_SRAM_SIZE_KB_S) << 10)
/** Regular-leakage SRAM address */
#define CC2538_DEV_RLSRAM_ADDR  0x20000000
/** Regular-leakage SRAM size in bytes */
#define CC2538_DEV_RLSRAM_SIZE  (CC2538_DEV_SRAM_SIZE - CC2538_DEV_LLSRAM_SIZE)
/** Low-leakage SRAM address */
#define CC2538_DEV_LLSRAM_ADDR  0x20004000
/** Low-leakage SRAM size in bytes */
#define CC2538_DEV_LLSRAM_SIZE  MIN(CC2538_DEV_SRAM_SIZE, 16384)
/** Security HW AES/SHA */
#define CC2538_DEV_AES_SHA      (!!(CC2538_DEV & CC2538_DEV_AES_SHA_M))
/** Security HW ECC/RSA */
#define CC2538_DEV_ECC_RSA      (!!(CC2538_DEV & CC2538_DEV_ECC_RSA_M))
/** @} */

#endif /* CC2538_DEV_H_ */

/**
 * @}
 * @}
 */
