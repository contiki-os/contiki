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

#ifndef __TSCH_SECURITY_H__
#define __TSCH_SECURITY_H__

#include "contiki.h"
#include "net/mac/tsch/tsch-asn.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/frame802154e-ie.h"

#ifdef TSCH_SECURITY_CONF_KEY_INDEX_EB
#define TSCH_SECURITY_KEY_INDEX_EB TSCH_SECURITY_CONF_KEY_INDEX_EB
#else
#define TSCH_SECURITY_KEY_INDEX_EB 1
#endif

#ifdef TSCH_SECURITY_CONF_SEC_LEVEL_EB
#define TSCH_SECURITY_KEY_SEC_LEVEL_EB TSCH_SECURITY_CONF_SEC_LEVEL_EB
#else
#define TSCH_SECURITY_KEY_SEC_LEVEL_EB 1 /* MIC-32 */
#endif

#ifdef TSCH_SECURITY_CONF_KEY_INDEX_ACK
#define TSCH_SECURITY_KEY_INDEX_ACK TSCH_SECURITY_CONF_KEY_INDEX_ACK
#else
#define TSCH_SECURITY_KEY_INDEX_ACK 2
#endif

#ifdef TSCH_SECURITY_CONF_SEC_LEVEL_ACK
#define TSCH_SECURITY_KEY_SEC_LEVEL_ACK TSCH_SECURITY_CONF_SEC_LEVEL_ACK
#else
#define TSCH_SECURITY_KEY_SEC_LEVEL_ACK 5 /* Ecnryption + MIC-32 */
#endif

#ifdef TSCH_SECURITY_CONF_KEY_INDEX_OTHER
#define TSCH_SECURITY_KEY_INDEX_OTHER TSCH_SECURITY_CONF_KEY_INDEX_OTHER
#else
#define TSCH_SECURITY_KEY_INDEX_OTHER 2
#endif

#ifdef TSCH_SECURITY_CONF_SEC_LEVEL_OTHER
#define TSCH_SECURITY_KEY_SEC_LEVEL_ACK TSCH_SECURITY_CONF_SEC_LEVEL_OTHER
#else
#define TSCH_SECURITY_KEY_SEC_LEVEL_OTHER 5 /* Ecnryption + MIC-32 */
#endif

int tsch_security_mic_len(frame802154_t *frame);
int tsch_security_secure_frame(uint8_t *hdr, uint8_t *outbuf,
    int hdrlen, int datalen, struct asn_t *asn);
int tsch_security_parse_frame(uint8_t *hdr, int hdrlen, int datalen,
    frame802154_t *frame, const linkaddr_t *sender, struct asn_t *asn);

#endif /* __TSCH_SECURITY_H__ */
