/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 * \file  platform/seedeye/contiki-conf.h
 * \brief Contiki configuration file for the SEEDEYE port.
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

#include <inttypes.h>

#include "platform-conf.h"

#define CCIF
#define CLIF

typedef uint8_t  u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int32_t  s32_t;
typedef uint16_t uip_stats_t;

typedef uint32_t clock_time_t;

typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_DIFF(a,b) ((int32_t)((a)-(b)))

#define RF_CHANNEL                              13

#define PROFILE_CONF_ON 0
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON 1
#endif /* ENERGEST_CONF_ON */

#ifdef NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_NETWORK                   sicslowpan_driver
#define NETSTACK_CONF_FRAMER                    framer_802154
#define NETSTACK_CONF_MAC                       nullmac_driver
#define NETSTACK_CONF_RDC                       nullrdc_driver
#define NETSTACK_CONF_RADIO                     mrf24j40_driver
#define LINKADDR_CONF_SIZE                      8
#else
#define NETSTACK_CONF_NETWORK                   rime_driver
#define NETSTACK_CONF_FRAMER                    framer_802154
#define NETSTACK_CONF_MAC                       nullmac_driver
#define NETSTACK_CONF_RDC                       nullrdc_driver
#define NETSTACK_CONF_RADIO                     mrf24j40_driver
#define LINKADDR_CONF_SIZE                      2
#endif

#define RDC_CONF_HARDWARE_CSMA                  1

#ifdef NETSTACK_CONF_WITH_IPV6
#define UIP_CONF_ROUTER                         1

/* IPv6 configuration options */
#define NETSTACK_CONF_WITH_IPV6                           1
#define NBR_TABLE_CONF_MAX_NEIGHBORS                    20 /* number of neighbors */
#define UIP_CONF_MAX_ROUTES                  20 /* number of routes */
#define UIP_CONF_ND6_SEND_RA                    0
#define UIP_CONF_ND6_REACHABLE_TIME             600000
#define UIP_CONF_ND6_RETRANS_TIMER              10000


#define UIP_CONF_BUFFER_SIZE                    240

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                       16
#endif /* QUEUEBUF_CONF_NUM */

/* UDP configuration options */
#define UIP_CONF_UDP                            1
#define UIP_CONF_UDP_CHECKSUMS                  1
#define UIP_CONF_UDP_CONNS                      10

/* 6lowpan options (for ipv6) */
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD   63
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1
#define SICSLOWPAN_CONF_MAXAGE                  8
#endif /* SICSLOWPAN_CONF_FRAG */

/* General configuration options */
#define UIP_CONF_STATISTICS                     0
#define UIP_CONF_LOGGING                        0
#define UIP_CONF_BROADCAST                      1
#define UIP_CONF_LLH_LEN                        0
#define UIP_CONF_LL_802154                      1
#endif

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#endif /* CONTIKI_CONF_H */
