/*
 * Copyright (c) 2016, University of Bristol - http://www.bristol.ac.uk
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-trng CC13xx/CC26xx Random Number Generator
 *
 * Driver for the CC13xx/CC26xx Random Number Generator
 *
 * @{
 *
 * \file
 *
 * Header file for the CC13xx/CC26xx TRNG driver
 */
/*---------------------------------------------------------------------------*/
#ifndef SOC_TRNG_H_
#define SOC_TRNG_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define SOC_TRNG_RAND_ASYNC_REQUEST_ERROR 0 /**< Async request rejected */
#define SOC_TRNG_RAND_ASYNC_REQUEST_OK    1 /**< Async request accepted */
/*---------------------------------------------------------------------------*/
#define SOC_TRNG_REFILL_CYCLES_MIN 0x00000100
#define SOC_TRNG_REFILL_CYCLES_MAX 0x00000000
/*---------------------------------------------------------------------------*/
/**
 * \brief Pointer to a callback to be provided as an argument to
 * soc_trng_rand_asynchronous()
 */
typedef void (*soc_trng_callback_t)(uint64_t rand);
/*---------------------------------------------------------------------------*/
/**
 * \name TRNG functions
 * @{
 */

/**
 * \brief Returns a minimum entropy random number
 * \return The random number
 *
 * This function is synchronous. This function will make sure the PERIPH PD is
 * powered and the TRNG is clocked. The function will then configure the TRNG
 * to generate a random number of minimum entropy. These numbers are not
 * suitable for cryptographic usage, but their generation is very fast.
 *
 * If a high-entropy random number is currently being generated, this function
 * will return a cached random number. The cache is of configurable size and
 * can hold a maximum SOC_TRNG_CONF_CACHE_LEN numbers. If the cache gets
 * emptied while high-entropy generation is in progress (e.g. because a
 * function requested many random numbers in a row), this function will return
 * 0. Care must therefore be taken when the return value is 0, which can also
 * be a valid random number.
 *
 * This function can be safely called from within an interrupt context.
 */
uint64_t soc_trng_rand_synchronous();

/**
 * \brief Initialise the CC13xx/CC26xx TRNG driver
 */
void soc_trng_init(void);

/**
 * \brief Request a 64-bit, configurable-entropy random number
 * \param samples Controls the entropy generated for the random number
 * \param cb A callback function to be called when the generation is complete
 * \retval SOC_TRNG_RAND_ASYNC_REQUEST_ERROR There was an error adding request.
 * \retval SOC_TRNG_RAND_ASYNC_REQUEST_OK Request successfully registered
 *
 * This function is asynchronous, it will start generation of a random number
 * and will return. The caller must provide a callback that will be called when
 * the generation is complete. This callback must either use the random number
 * immediately or store it, since it will not be possible to retrieve it again
 * later form the soc-trng module.
 *
 * Only one generation can be active at any given point in time. If this
 * function gets called when a generation is already in progress, it will
 * return SOC_TRNG_RAND_ASYNC_REQUEST_ERROR.
 *
 * The function will configure the TRNG to generate entropy by sampling the
 * FROs for a number clock cycles controlled by the samples argument. The 8 LS
 * bits of this argument will be truncated by CC13xxware/CC26xxware and the
 * resulting number of clock cycles will be (samples >> 8) * 2^8. Increasing
 * the value of this argument increases entropy, but it also increases latency.
 * Maximum entropy can be generated by passing SOC_TRNG_REFILL_CYCLES_MAX, but
 * this will take approximately 350ms. Consult the chip's technical reference
 * manual for advice on what would constitute sufficient entropy for random
 * numbers meant to be used for crypto.
 *
 * While this function is executing, calls to soc_trng_rand_synchronous() will
 * return cached random numbers.
 *
 * This function is not re-entrant and must not be called from an interrupt
 * context.
 */
uint8_t soc_trng_rand_asynchronous(uint32_t samples, soc_trng_callback_t cb);
/** @} */
/*---------------------------------------------------------------------------*/
#endif /* SOC_TRNG_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
