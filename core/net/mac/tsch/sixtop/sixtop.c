/*
 * Copyright (c) 2016, Yasuyuki Tanaka
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup sixtop
 * @{
 */
/**
 * \file
 *         6TiSCH Operation Sublayer (6top)
 *
 * \author
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */

#include "lib/assert.h"

#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/mac/frame802154.h"
#include "net/mac/frame802154e-ie.h"

#include "sixtop.h"
#include "sixtop-conf.h"
#include "sixp.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

const sixtop_sf_t *scheduling_functions[SIXTOP_MAX_SCHEDULING_FUNCTIONS];

const sixtop_sf_t *sixtop_find_sf(uint8_t sfid);

/*---------------------------------------------------------------------------*/
int
sixtop_add_sf(const sixtop_sf_t *sf)
{
  int i;

  assert(sf != NULL);

  PRINTF("6top: sixtop_add_sf() is adding a SF [SFID:%u]\n", sf->sfid);

  if(sixtop_find_sf(sf->sfid) != NULL) {
    PRINTF("6top: sixtop_add_sf() fails because of duplicate SF\n");
    return -1;
  }

  for(i = 0; i < SIXTOP_MAX_SCHEDULING_FUNCTIONS; i++) {
    if(scheduling_functions[i] == NULL) {
      scheduling_functions[i] = sf;
      if(sf->init != NULL) {
        sf->init();
      }
      break;
    }
  }

  if(i == SIXTOP_MAX_SCHEDULING_FUNCTIONS) {
    PRINTF("6top: sixtop_add_sf() fails because of no memory\n");
    return -1;
  }

  if(sf->init != NULL) {
    sf->init();
  }
  PRINTF("6top: SF [SFID:%u] has been added and initialized\n", sf->sfid);
  return 0;
}
/*---------------------------------------------------------------------------*/
const sixtop_sf_t *
sixtop_find_sf(uint8_t sfid)
{
  int i;

  for(i = 0; i < SIXTOP_MAX_SCHEDULING_FUNCTIONS; i++) {
    if(scheduling_functions[i] != NULL &&
       scheduling_functions[i]->sfid == sfid) {
      return (const sixtop_sf_t *)scheduling_functions[i];
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
void
sixtop_output(const linkaddr_t *dest_addr, mac_callback_t callback, void *arg)
{
  uint8_t *p;
  struct ieee802154_ies ies;
  int len;

  assert(dest_addr != NULL);
  if(dest_addr == NULL) {
    PRINTF("6top: sixtop_output() fails because dest_addr is NULL\n");
    if(callback != NULL) {
      callback(arg, MAC_TX_ERR_FATAL, 0);
    }
    return;
  }

  /* prepend 6top Sub-IE ID */
  if(packetbuf_hdralloc(1) != 1) {
    PRINTF("6top: sixtop_output() fails because of no room for Sub-IE ID\n");
    return;
  }
  p = packetbuf_hdrptr();
  p[0] = SIXP_SUBIE_ID;

  /*
   * prepend Payload IE header; 2 octets
   * only sixtop_ie_content_len matters in frame80215e_create_ie_ietf().
   */
  memset(&ies, 0, sizeof(ies));
  ies.sixtop_ie_content_len = packetbuf_totlen();
  if(packetbuf_hdralloc(2) != 1 ||
     (len = frame80215e_create_ie_ietf(packetbuf_hdrptr(),
                                       2,
                                       &ies)) < 0) {
    PRINTF("6top: sixtop_output() fails because of Payload IE Header\n");
    if(callback != NULL) {
      callback(arg, MAC_TX_ERR_FATAL, 0);
    }
    return;
  }

  /* append Payload Termination IE to the data field; 2 octets */
  memset(&ies, 0, sizeof(ies));
  if((len = frame80215e_create_ie_payload_list_termination(
       (uint8_t *)packetbuf_dataptr() + packetbuf_datalen(),
       PACKETBUF_SIZE - packetbuf_totlen(),
       &ies)) < 0) {
    PRINTF("6top: sixtop_output() fails because of Payload Termination IE\n");
    callback(arg, MAC_TX_ERR_FATAL, 0);
    return;
  }
  packetbuf_set_datalen(packetbuf_datalen() + len);

  /* prepend Termination 1 IE to the header field; 2 octets */
  memset(&ies, 0, sizeof(ies));
  if(packetbuf_hdralloc(2) &&
     frame80215e_create_ie_header_list_termination_1(packetbuf_hdrptr(),
                                                     2,
                                                     &ies) < 0) {
    PRINTF("6top: sixtop_output() fails because of Header Termination 1 IE\n");
    callback(arg, MAC_TX_ERR_FATAL, 0);
    return;
  }

  /* specify with PACKETBUF_ATTR_METADATA that packetbuf has IEs */
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_METADATA, 1);

  /* 6P packet is data frame */
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_DATAFRAME);

  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, dest_addr);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);

  NETSTACK_MAC.send(callback, arg);
}
/*---------------------------------------------------------------------------*/
void
sixtop_input(void (*llsec_input)(void))
{
  uint8_t *hdr_ptr, *payload_ptr;
  uint16_t hdr_len, payload_len;

  frame802154_t frame;
  struct ieee802154_ies ies;
  linkaddr_t src_addr;

  assert(llsec_input != NULL);

  /*
   * A received *DATA* frame is supposed to be stored in packetbuf by
   * framer_802154.parse(). packetbuf_dataptr() points at the starting address
   * of the IE field or Frame Payload field if it's available. FCS should not be
   * in packetbuf, which is expected to be stripped at a radio.
   */

  payload_ptr = packetbuf_dataptr();
  payload_len = packetbuf_datalen();
  hdr_len = packetbuf_hdrlen();
  hdr_ptr = payload_ptr - hdr_len;

  memcpy(&src_addr, packetbuf_addr(PACKETBUF_ADDR_SENDER), sizeof(src_addr));

  if(frame802154_parse(hdr_ptr, hdr_len, &frame) == 0) {
    /* parse error; should not occur, anyway */
    PRINTF("6top: frame802154_parse error\n");
    return;
  }

  /*
   * We don't need to check the frame version nor frame type. The frame version
   * is turned out to be 0b10 automatically if the frame has a IE list. The
   * frame type is supposed to be DATA as mentioned above.
   */
  assert(frame.fcf.frame_version == FRAME802154_IEEE802154E_2012);
  assert(frame.fcf.frame_type == FRAME802154_DATAFRAME);
  if(frame.fcf.ie_list_present &&
     frame802154e_parse_information_elements(payload_ptr,
                                             payload_len, &ies) >= 0 &&
     ies.sixtop_ie_content_ptr != NULL &&
     ies.sixtop_ie_content_len > 0) {

    sixp_input(ies.sixtop_ie_content_ptr, ies.sixtop_ie_content_len,
               &src_addr);

    /*
     * move payloadbuf_dataptr() to the beginning of the next layer for further
     * processing
     */
    packetbuf_hdrreduce(ies.sixtop_ie_content_ptr - payload_ptr +
                        ies.sixtop_ie_content_len);
  }

  /* Pass to the upper layer */
  llsec_input();
}
/*---------------------------------------------------------------------------*/
void
sixtop_init(void)
{
  int i;

  sixp_init();

  for(i = 0; i < SIXTOP_MAX_SCHEDULING_FUNCTIONS; i++) {
    scheduling_functions[i] = NULL;
  }

  sixtop_init_sf();
}
/*---------------------------------------------------------------------------*/
void
sixtop_init_sf(void)
{
  int i;

  for(i = 0; i < SIXTOP_MAX_SCHEDULING_FUNCTIONS; i++) {
    if(scheduling_functions[i] != NULL &&
       scheduling_functions[i]->init != NULL) {
      scheduling_functions[i]->init();
    }
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
