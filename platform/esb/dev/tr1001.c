/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: tr1001.c,v 1.13 2010/03/02 22:40:39 nifi Exp $
 */
/**
 * \addtogroup esb
 * @{
 */

/**
 * \defgroup tr1001 TR1001 radio tranciever device driver
 * @{
 */

/**
 * \file
 * Device driver and packet framing for the RFM-TR1001 radio module.
 * \author Adam Dunkels <adam@sics.se>
 *
 * This file implements a device driver for the RFM-TR1001 radio
 * tranciever.
 *
 */

#include "contiki.h"
#include "contiki-esb.h"

#include "dev/tr1001.h"
#include "dev/radio-sensor.h"
#include "lib/me.h"
#include "lib/crc16.h"
#include "net/netstack.h"
#include "net/rime/rimestats.h"
#include "isr_compat.h"

#include <string.h>

#ifdef TR1001_CONF_BEEP_ON_BAD_CRC
#define BEEP_ON_BAD_CRC TR1001_CONF_BEEP_ON_BAD_CRC
#else
#define BEEP_ON_BAD_CRC 1
#endif /* TR1001_CONF_BEEP_ON_BAD_CRC */

#if BEEP_ON_BAD_CRC
#include "dev/beep.h"
#define BEEP_BEEP(t) beep_beep(t)
#else
#define BEEP_BEEP(t)
#endif /* BEEP_ON_BAD_CRC */

#define RXSTATE_READY     0
#define RXSTATE_RECEIVING 1
#define RXSTATE_FULL      2

#define SYNCH1 0x3c
#define SYNCH2 0x03

#ifdef TR1001_CONF_BUFFER_SIZE
#define RXBUFSIZE TR1001_CONF_BUFFER_SIZE
#else
#define RXBUFSIZE PACKETBUF_SIZE
#endif /* TR1001_CONF_BUFFER_SIZE */

/*
 * Pending data to send when using prepare/transmit functions.
 */
static const void *pending_data;

/*
 * The buffer which holds incoming data.
 */
unsigned char tr1001_rxbuf[RXBUFSIZE];

/*
 * The length of the packet that currently is being received.
 */
static unsigned short tr1001_rxlen = 0;

/*
 * The reception state.
 */
volatile unsigned char tr1001_rxstate = RXSTATE_READY;

static uint16_t rxcrc, rxcrctmp;

/*
 * The structure of the packet header.
 */
struct tr1001_hdr {
  uint8_t len[2];  /**< The 16-bit length of the packet in network byte
		     order. */
};

/*
 * The length of the packet header.
 */
#define TR1001_HDRLEN sizeof(struct tr1001_hdr)

#define OFF 0
#define ON 1
static uint8_t onoroff = OFF;

#define NUM_SYNCHBYTES 4

void tr1001_default_rxhandler(unsigned char c);
PT_THREAD(tr1001_default_rxhandler_pt(unsigned char c));
static struct pt rxhandler_pt;

/*
 * This timer is used to keep track of when the last byte was received
 * over the radio. If the inter-byte time is too large, the packet
 * currently being received is discarded and a new packet reception is
 * initiated.
 */
static struct timer rxtimer;

static unsigned short tmp_sstrength, sstrength;
static unsigned short tmp_count;

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

/*---------------------------------------------------------------------------*/
PROCESS(tr1001_process, "TR1001 driver");
/*---------------------------------------------------------------------------*/

static int prepare_packet(const void *data, unsigned short len);
static int transmit_packet(unsigned short len);
static int receiving_packet(void);
static int pending_packet(void);
static int channel_clear(void);
static int tr1001_on(void);
static int tr1001_off(void);

const struct radio_driver tr1001_driver = {
  tr1001_init,
  prepare_packet,
  transmit_packet,
  tr1001_send,
  tr1001_read,
  channel_clear,
  receiving_packet,
  pending_packet,
  tr1001_on,
  tr1001_off
};

/*---------------------------------------------------------------------------*/
/*
 * Turn on data transmission in On-Off-Keyed mode.
 */
