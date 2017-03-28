/*
 * Copyright (c) 2014, SICS Swedish ICT.
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

/**
 * \file
 *         TSCH security
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-security.h"
#include "net/mac/tsch/tsch-log.h"
#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "lib/ccm-star.h"
#include "lib/aes-128.h"
#include <stdio.h>
#include <string.h>

#if TSCH_LOG_LEVEL >= 1
#define DEBUG DEBUG_PRINT
#else /* TSCH_LOG_LEVEL */
#define DEBUG DEBUG_NONE
#endif /* TSCH_LOG_LEVEL */
#include "net/net-debug.h"

/* The two keys K1 and K2 from 6TiSCH minimal configuration
 * K1: well-known, used for EBs
 * K2: secret, used for data and ACK
 * */
static aes_key keys[] = {
  TSCH_SECURITY_K1,
  TSCH_SECURITY_K2
};
#define N_KEYS (sizeof(keys) / sizeof(aes_key))

/*---------------------------------------------------------------------------*/
static void
tsch_security_init_nonce(uint8_t *nonce,
                         const linkaddr_t *sender, struct tsch_asn_t *asn)
{
  memcpy(nonce, sender, 8);
  nonce[8] = asn->ms1b;
  nonce[9] = (asn->ls4b >> 24) & 0xff;
  nonce[10] = (asn->ls4b >> 16) & 0xff;
  nonce[11] = (asn->ls4b >> 8) & 0xff;
  nonce[12] = (asn->ls4b) & 0xff;
}
/*---------------------------------------------------------------------------*/
static int
tsch_security_check_level(const frame802154_t *frame)
{
  uint8_t required_security_level;
  uint8_t required_key_index;

  /* Sanity check */
  if(frame == NULL) {
    return 0;
  }

  /* Non-secured frame, ok iff we are not in a secured PAN
   * (i.e. scanning or associated to a non-secured PAN) */
  if(frame->fcf.security_enabled == 0) {
    return !(tsch_is_associated == 1 && tsch_is_pan_secured == 1);
  }

  /* The frame is secured, that we are not in an unsecured PAN */
  if(tsch_is_associated == 1 && tsch_is_pan_secured == 0) {
    return 0;
  }

  /* The frame is secured, check its security level */
  switch(frame->fcf.frame_type) {
    case FRAME802154_BEACONFRAME:
      required_security_level = TSCH_SECURITY_KEY_SEC_LEVEL_EB;
      required_key_index = TSCH_SECURITY_KEY_INDEX_EB;
      break;
    case FRAME802154_ACKFRAME:
      required_security_level = TSCH_SECURITY_KEY_SEC_LEVEL_ACK;
      required_key_index = TSCH_SECURITY_KEY_INDEX_ACK;
      break;
    default:
      required_security_level = TSCH_SECURITY_KEY_SEC_LEVEL_OTHER;
      required_key_index = TSCH_SECURITY_KEY_INDEX_OTHER;
      break;
  }
  return ((frame->aux_hdr.security_control.security_level ==
           required_security_level) &&
          frame->aux_hdr.key_index == required_key_index);
}
/*---------------------------------------------------------------------------*/
unsigned int
tsch_security_mic_len(const frame802154_t *frame)
{
  if(frame != NULL && frame->fcf.security_enabled) {
    return 2 << (frame->aux_hdr.security_control.security_level & 0x03);
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
unsigned int
tsch_security_secure_frame(uint8_t *hdr, uint8_t *outbuf,
                           int hdrlen, int datalen, struct tsch_asn_t *asn)
{
  frame802154_t frame;
  uint8_t key_index = 0;
  uint8_t security_level = 0;
  uint8_t with_encryption;
  uint8_t mic_len;
  uint8_t nonce[16];

  uint8_t a_len;
  uint8_t m_len;

  if(hdr == NULL || outbuf == NULL || hdrlen < 0 || datalen < 0) {
    return 0;
  }

  /* Parse the frame header to extract security settings */
  if(frame802154_parse(hdr, hdrlen + datalen, &frame) < 3) {
    return 0;
  }

  if(!frame.fcf.security_enabled) {
    /* Security is not enabled for this frame, we're done */
    return 0;
  }

  /* Read security key index */
  key_index = frame.aux_hdr.key_index;
  security_level = frame.aux_hdr.security_control.security_level;
  with_encryption = (security_level & 0x4) ? 1 : 0;
  mic_len = tsch_security_mic_len(&frame);

  if(key_index == 0 || key_index > N_KEYS) {
    return 0;
  }

  tsch_security_init_nonce(nonce, &linkaddr_node_addr, asn);

  if(with_encryption) {
    a_len = hdrlen;
    m_len = datalen;
  } else {
    a_len = hdrlen + datalen;
    m_len = 0;
  }

  /* Copy source data to output */
  if(hdr != outbuf) {
    memcpy(outbuf, hdr, a_len + m_len);
  }

  CCM_STAR.set_key(keys[key_index - 1]);

  CCM_STAR.aead(nonce,
                outbuf + a_len, m_len,
                outbuf, a_len,
                outbuf + hdrlen + datalen, mic_len, 1);

  return mic_len;
}
/*---------------------------------------------------------------------------*/
unsigned int
tsch_security_parse_frame(const uint8_t *hdr, int hdrlen, int datalen,
                          const frame802154_t *frame, const linkaddr_t *sender,
                          struct tsch_asn_t *asn)
{
  uint8_t generated_mic[16];
  uint8_t key_index = 0;
  uint8_t security_level = 0;
  uint8_t with_encryption;
  uint8_t mic_len;
  uint8_t nonce[16];
  uint8_t a_len;
  uint8_t m_len;

  if(frame == NULL || hdr == NULL || hdrlen < 0 || datalen < 0) {
    return 0;
  }

  if(!tsch_security_check_level(frame)) {
    /* Wrong security level */
    return 0;
  }

  /* No security: nothing more to check */
  if(!frame->fcf.security_enabled) {
    return 1;
  }

  key_index = frame->aux_hdr.key_index;
  security_level = frame->aux_hdr.security_control.security_level;
  with_encryption = (security_level & 0x4) ? 1 : 0;
  mic_len = tsch_security_mic_len(frame);

  /* Check if key_index is in supported range */
  if(key_index == 0 || key_index > N_KEYS) {
    return 0;
  }

  tsch_security_init_nonce(nonce, sender, asn);

  if(with_encryption) {
    a_len = hdrlen;
    m_len = datalen;
  } else {
    a_len = hdrlen + datalen;
    m_len = 0;
  }

  CCM_STAR.set_key(keys[key_index - 1]);

  CCM_STAR.aead(nonce,
                (uint8_t *)hdr + a_len, m_len,
                (uint8_t *)hdr, a_len,
                generated_mic, mic_len, 0);

  if(mic_len > 0 && memcmp(generated_mic, hdr + hdrlen + datalen, mic_len) != 0) {
    return 0;
  } else {
    return 1;
  }
}
