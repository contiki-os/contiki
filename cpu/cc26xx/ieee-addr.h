/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-ieee-addr CC26xx IEEE Address Control
 *
 * Driver for the retrieval of an IEEE address from flash
 *
 * The user can specify a hardcoded IEEE address through the
 * IEEE_ADDR_CONF_HARDCODED configuration macro.
 *
 * If the user does not hard-code an address, then one will be read from either
 * the primary location (InfoPage) or from the secondary location (on flash).
 *
 * In order to allow the user to easily program nodes with addresses, the
 * secondary location is given priority: If it contains a valid address then
 * it will be chosen in favour of the one on InfoPage.
 *
 * In this context, an address is valid if at least one of the 8 bytes does not
 * equal 0xFF. If all 8 bytes are 0xFF, then the primary location will be used.
 *
 * In all cases, the address is assumed to be written little-endian.
 *
 * Lastly, it is possible to override the 2 LSB's of the address by using the
 * NODE_ID make variable.
 * @{
 *
 * \file
 * Header file with register and macro declarations for the cc26xx IEEE address
 * driver
 */
/*---------------------------------------------------------------------------*/
#ifndef IEEE_ADDR_H_
#define IEEE_ADDR_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/**
 * \name IEEE address locations
 *
 * The address of the secondary location can be configured by the platform
 * or example
 *
 * @{
 */
#define IEEE_ADDR_LOCATION_PRIMARY   0x500012F0 /**< Primary IEEE address location */

#ifdef IEEE_ADDR_CONF_LOCATION_SECONDARY
#define IEEE_ADDR_LOCATION_SECONDARY IEEE_ADDR_CONF_LOCATION_SECONDARY
#else
#define IEEE_ADDR_LOCATION_SECONDARY 0x0001FFC8 /**< Secondary IEEE address location */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief Copy the node's IEEE address to a destination memory area
 * \param dst A pointer to the destination area where the IEEE address is to be
 *            written
 * \param len The number of bytes to write to destination area
 *
 * This function will copy \e len LS bytes and it will invert byte order in
 * the process. The factory address on devices is normally little-endian,
 * therefore you should expect dst to store the address in a big-endian order.
 */
void ieee_addr_cpy_to(uint8_t *dst, uint8_t len);
/*---------------------------------------------------------------------------*/
#endif /* IEEE_ADDR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
