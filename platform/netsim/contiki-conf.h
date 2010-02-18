#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define NETSTACK_CONF_RADIO   ethernode_driver

#define XMAC_CONF_ON_TIME RTIMER_SECOND
#define XMAC_CONF_OFF_TIME RTIMER_SECOND * 2

/*
 * Copyright (c) 2003, Adam Dunkels.
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
 * This file is part of the Contiki desktop OS
 *
 * $Id: contiki-conf.h,v 1.13 2010/02/18 23:10:06 adamdunkels Exp $
 *
 */

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL

#define CC_CONF_VA_ARGS                1

#define CCIF
#define CLIF

/*------------------------------------------------------------------------------*/
/**
 * \defgroup uipopttypedef uIP type definitions
 * @{
 */

#include <inttypes.h>

/**
 * The 8-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * char" works for most compilers.
 */
typedef uint8_t u8_t;

/**
 * The 16-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * short" works for most compilers.
 */
typedef uint16_t u16_t;

/**
 * The 32-bit unsigned data type.
 *
 * This may have to be tweaked for your particular compiler. "unsigned
 * short" works for most compilers.
 */
typedef uint32_t u32_t;

/**
 * The 32-bit signed data type.
 *
 * This may have to be tweaked for your particular compiler. "signed
 * short" works for most compilers.
 */
typedef int32_t s32_t;

/**
 * The statistics data type.
 *
 * This datatype determines how high the statistics counters are able
 * to count.
 */
typedef unsigned short uip_stats_t;

/** @} */


/*------------------------------------------------------------------------------*/

typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 1000


/*------------------------------------------------------------------------------*/

#define PACKETBUF_CONF_SIZE        128
#define PACKETBUF_CONF_HDR_SIZE    32
#define QUEUEBUF_CONF_STATS        1

#define UIP_CONF_UIP_IP4ADDR_T_WITH_U32 1

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_MAX_CONNECTIONS 40
#define UIP_CONF_MAX_LISTENPORTS 40
#define UIP_CONF_BUFFER_SIZE     120

#define UIP_CONF_BYTE_ORDER      UIP_LITTLE_ENDIAN

#define UIP_CONF_BROADCAST	 1

#define UIP_CONF_IP_FORWARD      1

/* TCP splitting does not work well with multi hop routing. */
#define UIP_CONF_TCP_SPLIT       0

#define UIP_CONF_LOGGING         1

#define UIP_CONF_UDP_CHECKSUMS   0

#define LOADER_CONF_ARCH "loader/dlloader.h"

#endif /* __CONTIKI_CONF_H__ */
