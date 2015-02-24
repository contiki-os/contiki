/*
 * Copyright (c) 2015, 3B Scientific GmbH - http://www.3bscientific.com/
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
 * \addtogroup cc3200-launchxl
 * @{
 *
 * \defgroup
 *
 * Headers of TI CC2520 architecture dependent functions for
 * cc3200-launchxl platform
 * @{
 *
 * \file
 * Headers of TI CC2520 architecture dependent functions for
 * cc3200-launchxl platform
 *
 * \author
 * 		   Björn Rennfanz <bjoern.rennfanz@3bscientific.com>
 *         Dominik Grauert <dominik.grauert@3bscientific.com>
 */

#ifndef CC2520_ARCH_H_
#define CC2520_ARCH_H_

#ifndef BV
#define BV(x) 					(1<<(x))
#endif

/*
 * Definitions for CC2520 FSMSTAT1 register
 */
#define CC2520_FSMSTAT1_FIFO	(1 << 7)
#define CC2520_FSMSTAT1_FIFOP	(1 << 6)
#define CC2520_FSMSTAT1_SFD		(1 << 5)
#define CC2520_FSMSTAT1_CCA		(1 << 4)

/*
 * Prototypes for CC2520
 */
uint8_t cc2520_arch_getreg(uint16_t regname);

/*
 * Platform MSP430 emulation for CC2520 driver
 */
int splhigh(void);
void splx(int saved);

#endif /* CC2520_ARCH_H_ */
