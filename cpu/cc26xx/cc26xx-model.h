/*
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-models CC26xx models
 *
 * The CC26xx comes out in various flavours. Most notable within the context
 * of this Contiki port: The CC2630 with IEEE (but no BLE) support and the
 * CC2650 with IEEE and BLE support.
 *
 * This port supports both models and will automatically turn off the BLE code
 * if the CC2630 is selected.
 *
 * @{
 */
/**
 * \file
 * Header file with definitions relating to various CC26xx variants
 */
/*---------------------------------------------------------------------------*/
#ifndef CC26XX_MODEL_H_
#define CC26XX_MODEL_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
/*---------------------------------------------------------------------------*/
#ifdef CC26XX_MODEL_CONF_CPU_VARIANT
#define CC26XX_MODEL_CPU_VARIANT CC26XX_MODEL_CONF_CPU_VARIANT
#else
#define CC26XX_MODEL_CPU_VARIANT 2650
#endif

#if (CC26XX_MODEL_CPU_VARIANT != 2630) && (CC26XX_MODEL_CPU_VARIANT != 2650)
#error Incorrect CC26xx variant selected.
#error Check the value of CC26XX_MODEL_CONF_CPU_VARIANT
#error Supported values: 2630 and 2650
#endif
/*---------------------------------------------------------------------------*/
#endif /* CC26XX_MODEL_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
