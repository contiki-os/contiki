/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *         Example glue code between the existing MAC code and the
 *         Contiki mac interface
 *
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Eric Gnoske <egnoske@gmail.com>
 *         Blake Leverett <bleverett@gmail.com>
 *
 * \addtogroup rf230mac
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "net/packetbuf.h"
#include "zmac.h"
#include "mac.h"
#include "frame.h"
#include "radio.h"
#include "tcpip.h"
#include "sicslowmac.h"
#include "sicslowpan.h"
#include "ieee-15-4-manager.h"

/* Macros */
#define DEBUG 0
#define MAX_EVENTS 10

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define SICSLOW_CORRECTION_DELAY 70
#else
#define PRINTF(...)
#define SICSLOW_CORRECTION_DELAY 7
#endif

#ifdef JACKDAW
#include "sicslow_ethernet.h"
#define LOG_FRAME(x,y) mac_logTXtoEthernet(x,y)
#else
#define LOG_FRAME(x,y)
#endif

/* Globals */
static struct mac_driver mac_driver_struct;
static struct mac_driver *pmac_driver = &mac_driver_struct;
extern ieee_15_4_manager_t ieee15_4ManagerAddress;
static parsed_frame_t *parsed_frame;

/* The core mac layer has a pointer to the driver name in the first field.
 * It calls the radio driver with radio->send, which is the first field of the radio driver.
 * This glue directs radio->send to the custom mac layer.
 */
const struct mac_driver sicslowmac_driver = {
  (char *)sicslowmac_dataRequest,   //Remove compiler warning.
  /*   read_packet, */
  /*   set_receive_function, */
  /*   on, */
  /*   off, */
};

static struct {
  uint8_t head;
  uint8_t tail;
  event_object_t event_object[MAX_EVENTS];
} event_queue;

/* Prototypes */
static void setinput(void (*r)(const struct mac_driver *d));
void (*pinput)(const struct mac_driver *r);
void sicslowmac_unknownIndication(void);


void (*sicslowmac_snifferhook)(const struct mac_driver *r) = NULL;


/*---------------------------------------------------------------------------*/
/**
 * \brief Checks for any pending events in the queue.
 *
 * \return True if there is a pending event, else false.
 */
