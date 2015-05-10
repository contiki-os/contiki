/*
 * Copyright (c) 2015, Hasso-Plattner-Institut.
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
 *         Uses pairwise session keys for securing frames.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "net/llsec/adaptivesec/coresec-strategy.h"
#include "net/llsec/adaptivesec/akes.h"
#include "net/mac/framer-802154.h"
#include "net/mac/contikimac/contikimac-framer.h"
#include "net/llsec/anti-replay.h"
#include "net/nbr-table.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"
#include "net/netstack.h"
#include "net/cmd-broker.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "dev/watchdog.h"
#include <string.h>

#ifdef CORESEC_STRATEGY_CONF_MAX_BUFFERED_MICS
#define MAX_BUFFERED_MICS CORESEC_STRATEGY_CONF_MAX_BUFFERED_MICS
#else /* CORESEC_STRATEGY_CONF_MAX_BUFFERED_MICS */
#define MAX_BUFFERED_MICS 5
#endif /* CORESEC_STRATEGY_CONF_MAX_BUFFERED_MICS */

#define WITH_BROADCAST_ENCRYPTION (ADAPTIVESEC_BROADCAST_SEC_LVL & (1 << 2))

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

struct mic {
  uint8_t u8[ADAPTIVESEC_BROADCAST_MIC_LEN];
};

#if AKES_NBR_WITH_PAIRWISE_KEYS && AKES_NBR_WITH_INDICES

static struct mic mics[MAX_BUFFERED_MICS];
static uint8_t next_mic_index;
static struct cmd_broker_subscription subscription;

/*---------------------------------------------------------------------------*/
/**
 * Payload format:
 * | 0x0d | 0x00 | CCM*-MIC for neighbor 0 | ... | CCM*-MIC for last neighbor |
 */
