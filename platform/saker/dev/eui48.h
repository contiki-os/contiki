/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 * \addtogroup saker
 *
 * @{
 *
 * \defgroup saker-eui48 EUI-48 address provider
 *
 * Driver for the retrieval of an unique EUI-48 address from the external
 * I2C-EEPROM.
 * In fact it's just a wrapper around the i2c-eeprom module
 * in order to provide a similar interface like the ieee-addr module.
 *
 * @{
 *
 * \file
 *      Header file for EUI-48 address driver
 */

#ifndef EUI48_H_
#define EUI48_H_

#include "contiki.h"

/*---------------------------------------------------------------------------*/
/** The size of the EUI-48 (48 bit) */
#define EUI48_SIZE              6
/*---------------------------------------------------------------------------*/
/**
 * \brief Retrieve a unique EUI-48
 *
 * \param buf Destination buffer for the EUI-48
 * \param buf_size The size of the destination buffer
 * \return 0 in case of errors, else number of bytes copied
 */
int
eui48_addr_cpy_to(uint8_t *buf, uint8_t buf_size);
/*---------------------------------------------------------------------------*/

#endif /* EUI48_H_ */

/**
 * @}
 * @}
 */
