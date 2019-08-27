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
#include "net/packetbuf.h"
#include "net/netstack.h"

#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-packet.h"
#include "net/mac/tsch/tsch-log.h"

//#include "net/mac/framer-802154.h"
#include "net/mac/tsch/mlbot/mlbot-packet.h"
#include "net/mac/tsch/mlbot/mlbot.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if TSCH_LOG_LEVEL >= 1
#define DEBUG DEBUG_PRINT
#else /* TSCH_LOG_LEVEL */
#define DEBUG DEBUG_NONE
#endif /* TSCH_LOG_LEVEL */
#include "net/net-debug.h"


/*---------------------------------------------------------------------------*/
/*  LB packet init
*   Aqui nos aseguramos de que la estructura del nuevo LB este completamente limpia
*/
  uint8_t mlbot_lb_init (light_beacon_t *lb){

      //hay que hacer algo con esto o quitarlo
      lb->lb_handle = 1;               
      linkaddr_copy((linkaddr_t *)&lb->src_addr, &linkaddr_node_addr);
      lb->src_pid = frame802154_get_pan_id();
  
      lb->payload.mac_join_metric = tsch_join_priority;
      lb->payload.next_eb_channel = 0;
      lb->payload.time_until_next_eb.ls2b = 0L;
      lb->payload.time_until_next_eb.ms1b = 0;

      lb->next_eb_clock_time = 0;
      lb->link.lqi = 0;                    
      lb->link.rssi = 0;                    
     
      return (lb->lb_handle);
  
  }

/*---------------------------------------------------------------------------*/
/*  Create an LB packet
    El LB no tiene IEs y siempre tiene un tamaño fijo. 
    Aquí se ensambla el frame que va a contener el LB
    en el lb_frame, y luego se construye en el buffer buf
    Param lb Es un light beacon en una estructura tipo light_beacon_t
    Param buf Es el buffer donde será costruido el frame
    retrun / tamanno del header del paquete creado
*/
int mlbot_packet_create_lb(light_beacon_t *lb, uint8_t *buf)
{
  uint8_t hdr_len = 0;
  //uint32_t temp_time_until_next_eb;

  frame802154_t lb_frame;
  /* Reservamos memoria para todo el payload */
  uint8_t lb_payload[5];
  lb_frame.payload = &lb_payload[0];

  /* Create 802.15.4 header */
  lb_frame.fcf.frame_type = FRAME802154_BEACONFRAME;
  lb_frame.fcf.ie_list_present = 0;
  lb_frame.fcf.frame_version = FRAME802154_IEEE802154E_2012;
  lb_frame.fcf.src_addr_mode = LINKADDR_SIZE > 2 ? FRAME802154_LONGADDRMODE : FRAME802154_SHORTADDRMODE;
  lb_frame.fcf.dest_addr_mode = FRAME802154_SHORTADDRMODE;
  lb_frame.fcf.sequence_number_suppression = 1;

  lb_frame.fcf.panid_compression = 0;
  lb_frame.src_pid = frame802154_get_pan_id();
  lb_frame.dest_pid = frame802154_get_pan_id();
  linkaddr_copy((linkaddr_t *)&lb_frame.src_addr, &linkaddr_node_addr);
  lb_frame.dest_addr[0] = 0xff;
  lb_frame.dest_addr[1] = 0xff;

  /*  Prepare the LB payload
  *   Have 3 fields
  *     macNextEbChannel: 1 byte (5 bits to be use)
  *     macJoinMetric: 1 byte 
  *     macNextEbOffser: 2 - 3 bytes
  */

  lb_payload[0] = lb->payload.next_eb_channel;
  lb_payload[1] = lb->payload.mac_join_metric;

  /* En el payload el time_until_next_eb sera organizada desde el byte ls hasta ms */
  lb_payload[2] = 0x00FF & lb->payload.time_until_next_eb.ls2b;
  lb_payload[3] = 0x00FF & lb->payload.time_until_next_eb.ls2b >> 8;

  lb_frame.payload_len = 4;

  /* Si hay algo en el 3er byte se envia */
  if (lb->payload.time_until_next_eb.ms1b != 0){
    lb_payload[4] = lb->payload.time_until_next_eb.ms1b;
    lb_frame.payload_len ++;
  }
  
  PRINTF("TSCH-MLBOT: LB payload -> next_ch: %u, jp: %u, next_time: %#02X%02X", 
        lb_frame.payload[0], 
        lb_frame.payload[1], 
        lb_frame.payload[3], lb_frame.payload[2]);
  if (lb_frame.payload_len == 5){
    PRINTF("+%u",lb_frame.payload[4]);
  }
  PRINTF("\n");

  /* Creat the frame, the dow build the header and return it size */
  if((hdr_len = frame802154_create(&lb_frame, buf)) == 0) {
    return 0;
  }

  int frame_len = hdr_len;

  int i;
  for (i = 0; i < lb_frame.payload_len; i++){
    buf[frame_len] = lb_frame.payload[i];
    frame_len ++;
  }

  return frame_len;
}

