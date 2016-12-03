/*
 * Copyright (c) 2015, Scanimetrics - http://www.scanimetrics.com
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
/*---------------------------------------------------------------------------*/
/**
 * \file
 * Implementation of the ContikiMAC using the asyncronous driver
 * interface
 * \author
 * Billy Kozak <bkozak@scanimetrics.com>
 *
 */
/*---------------------------------------------------------------------------*/
#include "net/netstack.h"
#if NETSTACK_USE_RFASYNC
/*---------------------------------------------------------------------------*/
/*---------------------------------INCLUDES----------------------------------*/
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "net/mac/contikimac/contikimac.h"
#include "net/mac/mac-sequence.h"
#include "net/mac/phase.h"
#include "net/netstack.h"
#include "dev/radio-async.h"
#include "net/queuebuf.h"
#include "lib/list.h"
#include "sys/pt.h"
#include "sys/rtimer.h"
#include "sys/compower.h"
#include "sys/isr-control.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/*------------------------------CONFIG CHECKING------------------------------*/
/*---------------------------------------------------------------------------*/
#if !(RTIMER_MULTIPLE_ACCESS)
#error "contikimac-async requires multiple rtimer support!"
#endif
/*---------------------------------------------------------------------------*/
/*-----------------------------------TYPES-----------------------------------*/
/*---------------------------------------------------------------------------*/
enum cm_event {
  CM_EVENT_TX_DONE_OK,
  CM_EVENT_TX_DONE_ERR,
  CM_EVENT_TX_COLLISION,
  CM_EVENT_CCA_DONE_BUSY,
  CM_EVENT_CCA_DONE_IDLE,
  CM_EVENT_CCA_DONE_ERR,
  CM_EVENT_RX_BURST_DONE,
  CM_EVENT_RX_ARRIVED,
  CM_EVENT_GOT_ACK,
  CM_EVENT_TIME_ERROR,
  CM_EVENT_TIME_EXPIRED,
  CM_EVENT_ON_OK,
  CM_EVENT_ON_ERR,
  CM_EVENT_OFF_OK,
  CM_EVENT_OFF_ERR,
  CM_EVENT_NONE
};

enum tx_state {
  TX_STATE_IDLE,
  TX_STATE_PHASE_PENDING,
  TX_STATE_PHASE_WAIT,
  TX_STATE_PENDING,
  TX_STATE_RUNNING,
  TX_STATE_ACK_WAIT,
  TX_STATE_NO_ACK,
  TX_STATE_GOT_ACK,
  TX_STATE_DONE_OK,
  TX_STATE_DONE_COLLISION,
  TX_STATE_DONE_ERR,
  TX_STATE_DONE_NOACK
};

enum rx_state {
  RX_STATE_CYCLE_WAIT,
  RX_STATE_CCA,
  RX_STATE_PACKET_SEEN,
  RX_STATE_PACKET_GOT,
  RX_STATE_PACKET_BURST
};

struct mac_info {
  /* mac callback data */
  mac_callback_t mac_cb;
  void *mac_ptr;

  /* List of queued packets */
  struct rdc_buf_list *buf_list;
};
/*---------------------------------------------------------------------------*/
/*----------------------------------DEFINES----------------------------------*/
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#define PRINTDEBUG(...)
#endif

#define USE_RX_CCA_AS_TX_CCA 1

#define TX_FLAG_BROADCAST     0x01
#define TX_FLAG_RCVR_KNOWN    0x02
#define TX_FLAG_RCVR_AWAKE    0x04
#define TX_FLAG_PENDING       0x08
#define TX_FLAG_CHANNEL_CLEAR 0x10

#define LOCK_UNLOCKED 0
#define LOCK_TX_OWNED 1
#define LOCK_RX_OWNED 2

#define LOCK_ACQUIRE_SUCCESS 0
#define LOCK_ACQUIRE_FAILURE 1

#define CYCLE_MIN_SETUP RTIMER_MINIMAL_SAFE_SCHEDULE

#define RTIMER_INTER_PACKET_DEADLINE (RTIMER_ARCH_SECOND / 32)

#define YIELD_ON_OR_GOTO(pt, stmnt, label) \
  PT_YIELD_ON(pt, if((stmnt)) { goto label; })
#define STROBE_YIELD_ON_OR_GOTO(stmnt, label) \
  YIELD_ON_OR_GOTO(&strobe_state.pt, stmnt, label)
#define CCA_YIELD_ON_OR_GOTO(stmnt, label) \
  YIELD_ON_OR_GOTO(&cca_state.pt, stmnt, label)

#ifdef CONTIKIMAC_CONF_COMPOWER
#define CONTIKIMAC_COMPOWER CONTIKIMAC_CONF_COMPOWER
#else
#define CONTIKIMAC_COMPOWER 0
#endif

#define CCA_TIME (RFASYNC_RX_STABLE_TIME + RFASYNC_OFF_TIME + RFASYNC_ON_TIME)

#define CCA_CYCLE_ON (CCA_SLEEP_TIME > CCA_TIME)

#if CCA_CYCLE_ON
#define CCA_ADJUSTED_SLEEP (CCA_SLEEP_TIME - CCA_TIME)
#else
#define CCA_ADJUSTED_SLEEP CCA_SLEEP_TIME
#endif
/*---------------------------------------------------------------------------*/
/*-----------------------------------VARS------------------------------------*/
/*---------------------------------------------------------------------------*/
static volatile int driver_lock;
static volatile int radio_on;
static volatile int radio_stay_on;

/* static struct rtimer cca_timer; */

static struct {
  volatile int tx_state;
  /* set true if an ACK has been received */
  volatile int ack_done;

