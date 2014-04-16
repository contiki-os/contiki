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

#include "net/ip/uip.h"
#include "net/ip/uip-netfilter.h"
#include "lib/list.h"

#include <string.h>

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_TCP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])

/*---------------------------------------------------------------------------*/
LIST(netlist_input);
LIST(netlist_output);
/*---------------------------------------------------------------------------*/
/** \brief Compare port if not 0 */
#define COMPARE_PORT_IFF(filter_port, pkt_port) \
  ((*(uint16_t *)(filter_port)) == 0 \
   || (*(uint16_t *)(pkt_port)) == (*(uint16_t *)(filter_port)))
/** \brief Compare address if not NULL */
#define COMPARE_ADDR_IFF(filter_addr, pkt_addr) \
  ((filter_addr == NULL) \
   || uip_ipaddr_cmp(filter_addr, pkt_addr))
/*---------------------------------------------------------------------------*/
void
uip_netfilter_register(uint8_t dir, uip_netfilter_filter_t *f)
{
  if(dir == UIP_NETFILTER_INPUT) {
    list_add(netlist_input, f);
  } else {
    list_add(netlist_output, f);
  }
}
/*---------------------------------------------------------------------------*/
void
uip_netfilter_unregister(uint8_t dir, uip_netfilter_filter_t *f)
{
  if(dir == UIP_NETFILTER_INPUT) {
    list_remove(netlist_input, f);
  } else {
    list_remove(netlist_output, f);
  }
}
/*---------------------------------------------------------------------------*/
void
uip_netfilter_input(void)
{
  register struct uip_netfilter_filter_st *p;
  uint8_t *buff = NULL;
  for(p = list_head(netlist_input);
      p != NULL && uip_len > 0;
      p = list_item_next(p)) {
    if(p->proto == UIP_IP_BUF->proto) {
      if(p->proto == UIP_PROTO_TCP || p->proto == UIP_PROTO_UDP) {
        buff = (uint8_t *)UIP_TCP_UDP_BUF;
      } else {
        /* xxx not supported */
        continue;
      } if(COMPARE_PORT_IFF(&p->srcport, &buff[0])
           && COMPARE_PORT_IFF(&p->destport, &buff[2])
           && COMPARE_ADDR_IFF(p->srcaddr, &UIP_IP_BUF->srcipaddr)
           && COMPARE_ADDR_IFF(p->dstaddr, &UIP_IP_BUF->destipaddr)) {
        p->callback();
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
