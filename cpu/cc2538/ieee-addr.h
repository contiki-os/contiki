/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
 * \defgroup cc2538-ieee-addr cc2538 IEEE Address Control
 *
 * Driver for the retrieval of an IEEE address from flash
 * @{
 *
 * \file
 * Header file with register and macro declarations for the cc2538 IEEE address
 * driver
 */
#ifndef IEEE_ADDR_H_
#define IEEE_ADDR_H_

#include "contiki-conf.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \name TI OUI
 * @{
 */
#define IEEE_ADDR_OUI_TI   { 0x00, 0x12, 0x4B } /**< TI OUI */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name IEEE address locations
 *
 * The address of the secondary location can be configured by the platform
 * or example
 *
 * @{
 */
#define IEEE_ADDR_LOCATION_PRIMARY   0x00280028 /**< Primary IEEE address location */

#ifdef IEEE_ADDR_CONF_LOCATION_SECONDARY
#define IEEE_ADDR_LOCATION_SECONDARY IEEE_ADDR_CONF_LOCATION_SECONDARY
#else
#define IEEE_ADDR_LOCATION_SECONDARY 0x0027FFCC /**< Secondary IEEE address location */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief Select which address location to use
 * @{
 */
#if IEEE_ADDR_CONF_USE_SECONDARY_LOCATION
#define IEEE_ADDR_LOCATION IEEE_ADDR_LOCATION_SECONDARY
#else
#define IEEE_ADDR_LOCATION IEEE_ADDR_LOCATION_PRIMARY
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/*
 * \brief Copy the node's IEEE address to a destination memory area
 * \param dst A pointer to the destination area where the IEEE address is to be
 *            written
 * \param len The number of bytes to write to destination area
 *
 * The address will be read from an InfoPage location or a hard-coded address
 * will be used, depending on the value of configuration parameter
 * IEEE_ADDR_CONF_HARDCODED
 *
 * This function will copy \e len LS bytes
 *
 * The destination address will be populated with dst[0] holding the MSB and
 * dst[len - 1] holding the LSB
 */
void ieee_addr_cpy_to(uint8_t *dst, uint8_t len);

#endif /* IEEE_ADDR_H_ */

/**
 * @}
 * @}
 */
