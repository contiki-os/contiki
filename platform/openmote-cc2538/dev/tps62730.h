/*
 * Copyright (c) 2014, OpenMote Technologies, S.L.
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
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup openmote-cc2538
 * @{
 *
 * \defgroup openmote-tps62730 TPS62730 voltage regulator
 *
 * Driver for the TPS62730 voltage regulator, to enable power from
 * the battery voltage (bypass, Vout=Vin, Iq < 1uA) or through the
 * buck regulator (on, Vout=2.1V, Iq = 30uA)
 * @{
 *
 * \file
 * Driver for the TPS62730 voltage regulator
 *
 * \author
 * Pere Tuset <peretuset@openmote.com>
 */
/*---------------------------------------------------------------------------*/
#ifndef TPS62730_H_
#define TPS62730_H_
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the TPS62730 voltage regulator in bypass mode
 */
void tps62730_init(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Set TPS62730 to on, Vout = 2.2V, Iq = 30 uA
 */
void tps62730_on(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief Set TPS62730 to bypass, Vout = Vin, Iq < 1 uA
 */
void tps62730_bypass(void);
/*---------------------------------------------------------------------------*/
#endif /* TPS62730_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
