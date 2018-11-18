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
 *         Deletes inactive permanent neighbors.
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "net/llsec/adaptivesec/akes-delete.h"
#include "net/llsec/adaptivesec/akes.h"
#include "net/packetbuf.h"

#ifdef AKES_DELETE_CONF_UPDATE_CHECK_INTERVAL
#define UPDATE_CHECK_INTERVAL AKES_DELETE_CONF_UPDATE_CHECK_INTERVAL
#else /* AKES_DELETE_CONF_UPDATE_CHECK_INTERVAL */
#define UPDATE_CHECK_INTERVAL (1) /* seconds */
#endif /* AKES_DELETE_CONF_UPDATE_CHECK_INTERVAL */

#ifdef AKES_DELETE_CONF_MAX_RETRANSMISSIONS
#define MAX_RETRANSMISSIONS AKES_DELETE_CONF_MAX_RETRANSMISSIONS
#else /* AKES_DELETE_CONF_MAX_RETRANSMISSIONS */
#define MAX_RETRANSMISSIONS 2
#endif /* AKES_DELETE_CONF_MAX_RETRANSMISSIONS */

#ifdef AKES_DELETE_CONF_UPDATEACK_WAITING_PERIOD
#define UPDATEACK_WAITING_PERIOD AKES_DELETE_CONF_UPDATEACK_WAITING_PERIOD
#else /* AKES_DELETE_CONF_UPDATEACK_WAITING_PERIOD */
#define UPDATEACK_WAITING_PERIOD (15) /* seconds */
#endif /* AKES_DELETE_CONF_UPDATEACK_WAITING_PERIOD */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */

PROCESS(delete_process, "delete_process");

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(delete_process, ev, data)
{
  static struct etimer update_check_timer;
  struct akes_nbr_entry *next;
  static linkaddr_t addr;

  PROCESS_BEGIN();

  while(1) {
    /* randomize the transmission time of UPDATEs to avoid collisions */
    etimer_set(&update_check_timer, adaptivesec_random_clock_time(
        (UPDATE_CHECK_INTERVAL * CLOCK_SECOND) - (CLOCK_SECOND / 2),
        (UPDATE_CHECK_INTERVAL * CLOCK_SECOND) + (CLOCK_SECOND / 2)));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&update_check_timer));
    PRINTF("akes-delete: #permanent = %d\n", akes_nbr_count(AKES_NBR_PERMANENT));
    next = akes_nbr_head();
    while(next) {
      if(!next->permanent || !akes_nbr_is_expired(next, AKES_NBR_PERMANENT)) {
        next = akes_nbr_next(next);
        continue;
      }
      linkaddr_copy(&addr, akes_nbr_get_addr(next));

      /* send UPDATE */
      akes_send_update(next);
      PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
      PRINTF("akes-delete: Sent UPDATE\n");
      etimer_set(&update_check_timer, UPDATEACK_WAITING_PERIOD * CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&update_check_timer));

      next = akes_nbr_get_entry(&addr);
      if(next
          && next->permanent
          && akes_nbr_is_expired(next, AKES_NBR_PERMANENT)) {
        akes_nbr_delete(next, AKES_NBR_PERMANENT);
      }
      next = akes_nbr_head();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
akes_delete_on_update_sent(void *ptr, int status, int transmissions)
{
  process_poll(&delete_process);
}
/*---------------------------------------------------------------------------*/
void
akes_delete_init(void)
{
  process_start(&delete_process, NULL);
}
/*---------------------------------------------------------------------------*/
