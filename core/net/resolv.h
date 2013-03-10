/**
 * \file
 * uIP DNS resolver code header file.
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2002-2003, Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 *
 */
#ifndef __RESOLV_H__
#define __RESOLV_H__

#include "contiki.h"
#include "uip.h"

/** If RESOLV_CONF_SUPPORTS_MDNS is set, then queries
 *  for domain names in the `local` TLD will use MDNS and
 *  will respond to MDNS queries for this device's hostname,
 *  as described by draft-cheshire-dnsext-multicastdns.
 */
#ifndef RESOLV_CONF_SUPPORTS_MDNS
#define RESOLV_CONF_SUPPORTS_MDNS     (1)
#endif

/**
 * Event that is broadcasted when a DNS name has been resolved.
 */
CCIF extern process_event_t resolv_event_found;

/* Functions. */
CCIF void resolv_conf(const uip_ipaddr_t * dnsserver);

CCIF uip_ipaddr_t *resolv_getserver(void);

enum {
  /** Hostname is fresh and usable. This response is cached and will eventually
   *  expire to RESOLV_STATUS_EXPIRED.*/
  RESOLV_STATUS_CACHED = 0,

  /** Hostname was not found in the cache. Use resolv_query() to look it up. */
  RESOLV_STATUS_UNCACHED,

  /** Hostname was found, but it's status has expired. The address returned
   *  should not be used. Use resolv_query() to freshen it up.
   */
  RESOLV_STATUS_EXPIRED,

  /** The server has returned a not-found response for this domain name.
   *  This response is cached for the period described in the server.
   *  You may issue a new query at any time using resolv_query(), but
   *  you will generally want to wait until this domain's status becomes
   *  RESOLV_STATUS_EXPIRED.
   */
  RESOLV_STATUS_NOT_FOUND,

  /** This hostname is in the process of being resolved. Try again soon. */
  RESOLV_STATUS_RESOLVING,

  /** Some sort of server error was encountered while trying to look up this
   *  record. This response is cached and will eventually expire to
   *  RESOLV_STATUS_EXPIRED.
   */
  RESOLV_STATUS_ERROR,
};

typedef uint8_t resolv_status_t;

CCIF resolv_status_t resolv_lookup(const char *name, uip_ipaddr_t ** ipaddr);

CCIF void resolv_query(const char *name);

#if RESOLV_CONF_SUPPORTS_MDNS
CCIF void resolv_set_hostname(const char *hostname);

CCIF const char *resolv_get_hostname(void);
#endif

PROCESS_NAME(resolv_process);

#endif /* __RESOLV_H__ */