static void
prepare_announce(void)
{
  uint16_t seqno;
  struct akes_nbr_entry *next;
  uint8_t announced_mics[NBR_TABLE_MAX_NEIGHBORS * ADAPTIVESEC_BROADCAST_MIC_LEN];
  uint8_t *payload;
  uint8_t announced_mics_len;
  uint8_t max_index;
  uint8_t local_index;

  seqno = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
  max_index = 0;
  next = akes_nbr_head();
  while(next) {
    if(next->permanent) {
      local_index = akes_nbr_index_of(next->permanent);
      adaptivesec_aead(next->permanent->pairwise_key,
          0,
          announced_mics + (local_index * ADAPTIVESEC_BROADCAST_MIC_LEN),
          1);
      if(local_index > max_index) {
        max_index = local_index;
      }
    }
    next = akes_nbr_next(next);
  }

  payload = adaptivesec_prepare_command(CORESEC_STRATEGY_ANNOUNCE_IDENTIFIER, &linkaddr_null);
  /*
   * Adding 0xff00 avoids that csma.c confuses sequence numbers of
   * ANNOUNCES and corresponding broadcast frames
   */
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, 0xff00 + seqno);

  /* write payload */
  /* TODO We currently assume that all MICs fit within a single ANNOUNCE command */
  payload[0] = 0;
  announced_mics_len = (max_index + 1) * ADAPTIVESEC_BROADCAST_MIC_LEN;
  memcpy(payload + 1, announced_mics, announced_mics_len);
  packetbuf_set_datalen(1 + 1 + announced_mics_len);
}
/*---------------------------------------------------------------------------*/
static void
send_broadcast(mac_callback_t sent, void *ptr)
{
  struct queuebuf *qb;
#if CONTIKIMAC_FRAMER_ENABLED
  uint8_t datalen;

  datalen = packetbuf_datalen();
#endif /* CONTIKIMAC_FRAMER_ENABLED */

  qb = queuebuf_new_from_packetbuf();
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  if(!qb || (NETSTACK_FRAMER.create() < 0)) {
    PRINTF("coresec-strategy: Did not send broadcast\n");
    if(qb) {
      queuebuf_free(qb);
    }
    sent(ptr, MAC_TX_ERR, 0);
    return;
  }
#if CONTIKIMAC_FRAMER_ENABLED
  packetbuf_set_datalen(datalen);
  ((uint8_t *)packetbuf_dataptr())[-1] = 0x00;
#endif /* CONTIKIMAC_FRAMER_ENABLED */

  prepare_announce();
  adaptivesec_send_command_frame();
  watchdog_periodic();

  queuebuf_to_packetbuf(qb);
  queuebuf_free(qb);
#if WITH_BROADCAST_ENCRYPTION
  {
    uint8_t ignore[ADAPTIVESEC_BROADCAST_MIC_LEN];

    if(adaptivesec_get_sec_lvl() & (1 << 2)) {
      adaptivesec_aead(adaptivesec_group_key, 1, ignore, 1);
    }
  }
#endif /* WITH_BROADCAST_ENCRYPTION */
  NETSTACK_MAC.send(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static void
send(mac_callback_t sent, void *ptr)
{
  if(packetbuf_holds_broadcast()) {
    send_broadcast(sent, ptr);
  } else {
    NETSTACK_MAC.send(sent, ptr);
  }
}
/*---------------------------------------------------------------------------*/
static int
is_mic_stored(uint8_t *mic)
{
  uint8_t i;

  for(i = 0; i < MAX_BUFFERED_MICS; i++) {
    if(!memcmp(mic, mics[i].u8, ADAPTIVESEC_BROADCAST_MIC_LEN)) {
      return 1;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static enum cmd_broker_result
on_command(uint8_t cmd_id, uint8_t *payload)
{
  struct akes_nbr_entry *entry;
  uint8_t *max_payload;

  if(cmd_id != CORESEC_STRATEGY_ANNOUNCE_IDENTIFIER) {
    return CMD_BROKER_UNCONSUMED;
  }

  entry = akes_nbr_get_sender_entry();
  if(!entry || !entry->permanent) {
    return CMD_BROKER_ERROR;
  }

  PRINTF("coresec-strategy: Received ANNOUNCE\n");

  /* calculate CCM*-MIC location */
  payload += 1 + (entry->permanent->foreign_index * ADAPTIVESEC_BROADCAST_MIC_LEN);

  /* check if CCM*-MIC location is within ANNOUNCE */
  max_payload = ((uint8_t *)packetbuf_dataptr()) + packetbuf_datalen() - 1;
  if(payload + ADAPTIVESEC_BROADCAST_MIC_LEN - 1 > max_payload) {
    PRINTF("coresec-strategy: Out of bounds\n");
    return CMD_BROKER_ERROR;
  }

  /*
   * check if contained CCM*-MIC is already stored, e.g.,
   * due to duplicated ANNOUNCE
   */
  if(is_mic_stored(payload)) {
    PRINTF("coresec-strategy: Already stored\n");
    return CMD_BROKER_ERROR;
  }

  /* store CCM*-MIC */
  memcpy(mics[next_mic_index].u8, payload, ADAPTIVESEC_BROADCAST_MIC_LEN);
  if(++next_mic_index == MAX_BUFFERED_MICS) {
    next_mic_index = 0;
  }

  return CMD_BROKER_CONSUMED;
}
/*---------------------------------------------------------------------------*/
static int
on_frame_created(void)
{
  uint8_t sec_lvl;
  enum akes_nbr_status status;
  struct akes_nbr_entry *entry;
  uint8_t *dataptr;
  uint8_t datalen;

  sec_lvl = adaptivesec_get_sec_lvl();
  if(sec_lvl && !packetbuf_holds_broadcast()) {
    status = akes_get_receiver_status();
    entry = akes_nbr_get_receiver_entry();

    if(!entry || !entry->refs[status]) {
      return 0;
    }

    dataptr = packetbuf_dataptr();
    datalen = packetbuf_datalen();

    adaptivesec_aead(entry->refs[status]->pairwise_key,
        sec_lvl & (1 << 2),
        dataptr + datalen,
        1);
    packetbuf_set_datalen(datalen + ADAPTIVESEC_UNICAST_MIC_LEN);
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
verify_broadcast(struct akes_nbr *sender)
{
  uint8_t mic[ADAPTIVESEC_BROADCAST_MIC_LEN];

#if WITH_BROADCAST_ENCRYPTION
  adaptivesec_aead(sender->group_key, adaptivesec_get_sec_lvl() & (1 << 2), mic, 0);
#endif /* WITH_BROADCAST_ENCRYPTION */
  adaptivesec_aead(sender->pairwise_key, 0, mic, 0);

  return !is_mic_stored(mic);
}
/*---------------------------------------------------------------------------*/
static enum adaptivesec_verify
verify(struct akes_nbr *sender)
{
  if(packetbuf_holds_broadcast()) {
    if(verify_broadcast(sender)) {
      PRINTF("coresec-strategy: Inauthentic broadcast\n");
      return ADAPTIVESEC_VERIFY_INAUTHENTIC;
    }
  } else {
#if ANTI_REPLAY_WITH_SUPPRESSION
    packetbuf_set_attr(PACKETBUF_ATTR_NEIGHBOR_INDEX, sender->foreign_index);
#endif /* ANTI_REPLAY_WITH_SUPPRESSION */
    if(adaptivesec_verify(sender->pairwise_key)) {
      PRINTF("coresec-strategy: Inauthentic unicast\n");
      return ADAPTIVESEC_VERIFY_INAUTHENTIC;
    }
  }

  if(anti_replay_was_replayed(&sender->anti_replay_info)) {
    PRINTF("coresec-strategy: Replayed\n");
    return ADAPTIVESEC_VERIFY_REPLAYED;
  }

  return ADAPTIVESEC_VERIFY_SUCCESS;
}
/*---------------------------------------------------------------------------*/
static uint8_t
get_overhead(void)
{
  return packetbuf_holds_broadcast() ? 0 : ADAPTIVESEC_UNICAST_MIC_LEN;
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  subscription.on_command = on_command;
  cmd_broker_subscribe(&subscription);
}
/*---------------------------------------------------------------------------*/
const struct adaptivesec_strategy coresec_strategy = {
  send,
  on_frame_created,
  verify,
  get_overhead,
  init
};
/*---------------------------------------------------------------------------*/
#endif /* AKES_NBR_WITH_PAIRWISE_KEYS && AKES_NBR_WITH_INDICES */
