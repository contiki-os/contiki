/*
 * Copyright (c) 2014, CETIC.
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
 */

/**
 * \file
 *         Simple CoAP Library
 * \author
 *         6LBR Team <6lbr@cetic.be>
 */
#ifndef DEVICE_INFO_RESOURCE_H_
#define DEVICE_INFO_RESOURCE_H_

#include "contiki.h"
#include "coap-common.h"

#ifdef RES_CONF_DEVICE_MANUFACTURER_VALUE
#define RES_DEVICE_MANUFACTURER_VALUE RES_CONF_DEVICE_MANUFACTURER_VALUE
#else
#ifdef CONTIKI_TARGET_SKY
#define RES_DEVICE_MANUFACTURER_VALUE "Moteiv"
#elif defined CONTIKI_TARGET_Z1
#define RES_DEVICE_MANUFACTURER_VALUE "Zolertia"
#elif defined CONTIKI_TARGET_WISMOTE
#define RES_DEVICE_MANUFACTURER_VALUE "Arago"
#elif defined CONTIKI_TARGET_ECONOTAG
#define RES_DEVICE_MANUFACTURER_VALUE "Redwire"
#elif defined CONTIKI_TARGET_CC2538DK || defined CONTIKI_TARGET_SRF06_CC26XX
#define RES_DEVICE_MANUFACTURER_VALUE "TI"
#elif defined CONTIKI_TARGET_NATIVE
#define RES_DEVICE_MANUFACTURER_VALUE "Native"
#else
#define RES_DEVICE_MANUFACTURER_VALUE "Unknown"
#endif
#endif

#ifdef RES_CONF_DEVICE_MODEL_VALUE
#define RES_DEVICE_MODEL_VALUE RES_CONF_DEVICE_MODEL_VALUE
#else
#ifdef CONTIKI_TARGET_SKY
#define RES_DEVICE_MODEL_VALUE "Sky"
#elif defined CONTIKI_TARGET_Z1
#define RES_DEVICE_MODEL_VALUE "Z1"
#elif defined CONTIKI_TARGET_WISMOTE
#define RES_DEVICE_MODEL_VALUE "Wismote"
#elif defined CONTIKI_TARGET_ECONOTAG
#define RES_DEVICE_MODEL_VALUE "Econotag"
#elif defined CONTIKI_TARGET_CC2538DK
#define RES_DEVICE_MODEL_VALUE "CC2538DK"
#elif defined CONTIKI_TARGET_SRF06_CC26XX
#if BOARD_SENSORTAG
#define RES_DEVICE_MODEL_VALUE "Sensortag"
#else
#define RES_DEVICE_MODEL_VALUE "CC26XX"
#endif
#elif defined CONTIKI_TARGET_NATIVE
#define RES_DEVICE_MODEL_VALUE "Native"
#else
#define RES_DEVICE_MODEL_VALUE "Unknown"
#endif
#endif

#ifdef RES_CONF_DEVICE_MODEL_HW_VALUE
#define RES_DEVICE_MODEL_HW_VALUE RES_CONF_DEVICE_MODEL_HW_VALUE
#else
#define RES_DEVICE_MODEL_HW_VALUE "0"
#endif

#ifdef RES_CONF_DEVICE_MODEL_SW_VALUE
#define RES_DEVICE_MODEL_SW_VALUE RES_CONF_DEVICE_MODEL_SW_VALUE
#else
#define RES_DEVICE_MODEL_SW_VALUE CONTIKI_VERSION_STRING
#endif

#ifdef RES_CONF_DEVICE_SERIAL_VALUE
#define RES_DEVICE_SERIAL_VALUE RES_CONF_DEVICE_SERIAL_VALUE
#else
#define RES_DEVICE_SERIAL_VALUE "0"
#endif

#ifdef RES_CONF_DEVICE_TIME_VALUE
#define RES_DEVICE_TIME_VALUE RES_CONF_DEVICE_TIME_VALUE
#else
#define RES_DEVICE_TIME_VALUE clock_seconds()
#endif

#ifdef RES_CONF_DEVICE_UPTIME_VALUE
#define RES_DEVICE_UPTIME_VALUE RES_CONF_DEVICE_UPTIME_VALUE
#else
#define RES_DEVICE_UPTIME_VALUE clock_seconds()
#endif

#endif /* DEVICE_INFO_RESOURCE_H_ */
