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
 */

#include "radio-uip-uaodv.h"
#include "net/hc.h"
#include "net/uip.h"
#include "net/uaodv.h"
#include "net/uaodv-rt.h"
#include "net/uaodv-def.h"
#include "lib/crc16.h"
#include "list.h"
#include <string.h>
#include <stdio.h>

/* Packet buffer size and retransmission settings */
#define MAX_BUFFERED_PACKETS 10
#define MAX_RETRANSMISSIONS_RREP 16
#define MAX_RETRANSMISSIONS_UNICAST 16

/* Forward packet (header) */
#define FWD_ID "fWd:"
#define FWD_ID_LENGTH 4
#define FWD_NEXT_IP FWD_ID_LENGTH
#define FWD_PACKET_LENGTH (FWD_NEXT_IP + 4)

/* Acknowledgement packet */
#define ACK_ID "aCk"
#define ACK_ID_LENGTH 3
#define ACK_CRC ACK_ID_LENGTH
#define ACK_PACKET_LENGTH (ACK_ID_LENGTH + 2)
#define ACK_TIMEOUT (CLOCK_SECOND / 50) * (random_rand() % 100)

enum {
  EVENT_SEND_ACK
};

struct buf_packet {
  struct buf_packet *next;
  uint8_t data[UIP_BUFSIZE];
  int len;
  uint8_t resends;
  uint8_t acked;
  uint8_t want_ack;
  uint16_t crc;
  uip_ipaddr_t finaldest;
  struct etimer etimer;
};

LIST(buf_packet_list);
MEMB(buf_packet_mem, struct buf_packet, MAX_BUFFERED_PACKETS);

PROCESS(radio_uip_process, "radio uIP uAODV process");

static const struct radio_driver *radio;



/*---------------------------------------------------------------------------*/
static void receiver(const struct radio_driver *d);
uint8_t radio_uip_uaodv_send(void);
void radio_uip_uaodv_init(const struct radio_driver *d);
int radio_uip_handle_ack(uint8_t *buf, int len);
uint16_t radio_uip_calc_crc(uint8_t *buf, int len);
int radio_uip_buffer_outgoing_packet(uint8_t *buf, int len, uip_ipaddr_t *dest, int max_sends);
int radio_uip_is_ack(uint8_t *buf, int len);
int radio_uip_uaodv_add_header(uint8_t *buf, int len, uip_ipaddr_t *addr);
int radio_uip_uaodv_remove_header(uint8_t *buf, int len);
void radio_uip_uaodv_change_header(uint8_t *buf, int len, uip_ipaddr_t *addr);
int radio_uip_uaodv_header_exists(uint8_t *buf, int len);
int radio_uip_uaodv_is_broadcast(uip_ipaddr_t *addr);
int radio_uip_uaodv_fwd_is_broadcast(uint8_t *buf, int len);
int radio_uip_uaodv_fwd_is_me(uint8_t *buf, int len);
int radio_uip_uaodv_dest_is_me(uint8_t *buf, int len);
int radio_uip_uaodv_dest_port(uint8_t *buf, int len);
/*---------------------------------------------------------------------------*/

