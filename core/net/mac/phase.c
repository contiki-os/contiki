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
 */

/**
 * \file
 *         Common functionality for phase optimization in duty cycling radio protocols
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/mac/phase.h"
#include "net/packetbuf.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "net/queuebuf.h"
#include "net/nbr-table.h"

#if PHASE_CONF_DRIFT_CORRECT
#define PHASE_DRIFT_CORRECT PHASE_CONF_DRIFT_CORRECT
#else
#define PHASE_DRIFT_CORRECT 0
#endif

struct phase {
  rtimer_clock_t time;
#if PHASE_DRIFT_CORRECT
  rtimer_clock_t drift;
#endif
  uint8_t noacks;
  struct timer noacks_timer;
};

struct phase_queueitem {
  struct ctimer timer;
  mac_callback_t mac_callback;
  void *mac_callback_ptr;
  struct queuebuf *q;
  struct rdc_buf_list *buf_list;
};

#if PHASE_CONF_DRIFT_CORRECT
#define PHASE_DRIFT(e) ((e)->drift)
#else
#define PHASE_DRIFT(e) 0
#endif

#define PHASE_DEFER_THRESHOLD 1
#define PHASE_QUEUESIZE       8

#define MAX_NOACKS            16

#define MAX_NOACKS_TIME       CLOCK_SECOND * 30

MEMB(queued_packets_memb, struct phase_queueitem, PHASE_QUEUESIZE);
NBR_TABLE(struct phase, nbr_phase);

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#define PRINTDEBUG(...)
#endif
/*---------------------------------------------------------------------------*/
void
phase_update(const linkaddr_t *neighbor, rtimer_clock_t time,
             int mac_status)
{
  struct phase *e;

  /* If we have an entry for this neighbor already, we renew it. */
  e = nbr_table_get_from_lladdr(nbr_phase, neighbor);
  if(e != NULL) {
    if(mac_status == MAC_TX_OK) {
#if PHASE_DRIFT_CORRECT
      e->drift = time-e->time;
#endif
      e->time = time;
    }
    /* If the neighbor didn't reply to us, it may have switched
       phase (rebooted). We try a number of transmissions to it
       before we drop it from the phase list. */
    if(mac_status == MAC_TX_NOACK) {
      PRINTF("phase noacks %d to %d.%d\n", e->noacks, neighbor->u8[0], neighbor->u8[1]);
      e->noacks++;
      if(e->noacks == 1) {
        timer_set(&e->noacks_timer, MAX_NOACKS_TIME);
      }
      if(e->noacks >= MAX_NOACKS || timer_expired(&e->noacks_timer)) {
        PRINTF("drop %d\n", neighbor->u8[0]);
        nbr_table_remove(nbr_phase, e);
        return;
      }
    } else if(mac_status == MAC_TX_OK) {
      e->noacks = 0;
    }
  } else {
    /* No matching phase was found, so we allocate a new one. */
    if(mac_status == MAC_TX_OK && e == NULL) {
      e = nbr_table_add_lladdr(nbr_phase, neighbor, NBR_TABLE_REASON_MAC, NULL);
      if(e) {
        e->time = time;
#if PHASE_DRIFT_CORRECT
      e->drift = 0;
#endif
      e->noacks = 0;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(void *ptr)
{
  struct phase_queueitem *p = ptr;

  if(p->buf_list == NULL) {
    queuebuf_to_packetbuf(p->q);
    queuebuf_free(p->q);
    NETSTACK_RDC.send(p->mac_callback, p->mac_callback_ptr);
  } else {
    NETSTACK_RDC.send_list(p->mac_callback, p->mac_callback_ptr, p->buf_list);
  }

  memb_free(&queued_packets_memb, p);
}
/*---------------------------------------------------------------------------*/
phase_status_t
phase_wait_rtime(const linkaddr_t *neighbor, rtimer_clock_t cycle_time,
                rtimer_clock_t guard_time, rtimer_clock_t* wait)
{
  rtimer_clock_t now, sync;
  int32_t s;

  /* We go through the list of phases to find if we have recorded a
     phase for this particular neighbor. If so, we can compute the
     time for the next expected phase and setup a ctimer to switch on
     the radio just before the phase. */
  struct phase *e = nbr_table_get_from_lladdr(nbr_phase, neighbor);

  if(e == NULL) {
    return PHASE_UNKNOWN;
  }

  now = RTIMER_NOW();
  sync = (e == NULL) ? now : e->time;

  if(PHASE_DRIFT_CORRECT && (PHASE_DRIFT(e) > cycle_time)) {
    /* drift per cycle */
    s = PHASE_DRIFT(e) % cycle_time / (PHASE_DRIFT(e) / cycle_time);
    /* estimated drift to now */
    s = s * (now - sync) / cycle_time;
    sync += s;
  }

  /* Check if cycle_time is a power of two */
  if(!(cycle_time & (cycle_time - 1))) {
    /* Faster if cycle_time is a power of two */
    *wait = (rtimer_clock_t)((sync - now) & (cycle_time - 1));
  } else {
    /* Works generally */
    *wait = cycle_time - (rtimer_clock_t)((now - sync) % cycle_time);
  }

  if(*wait < guard_time) {
    *wait += cycle_time;
  }

  *wait = *wait - guard_time;

  return PHASE_OK;
}
/*---------------------------------------------------------------------------*/
phase_status_t
phase_wait(const linkaddr_t *neighbor, rtimer_clock_t cycle_time,
           rtimer_clock_t guard_time,
           mac_callback_t mac_callback, void *mac_callback_ptr,
           struct rdc_buf_list *buf_list)
{
  rtimer_clock_t wait;
  rtimer_clock_t expected;
  phase_status_t status;
  clock_time_t ctimewait;
  rtimer_clock_t now;

  status = phase_wait_rtime(neighbor, cycle_time, guard_time, &wait);

  if(status == PHASE_UNKNOWN) {
    return status;
  }

  ctimewait = (CLOCK_SECOND * wait) / RTIMER_ARCH_SECOND;

  if(ctimewait > PHASE_DEFER_THRESHOLD) {
    struct phase_queueitem *p;

    p = memb_alloc(&queued_packets_memb);
    if(p != NULL) {
      if(buf_list == NULL) {
        packetbuf_set_attr(PACKETBUF_ATTR_IS_CREATED_AND_SECURED, 1);
        p->q = queuebuf_new_from_packetbuf();
        if(p->q == NULL) {
          /* memory allocation failed */
          memb_free(&queued_packets_memb, p);
          return PHASE_UNKNOWN;
        }
      }
      p->mac_callback = mac_callback;
      p->mac_callback_ptr = mac_callback_ptr;
      p->buf_list = buf_list;
      ctimer_set(&p->timer, ctimewait, send_packet, p);
      return PHASE_DEFERRED;
    }
  }
  now = RTIMER_NOW();
  expected = now + wait;
  if(!RTIMER_CLOCK_LT(expected, now)) {
    /* Wait until the receiver is expected to be awake */
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), expected));
  }
  return PHASE_SEND_NOW;
}
/*---------------------------------------------------------------------------*/
void
phase_init(void)
{
  memb_init(&queued_packets_memb);
  nbr_table_register(nbr_phase, NULL);
}
/*---------------------------------------------------------------------------*/
