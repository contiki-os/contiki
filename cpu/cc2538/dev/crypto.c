/*
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
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
 * \addtogroup cc2538-crypto
 * @{
 *
 * \file
 * Implementation of the cc2538 AES/SHA cryptoprocessor driver
 */
#include "contiki.h"
#include "dev/sys-ctrl.h"
#include "dev/crypto.h"
#include "reg.h"
/*---------------------------------------------------------------------------*/
void
crypto_init(void)
{
  volatile int i;

  crypto_enable();

  /* Reset the AES/SHA cryptoprocessor */
  REG(SYS_CTRL_SRSEC) |= SYS_CTRL_SRSEC_AES;
  for(i = 0; i < 16; i++);
  REG(SYS_CTRL_SRSEC) &= ~SYS_CTRL_SRSEC_AES;
}
/*---------------------------------------------------------------------------*/
void
crypto_enable(void)
{
  /* Enable the clock for the AES/SHA cryptoprocessor */
  REG(SYS_CTRL_RCGCSEC) |= SYS_CTRL_RCGCSEC_AES;
  REG(SYS_CTRL_SCGCSEC) |= SYS_CTRL_SCGCSEC_AES;
  REG(SYS_CTRL_DCGCSEC) |= SYS_CTRL_DCGCSEC_AES;
}
/*---------------------------------------------------------------------------*/
void
crypto_disable(void)
{
  /* Gate the clock for the AES/SHA cryptoprocessor */
  REG(SYS_CTRL_RCGCSEC) &= ~SYS_CTRL_RCGCSEC_AES;
  REG(SYS_CTRL_SCGCSEC) &= ~SYS_CTRL_SCGCSEC_AES;
  REG(SYS_CTRL_DCGCSEC) &= ~SYS_CTRL_DCGCSEC_AES;
}

/** @} */
