/*
 * Copyright (c) 2019, Sancti Spiritus, Cuba.
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
 *         MLBOT packet format management
 * \author
 *         Bernardo Yaser León Ávila <bernardoyla@gmail.com>
 */

/*---------------------------------------------------------------------------*/
/* Includes */

#include "contiki.h"
#include "sys/rtimer.h"

#include "net/netstack.h"

#include "net/mac/tsch/tsch-log.h"
#include "net/mac/tsch/tsch-slot-operation.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/mac/tsch/tsch-asn.h"

#include "net/mac/tsch/mlbot/mlbot.h"
#include "net/mac/tsch/mlbot/mlbot-packet.h"


#include <stdio.h>
#include <string.h>

#if TSCH_LOG_LEVEL >= 1
#define DEBUG DEBUG_PRINT
#else /* TSCH_LOG_LEVEL */
#define DEBUG DEBUG_NONE
#endif /* TSCH_LOG_LEVEL */
#include "net/net-debug.h"

/*---------------------------------------------------------------------------*/
/*  Create gloval vars
*/
long unsigned next_eb_scheduled;

uint16_t mlbot_lb_interval;

int lb_sended;

/* Timestamp in which the next EB will be transmitted */
rtimer_clock_t next_time_eb;
/* Time in ms needed for the next EB to be transmitted */
static unsigned long next_time_to_eb;
/* ASN where the next EB will be transmitted */
static struct tsch_asn_t next_asn_eb;
/* How many ASN left to reach the next EB */
static uint16_t asn_until_next_eschedule_eb;

light_beacon_t lb;
uint8_t lb_packet_buf [LB_MAX_LEN];
uint8_t lb_packet_buf_lengt;

/*---------------------------------------------------------------------------*/
/* The main MLBOT process */
PROCESS(mlbot_process, "MLBOT: main process");


/*---------------------------------------------------------------------------*/
/**** Funtions ****/

/*---------------------------------------------------------------------------*/
/* Here it is checked if the package I enter is a valid LB */
int check_lb_packet(struct input_packet *input_packet, frame802154_t *lb_frame) {
  
  if (input_packet == NULL) {
    return 0;
  }
/* Without modifying the package we search FCF Frame Control Field
   * It is the 2nd byte of the payload: input_packet-> payload [1]
   * that the frame version is right
   * that is an beacon - input_packet->payload[0] & 7 == 0b0000
   * that does not have IEs: bit 1: & 2
   * If it is a beacon without IEs, it is an LB
   * */

  if (((input_packet->payload[1] >> 4) & 0b11) != FRAME802154_IEEE802154E_2012){
    PRINTF("MLBOT: Wrong frame version\n");
    return 0;
  }
  if ((input_packet->payload[0] & 0b0111) != FRAME802154_BEACONFRAME) {
    PRINTF("MLBOT: Is not a beacon\n");
    return 0;
  }
  if (input_packet->payload[1] & 0b0010) {
    PRINTF("MLBOT: Has IE, is EB\n");
    return 0;
  }
  /* If is here, is a LB */
  if (frame802154_parse ((uint8_t *)input_packet->payload, input_packet->len, lb_frame)){
    return input_packet->len;
  }
  PRINTF ("MLBOT: No se pudo parsear\n");
  return 0;
}


/*---------------------------------------------------------------------------*/
/* Here the LB sequence is prepared */
void prepare_lb_sequece ()
{
  /* next_eb_schedule is in CLOCK_TICKs, turn into RTIMER_TICKs */
  next_eb_scheduled = TSCH_CLOCK_TO_TICKS(next_eb_scheduled);
  asn_until_next_eschedule_eb = (next_eb_scheduled / tsch_timing[tsch_ts_timeslot_length]) + 1;

  /* The next EB ASN */
  next_asn_eb = tsch_current_asn;
  TSCH_ASN_INC(next_asn_eb, asn_until_next_eschedule_eb);

  /* Init the LB struct */
  mlbot_lb_init(&lb);

  /* The next EB channel */
  lb.payload.next_eb_channel = tsch_calculate_channel(&next_asn_eb, 0);

  /* The join priority */
  lb.payload.mac_join_metric = tsch_join_priority;

  /* Start to send LB */
  lb_sch = 1;

}

/*---------------------------------------------------------------------------*/
/* Update the LB */
void mlbot_lb_update(void) 
{
  /* Update how many asn should wait until the next eb  */
  asn_until_next_eschedule_eb = TSCH_ASN_DIFF(next_asn_eb, tsch_current_asn);

  /* The next time until EB in ms */
  next_time_to_eb = RTIMERTICKS_TO_US(tsch_timing[tsch_ts_timeslot_length]) * (asn_until_next_eschedule_eb - 1);
  /* The next time until EB in ms, esta variable sera actualizada continuamente */
  next_time_to_eb = next_time_to_eb - (2 * tsch_timing[tsch_ts_timeslot_length]);
  next_time_to_eb = (next_time_to_eb / 1000);

  lb.payload.time_until_next_eb.ls2b = (uint16_t)next_time_to_eb;
  /* If 2 bytes are no enough */
  if (next_time_to_eb > 0xFFFF) {
    lb.payload.time_until_next_eb.ms1b = (uint8_t) 0x00FF & (next_time_to_eb >> 16);
  }
  
  /* Create a lb's frame to send */
  lb_packet_buf_lengt = mlbot_packet_create_lb(&lb, lb_packet_buf);
  if (asn_until_next_eschedule_eb < TSCH_SCHEDULE_DEFAULT_LENGTH){
    lb_sch = 0;
  }
}

/*---------------------------------------------------------------------------*/
/* Send the LB */
int mlbot_send_lb(void)
{

  if (lb_packet_buf_lengt == 0) {
    PRINTF ("No packet to tx\n");
    return 0;
  }

  /* Hop channel */

  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, tsch_calculate_channel(&tsch_current_asn, current_link->channel_offset));

  if (NETSTACK_RADIO.prepare((void *)lb_packet_buf, (uint8_t)lb_packet_buf_lengt) == 0){
    
    NETSTACK_RADIO.on();
    NETSTACK_RADIO.transmit(lb_packet_buf_lengt);
    NETSTACK_RADIO.off(); 
  }  else {
    PRINTF ("No lb tx\n");
  }


  return 1;

}

/*---------------------------------------------------------------------------*/
/* The main MLBOT process */
PROCESS_THREAD(mlbot_process, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("MLBOT: Process started\n");
  
  next_eb_scheduled = 0;
  lb_sch = 0;

  while (1){
    PROCESS_YIELD_UNTIL(next_eb_scheduled);
    /* If it has been programmed and sent an EB... */
    if (next_eb_scheduled){
      
      PRINTF("EB\n");

      /* LB sequence is prepared */
      prepare_lb_sequece();
      
      /* This value has been transferred to next_time_to_eb in ms */
      next_eb_scheduled = 0;
    }

  }
  PROCESS_END();
}