uint8_t
mac_event_pending(void)
{
  return (event_queue.head != event_queue.tail);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Puts an event into the queue of events.
 *
 * \param object is a pointer to the event to add to queue.
 */
void
mac_put_event(event_object_t *object)
{
  uint8_t newhead;

  if ((event_queue.head + 1) % MAX_EVENTS == event_queue.tail){
    /* queue full, get outta here */
    return;
  }

  newhead = event_queue.head;

  /* store in queue */
  event_queue.event_object[newhead] = *object;

  /* calculate new head index */
  newhead++;
  if (newhead >= MAX_EVENTS){
    newhead = 0;
  }
  event_queue.head = newhead;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Pulls an event from the event queue.
 * Assumes that there is an event in the queue.  See mac_event_pending().
 *
 * \return Pointer to the event object, or NULL in the event of empty queue.
 */
event_object_t
*mac_get_event(void)
{
  event_object_t *object = NULL;
  volatile uint8_t newtail;

  newtail = event_queue.tail;

  object = &(event_queue.event_object[newtail]);

  /* calculate new tail */
  newtail++;
  if (newtail >= MAX_EVENTS){
    newtail = 0;
  }

  event_queue.tail = newtail;

  return(object);
}

void mac_pollhandler(void)
{
    mac_task(0, NULL);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief This is the main loop task for the MAC.  Called by the
 * main application loop.
 */
void
mac_task(process_event_t ev, process_data_t data)
{
  /* check for event in queue */
  event_object_t *event;

  if(mac_event_pending()){

      event = mac_get_event();

      /* Handle events from radio */
      if (event){

          if (event->event == MAC_EVENT_RX){
              /* got a frame, find out with kind of frame */
              parsed_frame = (parsed_frame_t *)event->data;
              if (parsed_frame->fcf->frameType == DATAFRAME){
                  sicslowmac_dataIndication();
              } else {
			  
		  		  /* Hook to cath unknown frames */
				sicslowmac_unknownIndication();
			  }
	  

			/* Frame no longer in use */
			parsed_frame->in_use = false;
          }

          if (event->event == MAC_EVENT_DROPPED){
              /* Frame was dropped */
              PRINTF("sicslowmac: Frame Dropped!\n");
          }
      }
  }
}
/*---------------------------------------------------------------------------*/
void
setinput(void (*r)(const struct mac_driver *d))
{
  pinput = r;
}
/*---------------------------------------------------------------------------*/
static uint8_t dest_reversed[UIP_LLADDR_LEN];
static uint8_t src_reversed[UIP_LLADDR_LEN];

#  define MSB(u16)        (((uint8_t* )&u16)[1])
#  define LSB(u16)        (((uint8_t* )&u16)[0])

void
sicslowmac_dataIndication(void)
{
  packetbuf_clear();

  
  #if UIP_LLADDR_LEN == 8
    /* Finally, get the stuff into the rime buffer.... */
    packetbuf_copyfrom(parsed_frame->payload, parsed_frame->payload_length);
    packetbuf_set_datalen(parsed_frame->payload_length);
  
  	memcpy(dest_reversed, (uint8_t *)parsed_frame->dest_addr, UIP_LLADDR_LEN);
	memcpy(src_reversed, (uint8_t *)parsed_frame->src_addr, UIP_LLADDR_LEN);
  
	/* Change addresses to expected byte order */
	byte_reverse((uint8_t *)dest_reversed, UIP_LLADDR_LEN);
	byte_reverse((uint8_t *)src_reversed, UIP_LLADDR_LEN);
  
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (const linkaddr_t *)dest_reversed);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (const linkaddr_t *)src_reversed);
	
  #elif UIP_CONF_USE_RUM	
    /* Finally, get the stuff into the rime buffer.... */
    packetbuf_copyfrom(parsed_frame->payload + UIP_DATA_RUM_OFFSET, parsed_frame->payload_length - UIP_DATA_RUM_OFFSET);
    packetbuf_set_datalen(parsed_frame->payload_length + UIP_DATA_RUM_OFFSET);
	
	dest_reversed[0] = MSB(parsed_frame->dest_pid);
	dest_reversed[1] = LSB(parsed_frame->dest_pid);
	dest_reversed[2] = 0;
	dest_reversed[3] = 0;  
	dest_reversed[4] = MSB(parsed_frame->payload[0]); //FinalDestAddr
	dest_reversed[5] = LSB(parsed_frame->payload[1]);
	
	src_reversed[0] = MSB(parsed_frame->src_pid);
	src_reversed[1] = LSB(parsed_frame->src_pid);
	src_reversed[2] = 0;
	src_reversed[3] = 0;  
	src_reversed[4] = MSB(parsed_frame->payload[2]); //originAddr
	src_reversed[5] = LSB(parsed_frame->payload[3]);	

  #else
    /* Finally, get the stuff into the rime buffer.... */
    packetbuf_copyfrom(parsed_frame->payload, parsed_frame->payload_length);
    packetbuf_set_datalen(parsed_frame->payload_length);
  
	dest_reversed[0] = MSB(parsed_frame->dest_pid);
	dest_reversed[1] = LSB(parsed_frame->dest_pid);
	dest_reversed[2] = 0;
	dest_reversed[3] = 0;  
	dest_reversed[4] = MSB(parsed_frame->dest_addr->addr16);
	dest_reversed[5] = LSB(parsed_frame->dest_addr->addr16);
	
	src_reversed[0] = MSB(parsed_frame->src_pid);
	src_reversed[1] = LSB(parsed_frame->src_pid);
	src_reversed[2] = 0;
	src_reversed[3] = 0;  
	src_reversed[4] = MSB(parsed_frame->src_addr->addr16);
	src_reversed[5] = LSB(parsed_frame->src_addr->addr16);

	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (const linkaddr_t *)dest_reversed);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (const linkaddr_t *)src_reversed);	
  
  #endif

  PRINTF("sicslowmac: hand off frame to sicslowpan \n");
  pinput(pmac_driver);
}

