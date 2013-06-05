/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \defgroup cc2538-reg cc2538 Register Manipulation
 *
 * Macros for hardware access, both direct and via the bit-band region.
 * @{
 *
 * \file
 * Header file with register manipulation macro definitions
 */
#ifndef REG_H_
#define REG_H_

#define REG(x)         (*((volatile unsigned long *)(x)))
#define REG_H(x)       (*((volatile unsigned short *)(x)))
#define REG_B(x)       (*((volatile unsigned char *)(x)))
#define REG_BIT_W(x, b)                                                     \
        REG(((unsigned long)(x) & 0xF0000000) | 0x02000000 |                \
              (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))
#define REG_BIT_H(x, b)                                                     \
        REG_H(((unsigned long)(x) & 0xF0000000) | 0x02000000 |              \
               (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))
#define REG_BIT_B(x, b)                                                     \
        REG_B(((unsigned long)(x) & 0xF0000000) | 0x02000000 |              \
               (((unsigned long)(x) & 0x000FFFFF) << 5) | ((b) << 2))

#endif /* REG_H_ */

/**
 * @}
 * @}
 */
