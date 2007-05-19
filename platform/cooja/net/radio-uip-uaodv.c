/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * @(#)$Id: radio-uip-uaodv.c,v 1.1 2007/05/19 14:25:43 fros4943 Exp $
 */

#include "radio-uip-uaodv.h"
#include "net/hc.h"
#include "net/uip.h"
#include "net/uaodv.h"
#include "net/uaodv-rt.h"
#include "net/uaodv-def.h"
#include <string.h>
#include <stdio.h>

static const struct radio_driver *radio;

#define uip_udp_sender() (&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->srcipaddr)

#define HEADER_ID_LENGTH strlen(uniqueID)
#define HEADER_NEXT_FWD HEADER_ID_LENGTH
#define HEADER_LENGTH HEADER_NEXT_FWD + 4
static char* uniqueID = "fWd:";

static struct uaodv_rt_entry *route;

/*---------------------------------------------------------------------------*/
int radio_uip_uaodv_add_header(u8_t *buf, int len, uip_ipaddr_t *addr);
int radio_uip_uaodv_remove_header(u8_t *buf, int len);
void radio_uip_uaodv_change_header(u8_t *buf, int len, uip_ipaddr_t *addr);
int radio_uip_uaodv_header_exists(u8_t *buf, int len);
int radio_uip_uaodv_is_broadcast(uip_ipaddr_t *addr);
int radio_uip_uaodv_fwd_is_broadcast(u8_t *buf, int len);
int radio_uip_uaodv_fwd_is_me(u8_t *buf, int len);
int radio_uip_uaodv_dest_is_me(u8_t *buf, int len);
int radio_uip_uaodv_dest_port(u8_t *buf, int len);
/*---------------------------------------------------------------------------*/
static void
receiver(const struct radio_driver *d)
{
  uip_len = d->read(&uip_buf[UIP_LLH_LEN], UIP_BUFSIZE - UIP_LLH_LEN);
  if (uip_len <= 0) {
    return;
  }
  
  /* If no uAODV header, receive as usual */
  if (!radio_uip_uaodv_header_exists(&uip_buf[UIP_LLH_LEN], uip_len)) {
    uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);
    tcpip_input();
    return;
  }
  
  /* If broadcast packet, strip header and receive without forwarding */
  if (radio_uip_uaodv_fwd_is_broadcast(&uip_buf[UIP_LLH_LEN], uip_len)) {
    /* Strip header and receive */
    uip_len = radio_uip_uaodv_remove_header(&uip_buf[UIP_LLH_LEN], uip_len);
    uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);
    tcpip_input();
    return;
  }
    
  /* Drop packet unless we are the uAODV forwarder */
  if (!radio_uip_uaodv_fwd_is_me(&uip_buf[UIP_LLH_LEN], uip_len)) {
    return;
  }
      
  /* If we are final destination, strip header and receive */
  if (radio_uip_uaodv_dest_is_me(&uip_buf[UIP_LLH_LEN], uip_len)) {
    uip_len = radio_uip_uaodv_remove_header(&uip_buf[UIP_LLH_LEN], uip_len);
    uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);
    tcpip_input();
    return;
  } 
  
  /* Find new uAODV forwarder and forward packet */
  /* TODO TTL? Hop-count? */
  /* TODO Assuming uAODV already knows a path */
  route = uaodv_rt_lookup((&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN + HEADER_LENGTH])->destipaddr));
  if (route != NULL) {
    radio_uip_uaodv_change_header(&uip_buf[UIP_LLH_LEN], uip_len, &route->nexthop);
    d->send(&uip_buf[UIP_LLH_LEN], uip_len);
  }
  
}
/*---------------------------------------------------------------------------*/
u8_t
radio_uip_uaodv_send(void)
{
  /* uAODV process messages should be sent as usual */
  if (radio_uip_uaodv_dest_port(&uip_buf[UIP_LLH_LEN], uip_len) == HTONS(UAODV_UDPPORT)) {
    uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
    return radio->send(&uip_buf[UIP_LLH_LEN], uip_len);
  } 
  
  if (radio_uip_uaodv_is_broadcast(uip_udp_sender())) {
    uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
    
    /* We can use the broadcast address immediately */
    uip_len = radio_uip_uaodv_add_header(&uip_buf[UIP_LLH_LEN], uip_len, uip_udp_sender()); /* TODO Correct? */
  } else {
    uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
    
    /* Fetch uAODV route (must be prepared!) */
    route = uaodv_rt_lookup((&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr));
    if (route == NULL) return UIP_FW_NOROUTE;
    uip_len = radio_uip_uaodv_add_header(&uip_buf[UIP_LLH_LEN], uip_len, &route->nexthop); /* TODO Correct? */
  }
  
  return radio->send(&uip_buf[UIP_LLH_LEN], uip_len);
}
/*---------------------------------------------------------------------------*/
void
radio_uip_uaodv_init(const struct radio_driver *d)
{
  radio = d;
  radio->set_receive_function(receiver);
  radio->on();}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_add_header(u8_t *buf, int len, uip_ipaddr_t *addr)
{
  memcpy(&buf[HEADER_LENGTH], buf, len);
  memcpy(buf, uniqueID, HEADER_ID_LENGTH);
  memcpy(&buf[HEADER_NEXT_FWD], (char*)addr, 4);
  return HEADER_LENGTH + len;     
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_remove_header(u8_t *buf, int len)
{
  memcpy(buf, &buf[HEADER_LENGTH], len);
  return HEADER_LENGTH + len;     
  return len - HEADER_LENGTH;     
}
/*---------------------------------------------------------------------------*/
void
radio_uip_uaodv_change_header(u8_t *buf, int len, uip_ipaddr_t *addr)
{
  memcpy(&buf[HEADER_NEXT_FWD], addr, 4);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_header_exists(u8_t *buf, int len)
{
  return !memcmp(buf, uniqueID, HEADER_ID_LENGTH);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_is_broadcast(uip_ipaddr_t *addr)
{
  return ((u8_t*)addr)[3] == 255; /* TODO Only comparing with the last */
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_fwd_is_broadcast(u8_t *buf, int len)
{
  return radio_uip_uaodv_is_broadcast((uip_ipaddr_t*) &buf[HEADER_NEXT_FWD]);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_fwd_is_me(u8_t *buf, int len)
{
  return !memcmp(&buf[HEADER_NEXT_FWD], &uip_hostaddr, 4);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_dest_is_me(u8_t *buf, int len)
{
  return !memcmp((&((struct uip_udpip_hdr *)&buf[HEADER_LENGTH])->destipaddr), &uip_hostaddr, 4);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_dest_port(u8_t *buf, int len)
{
  return (int) ((struct uip_udpip_hdr *)buf)->destport;
}
/*---------------------------------------------------------------------------*/
