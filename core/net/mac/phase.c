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
 * $Id: phase.c,v 1.1 2010/02/18 20:58:59 adamdunkels Exp $
 */

/**
 * \file
 *         Common functionality for phase optimization in duty cycling radio protocols
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/mac/phase.h"
#include "net/rime/packetbuf.h"

/*---------------------------------------------------------------------------*/
void
phase_register(const struct phase_list *list,
                   const rimeaddr_t * neighbor, rtimer_clock_t time)
{
  struct phase *e;

  /* If we have an entry for this neighbor already, we renew it. */
  for(e = list_head(*list->list); e != NULL; e = e->next) {
    if(rimeaddr_cmp(neighbor, &e->neighbor)) {
      e->time = time;
      break;
    }
  }
  /* No matching phase was found, so we allocate a new one. */
  if(e == NULL) {
    e = memb_alloc(list->memb);
    if(e == NULL) {
      /* We could not allocate memory for this phase, so we drop
         the last item on the list and reuse it for our phase. */
      e = list_chop(*list->list);
    }
    rimeaddr_copy(&e->neighbor, neighbor);
    e->time = time;
    list_push(*list->list, e);
  }
}
/*---------------------------------------------------------------------------*/
void
phase_wait(struct phase_list *list,
           const rimeaddr_t *neighbor, rtimer_clock_t cycle_time,
           rtimer_clock_t wait_before)
{
  struct phase *e;
  
  /* We go through the list of phases to find if we have recorded
     an phase with this particular neighbor. If so, we can compute
     the time for the next expected phase and setup a ctimer to
     switch on the radio just before the phase. */
  for(e = list_head(*list->list); e != NULL; e = e->next) {
    const rimeaddr_t *neighbor = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

    if(rimeaddr_cmp(neighbor, &e->neighbor)) {
      rtimer_clock_t wait, now, expected;

      /* We expect phases to happen every CYCLE_TIME time
         units. The next expected phase is at time e->time +
         CYCLE_TIME. To compute a relative offset, we subtract
         with clock_time(). Because we are only interested in turning
         on the radio within the CYCLE_TIME period, we compute the
         waiting time with modulo CYCLE_TIME. */

      now = RTIMER_NOW();
      wait = (rtimer_clock_t)((e->time - now) & (cycle_time - 1));
      if(wait < wait_before) {
        wait += cycle_time;
      }
      expected = now + wait - wait_before;
      if(!RTIMER_CLOCK_LT(expected, now)) {
        /* Wait until the receiver is expected to be awake */
        while(RTIMER_CLOCK_LT(RTIMER_NOW(), expected));
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
phase_init(struct phase_list *list)
{
  list_init(*list->list);
  memb_init(list->memb);
}
/*---------------------------------------------------------------------------*/
