/*
 * Copyright (c) 2015, Hasso-Plattner-Institut.
 * All rights reserved.
 *
 * Port to cc26xx:
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
 * This file is port of cc2538/aes driver API for cc26xx
 */
/**
 * \addtogroup cc26xx-aes
 * @{
 *
 * \defgroup cc26xx-aes-128 CC26xx AES-128
 *
 * AES-128 driver for the CC26xx SoC
 * @{
 *
 * \file
 *         Header file of the AES-128 driver for the CC26xx SoC
 * \author Alexander Lityagin   alexraynepe196@gmail.com
 *                              alexraynepe196@hotbox.ru
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */
#ifndef CC26xx_AES_128_H_
#define CC26xx_AES_128_H_

#include "lib/aes-128.h"
/*---------------------------------------------------------------------------*/
#ifdef CC26XX_AES_128_CONF_KEY_AREA
#define CC26XX_AES_128_KEY_AREA         CC26XX_AES_128_CONF_KEY_AREA
#else
#define CC26XX_AES_128_KEY_AREA         0
#endif
/*---------------------------------------------------------------------------*/
extern const struct aes_128_driver cc26xx_aes_128_driver;

#endif /* CC2538_AES_128_H_ */

/**
 * @}
 * @}
 */
