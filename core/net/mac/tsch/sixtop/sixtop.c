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
 *
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

#include "net/packetbuf.h"
#include "net/mac/frame802154.h"
#include "net/mac/frame802154e-ie.h"

#include "sixtop.h"
#include "sixp.h"

#define DEBUG DEBUG_PRINT
#include "net/net-debug.h"

#if NETSTACK_MAC != tschmac_driver
#error sixtop needs tschmac_driver to be set to NETSTACK_MAC
#endif

const sixtop_sf_t *schedule_functions[SIXTOP_MAX_SCHEDULE_FUNCTIONS];

/*---------------------------------------------------------------------------*/
int
sixtop_add_sf(const sixtop_sf_t *sf)
{
  int i;

  assert(sf != NULL);

  if(sixtop_find_sf(sf->sfid) != NULL) {
    PRINTF("6top: there is another SF installed with the same sfid [%u]\n",
           sf->sfid);
    return -1;
  }

  for(i = 0; i < SIXTOP_MAX_SCHEDULE_FUNCTIONS; i++) {
    if(schedule_functions[i] == NULL) {
      schedule_functions[i] = sf;
      if(sf->init != NULL) {
        sf->init();
      }
      break;
    }
  }

  if(i == SIXTOP_MAX_SCHEDULE_FUNCTIONS) {
    PRINTF("6top: no room to install the specified SF [sfid=%u]\n",
           sf->sfid);
    return -1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
const sixtop_sf_t *
sixtop_find_sf(uint8_t sfid)
{
  int i;

  for(i = 0; i < SIXTOP_MAX_SCHEDULE_FUNCTIONS; i++) {
    if(schedule_functions[i] != NULL &&
       schedule_functions[i]->sfid == sfid) {
      return (const sixtop_sf_t *)schedule_functions[i];
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
void
sixtop_output(const linkaddr_t *dest_addr, mac_callback_t callback, void *arg)
{
  assert(dest_addr != NULL);

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

  for(i = 0; i < SIXTOP_MAX_SCHEDULE_FUNCTIONS; i++) {
    schedule_functions[i] = NULL;
  }
}
/*---------------------------------------------------------------------------*/
void
sixtop_init_sf(void)
{
  int i;

  for(i = 0; i < SIXTOP_MAX_SCHEDULE_FUNCTIONS; i++) {
    if(schedule_functions[i] != NULL &&
       schedule_functions[i]->init != NULL) {
      schedule_functions[i]->init();
    }
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
