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
 *         Log functions for TSCH, meant for logging from interrupt
 *         during a timeslot operation. Saves ASN, slot and link information
 *         and adds the log to a ringbuf for later printout.
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 *
 */

#include "contiki.h"
#include <stdio.h>
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "net/mac/tsch/tsch-private.h"
#include "net/mac/tsch/tsch-log.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-slot-operation.h"
#include "lib/ringbufindex.h"

#if TSCH_LOG_LEVEL >= 1
#define DEBUG DEBUG_PRINT
#else /* TSCH_LOG_LEVEL */
#define DEBUG DEBUG_NONE
#endif /* TSCH_LOG_LEVEL */
#include "net/net-debug.h"

#if TSCH_LOG_LEVEL >= 2 /* Skip this file for log levels 0 or 1 */

PROCESS_NAME(tsch_pending_events_process);

/* Check if TSCH_LOG_QUEUE_LEN is a power of two */
#if (TSCH_LOG_QUEUE_LEN & (TSCH_LOG_QUEUE_LEN - 1)) != 0
#error TSCH_LOG_QUEUE_LEN must be power of two
#endif
static struct ringbufindex log_ringbuf;
static struct tsch_log_t log_array[TSCH_LOG_QUEUE_LEN];
static int log_dropped = 0;

#define LOG_PRINTF(...) printf(__VA_ARGS__)

/*---------------------------------------------------------------------------*/
/* Process pending log messages */
void
tsch_log_process_pending(void)
{
  static int last_log_dropped = 0;
  int16_t log_index;
  /* Loop on accessing (without removing) a pending input packet */
  if(log_dropped != last_log_dropped) {
      LOG_PRINTF("TSCH:! logs dropped %u\n", log_dropped);
    last_log_dropped = log_dropped;
  }
  while((log_index = ringbufindex_peek_get(&log_ringbuf)) != -1) {
    struct tsch_log_t *log = &log_array[log_index];
    if(log->link == NULL) {
        LOG_PRINTF("TSCH: {asn-%x.%lx link-NULL} ", log->asn.ms1b, log->asn.ls4b);
    } else {
      struct tsch_slotframe *sf = tsch_schedule_get_slotframe_by_handle(log->link->slotframe_handle);
      LOG_PRINTF("TSCH: {asn-%x.%lx link-%u-%u-%u-%u ch-%u} ",
             log->asn.ms1b, log->asn.ls4b,
             log->link->slotframe_handle, sf ? sf->size.val : 0, log->link->timeslot, log->link->channel_offset,
             tsch_calculate_channel(&log->asn, log->link->channel_offset));
    }
    switch(log->type) {
      case tsch_log_tx:
          LOG_PRINTF("%s-%u-%u %u tx %x, st %d-%d",
            log->tx.dest == 0 ? "bc" : "uc", log->tx.is_data, log->tx.sec_level,
                log->tx.datalen,
                log->tx.dest,
                log->tx.mac_tx_status, log->tx.num_tx);
        if(log->tx.drift_used) {
            LOG_PRINTF(", dr %d", log->tx.drift);
        }
        LOG_PRINTF("\n");
        break;
      case tsch_log_rx:
          LOG_PRINTF("%s-%u-%u %u rx %x",
            log->rx.is_unicast == 0 ? "bc" : "uc", log->rx.is_data, log->rx.sec_level,
                log->rx.datalen,
                log->rx.src);
        if(log->rx.drift_used) {
            LOG_PRINTF(", dr %d", log->rx.drift);
        }
        LOG_PRINTF(", edr %d\n", (int)log->rx.estimated_drift);
        break;
      case tsch_log_message:
          LOG_PRINTF("%s\n", log->message);
        break;
      case tsch_log_text:
          LOG_PRINTF(log->text);
        break;
      case tsch_log_fmt:
          LOG_PRINTF(log->fmt.text
                     , log->fmt.arg[0], log->fmt.arg[1], log->fmt.arg[2], log->fmt.arg[3]);
        break;

      case tsch_log_fmt8:
          LOG_PRINTF(log->fmt.text
                     , log->fmt.arg[0], log->fmt.arg[1], log->fmt.arg[2], log->fmt.arg[3]
                     , log->fmt.arg[4], log->fmt.arg[5], log->fmt.arg[6], log->fmt.arg[7]
                     );
        break;

      case tsch_log_change_timesrc:
          LOG_PRINTF("TSCH: update time source: %x -> %x\n"
                  , TSCH_LOG_ID_FROM_LINKADDR(&log->timesrc_change.was)
                  , TSCH_LOG_ID_FROM_LINKADDR(&log->timesrc_change.now)
                  );
          break;

      case tsch_log_frame:
          LOG_PRINTF("%s\n    Frame version %u, type %u, FCF %02x %02x\n"
                  , log->frame.msg
                  , log->frame.frame_version, log->frame.frame_type
                  , log->frame.raw[0], log->frame.raw[1]);
          net_debug_lladdr_snprint(log->frame.tmp, sizeof(log->frame.tmp), &log->frame.src_addr);
          LOG_PRINTF("TSCH:! parse_eb: frame was from 0x%x/%s"
                  , log->frame.src_pid, log->frame.tmp);
          net_debug_lladdr_snprint(log->frame.tmp, sizeof(log->frame.tmp), &log->frame.dst_addr);
          LOG_PRINTF(" to 0x%x/%s\n", log->frame.dst_pid, log->frame.tmp);
          break;

      case tsch_log_packet:
          LOG_PRINTF(log->packet.fmt
                  , log->packet.p
                  , log->packet.index
                  );
          break;

      case tsch_log_packet_verbose:
          LOG_PRINTF("TSCH: %s locked(%u), n:%p p:%p[%u]->buf:%p\n"
                  , log->packet.fmt, log->packet.locked
                  , log->packet.n
                  , log->packet.p, log->packet.index
                  , log->packet.qb
                  );
          break;
    }
    /* Remove input from ringbuf */
    ringbufindex_get(&log_ringbuf);
  }
}
/*---------------------------------------------------------------------------*/
/* Prepare addition of a new log.
 * Returns pointer to log structure if success, NULL otherwise */
