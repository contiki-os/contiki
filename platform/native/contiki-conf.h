/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * This file is part of the Contiki operating system.
 *
 * @(#)$Id: contiki-conf.h,v 1.12 2009/09/09 21:11:24 adamdunkels Exp $
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <inttypes.h>

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1
/*#define CC_CONF_INLINE                 inline*/

#define CCIF
#define CLIF

typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef  int32_t s32_t;
typedef unsigned short uip_stats_t;

#define UIP_CONF_UDP             1
#define UIP_CONF_MAX_CONNECTIONS 40
#define UIP_CONF_MAX_LISTENPORTS 40
#define UIP_CONF_BUFFER_SIZE     420
#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN
#define UIP_CONF_TCP       1
#define UIP_CONF_TCP_SPLIT       1
#define UIP_CONF_LOGGING         0
#define UIP_CONF_UDP_CHECKSUMS   1

#if UIP_CONF_IPV6
#define UIP_CONF_IPV6_CHECKS     1
#define UIP_CONF_IPV6_QUEUE_PKT  1
#define UIP_CONF_IPV6_REASSEMBLY 0
#define UIP_CONF_NETIF_MAX_ADDRESSES  3
#define UIP_CONF_ND6_MAX_PREFIXES     3
#define UIP_CONF_ND6_MAX_NEIGHBORS    4
#define UIP_CONF_ND6_MAX_DEFROUTERS   2
#define UIP_CONF_ICMP6           1
#endif /* UIP_CONF_ICMP6 */

typedef unsigned long clock_time_t;

#define CLOCK_CONF_SECOND 1000

#define LOG_CONF_ENABLED 1

/* Not part of C99 but actually present */
int strcasecmp(const char*, const char*);

#endif /* __CONTIKI_CONF_H__ */
