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
 * $Id: radio-arch.c,v 1.1 2006/08/21 12:11:20 fros4943 Exp $
 */

#include "dev/radio-arch.h"
#include "dev/radio.h"

#include "lib/simEnvChange.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "net/uip.h"
#include "net/uip-fw.h"
#include "sys/etimer.h"

#include "sys/log.h"

const struct simInterface radio_interface;

// COOJA variables
char simReceivedPacket;
char simSentPacket;
char simReceivedPacketData[UIP_BUFSIZE];
char simSentPacketData[UIP_BUFSIZE];
int simSentPacketSize;
int simReceivedPacketSize;
char simEtherBusy;
int retryCounter;
char simRadioHWOn = 1;


// Ether process
PROCESS(ether_process, "Simulated Ether");

PROCESS_THREAD(ether_process, ev, data)
{
  static struct etimer send_timer;

  PROCESS_BEGIN();

  // All outgoing messages pass through this process
  // By using the COOJA variables simEtherBusy and !!!!TODO signalstrength!!!!
  // this may be used to imitate a simple MAC protocol.
  while(1) {
    PROCESS_WAIT_EVENT();

    // MAC protocol imitiation
    // (this process is polled from simDoSend())

    // Confirm we actually have data to send and radio hardware is on
    if (simRadioHWOn && simSentPacketSize > 0) {

      // Wait some random time to avoid initial collisions
	  // MAC uses external random generator to get stochastic radio behaviour
      etimer_set(&send_timer, rand() % 20);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

      retryCounter = 0;
      while (simEtherBusy && retryCounter < 5) {
	retryCounter++;

	// Wait some random time hoping ether will free
	// MAC uses external random generator to get stochastic radio behaviour
	etimer_set(&send_timer, rand() % 20);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));
      }

      if (simEtherBusy) {
		log_message("MAC layer skipping packet", "");
      } else {
	// Tell COOJA about our new packet
	simSentPacket = 1;
      }
    }
  }

  PROCESS_END();
}


/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
  // Handle incoming network packets if any
  if (simReceivedPacket) {


    // If hardware is turned off, just remove packet
    if (!simRadioHWOn) {
      simReceivedPacket = 0;
      simReceivedPacketSize = 0;
      return;
    }

    // Reset flag
    simReceivedPacket = 0;

    if (simReceivedPacketSize == 0) {
      fprintf(stderr, "simReceivedPacketSize == 0: Didn't I receive a packet?\n");
      return;
    }

    // Copy incoming data to correct buffers and call handling routines
    uip_len = simReceivedPacketSize;

    if(uip_len > UIP_BUFSIZE) {
      fprintf(stderr, "doInterfaceActionsBeforeTick>> uip_len too large - dropping\n");
      uip_len = 0;
    } else {
      memcpy(&uip_buf[UIP_LLH_LEN], &simReceivedPacketData[0], simReceivedPacketSize);
      simReceivedPacketSize = 0;

      // Handle new packet
      tcpip_input();
    }
  }
}

/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
  // Nothing to do
}
/*-----------------------------------------------------------------------------------*/
u8_t
simDoSend(void)
{
  // If hardware is turned off, just remove packet
  if (!simRadioHWOn) {
    // Should we reset uip_len if radio is off?
    uip_len = 0;
    return UIP_FW_DROPPED;
  }

  // If outgoing data, but too large, drop it
  if(uip_len > UIP_BUFSIZE) {
    fprintf(stderr, "simDoSend>> uip_len too large - dropping\n");
    uip_len = 0;
    return UIP_FW_TOOLARGE;
  }

  // If outgoing data, back it up, and wake ether process
  if (uip_len > 0) {
    // Backup packet data/size
    memcpy(&simSentPacketData[0], &uip_buf[UIP_LLH_LEN], uip_len);
    simSentPacketSize = uip_len;

    process_poll(&ether_process);
    return UIP_FW_OK;
  }
  return UIP_FW_ZEROLEN;
}
/*-----------------------------------------------------------------------------------*/
/**
 * \brief      Turn radio on.
 *
 *             This function turns the radio hardware on.
 */
void
radio_on(void) {
  simRadioHWOn = 1;
}
/*-----------------------------------------------------------------------------------*/
/**
 * \brief      Turn radio off.
 *
 *             This function turns the radio hardware off.
 */
void radio_off(void) {
  simRadioHWOn = 0;
}
/*-----------------------------------------------------------------------------------*/
SIM_INTERFACE(radio_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
