/**
 * \file
 * Functions for reading and writing MSP430 flash ROM.
 * \author Adam Dunkels <adam@sics.se> 
 */

/* Copyright (c) 2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __FLASH_H__
#define __FLASH_H__

/**
 * Setup function to be called before any of the flash programming functions.
 *
 */
void flash_setup(void);

/**
 * Function that is to be called after flashing is done.
 */
void flash_done(void);

/**
 * Write a 16-bit word to flash ROM.
 *
 * This function writes a 16-bit word to flash ROM. The function
 * flash_setup() must have been called first.
 *
 * \param addr A pointer to the address in flash ROM which is to be
 * written.
 *
 * \param word The 16-bit word that is to be written.
 */
void flash_write(unsigned short *addr, unsigned short word);

/**
 * Clear a 16-bit word in flash ROM.
 *
 * This function clears a 16-bit word in flash ROM. The function
 * flash_setup() must have been called first. 
 *
 * \param addr A pointer to the address in flash ROM which is to be
 * cleared.
 */
void flash_clear(unsigned short *addr);

#endif /* __FLASH_H__ */
