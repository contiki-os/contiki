/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         Example demonstrating the cc2431 location engine.
 *
 *         This file contains code for the blind node. The blind node must be
 *         equipped with a cc2431 SoC (as opposed to reference nodes which
 *         don't need to have a Loc. Eng.)
 *
 *         The blind node receives co-ordinates of reference nodes over
 *         broadcast rime. Once it has enough data (3+ reference nodes), it
 *         will calculate its own position.
 *
 *         We calculate with all potential values for parameter 'n' to
 *         demonstrate how 'n' influences the result of the calculation.
 *
 *         Optionally, send the result of the calculation to a collection node
 *
 *         More information on the cc2431 Location Engine can be found in:
 *         - cc2431 Datasheet
 *         - Texas Instruments Application Note 42
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#include "contiki.h"
#include "net/rime.h"
#include "cc2431_loc_eng.h"
#include "cc2430_sfr.h"

#include <string.h>
#include <stdio.h>

#define MAX_REF_NODES	        16  /* Do not change */

#define SAMPLE_RSSI           100 /* Used for testing */
#define SAMPLE_ALPHA          101

static struct meas_params parameters;
static struct refcoords ref_coords[MAX_REF_NODES];

/* Store our current location here to be transmitted to a collector node */
static uint8_t coords[2];

/*---------------------------------------------------------------------------*/
PROCESS(blindnode_bcast_rec, "Blind Node");
AUTOSTART_PROCESSES(&blindnode_bcast_rec);
/*---------------------------------------------------------------------------*/
/*
 * This handles the calculation cycle. Returns non-zero on error, 0 on success.
 *
 * When we move this outside the example, we will perhaps want to pass
 * struct refcoords *, struct meas_params *
 * instead of exposing our own data structures. If this happens, we will need
 * to add checks to our code to detect non-sane values
 */
static uint8_t
calculate()
{
  static int j, x;
  uint8_t valid_rssi = 0;

  /* Turn on the Engine */
  LOCENG = LOCENG_EN;
  while(!(LOCENG & LOCENG_EN));

  /* Reference Coordinate Load Stage */
  LOCENG |= LOCENG_REFLD;
  while(!(LOCENG & LOCENG_REFLD));

  for(j = 0; j < MAX_REF_NODES; j++) {
    /* Write the Reference Node x,y into the engine */
    REFCOORD = ref_coords[j].x;
    REFCOORD = ref_coords[j].y;
  }

  /* Reference Coordinate Load Stage Done. Proceed with measured params */
  LOCENG &= ~LOCENG_REFLD;
  LOCENG |= LOCENG_PARLD;

  /* Load Parameters */
  MEASPARM = parameters.alpha;
  MEASPARM = parameters.n;
  MEASPARM = parameters.x_min;
  MEASPARM = parameters.x_delta;
  MEASPARM = parameters.y_min;
  MEASPARM = parameters.y_delta;

  /* Load Neighbor RSSIs */
  for(j = 0; j < MAX_REF_NODES; j++) {
    if(parameters.rssi[j] != 0) {
      /* Range-check for the RSSI here, can only be in [-95 dBm , -40 dBm]
       * so we only accept 80 <= rssi <= 190*/
      if(parameters.rssi[j] >= 80 && parameters.rssi[j] <= 190) {
        valid_rssi++;
      }
    }
    /* Write the value, even if it's zero */
    MEASPARM = parameters.rssi[j];
  }

  /* Done with measured parameters too */
  LOCENG &= ~LOCENG_PARLD;

  /* Only Calculate if we have 3+ reference nodes (non-zero RSSIs) */
  if(valid_rssi >= 3) {
    LOCENG |= LOCENG_RUN;
  } else {
    LOCENG = 0;
    printf("some error\n");
    return 1;
  }

  /* Block on the calculation, between 50us and 13ms */
  while(!(LOCENG & LOCENG_DONE));

  /*
   * LOCX contains an offset. Remove it to obtain our actual X value.
   * cc2431 datasheet, section 2.1.3
   */
  x = (LOCX - parameters.x_min + 1) % (parameters.x_delta + 1)
      + parameters.x_min;
  coords[0] = x;
  coords[1] = LOCY; /* No offset here */
  printf("n=%2u: X=%3u, Y=%3u\n", parameters.n, LOCX, LOCY);

  /* Turn it off */
  LOCENG = 0;

  return 0;
}

/*---------------------------------------------------------------------------*/
/*
 * We receive X, Y from reference nodes.
 * We store this in location J of the ref_coords array, where J is the LSB
 * of the reference node's rime address. So we can only accept data from nodes
 * with rime address ending in [0 , 15]
 */
static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  packetbuf_attr_t rssi; /* Careful here, this is uint16_t */

  if(from->u8[1] < MAX_REF_NODES) {
    memset(&ref_coords[from->u8[1] - 1], 0, sizeof(struct refcoords));

    /* Obtain incoming message's RSSI from contiki */
    rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
    /* Convert RSSI to the loc. eng. format */
    parameters.rssi[from->u8[1] - 1] = (-2 * rssi);
    /* Raw dump the packetbuf into the ref_coords struct */
    memcpy(&ref_coords[from->u8[1] - 1], packetbuf_dataptr(), 2 * sizeof(uint8_t));
  }

  return;
}
/*
 * Imaginary nodes to test functionality
 * All nodes at 1 meter distance, rssi = -40 (80)
 * Since the rssi at 1 meter = -40 (A), the blind node should think it's at
 * 5,5
 */
