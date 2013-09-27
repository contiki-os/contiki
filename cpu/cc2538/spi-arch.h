/*
 * Copyright (c) 2013, University of Michigan.
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
 * 3. Neither the name of the University nor the names of its contributors
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
 */
/**
 * \addtogroup cc2538
 * @{
 *
 * Implementation of the low-level SPI primitives such as waiting for the TX
 * FIFO to be ready, inserting into the TX FIFO, etc.
 * @{
 */
/**
 * \file
 * Header file for the cc2538 SPI commands
 */
#ifndef SPI_ARCH_H_
#define SPI_ARCH_H_

#include "contiki.h"
#include "dev/ssi.h"

#define SPI_WAITFORTxREADY() do { \
  while(!(REG(SSI0_BASE + SSI_SR) & SSI_SR_TNF)); \
} while (0)

#define SPI_TXBUF REG(SSI0_BASE + SSI_DR)
#define SPI_RXBUF REG(SSI0_BASE + SSI_DR)

#define SPI_WAITFOREOTx() do { \
  while(REG(SSI0_BASE + SSI_SR) & SSI_SR_BSY); \
} while (0)
#define SPI_WAITFOREORx() do { \
  while(!(REG(SSI0_BASE + SSI_SR) & SSI_SR_RNE)); \
} while (0)

#endif /* SPI_ARCH_H_ */

/**
 * @}
 * @}
 */