static void
txook(void)
{
  P3SEL = 0xf0;
  P5OUT |= 0x40;
  P5OUT &= 0x7f;
}
/*---------------------------------------------------------------------------*/
/*
 * Turn on data reception for the radio tranceiver.
 */
static void
rxon(void)
{
  P3SEL = 0xe0;
  P5OUT |= 0xc0;

  /* Enable the receiver. */
  ME1 |= URXE0;

  /* Turn on receive interrupt. */
  IE1 |= URXIE0;

}
/*---------------------------------------------------------------------------*/
/*
 * Turn off data reception for the radio tranceiver.
 */
static void
rxoff(void)
{
  P5OUT &= 0x3f;

  /* Disable the receiver. */
  ME1 &= ~URXE0;

  /* Turn off receive interrupt. */
  IE1 &= ~URXIE0;
}
/*---------------------------------------------------------------------------*/
/*
 * Clear the recevie buffer and reset the receiver state.
 */
static void
rxclear(void)
{
  tr1001_rxstate = RXSTATE_READY;
}
/*---------------------------------------------------------------------------*/
/*
 * Turn TR1001 radio transceiver off.
 */
/*---------------------------------------------------------------------------*/
static int
tr1001_off(void)
{
  if(onoroff == OFF) {
    return 1;
  }
  onoroff = OFF;
  rxoff();
  rxclear();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  return 1;
}
/*---------------------------------------------------------------------------*/
/*
 * Turn TR1001 radio transceiver on.
 */
/*---------------------------------------------------------------------------*/
static int
tr1001_on(void)
{
  if(onoroff == ON) {
    return 1;
  }

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  onoroff = ON;
  rxon();
  rxclear();
  return 1;
}
/*---------------------------------------------------------------------------*/
/*
 * Send a byte of data over the radio.
 *
 * \param b The byte to be sent.
 */
static void
send(unsigned char b)
{
  clock_time_t start;

  start = clock_time();

  /* Wait until the USART0 TX buffer is ready. */
  while((IFG1 & UTXIFG0) == 0) {
    /* Wait no more than one second. */
    if((clock_time_t)(clock_time() - start) > (clock_time_t)CLOCK_SECOND) {
      break;
    }
  }

  /* Send the byte. */
  TXBUF0 = b;
}
/*---------------------------------------------------------------------------*/
/*
 * Send a byte of data and its logical negation (all bits inverted)
 * over the radio.
 *
 * \param b The byte to be sent.
 */
