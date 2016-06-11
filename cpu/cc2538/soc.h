/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
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
 * \defgroup cc2538-soc cc2538 SoC
 *
 * Driver for the cc2538 SoC
 * @{
 *
 * \file
 * Header file with macro and function declarations for the cc2538 SoC
 */
#ifndef SOC_H_
#define SOC_H_

#include "contiki-conf.h"

#include <stdint.h>
/*----------------------------------------------------------------------------*/
/** \name SoC features
 * @{
 */
#define SOC_FEATURE_AES_SHA     0x00000002 /**< Security HW AES/SHA */
#define SOC_FEATURE_ECC_RSA     0x00000001 /**< Security HW ECC/RSA */
/** @} */
/*----------------------------------------------------------------------------*/
/** \name SoC functions
 * @{
 */

/** \brief Gets the SoC revision
 * \return The SoC revision as a byte with nibbles representing the major and
 * minor revisions
 */
uint8_t soc_get_rev(void);

/** \brief Gets the SRAM size of the SoC
 * \return The SRAM size in bytes
 */
uint32_t soc_get_sram_size(void);

/** \brief Gets the hardware features of the SoC that are enabled
 * \return The enabled hardware features as a bitmask of \c SOC_FEATURE_x values
 */
uint32_t soc_get_features(void);

/** \brief Prints SoC information */
void soc_print_info(void);

/** @} */

#endif /* SOC_H_ */

/**
 * @}
 * @}
 */
