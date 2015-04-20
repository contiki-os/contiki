/*
 * Copyright (c) 2014, Swedish Institute of Computer Science.
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
 *         during a link operation. Saves ASN and other link information
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
#include "lib/ringbufindex.h"

#if WITH_TSCH_LOG

PROCESS_NAME(tsch_pending_events_process);

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

#define TSCH_MAX_LOGS 16
#if (TSCH_MAX_LOGS & (TSCH_MAX_LOGS-1)) != 0
#error TSCH_MAX_LOGS must be power of two
#endif
static struct ringbufindex log_ringbuf;
static struct tsch_log_t log_array[TSCH_MAX_LOGS];
static int log_dropped = 0;

/* Process pending log messages */
void
tsch_log_process_pending()
{
  static int last_log_dropped = 0;
  int16_t log_index;
  /* Loop on accessing (without removing) a pending input packet */
  /* LOG("TSCH: logs in queue %u, total dropped %u\n", ringbufindex_elements(&log_ringbuf), log_dropped); */
  if(log_dropped != last_log_dropped) {
    LOG("TSCH:! logs dropped %u\n", log_dropped);
    last_log_dropped = log_dropped;
  }
  while((log_index = ringbufindex_peek_get(&log_ringbuf)) != -1) {
    struct tsch_log_t *log = &log_array[log_index];
    struct tsch_slotframe *sf = tsch_schedule_get_slotframe_from_handle(log->link->slotframe_handle);
    LOG("TSCH: {asn-%x.%lx link-%u-%u-%u-%u ch-%u} ",
        log->asn.ms1b, log->asn.ls4b,
        log->link->slotframe_handle, sf ? sf->size.val : 0, log->link->timeslot, log->link->channel_offset,
        tsch_calculate_channel(&log->asn, log->link->channel_offset));
    switch(log->type) {
      case tsch_log_tx:
        LOG("%s-%u %u tx %d, st %d-%d",
            log->tx.dest == 0 ? "bc" : "uc", log->tx.is_data,
                log->tx.datalen,
                log->tx.dest,
                log->tx.mac_tx_status, log->tx.num_tx);
        if(log->tx.drift_used) {
          LOG(", dr %d", log->tx.drift);
        }
        LOGA(NULL/*&log->tx.appdata*/, "");
        break;
      case tsch_log_rx:
        LOG("%s-%u %u rx %d",
            log->rx.is_unicast == 0 ? "bc" : "uc", log->rx.is_data,
                log->rx.datalen,
                log->rx.src);
        if(log->rx.drift_used) {
          LOG(", dr %d", log->rx.drift);
        }
        LOGA(NULL/*&log->rx.appdata*/,
            ", edr %d", (int)log->rx.estimated_drift);
        break;
      case tsch_log_message:
        LOG("%s\n", log->message);
        break;
    }
    /* Remove input from ringbuf */
    ringbufindex_get(&log_ringbuf);
  }
}

/* Prepare addition of a new log.
 * Returns pointer to log structure if success, NULL otherwise */
struct tsch_log_t *
tsch_log_prepare_add()
{
  int log_index = ringbufindex_peek_put(&log_ringbuf);
  if(log_index != -1) {
    struct tsch_log_t *log = &log_array[log_index];
    log->asn = current_asn;
    log->link = current_link;
    return log;
  } else {
    log_dropped++;
    return NULL;
  }
}

/* Actually add the previously prepared log */
void
tsch_log_commit()
{
  ringbufindex_put(&log_ringbuf);
  process_poll(&tsch_pending_events_process);
}

/* Initialize log module */
void
tsch_log_init()
{
  ringbufindex_init(&log_ringbuf, TSCH_MAX_LOGS);
}

#endif /* WITH_TSCH_LOG */
