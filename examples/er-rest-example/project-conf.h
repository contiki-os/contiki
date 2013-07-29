/*
 * Copyright (c) 2013, Matthias Kovatsch
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

#ifndef __PROJECT_ERBIUM_CONF_H__
#define __PROJECT_ERBIUM_CONF_H__

/* Some platforms have weird includes. */
#undef IEEE802154_CONF_PANID

/* Disabling RDC for demo purposes. Core updates often require more memory. */
/* For projects, optimize memory and enable RDC again. */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver

/* Increase rpl-border-router IP-buffer when using more than 64. */
#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE    64

/* Estimate your header size, especially when using Proxy-Uri. */
/*
#undef COAP_MAX_HEADER_SIZE
#define COAP_MAX_HEADER_SIZE    70
*/

/* The IP buffer size must fit all other hops, in particular the border router. */
/*
#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    1280
*/

/* Multiplies with chunk size, be aware of memory constraints. */
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS   4

/* Must be <= open transaction number, default is COAP_MAX_OPEN_TRANSACTIONS-1. */
/*
#undef COAP_MAX_OBSERVERS
#define COAP_MAX_OBSERVERS      2
*/

/* Filtering .well-known/core per query can be disabled to save space. */
/*
#undef COAP_LINK_FORMAT_FILTERING
#define COAP_LINK_FORMAT_FILTERING      0
*/

/* Save some memory for the sky platform. */
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS     10
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES   10

/* Reduce 802.15.4 frame queue to save RAM. */
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM       4

#undef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG	1

#endif /* __PROJECT_ERBIUM_CONF_H__ */
