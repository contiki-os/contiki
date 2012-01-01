/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: nullrdc.c,v 1.4 2010/11/23 18:11:00 nifi Exp $
 */

/**
 * \file
 *         A null RDC implementation that uses framer for headers.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/netstack.h"
#include <string.h>
#include "packetbuf.h"
#include "packetutils.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define MAX_CALLBACKS 16
static int callback_pos;

/* a structure for calling back when packet data is coming back
   from radio... */
struct tx_callback {
  mac_callback_t cback;
  int status;
  int tx;
  void *ptr;
  struct packetbuf_attr attr;
  struct packetbuf_addr addr;
};

static void send_callback(struct tx_callback *tx);


static struct tx_callback callbacks[MAX_CALLBACKS];

void packet_sent(uint8_t sessionid, uint8_t status, uint8_t tx)
{
  if(sessionid < MAX_CALLBACKS) {
    struct tx_callback *callback;
    callback = &callbacks[sessionid];
    packetbuf_attr_copyfrom(&callback->attr, &callback->addr);
    callback->status = status;
    callback->tx = tx;
    send_callback(callback);
  } else {
    printf("*** ERROR: too high session id %d\n", sessionid);
  }
}

static int
  setup_callback(mac_callback_t sent, void *ptr)
{
  int tmp = callback_pos;
  struct tx_callback *callback;
  callback = &callbacks[callback_pos];
  callback->cback = sent;
  callback->ptr = ptr;
  packetbuf_attr_copyto(&callback->attr, &callback->addr);
  
  callback_pos++;
  if(callback_pos >= MAX_CALLBACKS)
    callback_pos = 0;
  
  return tmp;
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  int ret;
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);

  /* ack or not ? */
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);

  if(NETSTACK_FRAMER.create() == FRAMER_FAILED) {
    /* Failed to allocate space for headers */
    PRINTF("br-rdc: send failed, too large header\n");
    ret = MAC_TX_ERR_FATAL;
  } else {
    int is_broadcast;
    uint8_t dsn;
    uint8_t sid;
    int size;
    uint8_t attbuf[64]; /* 3 bytes per attribute - hopefully enough with 64 */

    sid = setup_callback(sent, ptr);

    dsn = ((uint8_t *)packetbuf_hdrptr())[2] & 0xff;

    is_broadcast = rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                                &rimeaddr_null);

    /* here we send the data over SLIP to the radio-chip */
    /* NETSTACK_RADIO.transmit(packetbuf_totlen())); */
    /* and store the "call" with a timeout - for fail and if success
       we just make a callback */
    /* TODO: store the call for later "ack" / feedback handling... */
    if((size = packetutils_serialize_atts(attbuf, sizeof(attbuf))) > 0) {
      /* alloc and copy attributes */
      packetbuf_hdralloc(size);
      memcpy(packetbuf_hdrptr(), attbuf, size);	
      packetbuf_hdralloc(3);
      uint8_t *buf = (uint8_t *) packetbuf_hdrptr();
      buf[0] = '!';
      buf[1] = 'S';
      buf[2] = sid; /* sequence or session number for this packet */
      write_to_slip(packetbuf_hdrptr(), packetbuf_totlen());
    }
  }
}

/* */
static void send_callback(struct tx_callback *tx)
{
  mac_call_sent_callback(tx->cback, tx->ptr, tx->status, tx->tx);
}

/*---------------------------------------------------------------------------*/
static void
send_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
  if(buf_list != NULL) {
    queuebuf_to_packetbuf(buf_list->buf);
    send_packet(sent, ptr);
  }
}
/*---------------------------------------------------------------------------*/
static void
packet_input(void)
{
  if(NETSTACK_FRAMER.parse() == FRAMER_FAILED) {
    PRINTF("br-rdc: failed to parse %u\n", packetbuf_datalen());
  } else {
    NETSTACK_MAC.input();
  }
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(int keep_radio_on)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  callback_pos = 0;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver border_router_rdc_driver = {
  "br-rdc",
  init,
  send_packet,
  send_list,
  packet_input,
  on,
  off,
  channel_check_interval,
};
/*---------------------------------------------------------------------------*/
