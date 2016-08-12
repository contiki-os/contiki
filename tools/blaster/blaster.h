/*
 * Copyright (c) 2014, Lars Schmertmann <SmallLars@t-online.de>.
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
 *
 * This file is part of the Contiki operating system.
 *
 */

/* Purposes of the different flash blocks */
/* 0x18000 - 0x18FFF : Random access block 1.1 */
/* 0x19000 - 0x19FFF : Random access block 1.2 */
/* 0x1A000 - 0x1AFFF : Random access block 2.1 */
/* 0x1B000 - 0x1BFFF : Random access block 2.2 */
/* 0x1C000 - 0x1CFFF : Stack without pop function */
/* 0x1D000 - 0x1DFFF : Read only                  <- This is what blaster user */
/* 0x1E000 - 0x1EFFF : Read only                  <- This is what blaster user */
/* 0x1F000 - 0x1FFFF : System reserved */

#ifndef BLASTER_H_
#define BLASTER_H_

#define RES_D_CORE       0x1D000
#define LEN_D_CORE       0x6F

#define RES_CONFIG       0x1E000
#define LEN_CONFIG       0x12
#define RES_UUID         0x1E020
#define LEN_UUID         0x10
#define RES_PSK          0x1E030
#define LEN_PSK          0x10
#define RES_ANSCHARS     0x1E040
#define LEN_ANSCHARS     0x40
#define RES_ECC_BASE_X   0x1E080
#define LEN_ECC_BASE_X   0x20
#define RES_ECC_BASE_Y   0x1E0A0
#define LEN_ECC_BASE_Y   0x20
#define RES_ECC_ORDER    0x1E0C0
#define LEN_ECC_ORDER    0x20
#define RES_NAME         0x1E0E0
#define LEN_NAME         0x40
#define RES_MODEL        0x1E120
#define LEN_MODEL        0x40
#define RES_FLASHTIME    0x1E160
#define LEN_FLASHTIME    0x04

#endif /* BLASTER_H_ */
