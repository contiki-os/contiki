/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 */
/**
 * \addtogroup nrf52832
 * @{
 *
 * \addtogroup nrf52832-dev Device drivers
 * @{
 *
 * \addtogroup nrf52832-rng Hardware random number generator
 * @{
 *
 * \file
 *         Random number generator routines exploiting the nRF52 hardware
 *         capabilities.
 *
 *         This file overrides core/lib/random.c.
 *
 * \author
 *         Wojciech Bober <wojciech.bober@nordicsemi.no>
 */
#include <stddef.h>
#include <nrf_drv_rng.h>
#include "app_error.h"
/*---------------------------------------------------------------------------*/
/**
 * \brief Generates a new random number using the nRF52 RNG.
 * \return a random number.
 */
unsigned short
random_rand(void)
{
  unsigned short value = 42;
  uint8_t available;
  ret_code_t err_code;

  do {
    nrf_drv_rng_bytes_available(&available);
  } while (available < sizeof(value));

  err_code = nrf_drv_rng_rand((uint8_t *)&value, sizeof(value));
  APP_ERROR_CHECK(err_code);

  return value;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the nRF52 random number generator.
 * \param seed Ignored. It's here because the function prototype is in core.
 *
 */
void
random_init(unsigned short seed)
{
  (void)seed;
  ret_code_t err_code = nrf_drv_rng_init(NULL);
  APP_ERROR_CHECK(err_code);
}
/**
 * @}
 * @}
 * @}
 */