struct tsch_log_t *
tsch_log_prepare_add(void)
{
  int log_index = ringbufindex_peek_put(&log_ringbuf);
  if(log_index != -1) {
    struct tsch_log_t *log = &log_array[log_index];
    log->asn = tsch_current_asn;
    log->link = current_link;
    return log;
  } else {
    log_dropped++;
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
/* Actually add the previously prepared log */
void
tsch_log_commit(void)
{
  ringbufindex_put(&log_ringbuf);
  process_poll(&tsch_pending_events_process);
}
/*---------------------------------------------------------------------------*/
/* Initialize log module */
void
tsch_log_init(void)
{
  ringbufindex_init(&log_ringbuf, TSCH_LOG_QUEUE_LEN);
}

void tsch_log_puts(const char* txt){
    TSCH_LOG_ADD(tsch_log_text, log->text = txt; );
}

void tsch_log_printf3(const char* fmt, int arg1, int arg2, int arg3){
    TSCH_LOG_ADD(tsch_log_fmt,
                 log->fmt.text = fmt;
                 log->fmt.arg[0] = arg1;
                 log->fmt.arg[1] = arg2;
                 log->fmt.arg[2] = arg3;
                 );
}

void tsch_log_printf4(const char* fmt, int arg1, int arg2, int arg3, int arg4){
    TSCH_LOG_ADD(tsch_log_fmt,
                 log->fmt.text = fmt;
                 log->fmt.arg[0] = arg1;
                 log->fmt.arg[1] = arg2;
                 log->fmt.arg[2] = arg3;
                 log->fmt.arg[3] = arg4;
                 );
}

void tsch_log_printf8(const char* fmt
                      , int arg1, int arg2, int arg3, int arg4
                      , int arg5, int arg6, int arg7, int arg8){
    TSCH_LOG_ADD(tsch_log_fmt8,
                 log->fmt.text = fmt;
                 log->fmt.arg[0] = arg1;
                 log->fmt.arg[1] = arg2;
                 log->fmt.arg[2] = arg3;
                 log->fmt.arg[3] = arg4;
                 log->fmt.arg[4] = arg5;
                 log->fmt.arg[5] = arg6;
                 log->fmt.arg[6] = arg7;
                 log->fmt.arg[7] = arg8;
                 );
}

void tsch_log_print_frame(const char* msg, frame802154_t *frame, const void* raw){
    TSCH_LOG_ADD(tsch_log_frame,
                 log->frame.msg             = msg;
                 log->frame.frame_version   = frame->fcf.frame_version;
                 log->frame.frame_type      = frame->fcf.frame_type;
                 log->frame.src_pid         = frame->src_pid;
                 log->frame.dst_pid         = frame->dest_pid;
                 memcpy(&log->frame.src_addr, frame->src_addr, sizeof(log->frame.src_addr));
                 memcpy(&log->frame.dst_addr, frame->dest_addr, sizeof(log->frame.dst_addr));
                 memcpy(log->frame.raw, raw, sizeof(log->frame.raw));
                 );
}

#endif /* TSCH_LOG_LEVEL */
