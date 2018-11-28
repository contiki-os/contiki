/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
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

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/*---------------------------------------------------------------------------*/
/* The interval at which we toggle the "operating" LED (in seconds)*/
#define BLINK_INTERVAL                  3
/*
 * The interval we use to dump statistics (in units of BLINK_INTERVAL, or, if
 * that is 0, in seconds).
 */
#define STAT_INTERVAL                   10
/*
 * Enable printing the NAT64 table with the statistics. Works only if
 * STAT_INTERVAL is set.
 * To translate the protocol numbers, see core/net/ip64/ip64.c.
 */
#define PRINT_NAT64_TABLE               1

/* Enable printing the IP configuration that we got via DHCP or SLIP. */
#define PRINT_IP_CONF                   1

/* Enable printing the routers own IP addresses in the RPL DAG on startup. */
#define PRINT_LOCAL_IPS                 1

/* Enable printing additional low level radio statistics. */
#define PRINT_RIME_STATS                0
#if PRINT_RIME_STATS
#define RIMESTATS_CONF_ENABLED          1
#endif
/*---------------------------------------------------------------------------*/
/*
 * Web server configuration. To enable or choose a different webserver, see the
 * Makefile.
 */
/* Configures the internal web server */
#define WEBSERVER_CONF_ROUTE_LINKS      1
#define WEBSERVER_CONF_LOADTIME         1
#define WEBSERVER_CONF_FILESTATS        1
#define WEBSERVER_CONF_NEIGHBOR_STATUS  1

/* Adding links requires a larger RAM buffer. To avoid static allocation
 * the stack can be used for formatting; however tcp retransmissions
 * and multiple connections can result in garbled segments.
 */
#if WEBSERVER_CONF_ROUTE_LINKS
#define BUF_USES_STACK                  0
#endif

/* Size of the buffer for the html page. */
#define BUF_SIZE                        256
/*---------------------------------------------------------------------------*/
/* Configures the webserver under apps/webserver */
#define WEBSERVER_CONF_STATUSPAGE       1
#define WEBSERVER_CONF_CFS_CONNS        2
/*---------------------------------------------------------------------------*/
/* Webservers require bigger than standard values for these parameters. */
#define UIP_CONF_RECEIVE_WINDOW         128
#define UIP_CONF_BUFFER_SIZE            900
#define UIP_CONF_TCP_MSS                128
/*---------------------------------------------------------------------------*/
/* Radio configuration */
#define PLATFORM_CONF_USE_CC1200        1
/*---------------------------------------------------------------------------*/
/*
 * Configure the UIP fallback interface, which will be used for all traffic for
 * which we don't know the explicit route to the destination, i.e. everything
 * that is not in our RPL net.
 */
#if WITH_SLIP
#define UIP_FALLBACK_INTERFACE                  slip_interface
#define DBG_CONF_SLIP_MUX 1
#else
#define UIP_FALLBACK_INTERFACE                  ip64_eth_interface
#endif

#endif /* PROJECT_CONF_H_ */

