/**
 * \addtogroup dev
 * @{
 */

/**
 * \defgroup eeprom EEPROM API
 *
 * The EEPROM API defines a common interface for EEPROM access on
 * Contiki platforms.
 *
 * A platform with EEPROM support must implement this API.
 *
 * @{
 */

/**
 * \file
 * EEPROM functions.
 * \author Adam Dunkels <adam@sics.se>
 */

/* Copyright (c) 2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */


#ifndef EEPROM_H_
#define EEPROM_H_

typedef unsigned short eeprom_addr_t;

#define EEPROM_NULL			0

#ifdef EEPROM_CONF_SIZE
#define EEPROM_SIZE			(EEPROM_CONF_SIZE)
#else
#define EEPROM_SIZE			0 /* Default to no EEPROM */
#endif

#ifdef EEPROM_CONF_END_ADDR
#define EEPROM_END_ADDR		(EEPROM_CONF_END_ADDR)
#else
#define EEPROM_END_ADDR		(EEPROM_SIZE - 1)
#endif

/**
 * Write a buffer into EEPROM.
 *
 * This function writes a buffer of the specified size into EEPROM.
 *
 * \param addr The address in EEPROM to which the buffer should be written.
 *
 * \param buf A pointer to the buffer from which data is to be read.
 *
 * \param size The number of bytes to write into EEPROM.
 *
 *
 */
void eeprom_write(eeprom_addr_t addr, unsigned char *buf, int size);

/**
 * Read data from the EEPROM.
 *
 * This function reads a number of bytes from the specified address in
 * EEPROM and into a buffer in memory.
 *
 * \param addr The address in EEPROM from which the data should be read.
 *
 * \param buf A pointer to the buffer to which the data should be stored.
 *
 * \param size The number of bytes to read.
 *
 *
 */
void eeprom_read(eeprom_addr_t addr, unsigned char *buf, int size);

/**
 * Initialize the EEPROM module
 *
 * This function initializes the EEPROM module and is called from the
 * bootup code.
 *
 */
 
void eeprom_init(void);

#endif /* EEPROM_H_ */

/** @} */
/** @} */