void
sicslowmac_unknownIndication(void)
{
  if (sicslowmac_snifferhook) {

	  packetbuf_clear();

	  /* Finally, get the stuff into the rime buffer.... */
	  packetbuf_copyfrom(parsed_frame->payload, parsed_frame->payload_length);
	  packetbuf_set_datalen(parsed_frame->payload_length);
	  
  #if UIP_LLADDR_LEN == 8
  	memcpy(dest_reversed, (uint8_t *)parsed_frame->dest_addr, UIP_LLADDR_LEN);
	memcpy(src_reversed, (uint8_t *)parsed_frame->src_addr, UIP_LLADDR_LEN);
  
	/* Change addresses to expected byte order */
	byte_reverse((uint8_t *)dest_reversed, UIP_LLADDR_LEN);
	byte_reverse((uint8_t *)src_reversed, UIP_LLADDR_LEN);
  
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (const linkaddr_t *)dest_reversed);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (const linkaddr_t *)src_reversed);
	
  #elif UIP_CONF_USE_RUM	
	
	dest_reversed[0] = MSB(parsed_frame->dest_pid);
	dest_reversed[1] = LSB(parsed_frame->dest_pid);
	dest_reversed[2] = 0;
	dest_reversed[3] = 0;  
	dest_reversed[4] = MSB(parsed_frame->payload[0]); //FinalDestAddr
	dest_reversed[5] = LSB(parsed_frame->payload[1]);
	
	src_reversed[0] = MSB(parsed_frame->src_pid);
	src_reversed[1] = LSB(parsed_frame->src_pid);
	src_reversed[2] = 0;
	src_reversed[3] = 0;  
	src_reversed[4] = MSB(parsed_frame->payload[2]); //originAddr
	src_reversed[5] = LSB(parsed_frame->payload[3]);	

  #else
  
	dest_reversed[0] = MSB(parsed_frame->dest_pid);
	dest_reversed[1] = LSB(parsed_frame->dest_pid);
	dest_reversed[2] = 0;
	dest_reversed[3] = 0;  
	dest_reversed[4] = MSB(parsed_frame->dest_addr->addr16);
	dest_reversed[5] = LSB(parsed_frame->dest_addr->addr16);
	
	src_reversed[0] = MSB(parsed_frame->src_pid);
	src_reversed[1] = LSB(parsed_frame->src_pid);
	src_reversed[2] = 0;
	src_reversed[3] = 0;  
	src_reversed[4] = MSB(parsed_frame->src_addr->addr16);
	src_reversed[5] = LSB(parsed_frame->src_addr->addr16);

	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (const linkaddr_t *)dest_reversed);
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (const linkaddr_t *)src_reversed);	
  
  #endif

	  PRINTF("sicslowmac: hand off frame to sniffer \n");
	  
	  sicslowmac_snifferhook(pmac_driver);
	 }
  
}

/*---------------------------------------------------------------------------*/
/**
 * \brief      This is the implementation of the 15.4 MAC Data Request
 *             primitive.
 *
 * \return     Integer denoting success or failure.
 * \retval 0   Failure.
 * \retval 1   Success.
 *
 *             The data request primitive creates the frame header based
 *             on static and dynamic data. The static data will be refined
 *             in phase II of the project. The frame payload and length are
 *             retrieved from the rime buffer and rime length respectively.
 *
 *             When the header and payload are assembled into the
 *             frame_create_params structure, the frame is created
 *             by a call to frame_tx_create and then transmited via
 *             radio_send_data.
 */
/*---------------------------------------------------------------------------*/
int
sicslowmac_dataRequest(void)
{

  _delay_ms(SICSLOW_CORRECTION_DELAY);

   /* create structure to store result. */
  frame_create_params_t params;
  frame_result_t result;

#if NETSTACK_CONF_WITH_RIME
  /* Save the msduHandle in a global variable. */
  msduHandle = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);
