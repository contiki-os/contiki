/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Implementation of the cc2530 AES driver
 *
 * \author
 *         Justin King-Lacroix - <justin.king-lacroix@cs.ox.ac.uk>
 */
#ifndef CC2530_AES_H
#define CC2530_AES_H

#include "lib/aes-128.h"
#include "lib/ccm-star.h"

//ENCCS macros

//MODE options
#define CC2530_AES_MODE_CBC     0
#define CC2530_AES_MODE_CFB     1
#define CC2530_AES_MODE_OFB     2
#define CC2530_AES_MODE_CTR     3
#define CC2530_AES_MODE_ECB     4
#define CC2530_AES_MODE_CBC_MAC 5

//CMD options
#define CC2530_AES_CMD_ENCRYPT 0
#define CC2530_AES_CMD_DECRYPT 1
#define CC2530_AES_CMD_LOADKEY 2
#define CC2530_AES_CMD_LOADIV  3

//construction macro
#define CC2530_AES_ENCCS(mode, cmd, st) (uint8_t)(((uint8_t)(mode) << 4) | ((uint8_t)(cmd) << 1) | (uint8_t)(st))

//inspection macros
#define CC2530_AES_ENCCS_MODE(enccs) (uint8_t)((enccs) >> 4 & (uint8_t)0x7)
#define CC2530_AES_ENCCS_RDY(enccs)  (uint8_t)((enccs) >> 3 & (uint8_t)0x1)
#define CC2530_AES_ENCCS_CMD(enccs)  (uint8_t)((enccs) >> 1 & (uint8_t)0x3)
#define CC2530_AES_ENCCS_ST(enccs)   (uint8_t)((enccs)      & (uint8_t)0x1)


extern const struct aes_128_driver cc2530_aes_128_driver;
extern const struct ccm_star_driver cc2530_ccm_star_driver;

#endif //CC2530_AES_H