  /* Proto-thread state for sending strobes */
  struct pt pt;
  /* For timing between strobes */
  struct rtimer rt;
  /* The time at which the first and last strobes were started */
  rtimer_clock_t t_first;
  rtimer_clock_t t_last;

  struct mac_info info;

  /* Number of strobes sent so far */
  uint16_t count;
  /* sequence number of the packet being sent */
  uint8_t seqno;
  /* Some various boolean flags, marked volatile so that the CCA routine can
     modify it safely under some circumstances. */
  volatile uint8_t flags;
} strobe_state;

static struct {
  /* state of the rx */
  volatile int rx_state;
  /* if true, the rx is running periodic CCAs */
  volatile int on;
  /* Proto-thread state for performing CCA */
  struct pt pt;
  /* Time of the start of the current CCA cycle */
  rtimer_clock_t cycle_start;
  /* Time when the CCA detected a packet */
  rtimer_clock_t pkt_seen_time;
  /* For scheduling the CCA */
  struct rtimer rt;
  /* count of CCA periods with no radio activity */
  uint16_t silence_periods;
  /* multipurpose counter variable */
  uint16_t count;
} cca_state;
/*---------------------------------------------------------------------------*/
/*----------------------------FUNCTION PROTOTYPES----------------------------*/
/*---------------------------------------------------------------------------*/
/* channel checking functionality */
static int call_receiving(void (*cb)(enum cm_event, rtimer_clock_t));
static void receiving_done_cb(struct radio_async_cb_info info, void *ptr);
static int call_channel_clear(void (*cb)(enum cm_event, rtimer_clock_t));
static void channel_clear_done_cb(struct radio_async_cb_info info, void *ptr);

/* power control related functionality */
static int call_on(void (*cb)(enum cm_event, rtimer_clock_t));
static void on_done_cb(struct radio_async_cb_info info, void *ptr);
static int call_off(void (*cb)(enum cm_event, rtimer_clock_t));
static void off_done_cb(struct radio_async_cb_info info, void *ptr);

/* task scheduling and locking functionality */
static int tx_is_done(void);
static int tx_waiting(void);
static void release_driver_lock(void);
static int acquire_driver_lock(int owner);
static void schedule_next_cycle(void);

/* tx related functionality */
static PT_THREAD(strobe_sm(enum cm_event ev, rtimer_clock_t ev_time));
static int init_send(struct mac_info info, bool receiver_awake);
static void finish_strobes(void);
static void start_strobes(void);
static int tx_strobe(void);
static void tx_done_cb(struct radio_async_cb_info info, void *ptr);
static void set_tx_state(enum tx_state state);
static int wait_ack(rtimer_clock_t time);
static int inter_strobe_wait(rtimer_clock_t time);
static void rtimer_strobe_sm(struct rtimer *t, void *ptr);
static void cmd_cb_strobe_sm(enum cm_event ev, rtimer_clock_t ev_time);
static void continue_ack_wait(enum cm_event ev, rtimer_clock_t ev_time);
static void ack_done_cb(uint8_t seqno);
static int strobes_expired(rtimer_clock_t time);
static void strobe_cca_cb(enum cm_event ev, rtimer_clock_t ev_time);
static int strobe_cca(void);
static int tx_keep_going(void);
static int strobe_cca_sleep(rtimer_clock_t now);
static int strobe_cca_wait_stable(rtimer_clock_t now);

/* rx related functionality */
static PT_THREAD(cca_sm(enum cm_event ev, rtimer_clock_t ev_time));
static void cmd_cb_cca_sm(enum cm_event ev, rtimer_clock_t ev_time);
static void rtimer_call_cca_sm(struct rtimer *t, void *ptr);
static int cca_sleep(rtimer_clock_t now);
static int cca_wait_stable(rtimer_clock_t now);
static void cycle_start(struct rtimer *t, void *ptr);
static int cca_wait_check(rtimer_clock_t now);
static bool cca_time_expired(rtimer_clock_t time);
static bool cca_counts_expired(int *no_pkt);
static int schedule_rx_burst_stop(rtimer_clock_t now);
static void rx_isr_cb(bool pending);
static void set_packet_seen(void);

/* packet manipulation */
static int create_and_secure(struct rdc_buf_list *buf_list);

/* Driver API Functions */
static void init(void);
static void qsend_packet(mac_callback_t sent, void *ptr);
static void qsend_list(mac_callback_t sent, void *ptr,
                       struct rdc_buf_list *buf_list);
static void input_packet(void);
static int turn_on(void);
static int turn_off(int keep_radio_on);
static unsigned short duty_cycle(void);

/* Phase Optimization Functions */
static void schedule_phase(void);
static void phase_wait_done(struct rtimer *t, void *ptr);