#endif

  /* Build the FCF. */
  params.fcf.frameType = DATAFRAME;
  params.fcf.securityEnabled = false;
  params.fcf.framePending = false;
#if NETSTACK_CONF_WITH_RIME
  params.fcf.ackRequired = packetbuf_attr(PACKETBUF_ATTR_RELIABLE);
#endif
  params.fcf.panIdCompression = false;

  /* Insert IEEE 802.15.4 (2003) version bit. */
  params.fcf.frameVersion = IEEE802154_2003;

  /* Increment and set the data sequence number. */
  params.seq = macDSN++;

  /* Complete the addressing fields. */
  /**
     \todo For phase 1 the addresses are all long. We'll need a mechanism
     in the rime attributes to tell the mac to use long or short for phase 2.
  */
  params.fcf.srcAddrMode = LONGADDRMODE;
  params.dest_pid = ieee15_4ManagerAddress.get_dst_panid();

  if(packetbuf_holds_broadcast()) {
    /* Broadcast requires short address mode. */
    params.fcf.destAddrMode = SHORTADDRMODE;
    params.dest_pid = BROADCASTPANDID;
    params.dest_addr.addr16 = BROADCASTADDR;

  } else {

    /* Phase 1.5 - end nodes send to anyone? */
    memcpy(&params.dest_addr, (uint8_t *)packetbuf_addr(PACKETBUF_ADDR_RECEIVER), LONG_ADDR_LEN);
	
    /* Change from sicslowpan byte arrangement to sicslowmac */
    byte_reverse((uint8_t*)&params.dest_addr.addr64, LONG_ADDR_LEN);

    /* Phase 1 - end nodes only sends to pan coordinator node. */
    /* params.dest_addr.addr64 = ieee15_4ManagerAddress.get_coord_long_addr(); */
    params.fcf.destAddrMode = LONGADDRMODE;
  }

  /* Set the source PAN ID to the global variable. */
  params.src_pid = ieee15_4ManagerAddress.get_src_panid();

  /*
   * Set up the source address using only the long address mode for
   * phase 1.
   */
  params.src_addr.addr64 = ieee15_4ManagerAddress.get_long_addr();

  /* Copy the payload data. */
  params.payload_len = packetbuf_datalen();
  params.payload =  packetbuf_dataptr();

  /* Create transmission frame. */
  frame_tx_create(&params, &result);
  
  /* Log if needed */
  LOG_FRAME(&params, &result);

  /* Retry up to this many times to send the packet if radio is busy */
  uint8_t retry_count = 3;

  while(retry_count) {

	  PRINTF("sicslowmac: sending packet of length %d to radio, result:", result.length);
	  
	    

	  /* Send data to radio. */
	  radio_status_t rv = radio_send_data(result.length, result.frame);

	  if (rv == RADIO_SUCCESS) {
	      PRINTF(" Success\n");

		  return 1; /* True says that the packet could be sent */
      }


	  if (rv != RADIO_WRONG_STATE) {
	      PRINTF(" Failed\n");
	  	  return 0;
      }

	  PRINTF(" Radio busy, retrying\n");

      /** \todo: Fix delay in sicslowmac so they do not block receiving */

       //We have blocking delay here, it is safest this way. BUT doesn't solve the
	   //problem of TX when you are RXing.. as the RX code can't execute!
	  if (retry_count == 3) {
	 	  _delay_ms(10);
	  } else if (retry_count == 2) {
	      _delay_ms(50);
	  } else if (retry_count == 1) {
	      _delay_ms(200);
	  }

	  retry_count--;
  }

  PRINTF("sicslowmac: Unable to send packet, dropped\n");
  return 0;

}
/*---------------------------------------------------------------------------*/
/**
 * \brief Stub function that will be implemented in phase 2 to cause
 * end nodes to sleep.
 */
