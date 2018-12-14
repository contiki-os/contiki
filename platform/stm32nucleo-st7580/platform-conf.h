/*
 * Copyright (c) 2017, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup stm32nucleo-st7580
 * @{
 *
 * \defgroup stm32nucleo-st7580-peripherals User Button on STM32 Nucleo
 *
 * Defines some of the platforms capabilities
 * @{
 *
 * \file
 * Header file for the stm32nucleo-st7580 platform configuration
 */
/*---------------------------------------------------------------------------*/
#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__
/*---------------------------------------------------------------------------*/
#include <inttypes.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define PLATFORM_HAS_LEDS 1
#define PLATFORM_HAS_BUTTON 1
#define PLATFORM_HAS_RADIO 1

#define LEDS_GREEN  1 /*Nucleo LED*/
#define LEDS_RED    2

/*---------------------------------------------------------------------------*/
#define F_CPU                   32000000ul
#define RTIMER_ARCH_SECOND              32768
#define PRESCALER       ((F_CPU / (RTIMER_ARCH_SECOND * 2)))

#define UART1_CONF_TX_WITH_INTERRUPT        0
#define WITH_SERIAL_LINE_INPUT              1
#define TELNETD_CONF_NUMLINES               6
#define NETSTACK_CONF_RADIO                 st7580_radio_driver
#define NETSTACK_RADIO_MAX_PAYLOAD_LEN      255

/*---------------------------------------------------------------------------*/
/* define ticks/second for slow and fast clocks. Notice that these should be a
   power of two, eg 64,128,256,512 etc, for efficiency as POT's can be optimized
   well. */
#define CLOCK_CONF_SECOND             1000
/*---------------------------------------------------------------------------*/
typedef unsigned long clock_time_t;
/*---------------------------------------------------------------------------*/
#define CC_CONF_REGISTER_ARGS          0
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE                 inline

#define CCIF
#define CLIF
/*---------------------------------------------------------------------------*/
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef  int32_t s32_t;
typedef unsigned short uip_stats_t;
/*---------------------------------------------------------------------------*/
#define PACKETBUF_CONF_SIZE 255
#endif /* __PLATFORM_CONF_H__ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
