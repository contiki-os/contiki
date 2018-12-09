/*
 * Copyright (c) 2017, Alexander Lityagin   alexraynepe196@gmail.com
 *                                          alexraynepe196@hotbox.ru
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
 * This file ports cc2538/crypto.h API for cc26xx
 *
 * Author: Alexander Lityagin <alexraynepe196@gmail.com>
 */
/**
 * \addtogroup cc26xx-crypto
 * @{
 *
 * \file
 * Implementation of the cc26xx AES cryptoprocessor driver
 * @{
 *
 * \file
 * Header file for the cc26xx AES cryptoprocessor driver
 */
#ifndef CC26XX_CRYPTO_H_
#define CC26XX_CRYPTO_H_

#include <stdbool.h>
#include <contiki.h>
/*---------------------------------------------------------------------------*/
/** \name Crypto drivers return codes
 * @{
 */
typedef enum {
    CRYPTO_PENDING                  = (-1)
  , CRYPTO_SUCCESS                  = 0
  , CRYPTO_OK                       = CRYPTO_SUCCESS
  , CRYPTO_INVALID_PARAM            = 1
  , CRYPTO_NULL_ERROR               = 2
  , CRYPTO_RESOURCE_IN_USE          = 3
  //, CRYPTO_DMA_BUS_ERROR            = 4
} CRYPTO_Error;

/** @} */
/*---------------------------------------------------------------------------*/
/** \name Crypto macros
 * @{
 */

/** \brief Indicates whether the AES/SHA cryptoprocessor is enabled
 * \return Boolean value indicating whether the AES/SHA cryptoprocessor is
 * enabled
 */
#define CRYPTO_IS_ENABLED() (crypto_is_enabled())

/** @} */
/*---------------------------------------------------------------------------*/
/** \name Crypto functions
 * @{
 */

/** \brief Enables and resets the AES cryptoprocessor.
 * if notification process registered, also enables crypto IRQ
 */
void crypto_init(void);

/** \brief Enables the AES cryptoprocessor
 */
void crypto_enable(void);

/** \brief checks AES CRYPTO module is powered on and clocked
 */
bool crypto_is_enabled(void);

/** \brief Disables the AES cryptoprocessor
 * \note Call this function to save power when the cryptoprocessor is unused.
 */
void crypto_disable(void);

void crypto_cleanup();



#ifndef PROCESS_EVENT_CRYPTO
#define PROCESS_EVENT_CRYPTO    55
#endif

/** \brief Registers a process to be notified of the completion of a crypto
 * operation.
 * \param p Process to be polled upon IRQ.
 *          if process not provided, perform  broadcast event PROCESS_EVENT_CRYPTO,
 *              with crypto status
 * \note This function is only supposed to be called by the crypto drivers.
 */
void crypto_register_process_notification(struct process* p);

/** @} */

#endif /* CRYPTO_H_ */

/**
 * @}
 * @}
 */
