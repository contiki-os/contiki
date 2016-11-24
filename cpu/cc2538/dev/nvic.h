/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \defgroup cc2538-nvic cc2538 Nested Vectored Interrupt Controller
 *
 * Driver for the cc2538 NVIC controller
 * @{
 *
 * \file
 * Header file for the ARM Nested Vectored Interrupt Controller
 */
#ifndef NVIC_H_
#define NVIC_H_

#include "cc2538_cm3.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/** \name NVIC Constants and Configuration
 * @{
 */
#ifdef NVIC_CONF_VTABLE_ADDRESS
#define NVIC_VTABLE_ADDRESS     NVIC_CONF_VTABLE_ADDRESS
#else
extern void(*const vectors[])(void);
#define NVIC_VTABLE_ADDRESS     ((uint32_t)&vectors)
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/** \brief Initialises the NVIC driver */
void nvic_init();

#endif /* NVIC_H_ */

/**
 * @}
 * @}
 */