static void
send2(unsigned char b)
{
  uint16_t m;
  m = me_encode(b);
  send(m >> 8);
  send(m & 0xff);
}
static uint16_t
send2_crc16(unsigned char b, uint16_t crcacc)
{
  uint16_t m;
  m = me_encode(b);
  send(m >> 8);
  send(m & 0xff);
  return crc16_add(b, crcacc);
}
/*---------------------------------------------------------------------------*/
void
tr1001_set_txpower(unsigned char p)
{
  int i;

  /* Clamp maximum power. */
  if(p > 100) {
    p = 100;
  }

  /* First, run the potentiometer down to zero so that we know the
     start value of the potentiometer. */
  P2OUT &= 0xDF;                                /* P25 = 0 (down selected) */
  P2OUT &= 0xBF;                                /* P26 = 0 (chipselect on) */
  for(i = 0; i < 102; ++i) {
    P2OUT &= 0xEF;                              /* P24 = 0 (inc) */
    P2OUT |= 0x10;
  }

  /* Now, start to increase the value of the potentiometer until it
     reaches the desired value.*/

  P2OUT |= 0x20;                                /* P25 = 1 (up selected) */
  for(i = 0; i < p; ++i) {
    P2OUT &= 0xEF;                              /* P24 = 0 (inc) */
    P2OUT |= 0x10;
  }
  P2OUT |= 0x40;                                /* P26 = 1 (chipselect off) */
}
/*---------------------------------------------------------------------------*/
int
tr1001_init(void)
{
  PT_INIT(&rxhandler_pt);

  onoroff = OFF;

  UCTL0 = CHAR;                         /* 8-bit character */
  UTCTL0 = SSEL1;                       /* UCLK = SMCLK */

  tr1001_set_speed(TR1001_19200);

  ME1 |= UTXE0 + URXE0;                 /* Enable USART0 TXD/RXD */

  /* Turn on receive interrupt. */
  IE1 |= URXIE0;

  timer_set(&rxtimer, CLOCK_SECOND / 4);


  tr1001_on();
  tr1001_set_txpower(100);

  /* Reset reception state. */
  rxclear();

  process_start(&tr1001_process, NULL);

  return 1;
}
/*---------------------------------------------------------------------------*/
ISR(UART0RX, tr1001_rxhandler)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  tr1001_default_rxhandler_pt(RXBUF0);
  if(tr1001_rxstate == RXSTATE_FULL) {
    LPM4_EXIT;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
#if DEBUG
static void
dump_packet(int len)
{
  int i;
  for(i = 0; i < len; ++i) {
    LOG("%d: 0x%02x\n", i, tr1001_rxbuf[i]);
  }
}
#endif /* DEBUG */
/*---------------------------------------------------------------------------*/
PT_THREAD(tr1001_default_rxhandler_pt(unsigned char incoming_byte))
{
  static unsigned char rxtmp, tmppos;

  if(timer_expired(&rxtimer) && tr1001_rxstate != RXSTATE_FULL) {
    PT_INIT(&rxhandler_pt);
  }

  timer_restart(&rxtimer);

  if(tr1001_rxstate == RXSTATE_RECEIVING) {
    unsigned short signal = radio_sensor.value(0);
    tmp_sstrength += (signal >> 2);
    tmp_count++;
  }

  PT_BEGIN(&rxhandler_pt);

  while(1) {

    /* Reset reception state. */
    rxclear();

    /* Wait until we receive the first syncronization byte. */
    PT_WAIT_UNTIL(&rxhandler_pt, incoming_byte == SYNCH1);

    tr1001_rxstate = RXSTATE_RECEIVING;

    /* Read all incoming syncronization bytes. */
    PT_WAIT_WHILE(&rxhandler_pt, incoming_byte == SYNCH1);

    /* We should receive the second synch byte by now, otherwise we'll
       restart the protothread. */
    if(incoming_byte != SYNCH2) {
      PT_RESTART(&rxhandler_pt);
    }

    /* Start signal strength measurement */
    tmp_sstrength = 0;
    tmp_count = 0;

    /* Reset the CRC. */
    rxcrc = 0xffff;

    /* Read packet header. */
    for(tmppos = 0; tmppos < TR1001_HDRLEN; ++tmppos) {

      /* Wait for the first byte of the packet to arrive. */
      PT_YIELD(&rxhandler_pt);

      /* If the incoming byte isn't a valid Manchester encoded byte,
	 we start again from the beginning. */
      if(!me_valid(incoming_byte)) {
	BEEP_BEEP(1000);
	LOG("Incorrect manchester in header at byte %d/1\n", tmppos);
	RIMESTATS_ADD(badsynch);
	PT_RESTART(&rxhandler_pt);
      }

      rxtmp = me_decode8(incoming_byte);

      /* Wait for the next byte to arrive. */
      PT_YIELD(&rxhandler_pt);

      if(!me_valid(incoming_byte)) {
	BEEP_BEEP(1000);
	LOG("Incorrect manchester in header at byte %d/2\n", tmppos);
	RIMESTATS_ADD(badsynch);
	PT_RESTART(&rxhandler_pt);
      }

      /* Put together the two bytes into a single Manchester decoded
	 byte. */

      tr1001_rxbuf[tmppos] = (rxtmp << 4) | me_decode8(incoming_byte);

      /* Calculate the CRC. */
      rxcrc = crc16_add(tr1001_rxbuf[tmppos], rxcrc);


    }

    /* Since we've got the header, we can grab the length from it. */
    tr1001_rxlen = ((((struct tr1001_hdr *)tr1001_rxbuf)->len[0] << 8) +
		    ((struct tr1001_hdr *)tr1001_rxbuf)->len[1]);

    /* If the length is longer than we can handle, we'll start from
       the beginning. */
    if(tmppos + tr1001_rxlen > sizeof(tr1001_rxbuf)) {
      RIMESTATS_ADD(toolong);
      PT_RESTART(&rxhandler_pt);
    }

    /* Read packet data. */
    for(; tmppos < tr1001_rxlen + TR1001_HDRLEN; ++tmppos) {
      PT_YIELD(&rxhandler_pt);

      if(!me_valid(incoming_byte)) {
	LOG("Incorrect manchester 0x%02x at byte %d/1\n", incoming_byte,
	    tmppos - TR1001_HDRLEN);
	BEEP_BEEP(1000);
	RIMESTATS_ADD(badsynch);
	PT_RESTART(&rxhandler_pt);
      }

      rxtmp = me_decode8(incoming_byte);

      PT_YIELD(&rxhandler_pt);

      if(!me_valid(incoming_byte)) {
	LOG("Incorrect manchester at byte %d/2\n", tmppos - TR1001_HDRLEN);
	BEEP_BEEP(1000);
	RIMESTATS_ADD(badsynch);
	PT_RESTART(&rxhandler_pt);
      }

      tr1001_rxbuf[tmppos] = (rxtmp << 4) | me_decode8(incoming_byte);
      rxcrc = crc16_add(tr1001_rxbuf[tmppos], rxcrc);

    }

    /* Read the frame CRC. */
    for(tmppos = 0; tmppos < 4; ++tmppos) {

      PT_YIELD(&rxhandler_pt);

      if(!me_valid(incoming_byte)) {
	BEEP_BEEP(1000);
	RIMESTATS_ADD(badsynch);
	PT_RESTART(&rxhandler_pt);
      }

      rxcrctmp = (rxcrctmp << 4) | me_decode8(incoming_byte);
    }

    if(rxcrctmp == rxcrc) {
      /* A full packet has been received and the CRC checks out. We'll
	 request the driver to take care of the incoming data. */

      RIMESTATS_ADD(llrx);
      process_poll(&tr1001_process);

      /* We'll set the receive state flag to signal that a full frame
	 is present in the buffer, and we'll wait until the buffer has
	 been taken care of. */
      tr1001_rxstate = RXSTATE_FULL;
      PT_WAIT_UNTIL(&rxhandler_pt, tr1001_rxstate != RXSTATE_FULL);

    } else {
      LOG("Incorrect CRC\n");
      BEEP_BEEP(1000);
      RIMESTATS_ADD(badcrc);
    }
  }
  PT_END(&rxhandler_pt);
}
/*---------------------------------------------------------------------------*/
static int
prepare_packet(const void *data, unsigned short len)
{
  pending_data = data;
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
transmit_packet(unsigned short len)
{
  int ret = RADIO_TX_ERR;
  if(pending_data != NULL) {
    ret = tr1001_send(pending_data, len);
    pending_data = NULL;
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
int
tr1001_send(const void *packet, unsigned short len)
{
  int i;
  uint16_t crc16;

  LOG("tr1001_send: sending %d bytes\n", len);

  if(onoroff == ON) {
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  }
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  /* Prepare the transmission. */

  /* Delay the transmission for a short random duration. */
  clock_delay(random_rand() & 0x3ff);


  /* Check that we don't currently are receiving a packet, and if so
     we wait until the reception has been completed. Reception is done
     with interrupts so it is OK for us to wait in a while() loop. */

  while(tr1001_rxstate == RXSTATE_RECEIVING &&
	!timer_expired(&rxtimer)) {
    /* Delay the transmission for a short random duration. */
    clock_delay(random_rand() & 0x7ff);
  }


  /* Turn on OOK mode with transmission. */
  txook();

  /* According to the datasheet, the transmitter must wait for 12 us
     in order to settle. Empirical tests show that is it better to
     wait for something like 283 us... */
  clock_delay(200);


  /* Transmit preamble and synch bytes. */
  for(i = 0; i < 20; ++i) {
    send(0xaa);
  }
  /*  send(0xaa);
      send(0xaa);*/
  send(0xff);

  for(i = 0; i < NUM_SYNCHBYTES; ++i) {
    send(SYNCH1);
  }
  send(SYNCH2);

  crc16 = 0xffff;

  /* Send packet header. */
  crc16 = send2_crc16(len >> 8, crc16);
  crc16 = send2_crc16(len & 0xff, crc16);

  /* Send packet data. */
  for(i = 0; i < len; ++i) {
    crc16 = send2_crc16(((uint8_t *)packet)[i], crc16);
  }

  /* Send CRC */
  send2(crc16 >> 8);
  send2(crc16 & 0xff);

  /* Send trailing bytes. */
  send(0x33);
  send(0xcc);
  send(0x33);
  send(0xcc);

  /* Turn on (or off) reception again. */
  if(onoroff == ON) {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
    rxon();
    rxclear();
  } else {
    rxoff();
    rxclear();
  }

  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  RIMESTATS_ADD(lltx);

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
int
tr1001_read(void *buf, unsigned short bufsize)
{
  unsigned short tmplen;

  if(tr1001_rxstate == RXSTATE_FULL) {

#if DEBUG
    dump_packet(tr1001_rxlen + 2);
#endif /* DEBUG */

    tmplen = tr1001_rxlen;

    if(tmplen > bufsize) {
      LOG("tr1001_read: too large packet: %d/%d bytes\n", tmplen, bufsize);
      rxclear();
      RIMESTATS_ADD(toolong);
      return -1;
    }

    memcpy(buf, &tr1001_rxbuf[TR1001_HDRLEN], tmplen);

    /* header + content + CRC */
    sstrength = (tmp_count ? ((tmp_sstrength / tmp_count) << 2) : 0);

    rxclear();

    LOG("tr1001_read: got %d bytes\n", tmplen);

    return tmplen;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  return tr1001_rxstate == RXSTATE_RECEIVING &&
    !timer_expired(&rxtimer);
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return tr1001_rxstate == RXSTATE_FULL;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  /* TODO add CCA functionality */
  return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tr1001_process, ev, data)
{
  int len;
  PROCESS_BEGIN();

  /* Reset reception state now that the process is ready to receive data. */
  rxclear();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    packetbuf_clear();
    len = tr1001_read(packetbuf_dataptr(), PACKETBUF_SIZE);
    if(len > 0) {
      packetbuf_set_datalen(len);
      NETSTACK_RDC.input();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
tr1001_set_speed(unsigned char speed)
{

  if(speed == TR1001_19200) {
    /* Set TR1001 to 19200 */
    UBR00 = 0x80;                         /* 2,457MHz/19200 = 128 -> 0x80 */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x00;                        /* no modulation  */
  } else if(speed == TR1001_38400) {
    /* Set TR1001 to 38400 */
    UBR00 = 0x40;                         /* 2,457MHz/38400 = 64 -> 0x40 */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x00;                        /* no modulation  */
  } else if(speed == TR1001_57600) {
    UBR00 = 0x2a;                         /* 2,457MHz/57600 = 42.7 -> 0x2A */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x5b;                        /* */
  } else if(speed == TR1001_115200) {
    UBR00 = 0x15;                         /* 2,457MHz/115200 = 21.4 -> 0x15 */
    UBR10 = 0x00;                         /* */
    UMCTL0 = 0x4a;                        /* */
  } else {
    tr1001_set_speed(TR1001_19200);
  }
}
/*---------------------------------------------------------------------------*/
unsigned short
tr1001_sstrength(void)
{
  return sstrength;
}
/*---------------------------------------------------------------------------*/
/** @} */
/** @} */