int
mac_wake(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Stub function that will be implemented in phase 2 to cause
 * end nodes to sleep.
 */
int
mac_sleep(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
const struct mac_driver *
sicslowmac_init(const struct radio_driver *d)
{
  /* AD: commented out the radio_driver code for now.*/
  /*  radio = d;
      radio->set_receive_function(input_packet);
      radio->on();*/

  return &sicslowmac_driver;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      This is the implementation of the 15.4 MAC Reset Request
 *             primitive.
 * \param      setDefaultPIB True if the default PIB values should be set.
 * \return     Integer denoting success or failure.
 * \retval 0   Failure.
 * \retval 1   Success.
 *
 *             Sets all PIB values to default.
 */
void
sicslowmac_resetRequest (bool setDefaultPIB)
{
  if(setDefaultPIB){
    /* initialize all of the MAC PIB variables to their default values */
    macCoordShortAddress = 0xffff;
    macDSN = rand() % 256;
    macSrcPANId = SOURCE_PAN_ID;
    macDstPANId = DEST_PAN_ID;
    macShortAddress = 0xffff;
    /* Setup the address of this device by reading a stored address from eeprom. */
    /** \todo   This might be read from the serial eeprom onboard Raven. */
    AVR_ENTER_CRITICAL_REGION();
    eeprom_read_block ((void *)&macLongAddr, EEPROMMACADDRESS, 8);
	
	byte_reverse((uint8_t *) &macLongAddr, 8);
	
	
    AVR_LEAVE_CRITICAL_REGION();
  }
}

parsed_frame_t * sicslowmac_get_frame(void)
{
    return parsed_frame;
}

/*---------------------------------------------------------------------------*/
struct mac_driver * sicslowmac_get_driver(void)
{
	return pmac_driver;
}
/*---------------------------------------------------------------------------*/
PROCESS(mac_process, "802.15.4 MAC process");
PROCESS_THREAD(mac_process, ev, data)
{

  PROCESS_POLLHANDLER(mac_pollhandler());


  PROCESS_BEGIN();

  radio_status_t return_value;

  /* init radio */
  /** \todo: this screws up if calosc is set to TRUE, find out why? */
  return_value = radio_init(false, NULL, NULL, NULL);

#if DEBUG
  if (return_value == RADIO_SUCCESS) {
    printf("Radio init successful.\n");
  } else {
    printf("Radio init failed with return: %d\n", return_value);
  }
#endif

  uint8_t eeprom_channel;
  uint8_t eeprom_check;
  
  eeprom_channel = eeprom_read_byte((uint8_t *)9);
  eeprom_check = eeprom_read_byte((uint8_t *)10);
  
  if ((eeprom_channel < 11) || (eeprom_channel > 26) || ((uint8_t)eeprom_channel != (uint8_t)~eeprom_check)) {
#if UIP_CONF_USE_RUM
	eeprom_channel = 19; //Default
#else
	eeprom_channel = 24; //Default
#endif
  }

  radio_set_operating_channel(eeprom_channel);
  radio_use_auto_tx_crc(true);
  radio_set_trx_state(TRX_OFF);

  mac_init();

  /* Set up MAC function pointers and sicslowpan callback. */
  pmac_driver->set_receive_function = setinput;
  pmac_driver->send = sicslowmac_dataRequest;
  sicslowpan_init(pmac_driver);

  ieee_15_4_init(&ieee15_4ManagerAddress);

  radio_set_trx_state(RX_AACK_ON);

  while(1) {
    PROCESS_YIELD();
    mac_task(ev, data);

  }

  PROCESS_END();
}

void byte_reverse(uint8_t * bytes, uint8_t num)
{
  uint8_t tempbyte;
  
  uint8_t i, j;
  
  i = 0;
  j = num - 1;
  
  while(i < j) {
	  tempbyte = bytes[i];
	  bytes[i] = bytes[j];
	  bytes[j] = tempbyte;

	  j--;
	  i++; 
  }
  
  return;
}
