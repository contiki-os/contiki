/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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

/**
 * \file
 *         A null RDC implementation that uses framer for headers and sends
 *         the packets over slip instead of radio.
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/netstack.h"
#include "packetutils.h"
#include "native-slip.h"
#include "cetic_bridge.h"
#include <string.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

uint8_t mac_set;

extern long slip_sent;
extern long slip_received;

#define MAX_CALLBACKS 16
static int callback_pos;

/* a structure for calling back when packet data is coming back
   from radio... */
struct tx_callback {
  mac_callback_t cback;
  void *ptr;
  struct packetbuf_attr attrs[PACKETBUF_NUM_ATTRS];
  struct packetbuf_addr addrs[PACKETBUF_NUM_ADDRS];
};

static struct tx_callback callbacks[MAX_CALLBACKS];
/*---------------------------------------------------------------------------*/
void packet_sent(uint8_t sessionid, uint8_t status, uint8_t tx)
{
  if(sessionid < MAX_CALLBACKS) {
    struct tx_callback *callback;
    callback = &callbacks[sessionid];
    packetbuf_clear();
    packetbuf_attr_copyfrom(callback->attrs, callback->addrs);
    mac_call_sent_callback(callback->cback, callback->ptr, status, tx);
  } else {
    PRINTF("*** ERROR: too high session id %d\n", sessionid);
  }
}
/*---------------------------------------------------------------------------*/
static int
setup_callback(mac_callback_t sent, void *ptr)
{
  struct tx_callback *callback;
  int tmp = callback_pos;
  callback = &callbacks[callback_pos];
  callback->cback = sent;
  callback->ptr = ptr;
  packetbuf_attr_copyto(callback->attrs, callback->addrs);

  callback_pos++;
  if(callback_pos >= MAX_CALLBACKS) {
    callback_pos = 0;
  }

  return tmp;
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{
  int size;
  /* 3 bytes per packet attribute is required for serialization */
  uint8_t buf[PACKETBUF_NUM_ATTRS * 3 + PACKETBUF_SIZE + 3];
  uint8_t sid;

  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);

  /* ack or not ? */
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);

  if(NETSTACK_FRAMER.create() < 0) {
    /* Failed to allocate space for headers */
    PRINTF("br-rdc: send failed, too large header\n");
    mac_call_sent_callback(sent, ptr, MAC_TX_ERR_FATAL, 1);

  } else {
    /* here we send the data over SLIP to the radio-chip */
    size = 0;
#if SERIALIZE_ATTRIBUTES
    size = packetutils_serialize_atts(&buf[3], sizeof(buf) - 3);
#endif
    if(size < 0 || size + packetbuf_totlen() + 3 > sizeof(buf)) {
      PRINTF("br-rdc: send failed, too large header\n");
      mac_call_sent_callback(sent, ptr, MAC_TX_ERR_FATAL, 1);
    } else {
    	PRINTF("SLIP: Sending %d\n", size);
      sid = setup_callback(sent, ptr);

      buf[0] = '!';
      buf[1] = 'S';
      buf[2] = sid; /* sequence or session number for this packet */

      /* Copy packet data */
      memcpy(&buf[3 + size], packetbuf_hdrptr(), packetbuf_totlen());

      write_to_slip(buf, packetbuf_totlen() + size + 3);
    }
  }
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
  if(NETSTACK_FRAMER.parse() < 0) {
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
void
slip_print_stat()
{
  printf("bytes received over SLIP: %ld\n", slip_received);
  printf("bytes sent over SLIP: %ld\n", slip_sent);
}
/*---------------------------------------------------------------------------*/
void
slip_request_mac(void)
{
  printf("Fetching MAC address\n");
  mac_set = 0;
  write_to_slip((uint8_t *)"?M", 2);
}

void
slip_set_mac(const uint8_t *data)
{
  printf("Got MAC\n");
  memcpy(uip_lladdr.addr, data, sizeof(uip_lladdr.addr));
  rimeaddr_set_node_addr((rimeaddr_t *)uip_lladdr.addr);
  rimeaddr_copy( (rimeaddr_t *)&wsn_mac_addr, &rimeaddr_node_addr);
  mac_set = 1;
}
/*---------------------------------------------------------------------------*/
void
slip_set_rf_channel(uint8_t channel)
{
	static uint8_t  msg[3];
	msg[0] = '!';
	msg[1] = 'C';
	msg[2] = channel;
	write_to_slip(msg, 3);
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
