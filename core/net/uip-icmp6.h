/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         ICMPv6 echo request and error messages (RFC 4443)
 * \author Julien Abeille <jabeille@cisco.com> 
 * \author Mathilde Durvy <mdurvy@cisco.com>
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 */


#ifndef __ICMP6_H__
#define __ICMP6_H__

#include "net/uip.h"


/** \name ICMPv6 message types */
/** @{ */
#define ICMP6_DST_UNREACH                 1	/**< dest unreachable */
#define ICMP6_PACKET_TOO_BIG	            2	/**< packet too big */
#define ICMP6_TIME_EXCEEDED	            3	/**< time exceeded */
#define ICMP6_PARAM_PROB	               4	/**< ip6 header bad */
#define ICMP6_ECHO_REQUEST              128  /**< Echo request */
#define ICMP6_ECHO_REPLY                129  /**< Echo reply */

#define ICMP6_RS                        133  /**< Router Solicitation */
#define ICMP6_RA                        134  /**< Router Advertisement */
#define ICMP6_NS                        135  /**< Neighbor Solicitation */
#define ICMP6_NA                        136  /**< Neighbor advertisement */
#define ICMP6_REDIRECT                  137  /**< Redirect */

#define ICMP6_RPL                       155  /**< RPL */
/** @} */


/** \name ICMPv6 Destination Unreachable message codes*/
/** @{ */
#define ICMP6_DST_UNREACH_NOROUTE         0 /**< no route to destination */
#define ICMP6_DST_UNREACH_ADMIN	         1 /**< administratively prohibited */
#define ICMP6_DST_UNREACH_NOTNEIGHBOR     2 /**< not a neighbor(obsolete) */
#define ICMP6_DST_UNREACH_BEYONDSCOPE     2 /**< beyond scope of source address */
#define ICMP6_DST_UNREACH_ADDR	         3 /**< address unreachable */
#define ICMP6_DST_UNREACH_NOPORT          4 /**< port unreachable */
/** @} */

/** \name ICMPv6 Time Exceeded message codes*/
/** @{ */
#define ICMP6_TIME_EXCEED_TRANSIT         0 /**< ttl==0 in transit */
#define ICMP6_TIME_EXCEED_REASSEMBLY      1 /**< ttl==0 in reass */
/** @} */

/** \name ICMPv6 Parameter Problem message codes*/
/** @{ */
#define ICMP6_PARAMPROB_HEADER            0 /**< erroneous header field */
#define ICMP6_PARAMPROB_NEXTHEADER        1 /**< unrecognized next header */
#define ICMP6_PARAMPROB_OPTION            2 /**< unrecognized option */
/** @} */

/** \brief Echo Request constant part length */
#define UIP_ICMP6_ECHO_REQUEST_LEN        4

/** \brief ICMPv6 Error message constant part length */
#define UIP_ICMP6_ERROR_LEN               4

/** \brief ICMPv6 Error message constant part */
typedef struct uip_icmp6_error{
  uint32_t param;
} uip_icmp6_error;

/** \name ICMPv6 RFC4443 Message processing and sending */
/** @{ */
/** \
 * brief Process an echo request 
 *
 * Perform a few checks, then send an Echo reply. The reply is 
 * built here.
  */
void
uip_icmp6_echo_request_input(void);

/**
 * \brief Send an icmpv6 error message
 * \param type type of the error message
 * \param code of the error message
 * \param type 32 bit parameter of the error message, semantic depends on error 
 */
void
uip_icmp6_error_output(uint8_t type, uint8_t code, uint32_t param); 

/**
 * \brief Send an icmpv6 message
 * \param dest destination address of the message
 * \param type type of the message
 * \param code of the message
 * \param payload_len length of the payload
 */
void
uip_icmp6_send(uip_ipaddr_t *dest, int type, int code, int payload_len);


/** @} */

#endif /*__ICMP6_H__*/
/** @} */