/*---------------------------------------------------------------------------*/
static void
set_imaginary_ref_nodes() {
  ref_coords[0].x = 1;
  ref_coords[0].y = 5;
  parameters.rssi[0] = SAMPLE_RSSI;

  ref_coords[1].x = 5;
  ref_coords[1].y = 1;
  parameters.rssi[1] = SAMPLE_RSSI;

  ref_coords[2].x = 5;
  ref_coords[2].y = 9;
  parameters.rssi[2] = SAMPLE_RSSI;

  ref_coords[3].x = 9;
  ref_coords[3].y = 5;
  parameters.rssi[3] = SAMPLE_RSSI;
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast_call = { broadcast_recv };
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(blindnode_bcast_rec, ev, data)
{
  static struct etimer et;
  static uint8_t n;
  int i;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast));

  PROCESS_BEGIN();

  printf("Reading Chip ID: 0x%02x\n", CHIPID);
  /* Read our chip ID. If we are not cc2431, bail out */
  if(CHIPID != CC2431_CHIP_ID) {
    printf("Hardware does not have a location engine. Exiting.\n");
    PROCESS_EXIT();
  }

  /* OK, we are cc2431. Do stuff */
  n = 0;

  /* Initalise our structs and parameters */
  memset(ref_coords, 0, sizeof(struct refcoords) * MAX_REF_NODES);
  memset(&parameters, 0, sizeof(struct meas_params));

  /*
   * Just hard-coding measurement parameters here.
   * Ideally, this should be part of a calibration mechanism
   */
  parameters.alpha=SAMPLE_ALPHA;
  parameters.x_min=0;
  parameters.x_delta=255;
  parameters.y_min=0;
  parameters.y_delta=255;

  set_imaginary_ref_nodes();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    etimer_set(&et, CLOCK_SECOND);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /*
     * With the hard-coded parameters and locations, we will calculate
     * for all possible values of n [0 , 31]
     */
    parameters.n=n;
    calculate();
    n++;
    if(n==32) { n=0; }

    /* Send our calculated location to some monitoring node */
    packetbuf_copyfrom(&coords, 2*sizeof(uint8_t));
    broadcast_send(&broadcast);
  }
  PROCESS_END();
}
