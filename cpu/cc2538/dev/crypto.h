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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-crypto cc2538 AES/SHA cryptoprocessor
 *
 * Driver for the cc2538 AES/SHA cryptoprocessor
 * @{
 *
 * \file
 * Header file for the cc2538 AES/SHA cryptoprocessor driver
 */
#ifndef CRYPTO_H_
#define CRYPTO_H_

#include "contiki.h"
#include "dev/sys-ctrl.h"
#include "reg.h"
/*---------------------------------------------------------------------------*/
/** \name Crypto drivers return codes
 * @{
 */
#define CRYPTO_PENDING                (-1)
#define CRYPTO_SUCCESS                0
#define CRYPTO_INVALID_PARAM          1
#define CRYPTO_NULL_ERROR             2
#define CRYPTO_RESOURCE_IN_USE        3
#define CRYPTO_DMA_BUS_ERROR          4
/** @} */
/*---------------------------------------------------------------------------*/
/** \name Crypto macros
 * @{
 */

/** \brief Indicates whether the AES/SHA cryptoprocessor is enabled
 * \return Boolean value indicating whether the AES/SHA cryptoprocessor is
 * enabled
 */
#define CRYPTO_IS_ENABLED() (!!(REG(SYS_CTRL_RCGCSEC) & SYS_CTRL_RCGCSEC_AES))

/** @} */
/*---------------------------------------------------------------------------*/
/** \name Crypto functions
 * @{
 */

/** \brief Enables and resets the AES/SHA cryptoprocessor
 */
void crypto_init(void);

/** \brief Enables the AES/SHA cryptoprocessor
 */
void crypto_enable(void);

/** \brief Disables the AES/SHA cryptoprocessor
 * \note Call this function to save power when the cryptoprocessor is unused.
 */
void crypto_disable(void);

/** \brief Registers a process to be notified of the completion of a crypto
 * operation
 * \param p Process to be polled upon IRQ
 * \note This function is only supposed to be called by the crypto drivers.
 */
void crypto_register_process_notification(struct process *p);

/** @} */

#endif /* CRYPTO_H_ */

/**
 * @}
 * @}
 */
