/**
 * \addtogroup rime
 * @{
 */

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 *         Rime initialization and common code
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#include "net/netstack.h"
#include "net/rime.h"
#include "net/rime/chameleon.h"
#include "net/rime/route.h"
#include "net/rime/announcement.h"
#include "net/rime/broadcast-announcement.h"
#include "net/mac/mac.h"

#include "lib/list.h"

#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL
#define BROADCAST_ANNOUNCEMENT_CHANNEL RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL */
#define BROADCAST_ANNOUNCEMENT_CHANNEL 2
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_CHANNEL */

#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME
#define BROADCAST_ANNOUNCEMENT_BUMP_TIME RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME */
#define BROADCAST_ANNOUNCEMENT_BUMP_TIME CLOCK_SECOND * 32 / NETSTACK_RDC_CHANNEL_CHECK_RATE
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_BUMP_TIME */

#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME
#define BROADCAST_ANNOUNCEMENT_MIN_TIME RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME */
#define BROADCAST_ANNOUNCEMENT_MIN_TIME CLOCK_SECOND * 60
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MIN_TIME */

#ifdef RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME
#define BROADCAST_ANNOUNCEMENT_MAX_TIME RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME
#else /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME */
#define BROADCAST_ANNOUNCEMENT_MAX_TIME CLOCK_SECOND * 3600UL
#endif /* RIME_CONF_BROADCAST_ANNOUNCEMENT_MAX_TIME */


LIST(sniffers);

/*---------------------------------------------------------------------------*/
void
rime_sniffer_add(struct rime_sniffer *s)
{
  list_add(sniffers, s);
}
/*---------------------------------------------------------------------------*/
void
rime_sniffer_remove(struct rime_sniffer *s)
{
  list_remove(sniffers, s);
}
/*---------------------------------------------------------------------------*/
static void
input(void)
{
  struct rime_sniffer *s;
  struct channel *c;

  RIMESTATS_ADD(rx);
  c = chameleon_parse();
  
  for(s = list_head(sniffers); s != NULL; s = list_item_next(s)) {
    if(s->input_callback != NULL) {
      s->input_callback();
    }
  }
  
  if(c != NULL) {
    abc_input(c);
  }
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  queuebuf_init();
  packetbuf_clear();
  announcement_init();

  chameleon_init();
  
  /* XXX This is initializes the transmission of announcements but it
   * is not currently certain where this initialization is supposed to
   * be. Also, the times are arbitrarily set for now. They should
   * either be configurable, or derived from some MAC layer property
   * (duty cycle, sleep time, or something similar). But this is OK
   * for now, and should at least get us started with experimenting
   * with announcements.
   */
  broadcast_announcement_init(BROADCAST_ANNOUNCEMENT_CHANNEL,
                              BROADCAST_ANNOUNCEMENT_BUMP_TIME,
                              BROADCAST_ANNOUNCEMENT_MIN_TIME,
                              BROADCAST_ANNOUNCEMENT_MAX_TIME);
}
/*---------------------------------------------------------------------------*/
static void
packet_sent(void *ptr, int status, int num_tx)
{
  struct channel *c = ptr;
  struct rime_sniffer *s;
  
  switch(status) {
  case MAC_TX_COLLISION:
    PRINTF("rime: collision after %d tx\n", num_tx);
    break; 
  case MAC_TX_NOACK:
    PRINTF("rime: noack after %d tx\n", num_tx);
    break;
  case MAC_TX_OK:
    PRINTF("rime: sent after %d tx\n", num_tx);
    break;
  default:
    PRINTF("rime: error %d after %d tx\n", status, num_tx);
  }

  /* Call sniffers, pass along the MAC status code. */
  for(s = list_head(sniffers); s != NULL; s = list_item_next(s)) {
    if(s->output_callback != NULL) {
      s->output_callback(status);
    }
  }

  abc_sent(c, status, num_tx);
}
/*---------------------------------------------------------------------------*/
int
rime_output(struct channel *c)
{
  RIMESTATS_ADD(tx);
  if(chameleon_create(c)) {
    packetbuf_compact();

    NETSTACK_MAC.send(packet_sent, c);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct network_driver rime_driver = {
  "Rime",
  init,
  input
};
/** @} */