/*---------------------------------------------------------------------------*/
/* Update Time until next LB and next channel in LB packet */
int mlbot_packet_update_lb(light_beacon_t *lb, uint8_t *buf)
{
  PRINTF ("Update a LB packet");
  return 1;
}

/*---------------------------------------------------------------------------*/
/*Parse an LB payload
* \brief  Parse an LB packet payload. This funtion may used when 
          tsch_packet_parse_eb read a beacon and is not a EB.
  \param lb Light beacon struct to store the data parsed
  \param frame A beacon frame.
 */
int mlbot_packet_parse_lb(frame802154_t *frame, light_beacon_t *lb) 
{
  /* Is not a LB if */
  if (frame->fcf.frame_type != FRAME802154_BEACONFRAME ||
      frame->fcf.ie_list_present == 0 ||
      frame->payload_len == 0 || frame->payload_len > 5 ){
    PRINTF("TSCH-MLBOT:! parse_lb: frame is not a LB to");
    return 0; 
  }

  /* Read the LB payload
  *   Have 3 fields
  *     macNextEbChannel: 1 byte (5 bits to be use)
  *     macJoinMetric: 1 byte 
  *     macNextEbOffser: 2 - 3 bytes
  */
  lb->payload.next_eb_channel = frame->payload[0];
  lb->payload.mac_join_metric = frame->payload[1]; 
  
  lb->payload.time_until_next_eb.ls2b = (uint16_t)frame->payload[2];
  lb->payload.time_until_next_eb.ls2b = lb->payload.time_until_next_eb.ls2b | ((uint16_t)frame->payload[3] << 8);

  /* Load in clock_time when the next EB come from this node*/
  lb->next_eb_clock_time = (uint32_t)lb->payload.time_until_next_eb.ls2b;
  lb->next_eb_clock_time = lb->next_eb_clock_time | ((uint32_t)lb->payload.time_until_next_eb.ms1b << 16);
  lb->next_eb_clock_time = clock_time() + lb->next_eb_clock_time;
  
  if (frame->payload_len == 5){
    lb->payload.time_until_next_eb.ms1b = frame->payload[4];
  }

  /* Read the address and panid */
  linkaddr_copy((linkaddr_t *)&lb->src_addr, (linkaddr_t *)&frame->src_addr);
  lb->src_pid = frame->src_pid;

  /* Read the radio and link paramenters */
  lb->link.rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  lb->link.lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

  /* Add new neighbor */
  /* Esto hay que hacerlo con una lista
  cotiki ya trae implementaciones para las listas
  
  LIST(name_list);
  list_add(name_list, n);
  list_remove(name_list, n);
  list_head(name_list);
  list_init(name_list);
  todos definidos en list.h
   */
  
/*   struct tsch_neighbor *n = NULL;
  n = tsch_queue_get_nbr(lb->src_addr);

  if (mlbot_neighbor_amount != 0){

!!!!!!!!!

  }
  mlbot_neighbor_amount ++;
  lb->lb_handle = mlbot_neighbor_amount;
 */
  return 0;
}
