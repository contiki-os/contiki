/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *         uIP Netfilter
 * \author Víctor Ariño <victor.arino@tado.com>
 */

/*
 * Copyright (c) 2014, tado° GmbH.
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

#ifndef UIP_NETFILTER_H_
#define UIP_NETFILTER_H_

typedef struct uip_netfilter_filter_st {
  struct uip_netfilter_filter_st *next;
  uint8_t proto;
  uint16_t srcport, destport;
  uip_ipaddr_t *srcaddr, *dstaddr;
  void (*callback)(void);
} uip_netfilter_filter_t;

/** \brief Input filter */
#define UIP_NETFILTER_INPUT  0
/** \brief Output filter */
#define UIP_NETFILTER_OUTPUT 1

/**
 * \brief Register a network filter
 *
 * \param dir     Direction of the filter: \sa UIP_NETFILTER_INPUT,
 *                \sa UIP_NETFILTER_OUTPUT.
 * \param filter  Filter to register. Note it must be a static variable due
 *                to implementation.
 */
void
uip_netfilter_register(uint8_t dir, uip_netfilter_filter_t *filter);

/**
 * \brief Unregister the filter
 *
 * \param dir     Direction of the filter: \sa UIP_NETFILTER_INPUT,
 *                \sa UIP_NETFILTER_OUTPUT.
 * \param filter  Filter to unregister
 */
void
uip_netfilter_unregister(uint8_t dir, uip_netfilter_filter_t *filter);

/**
 * \internal
 * \brief Hook for the tcpip input
 */
void
uip_netfilter_input(void);

#endif /* UIP_NETFILTER_H_ */
/** @} */
