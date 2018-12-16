/*
 * Copyright (c) 2014, Hasso-Plattner-Institut.
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
 *         Protects against replay attacks by comparing with the last
 *         unicast or broadcast frame counter of the sender.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

/**
 * \addtogroup llsec802154
 * @{
 */

#include "net/llsec/anti-replay.h"
#include "net/packetbuf.h"

#if LLSEC802154_USES_FRAME_COUNTER

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

#if ANTI_REPLAY_WITH_SUPPRESSION
uint32_t anti_replay_my_broadcast_counter;
static uint32_t my_unicast_counter;
#else /* ANTI_REPLAY_WITH_SUPPRESSION */
static uint32_t my_counter;
#endif /* ANTI_REPLAY_WITH_SUPPRESSION */

/*----------------------------------------------------------------------------*/
void
anti_replay_parse_counter(uint8_t *p)
{
  frame802154_frame_counter_t frame_counter;

  memcpy(frame_counter.u8, p, 4);
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1, frame_counter.u16[0]);
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3, frame_counter.u16[1]);
}
/*---------------------------------------------------------------------------*/
void
anti_replay_write_counter(uint8_t *dst)
{
  frame802154_frame_counter_t frame_counter;

  frame_counter.u16[0] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1);
  frame_counter.u16[1] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3);
  memcpy(dst, frame_counter.u8, 4);
}
/*---------------------------------------------------------------------------*/
static void
order_and_set_counter(uint32_t counter)
{
  frame802154_frame_counter_t reordered_counter;
  
  reordered_counter.u32 = LLSEC802154_HTONL(counter);
  
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1, reordered_counter.u16[0]);
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3, reordered_counter.u16[1]);
}
/*---------------------------------------------------------------------------*/
int
anti_replay_set_counter(struct anti_replay_info *receiver_info)
{
#if ANTI_REPLAY_WITH_SUPPRESSION
  if(packetbuf_holds_broadcast()) {
    order_and_set_counter(++anti_replay_my_broadcast_counter);
  } else {
    if(++my_unicast_counter == 0xFFFFFFFF) {
      return 0;
    }
    order_and_set_counter(++receiver_info->my_unicast_counter.u32);
  }
#else /* ANTI_REPLAY_WITH_SUPPRESSION */
  order_and_set_counter(++my_counter);
#endif /* ANTI_REPLAY_WITH_SUPPRESSION */
  return anti_replay_get_counter() != 0xFFFFFFFF;
}
/*---------------------------------------------------------------------------*/
uint32_t
anti_replay_get_counter(void)
{
  frame802154_frame_counter_t disordered_counter;
  
  disordered_counter.u16[0] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1);
  disordered_counter.u16[1] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3);
  
  return LLSEC802154_HTONL(disordered_counter.u32);
}
/*---------------------------------------------------------------------------*/
void
anti_replay_suppress_counter(void)
{
  /* use last byte as sequence number */
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO,
      packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) & 0xFF);
}
/*---------------------------------------------------------------------------*/
#if ANTI_REPLAY_WITH_SUPPRESSION
void
anti_replay_restore_counter(struct anti_replay_info *info)
{
  uint8_t seqno;
  frame802154_frame_counter_t copied_counter;

  seqno = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
  copied_counter.u32 = packetbuf_holds_broadcast()
      ? info->his_broadcast_counter.u32
      : info->his_unicast_counter.u32;

  if(seqno < copied_counter.u8[0]) {
    copied_counter.u8[1]++;
    if(!copied_counter.u8[1]) {
      copied_counter.u8[2]++;
      if(!copied_counter.u8[2]) {
        copied_counter.u8[3]++;
      }
    }
  }
  copied_counter.u8[0] = seqno;

  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1, copied_counter.u16[0]);
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3, copied_counter.u16[1]);
}
#endif /* ANTI_REPLAY_WITH_SUPPRESSION */
/*---------------------------------------------------------------------------*/
void
anti_replay_init_info(struct anti_replay_info *info)
{
  memset(info, 0, sizeof(struct anti_replay_info));
#if ANTI_REPLAY_WITH_SUPPRESSION
  info->my_unicast_counter.u32 = my_unicast_counter;
#endif /* ANTI_REPLAY_WITH_SUPPRESSION */
}
/*---------------------------------------------------------------------------*/
int
anti_replay_was_replayed(struct anti_replay_info *info)
{
  uint32_t received_counter;

  received_counter = anti_replay_get_counter();

  if(packetbuf_holds_broadcast()) {
#if DEBUG
    if(received_counter < info->his_broadcast_counter.u32) {
      PRINTF("anti-replay: Broadcast out of order\n");
    }
#endif /* DEBUG */
    if(received_counter <= info->his_broadcast_counter.u32) {
      return 1;
    } else {
      info->his_broadcast_counter.u32 = received_counter;
      return 0;
    }
  } else {
#if DEBUG
    if(received_counter < info->his_unicast_counter.u32) {
      PRINTF("anti-replay: Unicast out of order\n");
    }
#endif /* DEBUG */
    if(received_counter <= info->his_unicast_counter.u32) {
      return 1;
    } else {
      info->his_unicast_counter.u32 = received_counter;
      return 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
#endif /* LLSEC802154_USES_FRAME_COUNTER */

/** @} */