/* Driver Process */
PROCESS(contikmac_async_process, "cm async process");
/*---------------------------------------------------------------------------*/
/*-------------------------------API FUNCTIONS-------------------------------*/
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  NETSTACK_RFASYNC.set_receiver_callbacks(ack_done_cb, rx_isr_cb);
  process_start(&contikmac_async_process, NULL);

  PT_INIT(&(cca_state.pt));

  turn_on();
}
/*---------------------------------------------------------------------------*/
static void
qsend_packet(mac_callback_t sent, void *ptr)
{
  /* This driver requires that a buffer list be provided because it is
     necesarry to have the packet in the packetbuf before calling
     mac_call_sent_callback and we don't have any way of putting the packet
     back in packetbuf without a buffer list. */
  mac_call_sent_callback(sent, ptr, MAC_TX_ERR_FATAL, 1);
}
/*---------------------------------------------------------------------------*/
static void
qsend_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
  struct mac_info info;

  /* by doing this first we put the packet into the packetbuf */
  if(create_and_secure(buf_list)) {
    mac_call_sent_callback(sent, ptr, MAC_TX_ERR_FATAL, 1);
    return;
  }

  if(strobe_state.tx_state != TX_STATE_IDLE) {
    if(buf_list == strobe_state.info.buf_list) {
      /* The mac layer is trying to re-send a packet we already have */
      PRINTF("qsend_list: resend\n");
      return;
    }
    /* I think that this means that the csma driver is trying to send to a
       different "neighbor" than is the currently running transmission.
       Ideally we shouldn't be responsible for buffering these distinct queues
       (afterall, csma already has buffers allocated) so we return
       MAC_TX_COLLISION for now so that csma is forced to deal with the packets
       itself. A better solution should be implemented in the future. */
    PRINTF("qsend_list: busy\n");
    mac_call_sent_callback(sent, ptr, MAC_TX_COLLISION, 1);
    return;
  }

  info.mac_cb = sent;
  info.mac_ptr = ptr;
  info.buf_list = buf_list;

  if(init_send(info, 0)) {
    mac_call_sent_callback(sent, ptr, MAC_TX_ERR_FATAL, 1);
  }
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  const linkaddr_t *rcvr_addr;
  const linkaddr_t *node_addr;

  if(packetbuf_datalen() == ACK_LEN) {
    return;
  }

  if(packetbuf_totlen() <= 0) {
    return;
  }

  if(NETSTACK_FRAMER.parse() < 0) {
    return;
  }

  if((packetbuf_datalen() <= 0) || (packetbuf_totlen() <= 0)) {
    return;
  }

  rcvr_addr = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  node_addr = &linkaddr_node_addr;

  if(!linkaddr_cmp(rcvr_addr, node_addr) && !packetbuf_holds_broadcast()) {
    return;
  }

  if(RDC_WITH_DUPLICATE_DETECTION) {
    if(mac_sequence_is_duplicate()) {
      return;
    } else {
      mac_sequence_register_seqno();
    }
  }

  NETSTACK_MAC.input();
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  isr_state_t isr_state;

  if(cca_state.on == 0) {
    isr_state = atomic_section_enter();

    cca_state.on = 1;
    radio_stay_on = 0;

    if(driver_lock == LOCK_UNLOCKED) {
      acquire_driver_lock(LOCK_RX_OWNED);
      cca_state.cycle_start = RTIMER_NOW();
      cca_sm(CM_EVENT_NONE, RTIMER_NOW());
    } else if(driver_lock == LOCK_TX_OWNED) {
      cca_state.cycle_start = RTIMER_NOW();
      schedule_next_cycle();
    } else {
      /* if rx owned, the rx will handle everything itself */
    }

    atomic_section_exit(isr_state);
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  radio_stay_on = keep_radio_on;
  cca_state.on = 0;
  return 1;
}
/*---------------------------------------------------------------------------*/
static unsigned short
duty_cycle(void)
{
  return (1ul * CLOCK_SECOND * CYCLE_TIME) / RTIMER_ARCH_SECOND;
}
/*---------------------------------------------------------------------------*/
/*-------------------------TASK SCHEDULING & LOCKING-------------------------*/
/*---------------------------------------------------------------------------*/
static int
acquire_driver_lock(int owner)
{
  int ret = LOCK_ACQUIRE_FAILURE;
  isr_state_t state = atomic_section_enter();
  if((driver_lock == LOCK_UNLOCKED) || (driver_lock == owner)) {
    driver_lock = owner;
    ret = LOCK_ACQUIRE_SUCCESS;
  }
  atomic_section_exit(state);

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
release_driver_lock(void)
{
  driver_lock = LOCK_UNLOCKED;
}
/*---------------------------------------------------------------------------*/
static int
tx_is_done(void)
{
  return strobe_state.tx_state >= TX_STATE_DONE_OK;
}
/*---------------------------------------------------------------------------*/
static int
tx_ret_code(void)
{
  enum tx_state tx_state = strobe_state.tx_state;
  if(tx_state == TX_STATE_DONE_OK) {
    return MAC_TX_OK;
  } else if(tx_state == TX_STATE_DONE_COLLISION) {
    return MAC_TX_COLLISION;
  } else if(tx_state == TX_STATE_DONE_ERR) {
    return MAC_TX_ERR_FATAL;
  } else if(tx_state == TX_STATE_DONE_NOACK) {
    return MAC_TX_NOACK;
  }

  return MAC_TX_ERR_FATAL;
}
/*---------------------------------------------------------------------------*/
static int
tx_waiting(void)
{
  return strobe_state.tx_state == TX_STATE_PENDING;
}
/*---------------------------------------------------------------------------*/
/*------------------------------DRIVER PROCESS-------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(contikmac_async_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD();
    if(ev != PROCESS_EVENT_POLL) {
      continue;
    }

    if(tx_is_done()) {
      finish_strobes();
    } else if(tx_waiting()) {
      start_strobes();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*----------------------------PACKET MANIPULATION----------------------------*/
/*---------------------------------------------------------------------------*/
static int
create_and_secure(struct rdc_buf_list *buf_list)
{
  if(buf_list != NULL) {
    queuebuf_to_packetbuf(buf_list->buf);
  } else {
    return 1;
  }

  if(packetbuf_attr(PACKETBUF_ATTR_IS_CREATED_AND_SECURED)) {
    return 0;
  }

  /* create and secure this frame */
  if(buf_list != NULL && list_item_next(buf_list) != NULL) {
    packetbuf_set_attr(PACKETBUF_ATTR_PENDING, 1);
  }

  packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);

  if(NETSTACK_FRAMER.create() < 0) {
    PRINTF("contikimac: framer failed\n");
    return 1;
  }

  packetbuf_set_attr(PACKETBUF_ATTR_IS_CREATED_AND_SECURED, 1);

  if(buf_list != NULL) {
    queuebuf_update_from_packetbuf(buf_list->buf);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
/*-------------------------------POWER CONTROL-------------------------------*/
/*---------------------------------------------------------------------------*/
static int
call_on(void (*cb)(enum cm_event, rtimer_clock_t))
{
  if(radio_on) {
    cb(CM_EVENT_ON_OK, RTIMER_NOW());
    return 0;
  }

  if(NETSTACK_RFASYNC.on(on_done_cb, cb) == RADIO_ASYNC_RESULT_OK) {
    return 0;
  }

  PRINTF("call_on: failure\n");
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
on_done_cb(struct radio_async_cb_info info, void *ptr)
{
  void (*cb)(enum cm_event, rtimer_clock_t) = ptr;
  enum cm_event ev;

  if(info.status == RADIO_ASYNC_DONE_OK) {
    radio_on = 1;
    ev = CM_EVENT_ON_OK;
  } else {
    PRINTF("on: err %d\n", info.status);
    ev = CM_EVENT_ON_ERR;
  }

  if(cb) {
    cb(ev, info.time);
  }
}
/*---------------------------------------------------------------------------*/
static int
call_off(void (*cb)(enum cm_event, rtimer_clock_t))
{
  if(!radio_on || radio_stay_on) {
    cb(CM_EVENT_OFF_OK, RTIMER_NOW());
    return 0;
  }
  if(NETSTACK_RFASYNC.off(off_done_cb, cb) == RADIO_ASYNC_RESULT_OK) {
    return 0;
  }
  PRINTF("call_off: failure\n");
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
off_done_cb(struct radio_async_cb_info info, void *ptr)
{
  void (*cb)(enum cm_event, rtimer_clock_t) = ptr;
  enum cm_event ev;

  if(info.status == RADIO_ASYNC_DONE_OK) {
    radio_on = 0;
    ev = CM_EVENT_OFF_OK;
  } else {
    ev = CM_EVENT_OFF_ERR;
  }

  if(cb) {
    cb(ev, info.time);
  }
}
/*---------------------------------------------------------------------------*/
/*-----------------------------CHANNEL CHECKING------------------------------*/
/*---------------------------------------------------------------------------*/
static int
call_receiving(void (*cb)(enum cm_event, rtimer_clock_t))
{
  enum radio_async_result res;
  res = NETSTACK_RFASYNC.receiving_packet(receiving_done_cb, cb);
  if(res == RADIO_ASYNC_RESULT_OK) {
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
receiving_done_cb(struct radio_async_cb_info info, void *ptr)
{
  void (*cb)(enum cm_event, rtimer_clock_t) = ptr;
  enum cm_event ev;

  if(info.status == RADIO_ASYNC_DONE_POSITIVE) {
    ev = CM_EVENT_CCA_DONE_BUSY;
  } else if(info.status == RADIO_ASYNC_DONE_NEGATIVE) {
    ev = CM_EVENT_CCA_DONE_IDLE;
  } else {
    PRINTF("receiving err: %d\n", info.status);
    ev = CM_EVENT_CCA_DONE_ERR;
  }

  if(cb) {
    cb(ev, info.time);
  }
}
/*---------------------------------------------------------------------------*/
static int
call_channel_clear(void (*cb)(enum cm_event, rtimer_clock_t))
{
  enum radio_async_result res;
  res = NETSTACK_RFASYNC.channel_clear(channel_clear_done_cb, cb);
  if(res == RADIO_ASYNC_RESULT_OK) {
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
channel_clear_done_cb(struct radio_async_cb_info info, void *ptr)
{
  void (*cb)(enum cm_event, rtimer_clock_t) = ptr;
  enum cm_event ev;

  if(info.status == RADIO_ASYNC_DONE_POSITIVE) {
    ev = CM_EVENT_CCA_DONE_IDLE;
  } else if(info.status == RADIO_ASYNC_DONE_NEGATIVE) {
    ev = CM_EVENT_CCA_DONE_BUSY;
  } else {
    PRINTF("channel_clear err: %d\n", info.status);
    ev = CM_EVENT_CCA_DONE_ERR;
  }

  if(cb) {
    cb(ev, info.time);
  }
}
/*---------------------------------------------------------------------------*/
/*-------------------------------PERIODIC CCA--------------------------------*/
/*---------------------------------------------------------------------------*/
static
PT_THREAD(cca_sm(enum cm_event ev, rtimer_clock_t ev_time))
{
  int no_pkt = 0;

  int silent;
  PT_BEGIN(&(cca_state.pt));

  while(1) {
    cca_state.rx_state = RX_STATE_CCA;
    cca_state.count = 0;
    for(; cca_state.count < CCA_COUNT_MAX; cca_state.count++) {
      CCA_YIELD_ON_OR_GOTO(call_on(cmd_cb_cca_sm), prepare_next);
      if(ev != CM_EVENT_ON_OK) {
        goto prepare_next;
      }
      if(RFASYNC_RX_STABLE_TIME && (!cca_state.count || CCA_CYCLE_ON)) {
        CCA_YIELD_ON_OR_GOTO(cca_wait_stable(ev_time), prepare_next);
      }
      CCA_YIELD_ON_OR_GOTO(call_channel_clear(cmd_cb_cca_sm), prepare_next);
      if(ev == CM_EVENT_CCA_DONE_BUSY) {
        goto wait_for_packet;
      } else if(ev != CM_EVENT_CCA_DONE_IDLE) {
        goto prepare_next;
      }

      if(cca_state.count != (CCA_COUNT_MAX - 1)) {
        if(CCA_CYCLE_ON) {
          CCA_YIELD_ON_OR_GOTO(call_off(cmd_cb_cca_sm), prepare_next);
          if(ev != CM_EVENT_OFF_OK) {
            goto prepare_next;
          }
        }
        CCA_YIELD_ON_OR_GOTO(cca_sleep(ev_time), prepare_next);
      }
    }

    if(ev != CM_EVENT_CCA_DONE_BUSY) {
      no_pkt = 1;
      goto prepare_next;
    }

wait_for_packet:
    cca_state.count = 0;
    cca_state.silence_periods = 0;
    cca_state.pkt_seen_time = ev_time;
    set_packet_seen();

    do {
      cca_state.count += 1;
      CCA_YIELD_ON_OR_GOTO(call_receiving(cmd_cb_cca_sm), prepare_next);
      if(ev == CM_EVENT_CCA_DONE_IDLE) {
        cca_state.silence_periods += 1;
        silent = 1;
      } else if(ev == CM_EVENT_CCA_DONE_BUSY) {
        cca_state.silence_periods = 0;
        silent = 0;
      } else {
        /* error */
        goto prepare_next;
      }

      if(silent && cca_counts_expired(&no_pkt)) {
        goto prepare_next;
      }

      CCA_YIELD_ON_OR_GOTO(cca_wait_check(RTIMER_NOW()), prepare_next);

      if(cca_state.rx_state == RX_STATE_PACKET_GOT) {
        goto prepare_next;
      } else if(cca_state.rx_state == RX_STATE_PACKET_BURST) {
        CCA_YIELD_ON_OR_GOTO(schedule_rx_burst_stop(ev_time), prepare_next);
      }
    } while(!cca_time_expired(ev_time));

prepare_next:
    if(!tx_waiting()) {
      PT_YIELD_ON(&cca_state.pt, call_off(cmd_cb_cca_sm));
    } else if(USE_RX_CCA_AS_TX_CCA && no_pkt) {
      strobe_state.flags |= TX_FLAG_CHANNEL_CLEAR;
    }

    process_poll(&contikmac_async_process);
    release_driver_lock();

    PT_YIELD_ON(&cca_state.pt, schedule_next_cycle());
  }
  PT_END(&cca_state.pt);
}
/*---------------------------------------------------------------------------*/
static void
set_packet_seen(void)
{
  isr_state_t isr_state = atomic_section_enter();
  if(cca_state.rx_state == RX_STATE_CCA) {
    cca_state.rx_state = RX_STATE_PACKET_SEEN;
  }
  atomic_section_exit(isr_state);
}
/*---------------------------------------------------------------------------*/
static void
rx_isr_cb(bool pending)
{
  bool require_run = false;

  isr_state_t isr_state;

  isr_state = atomic_section_enter();
  if(cca_state.rx_state == RX_STATE_CCA) {
    cca_state.rx_state = pending ? RX_STATE_PACKET_BURST : RX_STATE_PACKET_GOT;
    if(rtimer_cancel(&cca_state.rt) == RTIMER_OK) {
      require_run = true;
    }
  } else if(cca_state.rx_state == RX_STATE_PACKET_SEEN) {
    cca_state.rx_state = pending ? RX_STATE_PACKET_BURST : RX_STATE_PACKET_GOT;
  } else if(cca_state.rx_state == RX_STATE_PACKET_BURST) {
    if(pending) {
      if(rtimer_cancel(&cca_state.rt) == RTIMER_OK) {
        schedule_rx_burst_stop(RTIMER_NOW());
      }
    } else {
      if(rtimer_cancel(&cca_state.rt) == RTIMER_OK) {
        require_run = true;
      }
    }
  }
  atomic_section_exit(isr_state);

  if(require_run) {
    cca_sm(CM_EVENT_RX_ARRIVED, RTIMER_NOW());
  }
}
/*---------------------------------------------------------------------------*/
static int
schedule_rx_burst_stop(rtimer_clock_t now)
{
  int ret;

  ret = rtimer_set(&cca_state.rt, now + RTIMER_INTER_PACKET_DEADLINE, 0,
                   rtimer_call_cca_sm, NULL);

  if(ret != RTIMER_OK) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static bool
cca_counts_expired(int *no_pkt)
{
  if(WITH_FAST_SLEEP && (cca_state.count > MAX_NONACTIVITY_PERIODS)) {
    return true;
  }

  if(cca_state.silence_periods > MAX_SILENCE_PERIODS) {
    *no_pkt = 1;
    return true;
  }

  return false;
}
/*---------------------------------------------------------------------------*/
static bool
cca_time_expired(rtimer_clock_t time)
{
  rtimer_clock_t expire = cca_state.pkt_seen_time +
    LISTEN_TIME_AFTER_PACKET_DETECTED;
  return !RTIMER_CLOCK_LT(time, expire);
}
/*---------------------------------------------------------------------------*/
static void
cmd_cb_cca_sm(enum cm_event ev, rtimer_clock_t ev_time)
{
  cca_sm(ev, ev_time);
}
/*---------------------------------------------------------------------------*/
static void
rtimer_call_cca_sm(struct rtimer *t, void *ptr)
{
  cca_sm(CM_EVENT_TIME_EXPIRED, t->time);
}
/*---------------------------------------------------------------------------*/
static int
cca_sleep(rtimer_clock_t now)
{
  int ret = rtimer_set(&cca_state.rt, now + CCA_ADJUSTED_SLEEP, 0,
                       rtimer_call_cca_sm, NULL);

  if(ret != RTIMER_OK) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
cca_wait_stable(rtimer_clock_t now)
{
  int ret;
  rtimer_clock_t next = now + RFASYNC_RX_STABLE_TIME;
  ret = rtimer_set(&cca_state.rt, next, 0, rtimer_call_cca_sm, NULL);

  if(ret != RTIMER_OK) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
cca_wait_check(rtimer_clock_t now)
{
  int ret;
  bool already_have = false;

  rtimer_clock_t next = now + CCA_CHECK_TIME + CCA_SLEEP_TIME;
  isr_state_t isr_state = atomic_section_enter();

  if(cca_state.rx_state == RX_STATE_PACKET_SEEN) {
    ret = rtimer_set(&cca_state.rt, next, 0, rtimer_call_cca_sm, NULL);
  } else {
    already_have = true;
  }

  atomic_section_exit(isr_state);

  if(already_have) {
    cca_sm(CM_EVENT_GOT_ACK, now);
    return 0;
  }

  if(ret != RTIMER_OK) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
cycle_start(struct rtimer *t, void *ptr)
{
  cca_state.cycle_start = t->time;

  if(acquire_driver_lock(LOCK_RX_OWNED)) {
    schedule_next_cycle();
  } else {
    cca_sm(CM_EVENT_TIME_EXPIRED, t->time);
  }
}
/*---------------------------------------------------------------------------*/
static void
schedule_next_cycle(void)
{
  rtimer_clock_t next = cca_state.cycle_start;
  rtimer_clock_t now;

  cca_state.rx_state = RX_STATE_CYCLE_WAIT;

  if(!cca_state.on && (!radio_stay_on || radio_on)) {
    /* don't schedule a next cycle if the cca is off unless the radio needs
       to be turned on due to the radio_stay_on flag being true. */
    return;
  }

  now = RTIMER_NOW();

  do {
    next += CYCLE_TIME;
  } while(RTIMER_CLOCK_LT(next, now + CYCLE_MIN_SETUP));

  rtimer_set(&cca_state.rt, next, 0, cycle_start, NULL);
}
/*---------------------------------------------------------------------------*/
/*------------------------------STROBE SENDING-------------------------------*/
/*---------------------------------------------------------------------------*/
static
PT_THREAD(strobe_sm(enum cm_event ev, rtimer_clock_t ev_time))
{
  PT_BEGIN(&(strobe_state.pt));

  strobe_state.count = 0;

  if(!(strobe_state.flags & (TX_FLAG_RCVR_AWAKE | TX_FLAG_CHANNEL_CLEAR))) {
    for(; strobe_state.count < CCA_COUNT_MAX; strobe_state.count++) {
      STROBE_YIELD_ON_OR_GOTO(call_on(cmd_cb_strobe_sm), stop_thread);
      if(ev != CM_EVENT_ON_OK) {
        goto stop_thread;
      }
      if(RFASYNC_RX_STABLE_TIME && (!strobe_state.count || CCA_CYCLE_ON)) {
        STROBE_YIELD_ON_OR_GOTO(strobe_cca_wait_stable(ev_time), power_off);
      }
      STROBE_YIELD_ON_OR_GOTO(strobe_cca(), power_off);
      if(ev == CM_EVENT_CCA_DONE_BUSY) {
        goto power_off;
      } else if(ev != CM_EVENT_CCA_DONE_IDLE) {
        goto power_off;
      }

      if(strobe_state.count != (CCA_COUNT_MAX - 1)) {
        if(CCA_CYCLE_ON) {
          STROBE_YIELD_ON_OR_GOTO(call_off(cmd_cb_strobe_sm), power_off);
          if(ev != CM_EVENT_OFF_OK) {
            goto power_off;
          }
        }
        STROBE_YIELD_ON_OR_GOTO(strobe_cca_sleep(ev_time), stop_thread);
      }
    }
  }

  if(!radio_on) {
    set_tx_state(TX_STATE_DONE_ERR);
    goto stop_thread;
  }

  if(!(strobe_state.flags & TX_FLAG_BROADCAST)) {
    NETSTACK_RFASYNC.start_ack_detect(strobe_state.seqno);
  }
  strobe_state.t_first = RTIMER_NOW();
  strobe_state.count = 0;
  do {
    strobe_state.count += 1;
    STROBE_YIELD_ON_OR_GOTO(tx_strobe(), strobes_done);
    if(ev != CM_EVENT_TX_DONE_OK) {
      goto strobes_done;
    }

    if(strobe_state.flags & TX_FLAG_BROADCAST) {
      if(strobes_expired(ev_time + INTER_PACKET_INTERVAL)) {
        goto timed_out;
      }
    }

    STROBE_YIELD_ON_OR_GOTO(inter_strobe_wait(ev_time), strobes_done);

    if(!(strobe_state.flags & TX_FLAG_BROADCAST)) {
      STROBE_YIELD_ON_OR_GOTO(wait_ack(ev_time), strobes_done);

      if(ev == CM_EVENT_GOT_ACK) {
        set_tx_state(TX_STATE_DONE_OK);
        goto power_off;
      }
    }
  } while(!strobes_expired(ev_time));

timed_out:
  if(strobe_state.flags & TX_FLAG_BROADCAST) {
    set_tx_state(TX_STATE_DONE_OK);
  } else {
    set_tx_state(TX_STATE_DONE_NOACK);
  }
strobes_done:
  if(!(strobe_state.flags & TX_FLAG_BROADCAST)) {
    NETSTACK_RFASYNC.stop_ack_detect();
  }
power_off:
  if(!tx_keep_going()) {
    STROBE_YIELD_ON_OR_GOTO(call_off(cmd_cb_strobe_sm), stop_thread);
  }
stop_thread:
  if(!tx_keep_going()) {
    release_driver_lock();
  }
  process_poll(&contikmac_async_process);
  PT_END(&(strobe_state.pt));
}
/*---------------------------------------------------------------------------*/
static int
strobe_cca_wait_stable(rtimer_clock_t now)
{
  int ret;
  rtimer_clock_t next = now + RFASYNC_RX_STABLE_TIME;
  ret = rtimer_set(&strobe_state.rt, next, 0, rtimer_strobe_sm, NULL);

  if(ret != RTIMER_OK) {
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
strobe_cca_sleep(rtimer_clock_t now)
{
  int ret = rtimer_set(&strobe_state.rt, now + CCA_ADJUSTED_SLEEP, 0,
                       rtimer_strobe_sm, NULL);

  if(ret != RTIMER_OK) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
strobe_cca_cb(enum cm_event ev, rtimer_clock_t ev_time)
{
  if(ev == CM_EVENT_CCA_DONE_BUSY) {
    set_tx_state(TX_STATE_DONE_COLLISION);
  } else if(ev != CM_EVENT_CCA_DONE_IDLE) {
    set_tx_state(TX_STATE_DONE_ERR);
  }
  strobe_sm(ev, ev_time);
}
/*---------------------------------------------------------------------------*/
static int
strobe_cca(void)
{
  if((call_channel_clear(strobe_cca_cb))) {
    set_tx_state(TX_STATE_DONE_ERR);
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
ack_timeout_expired(struct rtimer *t, void *ptr)
{
  bool timed_out = false;
  isr_state_t state = atomic_section_enter();

  if(strobe_state.tx_state == TX_STATE_ACK_WAIT) {
    set_tx_state(TX_STATE_NO_ACK);
    timed_out = true;
  }

  atomic_section_exit(state);

  if(timed_out) {
    strobe_sm(CM_EVENT_TIME_EXPIRED, t->time);
  }
}
/*---------------------------------------------------------------------------*/
static void
ack_done_cb(uint8_t seqno)
{
  rtimer_clock_t now = RTIMER_NOW();
  bool done_ok = false;
  isr_state_t state;

  strobe_state.ack_done = 1;

  state = atomic_section_enter();
  if(strobe_state.tx_state == TX_STATE_ACK_WAIT) {
    set_tx_state(TX_STATE_GOT_ACK);
    done_ok = true;
  } else {
    PRINTF("ack not ready\n");
    /* The ack got here either before we performed a channel check or after
       the rtimer already expired.
       If could be that the system is very busy or that there is a problem
       (maybe our timeouts need adjusting). */
  }
  atomic_section_exit(state);

  if(done_ok) {
    rtimer_cancel(&strobe_state.rt);
    strobe_sm(CM_EVENT_GOT_ACK, now);
  } else {
  }
}
/*---------------------------------------------------------------------------*/
static void
continue_ack_wait(enum cm_event ev, rtimer_clock_t ev_time)
{
  int rtimer_ret;
  isr_state_t state;
  bool ack_done;

  rtimer_clock_t expire_time = strobe_state.rt.time;
  bool cca_idle = (ev == CM_EVENT_CCA_DONE_IDLE);
  bool rtimer_fail = false;

  state = atomic_section_enter();
  ack_done = strobe_state.ack_done;
  if(!ack_done && !cca_idle) {
    rtimer_ret = rtimer_set(&strobe_state.rt, expire_time, 0,
                            ack_timeout_expired, NULL);

    if(rtimer_ret == RTIMER_OK) {
      set_tx_state(TX_STATE_ACK_WAIT);
    } else {
      rtimer_fail = true;
      set_tx_state(TX_STATE_DONE_ERR);
    }
  } else if(!ack_done) {
    set_tx_state(TX_STATE_NO_ACK);
  } else {
    set_tx_state(TX_STATE_GOT_ACK);
  }
  atomic_section_exit(state);

  if(!ack_done && cca_idle) {
    if(rtimer_fail) {
      strobe_sm(CM_EVENT_TIME_ERROR, ev_time);
    } else {
      strobe_sm(CM_EVENT_TIME_EXPIRED, ev_time);
    }
  } else if(ack_done) {
    strobe_sm(CM_EVENT_GOT_ACK, ev_time);
  }
}
/*---------------------------------------------------------------------------*/
static int
wait_ack(rtimer_clock_t time)
{
  /* We pre-set the expiry time here */
  strobe_state.rt.time = time + AFTER_ACK_DETECTECT_WAIT_TIME;

  if(call_receiving(continue_ack_wait)) {
    set_tx_state(TX_STATE_DONE_ERR);
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
rtimer_strobe_sm(struct rtimer *t, void *ptr)
{
  strobe_sm(CM_EVENT_TIME_EXPIRED, t->time);
}
/*---------------------------------------------------------------------------*/
static void
cmd_cb_strobe_sm(enum cm_event ev, rtimer_clock_t ev_time)
{
  strobe_sm(ev, ev_time);
}
/*---------------------------------------------------------------------------*/
static int
inter_strobe_wait(rtimer_clock_t time)
{
  int ret = rtimer_set(&strobe_state.rt, time + INTER_PACKET_INTERVAL, 0,
                       rtimer_strobe_sm, NULL);

  if(ret != RTIMER_OK) {
    set_tx_state(TX_STATE_DONE_ERR);
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
tx_strobe(void)
{
  strobe_state.t_last = RTIMER_NOW();

  if(NETSTACK_RFASYNC.transmit(tx_done_cb, NULL) == RADIO_ASYNC_RESULT_OK) {
    return 0;
  }
  set_tx_state(TX_STATE_DONE_ERR);
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
tx_done_cb(struct radio_async_cb_info info, void *ptr)
{
  enum cm_event ev;

  if(info.status == RADIO_ASYNC_DONE_OK) {
    ev = CM_EVENT_TX_DONE_OK;
  } else if(info.status == RADIO_ASYNC_DONE_COLLISION) {
    ev = CM_EVENT_TX_COLLISION;
    set_tx_state(TX_STATE_DONE_COLLISION);
  } else {
    ev = CM_EVENT_TX_DONE_ERR;
    set_tx_state(TX_STATE_DONE_ERR);
  }

  strobe_sm(ev, info.time);
}
/*---------------------------------------------------------------------------*/
static int
init_send(struct mac_info info, bool receiver_awake)
{
  enum radio_async_result ret;

  ret = NETSTACK_RFASYNC.prepare(packetbuf_hdrptr(), packetbuf_totlen());
  if(ret != RADIO_ASYNC_RESULT_OK) {
    PRINTF("init_send: prepare returned %d\n", ret);
    return 1;
  }

  PT_INIT(&(strobe_state.pt));
  strobe_state.count = 0;
  strobe_state.seqno = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO) & 0xFF;
  strobe_state.info = info;
  strobe_state.ack_done = 0;

  strobe_state.flags = 0;

  if(packetbuf_holds_broadcast()) {
    strobe_state.flags |= TX_FLAG_BROADCAST;
  }
  if(packetbuf_attr(PACKETBUF_ATTR_PENDING)) {
    strobe_state.flags |= TX_FLAG_PENDING;
  }
  if(receiver_awake) {
    strobe_state.flags |= TX_FLAG_RCVR_AWAKE;
  }

  /* setting TX_STATE_PENDING marks strobe_state.flags as finalized so
   that the flags variable can be written from a periodic CCA ISR. Therefore
   we do this last. */

  if(receiver_awake || !WITH_PHASE_OPTIMIZATION) {
    strobe_state.tx_state = TX_STATE_PENDING;
    start_strobes();
  } else {
    strobe_state.tx_state = TX_STATE_PHASE_PENDING;
    schedule_phase();
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
start_strobes(void)
{
  if(acquire_driver_lock(LOCK_TX_OWNED)) {
    /* when the rx is ready, it will schedule us to run later */
    return;
  }

  strobe_state.tx_state = TX_STATE_RUNNING;

  strobe_sm(CM_EVENT_NONE, RTIMER_NOW());
}
/*---------------------------------------------------------------------------*/
static void
finish_strobes(void)
{
  int mac_ret = tx_ret_code();
  bool next_failed = false;
  bool idle = false;
  struct mac_info info = strobe_state.info;
  struct rdc_buf_list *curr = strobe_state.info.buf_list;
  struct rdc_buf_list *next = list_item_next(curr);
  rtimer_clock_t t_last = strobe_state.t_last;
  bool was_awake = strobe_state.flags & TX_FLAG_RCVR_AWAKE;

  if(tx_keep_going()) {
    info.buf_list = next;
    if(create_and_secure(next) || init_send(info, 1)) {
      next_failed = true;
      idle = true;
    }
  } else {
    idle = true;
  }

  if(next_failed && (driver_lock == LOCK_TX_OWNED)) {
    /* unlock on failure before calling any callbacks becuase they might try to
       send a packet */
    release_driver_lock();
  }
  if(idle) {
    set_tx_state(TX_STATE_IDLE);
  }

  if(mac_ret != MAC_TX_OK) {
    PRINTF("strobe ret %d\n", mac_ret);
  }

  queuebuf_to_packetbuf(curr->buf);

  if(WITH_PHASE_OPTIMIZATION && !was_awake) {
    phase_update(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), t_last, mac_ret);
  }

  mac_call_sent_callback(info.mac_cb, info.mac_ptr, mac_ret, 1);

  if(next_failed) {
    PRINTF("next failed\n");
    queuebuf_to_packetbuf(next->buf);
    mac_call_sent_callback(info.mac_cb, info.mac_ptr, MAC_TX_ERR_FATAL, 1);
  }
}
/*---------------------------------------------------------------------------*/
static int
strobes_expired(rtimer_clock_t time)
{
  return !RTIMER_CLOCK_LT(time, STROBE_TIME + strobe_state.t_first);
}
/*---------------------------------------------------------------------------*/
static int
tx_keep_going(void)
{
  bool pending = (strobe_state.flags & TX_FLAG_PENDING);
  bool no_error = (strobe_state.tx_state == TX_STATE_DONE_OK);

  return pending && no_error;
}
/*---------------------------------------------------------------------------*/
static void
set_tx_state(enum tx_state state)
{
  strobe_state.tx_state = state;
}
/*---------------------------------------------------------------------------*/
/*----------------------------PHASE OPTIMIZATION-----------------------------*/
/*---------------------------------------------------------------------------*/
static void
schedule_phase(void)
{
  phase_status_t phase_status;
  rtimer_clock_t wait;
  int rt_ret;

  set_tx_state(TX_STATE_PHASE_WAIT);

  phase_status = phase_wait_rtime(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                                  CYCLE_TIME, GUARD_TIME, &wait);

  if(phase_status == PHASE_UNKNOWN) {
    phase_wait_done(NULL, NULL);
    return;
  }

  rt_ret = rtimer_set(&strobe_state.rt, RTIMER_NOW() + wait,
                      0, phase_wait_done, NULL);

  if(rt_ret != RTIMER_OK) {
    phase_wait_done(NULL, NULL);
  }
}
/*---------------------------------------------------------------------------*/
static void
phase_wait_done(struct rtimer *t, void *ptr)
{
  set_tx_state(TX_STATE_PENDING);
  start_strobes();
}
/*---------------------------------------------------------------------------*/
/*-------------------------------DRIVER STRUCT-------------------------------*/
/*---------------------------------------------------------------------------*/
const struct rdc_driver contikimac_async_driver = {
  "ContikiMAC-Async",
  init,
  qsend_packet,
  qsend_list,
  input_packet,
  turn_on,
  turn_off,
  duty_cycle,
};
/*---------------------------------------------------------------------------*/
#endif /* NETSTACK_USE_RFASYNC */
/*---------------------------------------------------------------------------*/