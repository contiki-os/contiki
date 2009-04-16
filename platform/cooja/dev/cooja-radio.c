/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: cooja-radio.c,v 1.10 2009/04/16 14:38:41 fros4943 Exp $
 */

#include <string.h>
#include "contiki.h"

#include "dev/radio.h"
#include "dev/cooja-radio.h"
#include "lib/simEnvChange.h"
#include "sys/cooja_mt.h"

#define USING_CCA 1
#define USING_CCA_BUSYWAIT 1
#define CCA_BUSYWAIT_MS 100
#define CCA_SS_THRESHOLD -95

#define SS_NOTHING -100

const struct simInterface radio_interface;

// COOJA variables
char simTransmitting = 0;
char simReceiving = 0;

char simInDataBuffer[COOJA_RADIO_BUFSIZE];
int simInSize = 0;
char simInPolled = 0;
char simOutDataBuffer[COOJA_RADIO_BUFSIZE];
int simOutSize = 0;

char simRadioHWOn = 1;
int simSignalStrength = SS_NOTHING;
int simLastSignalStrength = SS_NOTHING;
char simPower = 100;
int simRadioChannel = 26;
int inSendFunction = 0;

static void (* receiver_callback)(const struct radio_driver *);

const struct radio_driver cooja_radio =
  {
    radio_send,
    radio_read,
    radio_set_receiver,
    radio_on,
    radio_off,
  };

/*-----------------------------------------------------------------------------------*/
void
radio_set_receiver(void (* recv)(const struct radio_driver *))
{
  receiver_callback = recv;
}
/*-----------------------------------------------------------------------------------*/
int
radio_on(void)
{
  simRadioHWOn = 1;
  return 1;
}
/*-----------------------------------------------------------------------------------*/
int
radio_off(void)
{
  simRadioHWOn = 0;
  return 1;
}
/*---------------------------------------------------------------------------*/
void
radio_set_channel(int channel)
{
  simRadioChannel = channel;
}
/*-----------------------------------------------------------------------------------*/
int
radio_sstrength(void)
{
  return simLastSignalStrength;
}
/*-----------------------------------------------------------------------------------*/
int
radio_current_sstrength(void)
{
  return simSignalStrength;
}
/*-----------------------------------------------------------------------------------*/
void
radio_set_txpower(unsigned char power)
{
  /* 1 - 100: Number indicating output power */
  simPower = power;
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsBeforeTick(void)
{
  // If radio is turned off, do nothing
  if (!simRadioHWOn) {
    simInSize = 0;
    simInPolled = 0;
    return;
  }

  // Don't fall asleep while receiving (in main file)
  if (simReceiving) {
    simLastSignalStrength = simSignalStrength;
    simDontFallAsleep = 1;
    return;
  }

  // If no incoming radio data, do nothing
  if (simInSize == 0) {
    simInPolled = 0;
    return;
  }

  // Check size of received packet
  if (simInSize > COOJA_RADIO_BUFSIZE) {
    // Drop packet by not delivering
    return;
  }

  // ** Good place to add explicit manchester/gcr-encoding

  if(receiver_callback != NULL && !simInPolled) {
    simDoReceiverCallback = 1;
    simInPolled = 1;
  } else {
    simInPolled = 0;
    simDontFallAsleep = 1;
  }
}
/*---------------------------------------------------------------------------*/
int
radio_read(void *buf, unsigned short bufsize)
{
  int tmpInSize = simInSize;

  if( bufsize < simInSize ) {
    return 0;
  }

  if(simInSize > 0) {
    memcpy(buf, simInDataBuffer, simInSize);
    simInSize = 0;
    return tmpInSize;
  }
  return 0;
}
/*-----------------------------------------------------------------------------------*/
static void
doInterfaceActionsAfterTick(void)
{
  // Make sure we are awake during radio activity
  if (simReceiving || simTransmitting) {
    simDontFallAsleep = 1;
    return;
  }
}
/*-----------------------------------------------------------------------------------*/
int
radio_send(const void *payload, unsigned short payload_len)
{
  /* If radio already actively transmitting, drop packet*/
  if(inSendFunction) {
    return COOJA_RADIO_DROPPED;
  }

  inSendFunction = 1;

  /* If radio is turned off, do nothing */
  if(!simRadioHWOn) {
    inSendFunction = 0;
    return COOJA_RADIO_DROPPED;
  }

  /* Drop packet if data size too large */
  if(payload_len > COOJA_RADIO_BUFSIZE) {
    inSendFunction = 0;
    return COOJA_RADIO_TOOLARGE;
  }

  /* Drop packet if no data length */
  if(payload_len <= 0) {
    inSendFunction = 0;
    return COOJA_RADIO_ZEROLEN;
  }

  /* Copy packet data to temporary storage */
  memcpy(simOutDataBuffer, payload, payload_len);
  simOutSize = payload_len;

#if USING_CCA_BUSYWAIT
  /* Busy-wait until both radio HW and ether is ready */
  {
    int retries = 0;
    while(retries < CCA_BUSYWAIT_MS && !simNoYield &&
	  (simSignalStrength > CCA_SS_THRESHOLD || simReceiving)) {
      retries++;
      cooja_mt_yield();
      if(!(simSignalStrength > CCA_SS_THRESHOLD || simReceiving)) {
        /* Wait one extra tick before transmission starts */
        cooja_mt_yield();
      }
    }
  }
#endif /* USING_CCA_BUSYWAIT */

#if USING_CCA
  if(simSignalStrength > CCA_SS_THRESHOLD || simReceiving) {
    inSendFunction = 0;
    return COOJA_RADIO_DROPPED;
  }
#endif /* USING_CCA */

  if(simOutSize <= 0) {
    inSendFunction = 0;
    return COOJA_RADIO_DROPPED;
  }

  // - Initiate transmission -
  simTransmitting = 1;

  // Busy-wait while transmitting
  while(simTransmitting && !simNoYield) {
    cooja_mt_yield();
  }
  if (simTransmitting) {
    simDontFallAsleep = 1;
  }

  inSendFunction = 0;
  return COOJA_RADIO_OK;
}
/*-----------------------------------------------------------------------------------*/
void radio_call_receiver()
{
  receiver_callback(&cooja_radio);
}
/*-----------------------------------------------------------------------------------*/
SIM_INTERFACE(radio_interface,
	      doInterfaceActionsBeforeTick,
	      doInterfaceActionsAfterTick);
