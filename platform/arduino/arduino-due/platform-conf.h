/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
#include "compiler.h"

#ifndef PLATFORM_CONF_H_
#define PLATFORM_CONF_H_

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ---------------------------- POWER SAVE MODE -------------------------- */
/* Enable Standard PSM for the current node. */
#define WITH_STANDARD_PSM
/* Enable Advanced PSM for the current node. */
//#define WITH_MULTI_HOP_PSM
/* Enable soft beaconing for smart doze scheduling. */
//#define WITH_SOFT_BEACON_GENERATION
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ------------------------ LED DEBUG SUPPORT  -------------------------- */
/* Configure operation with LED Message Debug */
#define WITH_LED_DEBUGGING
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ----------------------------- USB SUPPORT ----------------------------- */
/* Configure operation with USB Host support */
#define WITH_USB_SUPPORT
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ------------------------- WLAN 802.11 SUPPORT ------------------------- */
/* Configure operation with AR9170 WiFi support. */
#define WITH_AR9170_WIFI_SUPPORT
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* Enable IPv6 */
#define WITH_UIP6			1 

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* --------------------------- WITH SLIP SUPPORT ------------------------- */
//#define WITH_SLIP
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/* We define here the maximum MAC packet size. 
 * Since we are using 802.11 we accept packets
 * larger than the default Contiki 802.15.4 ones
 */
#define PACKETBUF_CONF_SIZE		512

#define STRING_EOL    "\r"
#define STRING_HEADER "-- Contiki OS on SAM3X8E --\r\n" \
"-- "BOARD_NAME" --\r\n" \
"-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

/* Platform-dependent definitions */
#define CC_CONF_REGISTER_ARGS          0
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1
#define CC_CONF_INLINE                 inline

#define CCIF
#define CLIF

typedef unsigned short uip_stats_t;

#define UART1_CONF_TX_WITH_INTERRUPT    0
#define WITH_SERIAL_LINE_INPUT			1

/* A trick to resolve a compilation error with IAR. */
#ifdef __ICCARM__
#define UIP_CONF_DS6_AADDR_NBU			1
#endif

/* 
 * rtimer_second = 10500000;
 * A tick is ~95nanoseconds 
 */
#define RT_CONF_RESOLUTION              1

typedef unsigned long long rtimer_clock_t;

/* 32 bit main [etimer,ctimer] clock range */
typedef unsigned long clock_time_t;

/* 1msec main clock resolution */
#define CLOCK_CONF_SECOND 1000


#define RTIMER_CLOCK_LT(a,b)     ((signed short)((a)-(b)) < 0)

/* LEDs ports MB8xxx */
#define LED_ORANGE				LED0_GPIO

#define UIP_ARCH_ADD32           1
#define UIP_ARCH_CHKSUM          0

#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN

/* Needed by Rime [JOHN] */
//typedef unsigned char int8u;

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ------------------------------ SENSORS SUPPORT -------------------------- */
#define WITH_TILT_SENSOR
//#define WITH_G_SENSOR
//#define WITH_SOUND_SENSOR

/* For CFS - not originally included here. */
#define ROM_ERASE_UNIT_SIZE  512
#define XMEM_ERASE_UNIT_SIZE (64*1024L)


#define CFS_CONF_OFFSET_TYPE    long


/* Use the first 64k of external flash for node configuration */
#define NODE_ID_XMEM_OFFSET     (0 * XMEM_ERASE_UNIT_SIZE)

/* Use the second 64k of external flash for codeprop. */
#define EEPROMFS_ADDR_CODEPROP  (1 * XMEM_ERASE_UNIT_SIZE)

#define CFS_XMEM_CONF_OFFSET    (2 * XMEM_ERASE_UNIT_SIZE)
#define CFS_XMEM_CONF_SIZE      (1 * XMEM_ERASE_UNIT_SIZE)

#define CFS_RAM_CONF_SIZE 4096

#endif /* PLATFORM-CONF_H_ */