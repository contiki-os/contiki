/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#include "jsontree.h"
#define HTTPD_WS_CONF_USER_STATE struct jsontree_context json


/* #define JSON_WS_CONF_CALLBACK_PROTO "http" | "udp" | "cosm" */
#define JSON_WS_CONF_CALLBACK_PROTO "http"
#define JSON_WS_CONF_CALLBACK_PORT  80
#define JSON_WS_CONF_CALLBACK_INTERVAL 120

#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
/* #define NETSTACK_CONF_RDC     contikimac_driver */

#undef NULLRDC_CONF_802154_AUTOACK
#define NULLRDC_CONF_802154_AUTOACK 1

/* Reduce code size */
#undef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON 0

/* needs to be ~4 for fragmentation to work */
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 7
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES 7

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    140

/* #undef UIP_CONF_RECEIVE_WINDOW */
/* #define UIP_CONF_RECEIVE_WINDOW  35 */

#undef WEBSERVER_CONF_INBUF_SIZE
#define WEBSERVER_CONF_INBUF_SIZE 200

#undef WEBSERVER_CONF_OUTBUF_SIZE
#define WEBSERVER_CONF_OUTBUF_SIZE (UIP_TCP_MSS + 20 + 80)

#undef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS 3

#endif /* PROJECT_CONF_H_ */
