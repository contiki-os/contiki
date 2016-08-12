/*
 * Original file:
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
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
 * \addtogroup cc2538-crypto
 * @{
 *
 * \defgroup cc2538-aes cc2538 AES
 *
 * Driver for the cc2538 AES modes of the security core
 * @{
 *
 * \file
 * Header file for the cc2538 AES driver
 */
#ifndef AES_H_
#define AES_H_

#include "contiki.h"
#include "dev/crypto.h"

#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name AES register offsets
 * @{
 */
#define AES_DMAC_CH0_CTRL       0x4008B000 /**< Channel 0 control */
#define AES_DMAC_CH0_EXTADDR    0x4008B004 /**< Channel 0 external address */
#define AES_DMAC_CH0_DMALENGTH  0x4008B00C /**< Channel 0 DMA length */
#define AES_DMAC_STATUS         0x4008B018 /**< DMAC status */
#define AES_DMAC_SWRES          0x4008B01C /**< DMAC software reset */
#define AES_DMAC_CH1_CTRL       0x4008B020 /**< Channel 1 control */
#define AES_DMAC_CH1_EXTADDR    0x4008B024 /**< Channel 1 external address */
#define AES_DMAC_CH1_DMALENGTH  0x4008B02C /**< Channel 1 DMA length */
#define AES_DMAC_MST_RUNPARAMS  0x4008B078 /**< DMAC master run-time parameters */
#define AES_DMAC_PERSR          0x4008B07C /**< DMAC port error raw status */
#define AES_DMAC_OPTIONS        0x4008B0F8 /**< DMAC options */
#define AES_DMAC_VERSION        0x4008B0FC /**< DMAC version */
#define AES_KEY_STORE_WRITE_AREA \
                                0x4008B400 /**< Key store write area */
#define AES_KEY_STORE_WRITTEN_AREA \
                                0x4008B404 /**< Key store written area */
#define AES_KEY_STORE_SIZE      0x4008B408 /**< Key store size */
#define AES_KEY_STORE_READ_AREA 0x4008B40C /**< Key store read area */
#define AES_AES_KEY2_0          0x4008B500 /**< AES_KEY2_0 / AES_GHASH_H_IN_0 */
#define AES_AES_KEY2_1          0x4008B504 /**< AES_KEY2_1 / AES_GHASH_H_IN_1 */
#define AES_AES_KEY2_2          0x4008B508 /**< AES_KEY2_2 / AES_GHASH_H_IN_2 */
#define AES_AES_KEY2_3          0x4008B50C /**< AES_KEY2_3 / AES_GHASH_H_IN_3 */
#define AES_AES_KEY3_0          0x4008B510 /**< AES_KEY3_0 / AES_KEY2_4 */
#define AES_AES_KEY3_1          0x4008B514 /**< AES_KEY3_1 / AES_KEY2_5 */
#define AES_AES_KEY3_2          0x4008B518 /**< AES_KEY3_2 / AES_KEY2_6 */
#define AES_AES_KEY3_3          0x4008B51C /**< AES_KEY3_3 / AES_KEY2_7 */
#define AES_AES_IV_0            0x4008B540 /**< AES initialization vector */
#define AES_AES_IV_1            0x4008B544 /**< AES initialization vector */
#define AES_AES_IV_2            0x4008B548 /**< AES initialization vector */
#define AES_AES_IV_3            0x4008B54C /**< AES initialization vector */
#define AES_AES_CTRL            0x4008B550 /**< AES input/output buffer control and mode */
#define AES_AES_C_LENGTH_0      0x4008B554 /**< AES crypto length (LSW) */
#define AES_AES_C_LENGTH_1      0x4008B558 /**< AES crypto length (MSW) */
#define AES_AES_AUTH_LENGTH     0x4008B55C /**< Authentication length */
#define AES_AES_DATA_IN_OUT_0   0x4008B560 /**< Data input/output */
#define AES_AES_DATA_IN_OUT_1   0x4008B564 /**< Data Input/Output */
#define AES_AES_DATA_IN_OUT_2   0x4008B568 /**< Data Input/Output */
#define AES_AES_DATA_IN_OUT_3   0x4008B56C /**< Data Input/Output */
#define AES_AES_TAG_OUT_0       0x4008B570 /**< TAG */
#define AES_AES_TAG_OUT_1       0x4008B574 /**< TAG */
#define AES_AES_TAG_OUT_2       0x4008B578 /**< TAG */
#define AES_AES_TAG_OUT_3       0x4008B57C /**< TAG */
#define AES_HASH_DATA_IN_0      0x4008B600 /**< HASH data input */
#define AES_HASH_DATA_IN_1      0x4008B604 /**< HASH data input */
#define AES_HASH_DATA_IN_2      0x4008B608 /**< HASH data input */
#define AES_HASH_DATA_IN_3      0x4008B60C /**< HASH data input */
#define AES_HASH_DATA_IN_4      0x4008B610 /**< HASH data input */
#define AES_HASH_DATA_IN_5      0x4008B614 /**< HASH data input */
#define AES_HASH_DATA_IN_6      0x4008B618 /**< HASH data input */
#define AES_HASH_DATA_IN_7      0x4008B61C /**< HASH data input */
#define AES_HASH_DATA_IN_8      0x4008B620 /**< HASH data input */
#define AES_HASH_DATA_IN_9      0x4008B624 /**< HASH data input */
#define AES_HASH_DATA_IN_10     0x4008B628 /**< HASH data input */
#define AES_HASH_DATA_IN_11     0x4008B62C /**< HASH data input */
#define AES_HASH_DATA_IN_12     0x4008B630 /**< HASH data input */
#define AES_HASH_DATA_IN_13     0x4008B634 /**< HASH data input */
#define AES_HASH_DATA_IN_14     0x4008B638 /**< HASH data input */
#define AES_HASH_DATA_IN_15     0x4008B63C /**< HASH data input */
#define AES_HASH_IO_BUF_CTRL    0x4008B640 /**< Input/output buffer control and status */
#define AES_HASH_MODE_IN        0x4008B644 /**< Hash mode */
#define AES_HASH_LENGTH_IN_L    0x4008B648 /**< Hash length */
#define AES_HASH_LENGTH_IN_H    0x4008B64C /**< Hash length */
#define AES_HASH_DIGEST_A       0x4008B650 /**< Hash digest */
#define AES_HASH_DIGEST_B       0x4008B654 /**< Hash digest */
#define AES_HASH_DIGEST_C       0x4008B658 /**< Hash digest */
#define AES_HASH_DIGEST_D       0x4008B65C /**< Hash digest */
#define AES_HASH_DIGEST_E       0x4008B660 /**< Hash digest */
#define AES_HASH_DIGEST_F       0x4008B664 /**< Hash digest */
#define AES_HASH_DIGEST_G       0x4008B668 /**< Hash digest */
#define AES_HASH_DIGEST_H       0x4008B66C /**< Hash digest */
#define AES_CTRL_ALG_SEL        0x4008B700 /**< Algorithm select */
#define AES_CTRL_PROT_EN        0x4008B704 /**< Master PROT privileged access enable */
#define AES_CTRL_SW_RESET       0x4008B740 /**< Software reset */
#define AES_CTRL_INT_CFG        0x4008B780 /**< Interrupt configuration */
#define AES_CTRL_INT_EN         0x4008B784 /**< Interrupt enable */
#define AES_CTRL_INT_CLR        0x4008B788 /**< Interrupt clear */
#define AES_CTRL_INT_SET        0x4008B78C /**< Interrupt set */
#define AES_CTRL_INT_STAT       0x4008B790 /**< Interrupt status */
#define AES_CTRL_OPTIONS        0x4008B7F8 /**< Options */
#define AES_CTRL_VERSION        0x4008B7FC /**< Version */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_CHx_CTRL registers bit fields
 * @{
 */
#define AES_DMAC_CH_CTRL_PRIO   0x00000002 /**< Channel priority 0: Low 1: High */
#define AES_DMAC_CH_CTRL_EN     0x00000001 /**< Channel enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_CHx_DMALENGTH registers bit fields
 * @{
 */
#define AES_DMAC_CH_DMALENGTH_DMALEN_M \
                                0x0000FFFF /**< Channel DMA length in bytes mask */
#define AES_DMAC_CH_DMALENGTH_DMALEN_S 0   /**< Channel DMA length in bytes shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_STATUS register bit fields
 * @{
 */
#define AES_DMAC_STATUS_PORT_ERR \
                                0x00020000 /**< AHB port transfer errors */
#define AES_DMAC_STATUS_CH1_ACT 0x00000002 /**< Channel 1 active (DMA transfer on-going) */
#define AES_DMAC_STATUS_CH0_ACT 0x00000001 /**< Channel 0 active (DMA transfer on-going) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_SWRES register bit fields
 * @{
 */
#define AES_DMAC_SWRES_SWRES    0x00000001 /**< Software reset enable */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_MST_RUNPARAMS register bit fields
 * @{
 */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BURST_SIZE_4 \
                                (2 << 12)  /**< Maximum burst size: 4 bytes */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BURST_SIZE_8 \
                                (3 << 12)  /**< Maximum burst size: 8 bytes */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BURST_SIZE_16 \
                                (4 << 12)  /**< Maximum burst size: 16 bytes */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BURST_SIZE_32 \
                                (5 << 12)  /**< Maximum burst size: 32 bytes */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BURST_SIZE_64 \
                                (6 << 12)  /**< Maximum burst size: 64 bytes */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BURST_SIZE_M \
                                0x0000F000 /**< Maximum burst size mask */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BURST_SIZE_S \
                                12         /**< Maximum burst size shift */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_IDLE_EN \
                                0x00000800 /**< Idle insertion between bursts */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_INCR_EN \
                                0x00000400 /**< Fixed-length burst or single transfers */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_LOCK_EN \
                                0x00000200 /**< Locked transfers */
#define AES_DMAC_MST_RUNPARAMS_AHB_MST1_BIGEND \
                                0x00000100 /**< Big endian AHB master */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_PERSR register bit fields
 * @{
 */
#define AES_DMAC_PERSR_PORT1_AHB_ERROR \
                                0x00001000 /**< AHB bus error */
#define AES_DMAC_PERSR_PORT1_CHANNEL \
                                0x00000200 /**< Last serviced channel (0 or 1) */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_OPTIONS register bit fields
 * @{
 */
#define AES_DMAC_OPTIONS_NR_OF_CHANNELS_M \
                                0x00000F00 /**< Number of channels implemented mask */
#define AES_DMAC_OPTIONS_NR_OF_CHANNELS_S \
                                8          /**< Number of channels implemented shift */
#define AES_DMAC_OPTIONS_NR_OF_PORTS_M \
                                0x00000007 /**< Number of ports implemented mask */
#define AES_DMAC_OPTIONS_NR_OF_PORTS_S 0   /**< Number of ports implemented shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_DMAC_VERSION register bit fields
 * @{
 */
#define AES_DMAC_VERSION_HW_MAJOR_VERSION_M \
                                0x0F000000 /**< Major version number mask */
#define AES_DMAC_VERSION_HW_MAJOR_VERSION_S \
                                24         /**< Major version number shift */
#define AES_DMAC_VERSION_HW_MINOR_VERSION_M \
                                0x00F00000 /**< Minor version number mask */
#define AES_DMAC_VERSION_HW_MINOR_VERSION_S \
                                20         /**< Minor version number shift */
#define AES_DMAC_VERSION_HW_PATCH_LEVEL_M \
                                0x000F0000 /**< Patch level mask */
#define AES_DMAC_VERSION_HW_PATCH_LEVEL_S \
                                16         /**< Patch level shift */
#define AES_DMAC_VERSION_EIP_NUMBER_COMPL_M \
                                0x0000FF00 /**< EIP_NUMBER 1's complement mask */
#define AES_DMAC_VERSION_EIP_NUMBER_COMPL_S \
                                8          /**< EIP_NUMBER 1's complement shift */
#define AES_DMAC_VERSION_EIP_NUMBER_M \
                                0x000000FF /**< DMAC EIP-number mask */
#define AES_DMAC_VERSION_EIP_NUMBER_S 0    /**< DMAC EIP-number shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_KEY_STORE_SIZE register bit fields
 * @{
 */
#define AES_KEY_STORE_SIZE_KEY_SIZE_128 1  /**< Key size: 128 bits */
#define AES_KEY_STORE_SIZE_KEY_SIZE_192 2  /**< Key size: 192 bits */
#define AES_KEY_STORE_SIZE_KEY_SIZE_256 3  /**< Key size: 256 bits */
#define AES_KEY_STORE_SIZE_KEY_SIZE_M \
                                0x00000003 /**< Key size mask */
#define AES_KEY_STORE_SIZE_KEY_SIZE_S 0    /**< Key size shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_KEY_STORE_READ_AREA register bit fields
 * @{
 */
#define AES_KEY_STORE_READ_AREA_BUSY \
                                0x80000000 /**< Key store operation busy */
#define AES_KEY_STORE_READ_AREA_RAM_AREA_M \
                                0x0000000F /**< Key store RAM area select mask */
#define AES_KEY_STORE_READ_AREA_RAM_AREA_S \
                                0          /**< Key store RAM area select shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_AES_CTRL register bit fields
 * @{
 */
#define AES_AES_CTRL_CONTEXT_READY \
                                0x80000000 /**< Context data registers can be overwritten */
#define AES_AES_CTRL_SAVED_CONTEXT_READY \
                                0x40000000 /**< AES auth. TAG and/or IV block(s) available */
#define AES_AES_CTRL_SAVE_CONTEXT \
                                0x20000000 /**< Auth. TAG or result IV needs to be stored */
#define AES_AES_CTRL_CCM_M_M    0x01C00000 /**< CCM auth. field length mask */
#define AES_AES_CTRL_CCM_M_S    22         /**< CCM auth. field length shift */
#define AES_AES_CTRL_CCM_L_M    0x00380000 /**< CCM length field width mask */
#define AES_AES_CTRL_CCM_L_S    19         /**< CCM length field width shift */
#define AES_AES_CTRL_CCM        0x00040000 /**< AES-CCM mode */
#define AES_AES_CTRL_GCM        0x00030000 /**< AES-GCM mode */
#define AES_AES_CTRL_CBC_MAC    0x00008000 /**< AES-CBC MAC mode */
#define AES_AES_CTRL_CTR_WIDTH_32 (0 << 7) /**< CTR counter width: 32 bits */
#define AES_AES_CTRL_CTR_WIDTH_64 (1 << 7) /**< CTR counter width: 64 bits */
#define AES_AES_CTRL_CTR_WIDTH_96 (2 << 7) /**< CTR counter width: 96 bits */
#define AES_AES_CTRL_CTR_WIDTH_128 \
                                (3 << 7)   /**< CTR counter width: 128 bits */
#define AES_AES_CTRL_CTR_WIDTH_M \
                                0x00000180 /**< CTR counter width mask */
#define AES_AES_CTRL_CTR_WIDTH_S 7         /**< CTR counter width shift */
#define AES_AES_CTRL_CTR        0x00000040 /**< AES-CTR mode */
#define AES_AES_CTRL_CBC        0x00000020 /**< AES-CBC mode */
#define AES_AES_CTRL_KEY_SIZE_128 (1 << 3) /**< Key size: 128 bits */
#define AES_AES_CTRL_KEY_SIZE_192 (2 << 3) /**< Key size: 192 bits */
#define AES_AES_CTRL_KEY_SIZE_256 (3 << 3) /**< Key size: 256 bits */
#define AES_AES_CTRL_KEY_SIZE_M 0x00000018 /**< Key size mask */
#define AES_AES_CTRL_KEY_SIZE_S 3          /**< Key size shift */
#define AES_AES_CTRL_DIRECTION_ENCRYPT \
                                0x00000004 /**< Encrypt */
#define AES_AES_CTRL_INPUT_READY \
                                0x00000002 /**< AES input buffer empty */
#define AES_AES_CTRL_OUTPUT_READY \
                                0x00000001 /**< AES output block available */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_AES_C_LENGTH_1 register bit fields
 * @{
 */
#define AES_AES_C_LENGTH_1_C_LENGTH_M \
                                0x1FFFFFFF /**< Crypto length bits [60:32] mask */
#define AES_AES_C_LENGTH_1_C_LENGTH_S 0    /**< Crypto length bits [60:32] shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_HASH_IO_BUF_CTRL register bit fields
 * @{
 */
#define AES_HASH_IO_BUF_CTRL_PAD_DMA_MESSAGE \
                                0x00000080 /**< Hash engine message padding required */
#define AES_HASH_IO_BUF_CTRL_GET_DIGEST \
                                0x00000040 /**< Hash engine digest requested */
#define AES_HASH_IO_BUF_CTRL_PAD_MESSAGE \
                                0x00000020 /**< Last message data in HASH_DATA_IN, apply hash padding */
#define AES_HASH_IO_BUF_CTRL_RFD_IN \
                                0x00000004 /**< Hash engine input buffer can accept new data */
#define AES_HASH_IO_BUF_CTRL_DATA_IN_AV \
                                0x00000002 /**< Start processing HASH_DATA_IN data */
#define AES_HASH_IO_BUF_CTRL_OUTPUT_FULL \
                                0x00000001 /**< Output buffer registers available */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_HASH_MODE_IN register bit fields
 * @{
 */
#define AES_HASH_MODE_IN_SHA256_MODE \
                                0x00000008 /**< Hash mode */
#define AES_HASH_MODE_IN_NEW_HASH \
                                0x00000001 /**< New hash session */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_ALG_SEL register bit fields
 * @{
 */
#define AES_CTRL_ALG_SEL_TAG    0x80000000 /**< DMA operation includes TAG */
#define AES_CTRL_ALG_SEL_HASH   0x00000004 /**< Select hash engine as DMA destination */
#define AES_CTRL_ALG_SEL_AES    0x00000002 /**< Select AES engine as DMA source/destination */
#define AES_CTRL_ALG_SEL_KEYSTORE \
                                0x00000001 /**< Select Key Store as DMA destination */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_PROT_EN register bit fields
 * @{
 */
#define AES_CTRL_PROT_EN_PROT_EN \
                                0x00000001 /**< m_h_prot[1] asserted for DMA reads towards key store */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_SW_RESET register bit fields
 * @{
 */
#define AES_CTRL_SW_RESET_SW_RESET \
                                0x00000001 /**< Reset master control and key store */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_INT_CFG register bit fields
 * @{
 */
#define AES_CTRL_INT_CFG_LEVEL  0x00000001 /**< Level interrupt type */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_INT_EN register bit fields
 * @{
 */
#define AES_CTRL_INT_EN_DMA_IN_DONE \
                                0x00000002 /**< DMA input done interrupt enabled */
#define AES_CTRL_INT_EN_RESULT_AV \
                                0x00000001 /**< Result available interrupt enabled */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_INT_CLR register bit fields
 * @{
 */
#define AES_CTRL_INT_CLR_DMA_BUS_ERR \
                                0x80000000 /**< Clear DMA bus error status */
#define AES_CTRL_INT_CLR_KEY_ST_WR_ERR \
                                0x40000000 /**< Clear key store write error status */
#define AES_CTRL_INT_CLR_KEY_ST_RD_ERR \
                                0x20000000 /**< Clear key store read error status */
#define AES_CTRL_INT_CLR_DMA_IN_DONE \
                                0x00000002 /**< Clear DMA in done interrupt */
#define AES_CTRL_INT_CLR_RESULT_AV \
                                0x00000001 /**< Clear result available interrupt */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_INT_SET register bit fields
 * @{
 */
#define AES_CTRL_INT_SET_DMA_IN_DONE \
                                0x00000002 /**< Set DMA data in done interrupt */
#define AES_CTRL_INT_SET_RESULT_AV \
                                0x00000001 /**< Set result available interrupt */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_INT_STAT register bit fields
 * @{
 */
#define AES_CTRL_INT_STAT_DMA_BUS_ERR \
                                0x80000000 /**< DMA bus error detected */
#define AES_CTRL_INT_STAT_KEY_ST_WR_ERR \
                                0x40000000 /**< Write error detected */
#define AES_CTRL_INT_STAT_KEY_ST_RD_ERR \
                                0x20000000 /**< Read error detected */
#define AES_CTRL_INT_STAT_DMA_IN_DONE \
                                0x00000002 /**< DMA data in done interrupt status */
#define AES_CTRL_INT_STAT_RESULT_AV \
                                0x00000001 /**< Result available interrupt status */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_OPTIONS register bit fields
 * @{
 */
#define AES_CTRL_OPTIONS_TYPE_M 0xFF000000 /**< Device type mask */
#define AES_CTRL_OPTIONS_TYPE_S 24         /**< Device type shift */
#define AES_CTRL_OPTIONS_AHBINTERFACE \
                                0x00010000 /**< AHB interface available */
#define AES_CTRL_OPTIONS_SHA_256 \
                                0x00000100 /**< The HASH core supports SHA-256 */
#define AES_CTRL_OPTIONS_AES_CCM \
                                0x00000080 /**< AES-CCM available as single operation */
#define AES_CTRL_OPTIONS_AES_GCM \
                                0x00000040 /**< AES-GCM available as single operation */
#define AES_CTRL_OPTIONS_AES_256 \
                                0x00000020 /**< AES core supports 256-bit keys */
#define AES_CTRL_OPTIONS_AES_128 \
                                0x00000010 /**< AES core supports 128-bit keys */
#define AES_CTRL_OPTIONS_HASH   0x00000004 /**< HASH Core available */
#define AES_CTRL_OPTIONS_AES    0x00000002 /**< AES core available */
#define AES_CTRL_OPTIONS_KEYSTORE \
                                0x00000001 /**< KEY STORE available */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES_CTRL_VERSION register bit fields
 * @{
 */
#define AES_CTRL_VERSION_MAJOR_VERSION_M \
                                0x0F000000 /**< Major version number mask */
#define AES_CTRL_VERSION_MAJOR_VERSION_S \
                                24         /**< Major version number shift */
#define AES_CTRL_VERSION_MINOR_VERSION_M \
                                0x00F00000 /**< Minor version number mask */
#define AES_CTRL_VERSION_MINOR_VERSION_S \
                                20         /**< Minor version number shift */
#define AES_CTRL_VERSION_PATCH_LEVEL_M \
                                0x000F0000 /**< Patch level mask */
#define AES_CTRL_VERSION_PATCH_LEVEL_S 16  /**< Patch level shift */
#define AES_CTRL_VERSION_EIP_NUMBER_COMPL_M \
                                0x0000FF00 /**< EIP_NUMBER 1's complement mask */
#define AES_CTRL_VERSION_EIP_NUMBER_COMPL_S \
                                8          /**< EIP_NUMBER 1's complement shift */
#define AES_CTRL_VERSION_EIP_NUMBER_M \
                                0x000000FF /**< EIP-120t EIP-number mask */
#define AES_CTRL_VERSION_EIP_NUMBER_S 0    /**< EIP-120t EIP-number shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES drivers return codes
 * @{
 */
#define AES_KEYSTORE_READ_ERROR       5
#define AES_KEYSTORE_WRITE_ERROR      6
#define AES_AUTHENTICATION_FAILED     7
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES constants
 * @{
 */
#define AES_KEY_AREAS   8
#define AES_BLOCK_LEN   (128 / 8)
#define AES_IV_LEN      AES_BLOCK_LEN
#define AES_TAG_LEN     AES_BLOCK_LEN
/** @} */
/*---------------------------------------------------------------------------*/
/** \name AES functions
 * @{
 */

/** \brief Writes keys into the Key RAM
 * \param keys Pointer to AES Keys
 * \param key_size Key size: \c AES_KEY_STORE_SIZE_KEY_SIZE_x
 * \param count Number of keys (1 to \c AES_KEY_AREAS - \p start_area for
 * 128-bit keys, 1 to (\c AES_KEY_AREAS - \p start_area) / 2 for 192- and
 * 256-bit keys)
 * \param start_area Start area in Key RAM where to store the keys (0 to
 * \c AES_KEY_AREAS - 1, must be even for 192- and 256-bit keys)
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES error code
 * \note Calling this function with a value of \p key_size different from the
 * one passed for the previous calls causes the deletion of all previously
 * stored keys.
 */
uint8_t aes_load_keys(const void *keys, uint8_t key_size, uint8_t count,
                      uint8_t start_area);

/** \brief Starts an AES authentication/crypto operation
 * \param ctrl Contents of the \c AES_AES_CTRL register
 * \param key_area Area in Key RAM where the key is stored (0 to
 * \c AES_KEY_AREAS - 1)
 * \param iv Pointer to 128-bit initialization vector, or \c NULL
 * \param adata Pointer to additional authenticated data in SRAM, or \c NULL
 * \param adata_len Length of additional authenticated data in octets, or \c 0
 * \param data_in Pointer to input payload data in SRAM, or \c NULL
 * \param data_out Pointer to output payload data in SRAM (may be \p data_in),
 * or \c NULL
 * \param data_len Length of payload data in octets, or \c 0
 * \param process Process to be polled upon completion of the operation, or
 * \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES error code
 * \note This function is only supposed to be called by the AES drivers.
 */
uint8_t aes_auth_crypt_start(uint32_t ctrl, uint8_t key_area, const void *iv,
                             const void *adata, uint16_t adata_len,
                             const void *data_in, void *data_out,
                             uint16_t data_len, struct process *process);

/** \brief Checks the status of the AES authentication/crypto operation
 * \retval false Result not yet available, and no error occurred
 * \retval true Result available, or error occurred
 * \note This function is only supposed to be called by the AES drivers.
 */
uint8_t aes_auth_crypt_check_status(void);

/** \brief Gets the result of the AES authentication/crypto operation
 * \param iv Pointer to 128-bit result initialization vector, or \c NULL
 * \param tag Pointer to 128-bit result tag, or \c NULL
 * \return \c CRYPTO_SUCCESS if successful, or CRYPTO/AES error code
 * \note This function must be called only after \c aes_auth_crypt_start().
 * \note This function is only supposed to be called by the AES drivers.
 */
uint8_t aes_auth_crypt_get_result(void *iv, void *tag);

/** @} */

#endif /* AES_H_ */

/**
 * @}
 * @}
 */
