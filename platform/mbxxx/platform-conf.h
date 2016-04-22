/**
 * \addtogroup platform
 * @{ */
/**
 * \defgroup mbxxx-platform The STM32W MBXXX platform
 *
 * The STM32W MBXXX platform.
 *
 * @{
 */

/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*          Platform-conf.h for MBXXX.
* \author
*          Stefano Pascali <stefano.pascali@st.com>
*          Marco Grella    <marco.grella@st.com>
*/
/*---------------------------------------------------------------------------*/

#ifndef PLATFORM_CONF_H_
#define PLATFORM_CONF_H_

#include PLATFORM_HEADER

#include <inttypes.h>
#include <string.h>             // For memcpm().

/* Platform-dependent definitions */
#define CC_CONF_REGISTER_ARGS          0
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE                 inline

#define CCIF
#define CLIF

typedef unsigned short uip_stats_t;

#define UART1_CONF_TX_WITH_INTERRUPT    0
#define WITH_SERIAL_LINE_INPUT      1

/* rtimer_second = 11719 */
#define RT_CONF_RESOLUTION                      2

/* A trick to resolve a compilation error with IAR. */
#ifdef __ICCARM__
#define UIP_CONF_DS6_AADDR_NBU      1
#endif

typedef unsigned long clock_time_t;

#define CLOCK_CONF_SECOND 1000

typedef unsigned long rtimer_clock_t;

#define RTIMER_CLOCK_DIFF(a,b)     ((signed long)((a)-(b)))

#define LEDS_CONF_RED_PIN     boardDescription->io->leds[1].gpioPin
#define LEDS_CONF_GREEN_PIN   boardDescription->io->leds[0].gpioPin
#define LEDS_CONF_PORT        boardDescription->io->leds[1].gpioPort

#define LEDS_CONF_RED     (1<<LEDS_CONF_RED_PIN)
#define LEDS_CONF_GREEN     (1<<LEDS_CONF_GREEN_PIN)

#define UIP_ARCH_ADD32           1
#define UIP_ARCH_CHKSUM          0

#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN
#define EEPROM_CONF_SIZE	8000

#endif /* PLATFORM_CONF_H_ */
/** @} */
/** @} */
