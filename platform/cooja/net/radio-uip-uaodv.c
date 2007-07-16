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
 * @(#)$Id: radio-uip-uaodv.c,v 1.3 2007/07/16 07:40:55 fros4943 Exp $
 */

#include "radio-uip-uaodv.h"
#include "net/hc.h"
#include "net/uip.h"
#include "net/uaodv.h"
#include "net/uaodv-rt.h"
#include "net/uaodv-def.h"
#include <string.h>
#include <stdio.h>

#define UNICAST_ACKED 1 /* Define to acknowledge unicast packets */
#define BAD_REPLY_ON_NO_ROUTE 0 /* TODO */

#define uip_udp_sender() (&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->srcipaddr)

#define HEADER_ID_LENGTH strlen(uniqueFwdHeader)
#define HEADER_NEXT_FWD HEADER_ID_LENGTH
#define HEADER_LENGTH HEADER_NEXT_FWD + 4


#if UNICAST_ACKED

#include "lib/crc16.h"
#include "list.h"

enum {
  EVENT_SEND_ACK
};

struct bufferedPacket {
  struct bufferedPacket *next;
  u8_t data[UIP_BUFSIZE];
  int len;
  u8_t resends;
  u8_t acked;
  u16_t crc;
  uip_ipaddr_t finaldest;
  struct etimer etimer;
};

LIST(bufferedPackets);
MEMB(bufferedPacketsMEMB, struct bufferedPacket, 10);

PROCESS(radio_uip_ack_process, "radio uIP uAODV ack process");
#endif

static const char* uniqueFwdHeader = "fWd:";
#if UNICAST_ACKED
static const char* uniqueAckHeader = "aCk";
#define ACK_HEADER_LENGTH 3
#define ACK_LENGTH ACK_HEADER_LENGTH + 2
#endif
static const struct radio_driver *radio;
static struct uaodv_rt_entry *route;


/*---------------------------------------------------------------------------*/
#if UNICAST_ACKED
int radio_uip_wait_for_ack(u8_t *buf, int len, uip_ipaddr_t *dest);
int radio_uip_is_ack(u8_t *buf, int len);
#endif
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

#if UNICAST_ACKED
PROCESS_THREAD(radio_uip_ack_process, ev, data)
{
  struct bufferedPacket *bufPacket;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD();

    /* We should either send an ack, or resend a packet */
    if(ev == EVENT_SEND_ACK) {
      char ackPacket[ACK_LENGTH];
      
      /* Prepare and send ack */
      memcpy(ackPacket, uniqueAckHeader, ACK_HEADER_LENGTH);
      memcpy(&ackPacket[ACK_HEADER_LENGTH], &data, 2);
      radio->send(ackPacket, ACK_LENGTH);

    } else if(ev == PROCESS_EVENT_TIMER) {

      /* Locate which packet acknowledgement timed out */
      for(bufPacket = list_head(bufferedPackets);
          bufPacket != NULL;
          bufPacket = bufPacket->next) {
        if (etimer_expired(&bufPacket->etimer)) {

          if (bufPacket->acked) {
            /* This packet was already acknowledged! */
            list_remove(bufferedPackets, bufPacket);
            memb_free(&bufferedPacketsMEMB, bufPacket);
          } else if (bufPacket->resends < 3) {
            /* Resend packet a number of times */
            bufPacket->resends++;

            /* TODO Compress packets? */
            radio->send(bufPacket->data, bufPacket->len);
            etimer_restart(&bufPacket->etimer);
          } else {
            /* Too many resends, give up */
            /* TODO: Report non-delivered packet with bad dest? */
            uaodv_bad_dest(&bufPacket->finaldest);
            list_remove(bufferedPackets, bufPacket);
            memb_free(&bufferedPacketsMEMB, bufPacket);
          }
        }
      }
    }

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#endif

static void
receiver(const struct radio_driver *d)
{
  uip_len = d->read(&uip_buf[UIP_LLH_LEN], UIP_BUFSIZE - UIP_LLH_LEN);
  if (uip_len <= 0) {
    return;
  }

#if UNICAST_ACKED
  /* Detect and parse acknowledgements */
  if (radio_uip_is_ack(&uip_buf[UIP_LLH_LEN], uip_len)) {
    radio_uip_handle_ack(&uip_buf[UIP_LLH_LEN], uip_len);
    return;
  }
#endif

#if UNICAST_ACKED
  /* uAODV RREP and RERR messages should be acked as soon as possible */
  if (radio_uip_uaodv_dest_is_me(&uip_buf[UIP_LLH_LEN], uip_len) &&
      radio_uip_uaodv_dest_port(&uip_buf[UIP_LLH_LEN], uip_len) == HTONS(UAODV_UDPPORT)) {
    struct uaodv_msg *m = (struct uaodv_msg *)&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
    if (m->type == UAODV_RREP_TYPE || m->type == UAODV_RERR_TYPE) {
      u16_t crc;
      crc = radio_uip_calc_crc(&uip_buf[UIP_LLH_LEN], uip_len);
      process_post(&radio_uip_ack_process, EVENT_SEND_ACK, crc);
    }
  }
#endif
  
  /* If no uAODV header, receive as usual (compatibility) */
  if (!radio_uip_uaodv_header_exists(&uip_buf[UIP_LLH_LEN], uip_len)) {
    //uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);
    tcpip_input();
    return;
  }
  
  /* If broadcast packet, strip header and receive without forwarding */
  if (radio_uip_uaodv_fwd_is_broadcast(&uip_buf[UIP_LLH_LEN], uip_len)) {
    /* Strip header and receive */
    uip_len = radio_uip_uaodv_remove_header(&uip_buf[UIP_LLH_LEN], uip_len);
    //uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);
    tcpip_input();
    return;
  }
    
  /* Drop packet unless we are the uAODV forwarder */
  if (!radio_uip_uaodv_fwd_is_me(&uip_buf[UIP_LLH_LEN], uip_len)) {
    return;
  }
      
  /* If we are final destination, strip header and receive */
  if (radio_uip_uaodv_dest_is_me(&uip_buf[UIP_LLH_LEN + HEADER_LENGTH], uip_len)) {

#if UNICAST_ACKED
    {
      /* Unicast packets should be acked as soon as possible */
      u16_t crc;
      crc = radio_uip_calc_crc(&uip_buf[UIP_LLH_LEN], uip_len);
      process_post(&radio_uip_ack_process, EVENT_SEND_ACK, crc);
    }
#endif

    uip_len = radio_uip_uaodv_remove_header(&uip_buf[UIP_LLH_LEN], uip_len);
    //uip_len = hc_inflate(&uip_buf[UIP_LLH_LEN], uip_len);
    tcpip_input();
    return;
  } 
  
  /* Find new uAODV forwarder and forward packet */
  /* TODO TTL? Hop-count? */
  /* TODO Assuming uAODV already knows a path */
  route = uaodv_rt_lookup((&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN + HEADER_LENGTH])->destipaddr));
  if (route != NULL) {

#if UNICAST_ACKED
    {
      /* Ack last forwarder as soon as possible */
      u16_t crc;
      crc = radio_uip_calc_crc(&uip_buf[UIP_LLH_LEN], uip_len);
      process_post(&radio_uip_ack_process, EVENT_SEND_ACK, crc);
    }
#endif

    radio_uip_uaodv_change_header(&uip_buf[UIP_LLH_LEN], uip_len, &route->nexthop);

#if UNICAST_ACKED
    /* Wait for ack from next forwarded */
    radio_uip_wait_for_ack(&uip_buf[UIP_LLH_LEN], uip_len, &route->dest);
#endif

    d->send(&uip_buf[UIP_LLH_LEN], uip_len);

#if BAD_REPLY_ON_NO_ROUTE
  } else {
  	NOT IMPLEMENTED
#endif
  }
}
/*---------------------------------------------------------------------------*/
u8_t
radio_uip_uaodv_send(void)
{
  /* uAODV RREQ messages should be sent as usual */
  if (radio_uip_uaodv_dest_port(&uip_buf[UIP_LLH_LEN], uip_len) == HTONS(UAODV_UDPPORT)) {
    struct uaodv_msg *m = (struct uaodv_msg *)&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
    if (m->type == UAODV_RREQ_TYPE) {
      //uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
      return radio->send(&uip_buf[UIP_LLH_LEN], uip_len);
    }

#if UNICAST_ACKED
    /* uAODV RREP and RERR messages should be acked */
    radio_uip_wait_for_ack(&uip_buf[UIP_LLH_LEN], uip_len, &((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr);
#endif

    //uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
    return radio->send(&uip_buf[UIP_LLH_LEN], uip_len);
  } 

  if (radio_uip_uaodv_is_broadcast(uip_udp_sender())) {
    //uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
    
    /* We can use the broadcast address immediately */
    uip_len = radio_uip_uaodv_add_header(&uip_buf[UIP_LLH_LEN], uip_len, uip_udp_sender()); /* TODO Correct? */
  } else {
    //uip_len = hc_compress(&uip_buf[UIP_LLH_LEN], uip_len);
    
    /* Fetch uAODV route (must be prepared!) */
    route = uaodv_rt_lookup((&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr));
    if (route == NULL) return UIP_FW_NOROUTE;
    uip_len = radio_uip_uaodv_add_header(&uip_buf[UIP_LLH_LEN], uip_len, &route->nexthop); /* TODO Correct? */

#if UNICAST_ACKED
    /* We expect to get an ack for this unicast packet */
    radio_uip_wait_for_ack(&uip_buf[UIP_LLH_LEN], uip_len, &route->dest);
#endif
  }
  
  return radio->send(&uip_buf[UIP_LLH_LEN], uip_len);
}
/*---------------------------------------------------------------------------*/
void
radio_uip_uaodv_init(const struct radio_driver *d)
{
#if UNICAST_ACKED
  memb_init(&bufferedPacketsMEMB);
  list_init(bufferedPackets);
  process_start(&radio_uip_ack_process, NULL);
#endif

  radio = d;
  radio->set_receive_function(receiver);
  radio->on();
}
#if UNICAST_ACKED
/*---------------------------------------------------------------------------*/
u16_t
radio_uip_calc_crc(u8_t *buf, int len)
{
    u16_t crcacc = 0xffff;
    int counter;

    for (counter = 0; counter < len; counter++) {
      crcacc = crc16_add(buf[counter], crcacc);
    }
    return crcacc;
}
/*---------------------------------------------------------------------------*/
int
radio_uip_wait_for_ack(u8_t *buf, int len, uip_ipaddr_t *dest)
{
  struct bufferedPacket *bufPacket;

  /* Allocate storage memory */
  bufPacket = (struct bufferedPacket *)memb_alloc(&bufferedPacketsMEMB);
  if (bufPacket == NULL) {
    return 1;
  }

  /* Store packet specific data and set timer */
  memcpy(bufPacket->data, buf, len);
  bufPacket->len = len;
  bufPacket->resends = 0;
  bufPacket->acked = 0;
  memcpy(&bufPacket->finaldest, dest, 4);
  bufPacket->crc = radio_uip_calc_crc(&uip_buf[UIP_LLH_LEN], uip_len);
  PROCESS_CONTEXT_BEGIN(&radio_uip_ack_process);
  etimer_set(&bufPacket->etimer, CLOCK_SECOND * 1);
  PROCESS_CONTEXT_END(&radio_uip_ack_process);

  /* Add to list for later reference */
  list_add(bufferedPackets, bufPacket);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
radio_uip_is_ack(u8_t *buf, int len)
{
  struct bufferedPacket *bufPacket;

  if (uip_len != ACK_LENGTH)
    return 0;
  
  return strncmp(buf, uniqueAckHeader, ACK_HEADER_LENGTH) == 0;
}
/*---------------------------------------------------------------------------*/
int
radio_uip_handle_ack(u8_t *buf, int len)
{
  struct bufferedPacket *bufPacket;
  u16_t ackCRC;
  
  memcpy(&ackCRC, &buf[ACK_HEADER_LENGTH], 2);
  
  /* Locate which packet was acknowledged */
  for(bufPacket = list_head(bufferedPackets);
      bufPacket != NULL;
      bufPacket = bufPacket->next) {
    if (bufPacket->crc == ackCRC) {
      /* Signal packet has been acknowledged */
      bufPacket->acked = 1;
      return 0;
    }
  }
  return 1;
}
#endif
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_add_header(u8_t *buf, int len, uip_ipaddr_t *addr)
{
  memcpy(&buf[HEADER_LENGTH], buf, len);
  memcpy(buf, uniqueFwdHeader, HEADER_ID_LENGTH);
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
  return !memcmp(buf, uniqueFwdHeader, HEADER_ID_LENGTH);
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
  return !memcmp((&((struct uip_udpip_hdr *)buf)->destipaddr), &uip_hostaddr, 4);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_dest_port(u8_t *buf, int len)
{
  if (len < sizeof(struct uip_udpip_hdr))
    return -1;
  return (int) ((struct uip_udpip_hdr *)buf)->destport;
}
/*---------------------------------------------------------------------------*/
