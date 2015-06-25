/*
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
 * \defgroup cc2538-flash cc2538 flash memory
 *
 * Definitions for the cc2538 flash memory
 * @{
 *
 * \file
 * Header file for the flash memory definitions
 */
#ifndef FLASH_H_
#define FLASH_H_

#include "dev/cc2538-dev.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name Flash memory organization
 * @{
 */
#define FLASH_PAGE_SIZE 2048
#define FLASH_WORD_SIZE 4
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Flash lock bit page and CCA location
 * @{
 */
#define FLASH_CCA_ADDR  (CC2538_DEV_FLASH_ADDR + CC2538_DEV_FLASH_SIZE - \
                         FLASH_CCA_SIZE)        /**< Address */
#define FLASH_CCA_SIZE  0x0000002C		/**< Size in bytes */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Bootloader backdoor configuration bit fields
 * @{
 */
#define FLASH_CCA_BOOTLDR_CFG_DISABLE           0xEFFFFFFF /**< Disable backdoor function */
#define FLASH_CCA_BOOTLDR_CFG_ENABLE            0xF0FFFFFF /**< Enable backdoor function */
#define FLASH_CCA_BOOTLDR_CFG_ACTIVE_HIGH       0x08000000 /**< Selected pin on pad A active high */
#define FLASH_CCA_BOOTLDR_CFG_PORT_A_PIN_M      0x07000000 /**< Selected pin on pad A mask */
#define FLASH_CCA_BOOTLDR_CFG_PORT_A_PIN_S      24         /**< Selected pin on pad A shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Image valid definitions
 * @{
 */
#define FLASH_CCA_IMAGE_VALID                   0x00000000 /**< Indicates valid image in flash */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Lock page / debug definitions
 * @{
 */
#define FLASH_CCA_LOCKED                        0  /**< Page or debug locked if bit == 0 */
#define FLASH_CCA_LOCK_DEBUG_BYTE               31 /**< Lock byte containing the debug lock bit */
#define FLASH_CCA_LOCK_DEBUG_BIT                7  /**< Debug lock bit position in the corresponding lock byte */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Flash lock bit page and CCA layout
 * @{
 */
typedef struct {
  uint32_t bootldr_cfg; /**< Bootloader backdoor configuration (page bytes 2004 - 2007) */
  uint32_t image_valid; /**< Image valid (page bytes 2008 - 2011) */
  const void *app_entry_point; /**< Flash vector table address (page bytes 2012 - 2015) */
  uint8_t lock[32]; /**< Page and debug lock bits (page bytes 2016 - 2047) */
} flash_cca_lock_page_t;
/** @} */

#endif /* FLASH_H_ */

/**
 * @}
 * @}
 */
