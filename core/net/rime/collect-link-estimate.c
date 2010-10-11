/**
 * \addtogroup rimelinkestimate
 * @{
 */
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
 * $Id: collect-link-estimate.c,v 1.5 2010/10/11 23:34:02 adamdunkels Exp $
 */

/**
 * \file
 *         Implementation of Collect link estimate based on ETX
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "net/rime/collect.h"
#include "net/rime/collect-link-estimate.h"

/* This defines the window used by the ETX computation when computing
   the ETX. It cannot be larger than
   COLLECT_LINK_ESTIMATE_HISTORY_SIZE, which is defined in
   collect-link-estimate.h. */
#define ETX_HISTORY_WINDOW 8

#define INITIAL_LINK_ESTIMATE 16

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
set_all_estimates(struct collect_link_estimate *le, uint16_t value)
{
  int i;

  for(i = 0; i < ETX_HISTORY_WINDOW; i++) {
    le->history[i] = value;
  }
}
/*---------------------------------------------------------------------------*/
void
collect_link_estimate_new(struct collect_link_estimate *le)
{
  /* Start with a conservative / pessimistic estimate of link quality
     for new links. */
  set_all_estimates(le, 0/*INITIAL_LINK_ESTIMATE*/);
  le->historyptr = 0;
  le->num_estimates = 0;
}
/*---------------------------------------------------------------------------*/
void
collect_link_estimate_update_tx(struct collect_link_estimate *le, uint8_t tx)
{
  if(tx == 0) {
    /*    printf("ERROR tx == 0\n");*/
    return;
  }
  if(le != NULL) {
    /*    if(le->num_estimates == 0) {
      set_all_estimates(le, tx);
      } else*/ {
      le->history[le->historyptr] = tx;
      le->historyptr = (le->historyptr + 1) % ETX_HISTORY_WINDOW;
    }
    if(le->num_estimates < ETX_HISTORY_WINDOW) {
      le->num_estimates++;
    }
  }
}
/*---------------------------------------------------------------------------*/
void
collect_link_estimate_update_tx_fail(struct collect_link_estimate *le,
                                     uint8_t tx)
{
  collect_link_estimate_update_tx(le, tx * 2);
}
/*---------------------------------------------------------------------------*/
void
collect_link_estimate_update_rx(struct collect_link_estimate *n)
{

}
/*---------------------------------------------------------------------------*/
uint16_t
collect_link_estimate(struct collect_link_estimate *le)
{
  int i;
  uint16_t etx;

  if(le->num_estimates == 0) {
    return INITIAL_LINK_ESTIMATE * COLLECT_LINK_ESTIMATE_UNIT;
  }

  PRINTF("collect_link_estimate: ");
  etx = 0;
  for(i = 0; i < le->num_estimates; ++i) {
    PRINTF("%d+", le->history[i]);
    etx += le->history[(le->historyptr - i - 1) & (ETX_HISTORY_WINDOW - 1)];
  }
  PRINTF("/%d = %d\n", i,
         (COLLECT_LINK_ESTIMATE_UNIT * etx) / i);
  return (COLLECT_LINK_ESTIMATE_UNIT * etx) / i;
}
/*---------------------------------------------------------------------------*/
int
collect_link_estimate_num_estimates(struct collect_link_estimate *le)
{
  if(le != NULL) {
    return le->num_estimates;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
