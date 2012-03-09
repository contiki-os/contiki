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
 * @(#)$Id: contiki-conf.h,v 1.10 2010/03/18 20:11:54 dak664 Exp $
 */

#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include <inttypes.h>
#include <limits.h>

#define CC_CONF_REGISTER_ARGS          1
#define CC_CONF_FUNCTION_POINTER_ARGS  1
#define CC_CONF_FASTCALL
#define CC_CONF_VA_ARGS                1

#define CCIF
#define CLIF

/* These names are deprecated, use C99 names. */
typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef  int32_t s32_t;

typedef unsigned short uip_stats_t;


#if UIP_CONF_IPV6
/* The Windows build uses wpcap to connect to a host interface. It finds the interface by scanning for
 * an address, which can be specified here and overridden with the command line.
 * An ip4 or ip6 address can be used; this allows turning off the ip4 protocol on the interface.
 * The existing turorials use an ipv4 address, so we leave that as the default.
 * Non-windows builds don't use this define.
 */
//#define WPCAP_INTERFACE_ADDRESS    "fdfd::1"  //10.10.10.10 is the default (even for ipv6)

/* Minimal-net gets a 6 byte ethernet MAC assigned in uip.c, currently {0x00,0x06,0x98,0x00,0x02,0x32}
 * This gets converted to a link layer address of [fe80::206:98ff:fe00:232]
 * You could change the uip.c address when running multiple instances, however HARD_CODED_ADDRESS
 * can also specify a different MAC address if any of the last three bytes are non-zero. It can also
 * specify a prefix if any of the first four longs are nonzero. RPL builds use this to pass a prefix
 * to the border router and also to ensure it has a different link-layer address from the RPL end node.
 * HARD_CODED_ADDRESS can also be changed in /platform/minimal-net/contiki-main.c so as not to force complete
 * rebuilds when making multiple instances of a minimal-net client.
*/
//#define HARD_CODED_ADDRESS      "::10" //assign link-layer address fe80::ff:fe00:10, wait for RA or RPL prefix
//#define HARD_CODED_ADDRESS      "fdfd::" //assign prefix; address becomes fdfd::206:98ff:fe00:232
//#define HARD_CODED_ADDRESS      "fdfd::10" //assign prefix and ipv6 address fdfd::ff:fe00:10

/* The status.shtml page shows addresses, neighbors, and routes on ipv6 builds. Use this define to enable
 * the needed routines in httpd-cgi.c on the webserver6 build. The status page is present in
 * /apps/webserver/httpd-fs/ but not in the default /apps/webserver/httpd-fsdata.c file.
 * To include it run the PERL script /../../tools/makefsdata from the /apps/webserver/ directory.
 * NB: Webserver builds on all platforms will use the current httpd-fsdata.c file. The added 160 bytes
 * could overflow memory on the smaller platforms.
 */
#define WEBSERVER_CONF_STATUSPAGE   1

/* RPL currently works only on Windows. *nix would require converting the tun interface to two pcap tees. */ 
//#define UIP_CONF_IPV6_RPL           0
//#define RPL_BORDER_ROUTER           0
#endif   

#if UIP_CONF_IPV6_RPL
/* RPL motes use the uip.c link layer address or optionally the harded coded address (but without the prefix!)
 * Different instances can be made by changing the link layer portion of HARD_CODED_ADDRESS in contiki-main.c
 * Rename them to e.g. webserver6.10, webserver6.11, ...
 * They should all attach to a minimal-net rpl border that uses the same primary interface.
 * For multihop testing, configure intermediate notes as routers.
 */
#define HARD_CODED_ADDRESS      "bbbb::10"  //the prefix is ignored for a rpl node
#define UIP_CONF_ROUTER                 0
#define UIP_CONF_ND6_SEND_RA            0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#if RPL_BORDER_ROUTER
/* RPL border router accepts packets from the host through the fallback and directs them to
 * the primary interface. Thus the fallback and rpl dag prefix must be the same. The prefix of
 * the primary interface does not matter!
 * Rename this build to e.g. webrouter. Then on Windows create two loopback interfaces, bbbb:: and fdfd::
 * Attach the RPL end nodes to fdfd:: and the webrouter to fdfd:: with bbbb:: as the fallback.
 * Direct browser to bbbb::ff:fe00:1/status.html, bbbb::ff:fe00:10/status.html, bbbb::ff:fe00:20/status.html.
 * The traffic will go through the bbbb:: interface to the router, then out the fdfd:: interface to the end
 * nodes. The end nodes must be explicitly added as neighbors to avoid a blocking NS
 * netsh interface ipv6 add neighbor bbbb::ff:fe00:10 33-33-ff-33-44-10 interface=16 (# of the bbbb interface)
 * netsh interface ipv6 add neighbor bbbb::ff:fe00:20 33-33-ff-33-44-20 interface=16 (# of the bbbb interface)
 *
 * Instead of using the fdfd:: loopback it is also possible to attach the border router to another interface,
 * e.g. the jackdaw RNDIS <->  repeater. Then RPL will configure on the radio network and the RF motes will
 * be reached through bbbb::<mote link layer address>.
 * Possibly minimal-net RPL motes could also be added to this interface?
 * 
 */
#undef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER             1
//#define RPL_CONF_STATS              0
//#define UIP_CONF_BUFFER_SIZE	 1300
#undef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE rpl_interface
//#define WPCAP_FALLBACK_ADDRESS     "bbbb::1"  //bbbb::1 is the default fallback prefix
#undef HARD_CODED_ADDRESS
#define HARD_CODED_ADDRESS            "bbbb::1" //bbbb::ff:fe00:1 is the RPL border router default
//#define UIP_CONF_ND6_SEND_RA		0
//#define UIP_CONF_ND6_REACHABLE_TIME 600000
//#define UIP_CONF_ND6_RETRANS_TIMER  10000
#endif

#endif

#define UIP_CONF_MAX_LISTENPORTS      40
#define UIP_CONF_MAX_CONNECTIONS      40
#define UIP_CONF_BYTE_ORDER           UIP_LITTLE_ENDIAN
#define UIP_CONF_TCP_SPLIT            0
#define UIP_CONF_IP_FORWARD           0
#define UIP_CONF_LOGGING              0
#define UIP_CONF_UDP_CHECKSUMS        1

/* Not used but avoids compile errors while sicslowpan.c is being developed */
#define SICSLOWPAN_CONF_COMPRESSION       SICSLOWPAN_COMPRESSION_HC06

#define UIP_CONF_UDP                  1
#define UIP_CONF_TCP                  1

#if UIP_CONF_IPV6
#define UIP_CONF_IPV6_QUEUE_PKT       1
#define UIP_CONF_IPV6_CHECKS          1
#define UIP_CONF_IPV6_REASSEMBLY      1
//#define UIP_CONF_NETIF_MAX_ADDRESSES  5
//#define UIP_CONF_ND6_MAX_PREFIXES     3
//#define UIP_CONF_ND6_MAX_NEIGHBORS    40 
//#define UIP_CONF_ND6_MAX_DEFROUTERS   2
#define UIP_CONF_DS6_NBR_NBU     100
#define UIP_CONF_DS6_DEFRT_NBU   2
#define UIP_CONF_DS6_PREFIX_NBU  5
#define UIP_CONF_DS6_ROUTE_NBU   100
#define UIP_CONF_DS6_ADDR_NBU    10
#define UIP_CONF_DS6_MADDR_NBU   0
#define UIP_CONF_DS6_AADDR_NBU   0
#endif /* UIP_CONF_IPV6 */

typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 1000
#define INFINITE_TIME ULONG_MAX

#define LOG_CONF_ENABLED 1

/* Not part of C99 but actually present */
int strcasecmp(const char*, const char*);

#endif /* __CONTIKI_CONF_H__ */
