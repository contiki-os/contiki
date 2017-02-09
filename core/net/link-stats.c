/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 *
 * Authors: Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki.h"
#include "sys/clock.h"
#include "net/packetbuf.h"
#include "net/nbr-table.h"
#include "net/link-stats.h"
#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Half time for the freshness counter, in minutes */
#define FRESHNESS_HALF_LIFE             20
/* Statistics are fresh if the freshness counter is FRESHNESS_TARGET or more */
#define FRESHNESS_TARGET                 4
/* Maximum value for the freshness counter */
#define FRESHNESS_MAX                   16
/* Statistics with no update in FRESHNESS_EXPIRATION_TIMEOUT is not fresh */
#define FRESHNESS_EXPIRATION_TIME       (10 * 60 * (clock_time_t)CLOCK_SECOND)

/* EWMA (exponential moving average) used to maintain statistics over time */
#define EWMA_SCALE            100
#define EWMA_ALPHA             15
#define EWMA_BOOTSTRAP_ALPHA   30

/* ETX fixed point divisor. 128 is the value used by RPL (RFC 6551 and RFC 6719) */
#define ETX_DIVISOR     LINK_STATS_ETX_DIVISOR
/* Number of Tx used to update the ETX EWMA in case of no-ACK */
#define ETX_NOACK_PENALTY                   10
/* Initial ETX value */
#define ETX_INIT                             2

/* Per-neighbor link statistics table */
NBR_TABLE(struct link_stats, link_stats);

/* Called every FRESHNESS_HALF_LIFE minutes */
struct ctimer periodic_timer;

/* Used to initialize ETX before any transmission occurs. In order to
 * infer the initial ETX from the RSSI of previously received packets, use: */
/* #define LINK_STATS_CONF_INIT_ETX(stats) guess_etx_from_rssi(stats) */

#ifdef LINK_STATS_CONF_INIT_ETX
#define LINK_STATS_INIT_ETX(stats) LINK_STATS_CONF_INIT_ETX(stats)
#else /* LINK_STATS_INIT_ETX */
#define LINK_STATS_INIT_ETX(stats) (ETX_INIT * ETX_DIVISOR)
#endif /* LINK_STATS_INIT_ETX */

/*---------------------------------------------------------------------------*/
/* Returns the neighbor's link stats */
const struct link_stats *
link_stats_from_lladdr(const linkaddr_t *lladdr)
{
  return nbr_table_get_from_lladdr(link_stats, lladdr);
}
/*---------------------------------------------------------------------------*/
/* Are the statistics fresh? */
int
link_stats_is_fresh(const struct link_stats *stats)
{
  return (stats != NULL)
      && clock_time() - stats->last_tx_time < FRESHNESS_EXPIRATION_TIME
      && stats->freshness >= FRESHNESS_TARGET;
}
/*---------------------------------------------------------------------------*/
uint16_t
guess_etx_from_rssi(const struct link_stats *stats)
{
  if(stats != NULL) {
    if(stats->rssi == 0) {
      return ETX_INIT * ETX_DIVISOR;
    } else {
      /* A rough estimate of PRR from RSSI, as a linear function where:
       *      RSSI >= -60 results in PRR of 1
       *      RSSI <= -90 results in PRR of 0
       * prr = (bounded_rssi - RSSI_LOW) / (RSSI_DIFF)
       * etx = ETX_DIVOSOR / ((bounded_rssi - RSSI_LOW) / RSSI_DIFF)
       * etx = (RSSI_DIFF * ETX_DIVOSOR) / (bounded_rssi - RSSI_LOW)
       * */
#define ETX_INIT_MAX 3
#define RSSI_HIGH -60
#define RSSI_LOW  -90
#define RSSI_DIFF (RSSI_HIGH - RSSI_LOW)
      uint16_t etx;
      int16_t bounded_rssi = stats->rssi;
      bounded_rssi = MIN(bounded_rssi, RSSI_HIGH);
      bounded_rssi = MAX(bounded_rssi, RSSI_LOW + 1);
      etx = RSSI_DIFF * ETX_DIVISOR / (bounded_rssi - RSSI_LOW);
      return MIN(etx, ETX_INIT_MAX * ETX_DIVISOR);
    }
  }
  return 0xffff;
}
/*---------------------------------------------------------------------------*/
/* Packet sent callback. Updates stats for transmissions to lladdr */
void
link_stats_packet_sent(const linkaddr_t *lladdr, int status, int numtx)
{
  struct link_stats *stats;
  uint16_t packet_etx;
  uint8_t ewma_alpha;

  if(status != MAC_TX_OK && status != MAC_TX_NOACK) {
    /* Do not penalize the ETX when collisions or transmission errors occur. */
    return;
  }

  stats = nbr_table_get_from_lladdr(link_stats, lladdr);
  if(stats == NULL) {
    /* Add the neighbor */
    stats = nbr_table_add_lladdr(link_stats, lladdr, NBR_TABLE_REASON_LINK_STATS, NULL);
    if(stats != NULL) {
      stats->etx = LINK_STATS_INIT_ETX(stats);
    } else {
      return; /* No space left, return */
    }
  }

  /* Update last timestamp and freshness */
  stats->last_tx_time = clock_time();
  stats->freshness = MIN(stats->freshness + numtx, FRESHNESS_MAX);

  /* ETX used for this update */
  packet_etx = ((status == MAC_TX_NOACK) ? ETX_NOACK_PENALTY : numtx) * ETX_DIVISOR;
  /* ETX alpha used for this update */
  ewma_alpha = link_stats_is_fresh(stats) ? EWMA_ALPHA : EWMA_BOOTSTRAP_ALPHA;

  /* Compute EWMA and update ETX */
  stats->etx = ((uint32_t)stats->etx * (EWMA_SCALE - ewma_alpha) +
      (uint32_t)packet_etx * ewma_alpha) / EWMA_SCALE;
}
/*---------------------------------------------------------------------------*/
/* Packet input callback. Updates statistics for receptions on a given link */
void
link_stats_input_callback(const linkaddr_t *lladdr)
{
  struct link_stats *stats;
  int16_t packet_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

  stats = nbr_table_get_from_lladdr(link_stats, lladdr);
  if(stats == NULL) {
    /* Add the neighbor */
    stats = nbr_table_add_lladdr(link_stats, lladdr, NBR_TABLE_REASON_LINK_STATS, NULL);
    if(stats != NULL) {
      /* Initialize */
      stats->rssi = packet_rssi;
      stats->etx = LINK_STATS_INIT_ETX(stats);
    }
    return;
  }

  /* Update RSSI EWMA */
  stats->rssi = ((int32_t)stats->rssi * (EWMA_SCALE - EWMA_ALPHA) +
      (int32_t)packet_rssi * EWMA_ALPHA) / EWMA_SCALE;
}
/*---------------------------------------------------------------------------*/
/* Periodic timer called every FRESHNESS_HALF_LIFE minutes */
static void
periodic(void *ptr)
{
  /* Age (by halving) freshness counter of all neighbors */
  struct link_stats *stats;
  ctimer_reset(&periodic_timer);
  for(stats = nbr_table_head(link_stats); stats != NULL; stats = nbr_table_next(link_stats, stats)) {
    stats->freshness >>= 1;
  }
}
/*---------------------------------------------------------------------------*/
/* Initializes link-stats module */
void
link_stats_init(void)
{
  nbr_table_register(link_stats, NULL);
  ctimer_set(&periodic_timer, 60 * (clock_time_t)CLOCK_SECOND * FRESHNESS_HALF_LIFE,
      periodic, NULL);
}