/* Main process - handles (re)transmissions and acks */
PROCESS_THREAD(radio_uip_process, ev, data)
{
  struct buf_packet *packet;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD();

    if(ev == EVENT_SEND_ACK) {

      /* Prepare and send ack for given 16-bit CRC */
      uint8_t ackPacket[ACK_PACKET_LENGTH];
      memcpy(ackPacket, ACK_ID, ACK_ID_LENGTH);
      ackPacket[ACK_CRC] = ((uint16_t) data >> 8);
      ackPacket[ACK_CRC+1] = ((uint16_t) data & 0xff);
      radio->send(ackPacket, ACK_PACKET_LENGTH);

    } else if(ev == PROCESS_EVENT_TIMER) {
      /* Locate which packet acknowledgement timed out */
      for(packet = list_head(buf_packet_list);
          packet != NULL;
          packet = packet->next) {
        if (etimer_expired(&packet->etimer)) {

          if (packet->acked) {
            /* Already acked packet, remove silently */
            list_remove(buf_packet_list, packet);
            memb_free(&buf_packet_mem, packet);

          } else if (packet->resends > 0) {
            /* Resend packet */
            packet->resends--;
            etimer_set(&packet->etimer, ACK_TIMEOUT);

            radio->send(packet->data, packet->len);

          } else {
            /* Packet was resent maximum number of times */

            /* If an ack was expected, flag destination to bad */
            if (packet->want_ack && !uip_ipaddr_cmp(&packet->finaldest, &uip_broadcast_addr)) {
              uaodv_bad_dest(&packet->finaldest);
            }

            list_remove(buf_packet_list, packet);
            memb_free(&buf_packet_mem, packet);
          }            
        }
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
receiver(const struct radio_driver *d)
{
  uip_len = d->read(&uip_buf[UIP_LLH_LEN], UIP_BUFSIZE - UIP_LLH_LEN);
  if (uip_len <= 0) {
    return;
  }

  /* Detect and handle acknowledgements */
  if (radio_uip_is_ack(&uip_buf[UIP_LLH_LEN], uip_len)) {
    radio_uip_handle_ack(&uip_buf[UIP_LLH_LEN], uip_len);
    return;
  }

  /* If no uAODV header, receive as usual */
  if (!radio_uip_uaodv_header_exists(&uip_buf[UIP_LLH_LEN], uip_len)) {
    tcpip_input();
    return;
  }
  
  /* Drop packet unless we are the uAODV forwarder */
  if (!radio_uip_uaodv_fwd_is_me(&uip_buf[UIP_LLH_LEN], uip_len)) {
    return;
  }

  {
    /* Send ack as soon as possible */
    uint16_t crc;
    crc = radio_uip_calc_crc(&uip_buf[UIP_LLH_LEN], uip_len);
    process_post(&radio_uip_process, EVENT_SEND_ACK, (void*) (uint32_t) crc);
  }

  /* Strip header and receive packet */
  uip_len = radio_uip_uaodv_remove_header(&uip_buf[UIP_LLH_LEN], uip_len);
  tcpip_input();
}
/*---------------------------------------------------------------------------*/
uint8_t
radio_uip_uaodv_send(void)
{
  struct uaodv_rt_entry *route;
  
  /* Transmit broadcast packets without header */
  if (radio_uip_uaodv_is_broadcast(&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr)) {
    return radio_uip_buffer_outgoing_packet(&uip_buf[UIP_LLH_LEN], uip_len, (void*) &uip_broadcast_addr, 1);
  }

  /* Transmit uAODV packets with headers but without using route table */
  if (((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->proto == UIP_PROTO_UDP
      && radio_uip_uaodv_dest_port(&uip_buf[UIP_LLH_LEN], uip_len) == UIP_HTONS(UAODV_UDPPORT)) {
    uip_ipaddr_t nexthop;
    memcpy(&nexthop, &((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr, 4);

    uip_len = radio_uip_uaodv_add_header(
      &uip_buf[UIP_LLH_LEN], 
      uip_len, 
      &nexthop
    );

    /* Buffer packet for persistent transmission */
    return radio_uip_buffer_outgoing_packet(
      &uip_buf[UIP_LLH_LEN],
      uip_len,
      &((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN + FWD_PACKET_LENGTH])->destipaddr,
      MAX_RETRANSMISSIONS_RREP);
  }
  
  /* Fetch already prepared uAODV route */
  route = uaodv_rt_lookup_any((&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr));
  if (route == NULL || route->is_bad) {

    /* If we are forwarding, notify origin of this bad route */
    if (tcpip_is_forwarding) {
      uaodv_bad_dest((&((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])->destipaddr));
    }
      
    return UIP_FW_DROPPED;
  }
  
  /* Add header and buffer packet for persistent transmission */
  uip_len = radio_uip_uaodv_add_header(&uip_buf[UIP_LLH_LEN], uip_len, uip_ds6_route_nexthop(route)); /* TODO Correct? */
  return radio_uip_buffer_outgoing_packet(
    &uip_buf[UIP_LLH_LEN],
    uip_len,
    &route->dest,
    MAX_RETRANSMISSIONS_UNICAST);
}
/*---------------------------------------------------------------------------*/
void
radio_uip_uaodv_init(const struct radio_driver *d)
{
  /* Prepare buffers and start main process */
  memb_init(&buf_packet_mem);
  list_init(buf_packet_list);
  process_start(&radio_uip_process, NULL);

  radio = d;
  radio->set_receive_function(receiver);
  radio->on();
}
/*---------------------------------------------------------------------------*/
uint16_t
radio_uip_calc_crc(uint8_t *buf, int len)
{
    uint16_t crcacc = 0xffff;
    int counter;

    /* TODO Not effective */
    for (counter = 0; counter < len; counter++) {
      crcacc = crc16_add(buf[counter], crcacc);
    }
    return crcacc;
}
/*---------------------------------------------------------------------------*/
int
radio_uip_buffer_outgoing_packet(uint8_t *buf, int len, uip_ipaddr_t *dest, int max_sends)
{
  struct buf_packet *packet;

  uint16_t crc;
  
  /* Calculate packet's unique CRC */
  crc = radio_uip_calc_crc(&uip_buf[UIP_LLH_LEN], uip_len);

  /* Check if this packet is already being transmitted */
  for(packet = list_head(buf_packet_list);
    packet != NULL;
    packet = packet->next) {
    if (packet->crc == crc) {
      return UIP_FW_DROPPED;
    }
  }

  /* Allocate storage memory */
  packet = (struct buf_packet *)memb_alloc(&buf_packet_mem);
  if (packet == NULL) {
    return UIP_FW_DROPPED;
  }

  /* Prepare packet buffer */
  memcpy(packet->data, buf, len);
  packet->len = len;
  packet->resends = max_sends;
  packet->acked = 0;
  if (packet->resends > 1)
    packet->want_ack = 1;
  else
    packet->want_ack = 0;
  memcpy(&packet->finaldest, dest, 4);
  packet->crc = crc;

  /* Set first transmission to as soon as possible */
  PROCESS_CONTEXT_BEGIN(&radio_uip_process);
  etimer_set(&packet->etimer, 0);
  PROCESS_CONTEXT_END(&radio_uip_process);

  /* Add to buffered packets list */
  list_add(buf_packet_list, packet);

  return UIP_FW_OK;
}
/*---------------------------------------------------------------------------*/
int
radio_uip_is_ack(uint8_t *buf, int len)
{
  if (len != ACK_PACKET_LENGTH)
    return 0;
  
  return memcmp(buf, ACK_ID, ACK_ID_LENGTH) == 0;
  
}
/*---------------------------------------------------------------------------*/
int
radio_uip_handle_ack(uint8_t *buf, int len)
{
  struct buf_packet *packet;
  uint16_t ackCRC;
  
  ackCRC = (uint16_t) (buf[ACK_CRC] << 8) + (uint16_t) (0xff&buf[ACK_CRC+1]);
  
  /* Locate which packet was acknowledged */
  for(packet = list_head(buf_packet_list);
      packet != NULL;
      packet = packet->next) {
    if (packet->crc == ackCRC) {
      /* Signal packet has been acknowledged */
      packet->acked = 1;
      return 0;
    }
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_add_header(uint8_t *buf, int len, uip_ipaddr_t *addr)
{
  uint8_t tempbuf[len];
  memcpy(tempbuf, buf, len);
  memcpy(&buf[FWD_PACKET_LENGTH], tempbuf, len);
  memcpy(buf, FWD_ID, FWD_ID_LENGTH);
  memcpy(&buf[FWD_NEXT_IP], (char*)addr, 4);
  return FWD_PACKET_LENGTH + len;     
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_remove_header(uint8_t *buf, int len)
{
  uint8_t tempbuf[len];
  memcpy(tempbuf, &buf[FWD_PACKET_LENGTH], len);
  memcpy(buf, tempbuf, len);
  return len - FWD_PACKET_LENGTH;
}
/*---------------------------------------------------------------------------*/
void
radio_uip_uaodv_change_header(uint8_t *buf, int len, uip_ipaddr_t *addr)
{
  memcpy(&buf[FWD_NEXT_IP], addr, 4);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_header_exists(uint8_t *buf, int len)
{
  return !memcmp(buf, FWD_ID, FWD_ID_LENGTH);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_is_broadcast(uip_ipaddr_t *addr)
{
  return uip_ipaddr_cmp(addr, &uip_broadcast_addr);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_fwd_is_broadcast(uint8_t *buf, int len)
{
  return radio_uip_uaodv_is_broadcast((uip_ipaddr_t*) &buf[FWD_NEXT_IP]);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_fwd_is_me(uint8_t *buf, int len)
{
  return !memcmp(&buf[FWD_NEXT_IP], &uip_hostaddr, 4);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_dest_is_me(uint8_t *buf, int len)
{
  return !memcmp((&((struct uip_udpip_hdr *)buf)->destipaddr), &uip_hostaddr, 4);
}
/*---------------------------------------------------------------------------*/
int
radio_uip_uaodv_dest_port(uint8_t *buf, int len)
{
  if (len < sizeof(struct uip_udpip_hdr))
    return -1;
  return (int) ((struct uip_udpip_hdr *)buf)->destport;
}
/*---------------------------------------------------------------------------*/
