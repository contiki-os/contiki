/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
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
 * @(#)$Id: simple-cc2420.c,v 1.20 2008/01/14 16:19:25 thiemovoigt Exp $
 */
/*
 * This code is almost device independent and should be easy to port.
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"

#if defined(__AVR__)
#include <avr/io.h>
#elif defined(__MSP430__)
#include <io.h>
#endif

#include "dev/spi.h"
#include "dev/simple-cc2420.h"
#include "dev/cc2420_const.h"

#include "net/rime/rimestats.h"


#define FOOTER_LEN 2
#define CRC_LEN 2

#if SIMPLE_CC2420_CONF_TIMESTAMPS
#include "net/rime/timesynch.h"
#define TIMESTAMP_LEN 3
#else /* SIMPLE_CC2420_CONF_TIMESTAMPS */
#define TIMESTAMP_LEN 0
#endif /* SIMPLE_CC2420_CONF_TIMESTAMPS */

struct timestamp {
  uint16_t time;
  uint8_t authority_level;
};


#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

void simple_cc2420_arch_init(void);

/* XXX hack: these will be made as Chameleon packet attributes */
rtimer_clock_t simple_cc2420_time_of_arrival, simple_cc2420_time_of_departure;

int simple_cc2420_authority_level_of_sender;

static rtimer_clock_t setup_time_for_transmission;
static unsigned long total_time_for_transmission, total_transmission_len;
static int num_transmissions;

/*---------------------------------------------------------------------------*/
PROCESS(simple_cc2420_process, "CC2420 driver");
/*---------------------------------------------------------------------------*/

static void (* receiver_callback)(const struct radio_driver *);

int simple_cc2420_on(void);
int simple_cc2420_off(void);

int simple_cc2420_read(void *buf, unsigned short bufsize);

int simple_cc2420_send(const void *data, unsigned short len);

void simple_cc2420_set_receiver(void (* recv)(const struct radio_driver *d));


signed char simple_cc2420_last_rssi;
u8_t simple_cc2420_last_correlation;

const struct radio_driver simple_cc2420_driver =
  {
    simple_cc2420_send,
    simple_cc2420_read,
    simple_cc2420_set_receiver,
    simple_cc2420_on,
    simple_cc2420_off,
  };

static u8_t receive_on;
/* Radio stuff in network byte order. */
static u16_t pan_id;
/*---------------------------------------------------------------------------*/
static void
strobe(enum cc2420_register regname)
{
  FASTSPI_STROBE(regname);
}
/*---------------------------------------------------------------------------*/
static unsigned int
status(void)
{
  u8_t status;
  FASTSPI_UPD_STATUS(status);
  return status;
}
/*---------------------------------------------------------------------------*/
static void
on(void)
{
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  PRINTF("on\n");
  receive_on = 1;
  
  strobe(CC2420_SRXON);
  strobe(CC2420_SFLUSHRX);
  ENABLE_FIFOP_INT();
}
static void
off(void)
{
  u8_t spiStatusByte;
  
  PRINTF("off\n");
  receive_on = 0;
  /* Wait for transmission to end before turning radio off. */
  do {
    spiStatusByte = status();
  } while(spiStatusByte & BV(CC2420_TX_ACTIVE));
  
  strobe(CC2420_SRFOFF);
  DISABLE_FIFOP_INT();
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
}
/*---------------------------------------------------------------------------*/
static u8_t locked, lock_on, lock_off;
#define GET_LOCK() locked = 1
static void RELEASE_LOCK(void) {
  if(lock_on) {
    on();
    lock_on = 0;
  }
  if(lock_off) {
    off();
    lock_off = 0;
  }
  locked = 0;
}
/*---------------------------------------------------------------------------*/
static unsigned
getreg(enum cc2420_register regname)
{
  unsigned reg;
  FASTSPI_GETREG(regname, reg);
  return reg;
}
/*---------------------------------------------------------------------------*/
static void
setreg(enum cc2420_register regname, unsigned value)
{
  FASTSPI_SETREG(regname, value);
}
/*---------------------------------------------------------------------------*/
#define AUTOACK (1 << 4)
#define ADR_DECODE (1 << 11)
#define RXFIFO_PROTECTION (1 << 9)
#define CORR_THR(n) (((n) & 0x1f) << 6)
#define FIFOP_THR(n) ((n) & 0x7f)
#define RXBPF_LOCUR (1 << 13);
/*---------------------------------------------------------------------------*/
void
simple_cc2420_set_receiver(void (* recv)(const struct radio_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
void
simple_cc2420_init(void)
{
  u16_t reg;
  {
    int s = splhigh();
    simple_cc2420_arch_init();		/* Initalize ports and SPI. */
    DISABLE_FIFOP_INT();
    FIFOP_INT_INIT();
    splx(s);
  }

  /* Turn on voltage regulator and reset. */
  SET_VREG_ACTIVE();
  //clock_delay(250); OK
  SET_RESET_ACTIVE();
  clock_delay(127);
  SET_RESET_INACTIVE();
  //clock_delay(125); OK


  /* Turn on the crystal oscillator. */
  strobe(CC2420_SXOSCON);

  /* Turn off automatic packet acknowledgment. */
  reg = getreg(CC2420_MDMCTRL0);
  reg &= ~AUTOACK;
  setreg(CC2420_MDMCTRL0, reg);

  /* Turn off address decoding. */
  reg = getreg(CC2420_MDMCTRL0);
  reg &= ~ADR_DECODE;
  setreg(CC2420_MDMCTRL0, reg);

  /* Change default values as recomended in the data sheet, */
  /* correlation threshold = 20, RX bandpass filter = 1.3uA. */
  setreg(CC2420_MDMCTRL1, CORR_THR(20));
  reg = getreg(CC2420_RXCTRL1);
  reg |= RXBPF_LOCUR;
  setreg(CC2420_RXCTRL1, reg);
  
  /* Set the FIFOP threshold to maximum. */
  setreg(CC2420_IOCFG0, FIFOP_THR(127));

  /* Turn off "Security enable" (page 32). */
  reg = getreg(CC2420_SECCTRL0);
  reg &= ~RXFIFO_PROTECTION;
  setreg(CC2420_SECCTRL0, reg);

  simple_cc2420_set_chan_pan_addr(11, 0xffff, 0x0000, NULL);

  process_start(&simple_cc2420_process, NULL);
}
/*---------------------------------------------------------------------------*/
int
simple_cc2420_send(const void *payload, unsigned short payload_len)
{
  uint8_t spiStatusByte;
  int i;
  uint8_t total_len;
  struct timestamp timestamp;
  
  /* This code uses the CC2420 CCA (Clear Channel Assessment) to
   * implement Carrier Sense Multiple Access with Collision Avoidance
   * (CSMA-CA) and requires the receiver to be enabled and ready.
   */
  if(!receive_on) {
    return -2;
  }

  /*  PRINTF("simple_cc2420_send: %d bytes\n", payload_len);*/
  
  GET_LOCK();

  RIMESTATS_ADD(lltx);
  
  /* Wait for previous transmission to finish and RSSI. */
  do {
    spiStatusByte = status();
  } while(spiStatusByte & BV(CC2420_TX_ACTIVE) &&
	  !(spiStatusByte & BV(CC2420_RSSI_VALID)));

  /* Write packet to TX FIFO. */
  strobe(CC2420_SFLUSHTX);
  
  total_len = payload_len + TIMESTAMP_LEN + FOOTER_LEN;
  FASTSPI_WRITE_FIFO(&total_len, 1);
  
  FASTSPI_WRITE_FIFO(payload, payload_len);

#if SIMPLE_CC2420_CONF_TIMESTAMPS
  timestamp.authority_level = timesynch_authority_level();
  timestamp.time = timesynch_time();
  FASTSPI_WRITE_FIFO(&timestamp, TIMESTAMP_LEN);
#endif /* SIMPLE_CC2420_CONF_TIMESTAMPS */

  if(FIFOP_IS_1 && !FIFO_IS_1) {
    /* RXFIFO overflow, send on retransmit. */
    PRINTF("rxfifo overflow!\n");
    RELEASE_LOCK();
    return -4;
  }

  /* The TX FIFO can only hold one packet! Make sure to not overrun
   * FIFO by waiting for transmission to start here and synchronizing
   * with the CC2420_TX_ACTIVE check in cc2420_send.
   *
   * Note that we may have to wait up to 320 us (20 symbols) before
   * transmission starts.
   */
#ifdef TMOTE_SKY
#define LOOP_20_SYMBOLS 100	/* 326us (msp430 @ 2.4576MHz) */
#elif __AVR__
#define LOOP_20_SYMBOLS 500	/* XXX */
#endif
  strobe(CC2420_STXONCCA);
  for(i = LOOP_20_SYMBOLS; i > 0; i--) {
    if(SFD_IS_1) {
      rtimer_clock_t txtime = timesynch_time();
      /*      PRINTF("simple_cc2420: do_send() transmission has started\n");*/

      ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
      ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
      do {
	spiStatusByte = status();
      } while(spiStatusByte & BV(CC2420_TX_ACTIVE));
#if SIMPLE_CC2420_CONF_TIMESTAMPS
      setup_time_for_transmission = txtime - timestamp.time;
      
      if(num_transmissions < 10000) {
	total_time_for_transmission += timesynch_time() - txtime;
	total_transmission_len += total_len;
	num_transmissions++;
      }

#endif /* SIMPLE_CC2420_CONF_TIMESTAMPS */
      /* add time for power level also */
      { 
	int current_powerlevel;

	current_powerlevel = (int)(getreg(CC2420_TXCTRL) & 0x001f);
	ENERGEST_OFF_LEVEL(ENERGEST_TYPE_TRANSMIT,current_powerlevel);
      }
      ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
      ENERGEST_ON(ENERGEST_TYPE_LISTEN);
      
      RELEASE_LOCK();
      return 0;			/* Transmission has started. */
    }
  }
  RIMESTATS_ADD(contentiondrop);
  PRINTF("simple_cc2420: do_send() transmission never started\n");
  RELEASE_LOCK();
  return -3;			/* Transmission never started! */
}
/*---------------------------------------------------------------------------*/
static volatile u8_t packet_seen;
/*---------------------------------------------------------------------------*/
int
simple_cc2420_off(void)
{
  if(receive_on == 0) {
    return 1;
  }

  if(locked) {
    lock_off = 1;
    return 1;
  }

  if(packet_seen) {
    lock_off = 1;
    return 1;
  }
  
  off();
  return 1;
}
/*---------------------------------------------------------------------------*/
int
simple_cc2420_on(void)
{
  if(receive_on) {
    return 1;
  }
  if(locked) {
    lock_on = 1;
    return 1;
  }

  on();
  return 1;
}
/*---------------------------------------------------------------------------*/
void
simple_cc2420_set_channel(int channel)
{
  u16_t f = channel;
  
  f = 5 * (f - 11) + 357 + 0x4000;
  /*
   * Writing RAM requires crystal oscillator to be stable.
   */
  while(!(status() & (BV(CC2420_XOSC16M_STABLE))));
  
  setreg(CC2420_FSCTRL, f);
}
/*---------------------------------------------------------------------------*/
void
simple_cc2420_set_chan_pan_addr(unsigned channel, /* 11 - 26 */
				unsigned pan,
				unsigned addr,
				const u8_t *ieee_addr)
{
  /*
   * Subtract the base channel (11), multiply by 5, which is the
   * channel spacing. 357 is 2405-2048 and 0x4000 is LOCK_THR = 1.
   */
  u8_t spiStatusByte;
  u16_t f = channel;
        
  f = 5 * (f - 11) + 357 + 0x4000;
  /*
   * Writing RAM requires crystal oscillator to be stable.
   */
  do {
    spiStatusByte = status();
  } while(!(spiStatusByte & (BV(CC2420_XOSC16M_STABLE))));

  pan_id = pan;
  setreg(CC2420_FSCTRL, f);
  FASTSPI_WRITE_RAM_LE(&pan, CC2420RAM_PANID, 2, f);
  FASTSPI_WRITE_RAM_LE(&addr, CC2420RAM_SHORTADDR, 2, f);
  if(ieee_addr != NULL) {
    FASTSPI_WRITE_RAM_LE(ieee_addr, CC2420RAM_IEEEADDR, 8, f);
  }
}
/*---------------------------------------------------------------------------*/
/*
 * Interrupt either leaves frame intact in FIFO or reads *only* the
 * MAC header and sets rx_fifo_remaining_bytes.
 *
 * In order to quickly empty the FIFO ack processing is done at
 * interrupt priority rather than poll priority.
 */
static volatile rtimer_clock_t interrupt_time;
static volatile int interrupt_time_set;
int
simple_cc2420_interrupt(void)
{
  interrupt_time = timesynch_time();
  interrupt_time_set = 1;
  
  CLEAR_FIFOP_INT();
  process_poll(&simple_cc2420_process);
  packet_seen = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(simple_cc2420_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    /*    printf("Diff %d\n", rtimer_arch_now() - interrupt_time);*/
    
    if(receiver_callback != NULL) {
      receiver_callback(&simple_cc2420_driver);
    } else {
      PRINTF("simple_cc2420_process not receiving function\n");
      FASTSPI_STROBE(CC2420_SFLUSHRX);
      FASTSPI_STROBE(CC2420_SFLUSHRX);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
simple_cc2420_read(void *buf, unsigned short bufsize)
{
  u8_t footer[2];
  int len;
  struct timestamp t;
  
  if(!packet_seen) {
    return 0;
  }

  if(interrupt_time_set) {
#if SIMPLE_CC2420_CONF_TIMESTAMPS
    simple_cc2420_time_of_arrival = interrupt_time;
#endif /* SIMPLE_CC2420_CONF_TIMESTAMPS */
    interrupt_time_set = 0;
  } else {
    simple_cc2420_time_of_arrival = 0;
  }
  simple_cc2420_time_of_departure = 0;
  GET_LOCK();

  FASTSPI_READ_FIFO_BYTE(len);

  if(len > SIMPLE_CC2420_MAX_PACKET_LEN) {
    /* Oops, we must be out of sync. */
    FASTSPI_STROBE(CC2420_SFLUSHRX);
    FASTSPI_STROBE(CC2420_SFLUSHRX);
    packet_seen = 0;
    RIMESTATS_ADD(badsynch);
    RELEASE_LOCK();
    return 0;
  }

  if(len > 0) {
    /* Read payload and two bytes of footer */
    PRINTF("simple_cc2420_read: len %d\n", len);
    if(len <= FOOTER_LEN + TIMESTAMP_LEN) {
      FASTSPI_READ_FIFO_GARBAGE(len);
      RIMESTATS_ADD(tooshort);
    } else if(len - FOOTER_LEN - TIMESTAMP_LEN > bufsize) {
      PRINTF("simple_cc2420_read too big len=%d bufsize %d\n", len, bufsize);
      //     FASTSPI_READ_FIFO_GARBAGE(2);
      FASTSPI_READ_FIFO_NO_WAIT(buf, bufsize);
      FASTSPI_READ_FIFO_GARBAGE(len - bufsize - FOOTER_LEN - TIMESTAMP_LEN);
#if SIMPLE_CC2420_CONF_TIMESTAMPS
      FASTSPI_READ_FIFO_NO_WAIT(&t, TIMESTAMP_LEN); /* Time stamp */
#endif /* SIMPLE_CC2420_CONF_TIMESTAMPS */
      FASTSPI_READ_FIFO_NO_WAIT(footer, FOOTER_LEN);
      len = TIMESTAMP_LEN + FOOTER_LEN;
      RIMESTATS_ADD(toolong);
    } else {

      FASTSPI_READ_FIFO_NO_WAIT(buf, len - FOOTER_LEN - TIMESTAMP_LEN);
      /*      PRINTF("simple_cc2420_read: data\n");*/
      FASTSPI_READ_FIFO_NO_WAIT(&t, TIMESTAMP_LEN); /* Time stamp */
      FASTSPI_READ_FIFO_NO_WAIT(footer, FOOTER_LEN);
      /*      PRINTF("simple_cc2420_read: footer\n");*/
      if(footer[1] & FOOTER1_CRC_OK) {
	simple_cc2420_last_rssi = footer[0];
	simple_cc2420_last_correlation = footer[1] & FOOTER1_CORRELATION;
	RIMESTATS_ADD(llrx);
      } else {
	RIMESTATS_ADD(badcrc);
	len = TIMESTAMP_LEN + FOOTER_LEN;
      }
#if SIMPLE_CC2420_CONF_TIMESTAMPS
      simple_cc2420_time_of_departure =
	t.time +
	setup_time_for_transmission +
	(total_time_for_transmission * (len - 2)) / total_transmission_len;

      simple_cc2420_authority_level_of_sender = t.authority_level;

#endif /* SIMPLE_CC2420_CONF_TIMESTAMPS */
    }
  }
  
  /* Clean up in case of FIFO overflow!  This happens for every full
   * length frame and is signaled by FIFOP = 1 and FIFO = 0.
   */
  if(FIFOP_IS_1 && !FIFO_IS_1) {
    PRINTF("simple_cc2420_read: FIFOP_IS_1 1\n");
    strobe(CC2420_SFLUSHRX);
    strobe(CC2420_SFLUSHRX);
  }
  
  if(FIFOP_IS_1) {
    PRINTF("simple_cc2420_read: FIFOP_IS_1 2\n");
    /*    strobe(CC2420_SFLUSHRX);
	  strobe(CC2420_SFLUSHRX);*/
    /* Another packet has been received and needs attention. */
    process_poll(&simple_cc2420_process);
    packet_seen = 1;
  } else {
    packet_seen = 0;
  }
  
  RELEASE_LOCK();
  
  if(len < FOOTER_LEN + TIMESTAMP_LEN) {
    return 0;
  }

  return len - FOOTER_LEN - TIMESTAMP_LEN;
}
/*---------------------------------------------------------------------------*/
void
simple_cc2420_set_txpower(u8_t power)
{
  u16_t reg;

  GET_LOCK();
  reg = getreg(CC2420_TXCTRL);
  reg = (reg & 0xffe0) | (power & 0x1f);
  setreg(CC2420_TXCTRL, reg);
  RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
int
simple_cc2420_get_txpower()
{
  return (int)(getreg(CC2420_TXCTRL) & 0x001f);
}
/*---------------------------------------------------------------------------*/
int
simple_cc2420_rssi(void)
{
  int rssi;
  int radio_was_off = 0;
  
  if(!receive_on) {
    radio_was_off = 1;
    simple_cc2420_on();
  }
  while(!(status() & BV(CC2420_RSSI_VALID))) {
    /*    printf("simple_cc2420_rssi: RSSI not valid.\n");*/
  }

  rssi = (int)((signed char)getreg(CC2420_RSSI));

  if(radio_was_off) {
    simple_cc2420_off();
  }
  return rssi;
}
/*---------------------------------------------------------------------------*/
