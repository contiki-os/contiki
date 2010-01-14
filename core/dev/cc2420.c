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
 * @(#)$Id: cc2420.c,v 1.37 2010/01/14 23:32:05 adamdunkels Exp $
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

#include "dev/leds.h"
#include "dev/spi.h"
#include "dev/cc2420.h"
#include "dev/cc2420_const.h"

#include "net/rime/packetbuf.h"
#include "net/rime/rimestats.h"

#include "sys/timetable.h"

#define WITH_SEND_CCA 0


#if CC2420_CONF_TIMESTAMPS
#include "net/rime/timesynch.h"
#define TIMESTAMP_LEN 3
#else /* CC2420_CONF_TIMESTAMPS */
#define TIMESTAMP_LEN 0
#endif /* CC2420_CONF_TIMESTAMPS */
#define FOOTER_LEN 2

#ifndef CC2420_CONF_CHECKSUM
#define CC2420_CONF_CHECKSUM 0
#endif /* CC2420_CONF_CHECKSUM */

#ifndef CC2420_CONF_AUTOACK
#define CC2420_CONF_AUTOACK 0
#endif /* CC2420_CONF_AUTOACK */

#if CC2420_CONF_CHECKSUM
#include "lib/crc16.h"
#define CHECKSUM_LEN 2
#else
#define CHECKSUM_LEN 0
#endif /* CC2420_CONF_CHECKSUM */

#define AUX_LEN (CHECKSUM_LEN + TIMESTAMP_LEN + FOOTER_LEN)

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

void cc2420_arch_init(void);

/* XXX hack: these will be made as Chameleon packet attributes */
rtimer_clock_t cc2420_time_of_arrival, cc2420_time_of_departure;

int cc2420_authority_level_of_sender;

#if CC2420_CONF_TIMESTAMPS
static rtimer_clock_t setup_time_for_transmission;
static unsigned long total_time_for_transmission, total_transmission_len;
static int num_transmissions;
#endif /* CC2420_CONF_TIMESTAMPS */

/*---------------------------------------------------------------------------*/
PROCESS(cc2420_process, "CC2420 driver");
/*---------------------------------------------------------------------------*/

static void (* receiver_callback)(const struct radio_driver *);

int cc2420_on(void);
int cc2420_off(void);

int cc2420_read(void *buf, unsigned short bufsize);

int cc2420_send(const void *data, unsigned short len);

void cc2420_set_receiver(void (* recv)(const struct radio_driver *d));


signed char cc2420_last_rssi;
uint8_t cc2420_last_correlation;

const struct radio_driver cc2420_driver =
  {
    cc2420_send,
    cc2420_read,
    cc2420_set_receiver,
    cc2420_on,
    cc2420_off,
  };

static uint8_t receive_on;
/* Radio stuff in network byte order. */
static uint16_t pan_id;

static int channel;

/*---------------------------------------------------------------------------*/
static uint8_t rxptr; /* Pointer to the next byte in the rxfifo. */

static void
getrxdata(void *buf, int len)
{
  FASTSPI_READ_FIFO_NO_WAIT(buf, len);
  rxptr = (rxptr + len) & 0x7f;
}
static void
getrxbyte(uint8_t *byte)
{
  FASTSPI_READ_FIFO_BYTE(*byte);
  rxptr = (rxptr + 1) & 0x7f;
}
static void
flushrx(void)
{
  uint8_t dummy;

  FASTSPI_READ_FIFO_BYTE(dummy);
  FASTSPI_STROBE(CC2420_SFLUSHRX);
  FASTSPI_STROBE(CC2420_SFLUSHRX);
  rxptr = 0;
}
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
  uint8_t status;
  FASTSPI_UPD_STATUS(status);
  return status;
}
/*---------------------------------------------------------------------------*/
static uint8_t locked, lock_on, lock_off;

static void
on(void)
{
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  PRINTF("on\n");
  receive_on = 1;

  ENABLE_FIFOP_INT();
  strobe(CC2420_SRXON);
  flushrx();
}
static void
off(void)
{
  PRINTF("off\n");
  receive_on = 0;

  /* Wait for transmission to end before turning radio off. */
  while(status() & BV(CC2420_TX_ACTIVE));

  strobe(CC2420_SRFOFF);
  DISABLE_FIFOP_INT();
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
}
/*---------------------------------------------------------------------------*/
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
static void
set_txpower(uint8_t power)
{
  uint16_t reg;

  reg = getreg(CC2420_TXCTRL);
  reg = (reg & 0xffe0) | (power & 0x1f);
  setreg(CC2420_TXCTRL, reg);
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
cc2420_set_receiver(void (* recv)(const struct radio_driver *))
{
  receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
void
cc2420_init(void)
{
  uint16_t reg;
  {
    int s = splhigh();
    cc2420_arch_init();		/* Initalize ports and SPI. */
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

  /* Turn on/off automatic packet acknowledgment and address decoding. */
  reg = getreg(CC2420_MDMCTRL0);
#if CC2420_CONF_AUTOACK
  reg |= AUTOACK | ADR_DECODE;
#else
  reg &= ~(AUTOACK | ADR_DECODE);
#endif /* CC2420_CONF_AUTOACK */
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

  cc2420_set_pan_addr(0xffff, 0x0000, NULL);
  cc2420_set_channel(26);

  process_start(&cc2420_process, NULL);
}
/*---------------------------------------------------------------------------*/
int
cc2420_send(const void *payload, unsigned short payload_len)
{
  int i, txpower;
  uint8_t total_len;
#if CC2420_CONF_TIMESTAMPS
  struct timestamp timestamp;
#endif /* CC2420_CONF_TIMESTAMPS */
#if CC2420_CONF_CHECKSUM
  uint16_t checksum;
#endif /* CC2420_CONF_CHECKSUM */
  GET_LOCK();

  txpower = 0;
  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
    /* Remember the current transmission power */
    txpower = cc2420_get_txpower();
    /* Set the specified transmission power */
    set_txpower(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) - 1);
  }

  PRINTF("cc2420: sending %d bytes\n", payload_len);

  RIMESTATS_ADD(lltx);

  /* Wait for any previous transmission to finish. */
  while(status() & BV(CC2420_TX_ACTIVE));

  /* Write packet to TX FIFO. */
  strobe(CC2420_SFLUSHTX);

#if CC2420_CONF_CHECKSUM
  checksum = crc16_data(payload, payload_len, 0);
#endif /* CC2420_CONF_CHECKSUM */
  total_len = payload_len + AUX_LEN;
  FASTSPI_WRITE_FIFO(&total_len, 1);
  FASTSPI_WRITE_FIFO(payload, payload_len);
#if CC2420_CONF_CHECKSUM
  FASTSPI_WRITE_FIFO(&checksum, CHECKSUM_LEN);
#endif /* CC2420_CONF_CHECKSUM */

#if CC2420_CONF_TIMESTAMPS
  timestamp.authority_level = timesynch_authority_level();
  timestamp.time = timesynch_time();
  FASTSPI_WRITE_FIFO(&timestamp, TIMESTAMP_LEN);
#endif /* CC2420_CONF_TIMESTAMPS */

  /* The TX FIFO can only hold one packet. Make sure to not overrun
   * FIFO by waiting for transmission to start here and synchronizing
   * with the CC2420_TX_ACTIVE check in cc2420_send.
   *
   * Note that we may have to wait up to 320 us (20 symbols) before
   * transmission starts.
   */
#ifdef TMOTE_SKY
#define LOOP_20_SYMBOLS 400	/* 326us (msp430 @ 2.4576MHz) */
#elif __AVR__
#define LOOP_20_SYMBOLS 500	/* XXX */
#endif

#if WITH_SEND_CCA
  strobe(CC2420_SRXON);
  while(!(status() & BV(CC2420_RSSI_VALID)));
  strobe(CC2420_STXONCCA);
#else /* WITH_SEND_CCA */
  strobe(CC2420_STXON);
#endif /* WITH_SEND_CCA */

  for(i = LOOP_20_SYMBOLS; i > 0; i--) {
    if(SFD_IS_1) {
      if(!(status() & BV(CC2420_TX_ACTIVE))) {
        /* SFD went high yet we are not transmitting!
         * => We started receiving a packet right now */
        return RADIO_TX_ERR;
      }

#if CC2420_CONF_TIMESTAMPS
      rtimer_clock_t txtime = timesynch_time();
#endif /* CC2420_CONF_TIMESTAMPS */

      if(receive_on) {
	ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
      }
      ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

      /* We wait until transmission has ended so that we get an
	 accurate measurement of the transmission time.*/
      while(status() & BV(CC2420_TX_ACTIVE));

#if CC2420_CONF_TIMESTAMPS
      setup_time_for_transmission = txtime - timestamp.time;

      if(num_transmissions < 10000) {
	total_time_for_transmission += timesynch_time() - txtime;
	total_transmission_len += total_len;
	num_transmissions++;
      }

#endif /* CC2420_CONF_TIMESTAMPS */

#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS
      ENERGEST_OFF_LEVEL(ENERGEST_TYPE_TRANSMIT,cc2420_get_txpower());
#endif
      ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
      if(receive_on) {
	ENERGEST_ON(ENERGEST_TYPE_LISTEN);
      } else {
	/* We need to explicitly turn off the radio,
	 * since STXON[CCA] -> TX_ACTIVE -> RX_ACTIVE */
	off();
      }

      if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
        /* Restore the transmission power */
        set_txpower(txpower & 0xff);
      }

      RELEASE_LOCK();
      return RADIO_TX_OK;
    }
  }

  /* If we are using WITH_SEND_CCA, we get here if the packet wasn't
     transmitted because of other channel activity. */
  RIMESTATS_ADD(contentiondrop);
  PRINTF("cc2420: do_send() transmission never started\n");

  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
    /* Restore the transmission power */
    set_txpower(txpower & 0xff);
  }

  RELEASE_LOCK();
  return RADIO_TX_ERR;          /* Transmission never started! */
}
/*---------------------------------------------------------------------------*/
int
cc2420_off(void)
{
  /* Don't do anything if we are already turned off. */
  if(receive_on == 0) {
    return 1;
  }

  /* If we are called when the driver is locked, we indicate that the
     radio should be turned off when the lock is unlocked. */
  if(locked) {
    lock_off = 1;
    return 1;
  }

  /* If we are currently receiving a packet (indicated by SFD == 1),
     we don't actually switch the radio off now, but signal that the
     driver should switch off the radio once the packet has been
     received and processed, by setting the 'lock_off' variable. */
  if(SFD_IS_1) {
    lock_off = 1;
    return 1;
  }

  off();
  return 1;
}
/*---------------------------------------------------------------------------*/
int
cc2420_on(void)
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
int
cc2420_get_channel(void)
{
  return channel;
}
/*---------------------------------------------------------------------------*/
void
cc2420_set_channel(int c)
{
  uint16_t f;
  /*
   * Subtract the base channel (11), multiply by 5, which is the
   * channel spacing. 357 is 2405-2048 and 0x4000 is LOCK_THR = 1.
   */

  channel = c;

  f = 5 * (c - 11) + 357 + 0x4000;
  /*
   * Writing RAM requires crystal oscillator to be stable.
   */
  while(!(status() & (BV(CC2420_XOSC16M_STABLE))));

  /* Wait for any transmission to end. */
  while(status() & BV(CC2420_TX_ACTIVE));

  setreg(CC2420_FSCTRL, f);

  /* If we are in receive mode, we issue an SRXON command to ensure
     that the VCO is calibrated. */
  if(receive_on) {
    strobe(CC2420_SRXON);
  }

}
/*---------------------------------------------------------------------------*/
void
cc2420_set_pan_addr(unsigned pan,
			   unsigned addr,
			   const uint8_t *ieee_addr)
{
  uint16_t f = 0;
  /*
   * Writing RAM requires crystal oscillator to be stable.
   */
  while(!(status() & (BV(CC2420_XOSC16M_STABLE))));

  pan_id = pan;
  FASTSPI_WRITE_RAM_LE(&pan, CC2420RAM_PANID, 2, f);
  FASTSPI_WRITE_RAM_LE(&addr, CC2420RAM_SHORTADDR, 2, f);
  if(ieee_addr != NULL) {
    uint8_t addr[8];
    /* LSB first, MSB last for 802.15.4 addresses in CC2420 */
    for (f = 0; f < 8; f++) {
      addr[7 - f] = ieee_addr[f];
    }
    FASTSPI_WRITE_RAM_LE(addr, CC2420RAM_IEEEADDR, 8, f);
  }
}
/*---------------------------------------------------------------------------*/
/*
 * Interrupt leaves frame intact in FIFO.
 */
#if CC2420_CONF_TIMESTAMPS
static volatile rtimer_clock_t interrupt_time;
static volatile int interrupt_time_set;
#endif /* CC2420_CONF_TIMESTAMPS */
#if CC2420_TIMETABLE_PROFILING
#define cc2420_timetable_size 16
TIMETABLE(cc2420_timetable);
TIMETABLE_AGGREGATE(aggregate_time, 10);
#endif /* CC2420_TIMETABLE_PROFILING */
int
cc2420_interrupt(void)
{
#if CC2420_CONF_TIMESTAMPS
  if(!interrupt_time_set) {
    interrupt_time = timesynch_time();
    interrupt_time_set = 1;
  }
#endif /* CC2420_CONF_TIMESTAMPS */

  CLEAR_FIFOP_INT();
  process_poll(&cc2420_process);
#if CC2420_TIMETABLE_PROFILING
  timetable_clear(&cc2420_timetable);
  TIMETABLE_TIMESTAMP(cc2420_timetable, "interrupt");
#endif /* CC2420_TIMETABLE_PROFILING */
  return 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2420_process, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("cc2420_process: started\n");

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
#if CC2420_TIMETABLE_PROFILING
    TIMETABLE_TIMESTAMP(cc2420_timetable, "poll");
#endif /* CC2420_TIMETABLE_PROFILING */

    if(receiver_callback != NULL) {
      PRINTF("cc2420_process: calling receiver callback\n");
      receiver_callback(&cc2420_driver);
#if CC2420_TIMETABLE_PROFILING
      TIMETABLE_TIMESTAMP(cc2420_timetable, "end");
      timetable_aggregate_compute_detailed(&aggregate_time,
					   &cc2420_timetable);
      timetable_clear(&cc2420_timetable);
#endif /* CC2420_TIMETABLE_PROFILING */
    } else {
      PRINTF("cc2420_process not receiving function\n");
      flushrx();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
cc2420_read(void *buf, unsigned short bufsize)
{
  uint8_t footer[2];
  uint8_t len;
#if CC2420_CONF_CHECKSUM
  uint16_t checksum;
#endif /* CC2420_CONF_CHECKSUM */
#if CC2420_CONF_TIMESTAMPS
  struct timestamp t;
#endif /* CC2420_CONF_TIMESTAMPS */

  if(!FIFOP_IS_1) {
    /* If FIFO is 0, there is no packet in the RXFIFO. */
    return 0;
  }

  GET_LOCK();

  getrxbyte(&len);

  if(len > CC2420_MAX_PACKET_LEN) {
    /* Oops, we must be out of sync. */
    flushrx();
    RIMESTATS_ADD(badsynch);
    RELEASE_LOCK();
    return 0;
  }

  if(len <= AUX_LEN) {
    flushrx();
    RIMESTATS_ADD(tooshort);
    RELEASE_LOCK();
    return 0;
  }

  if(len - AUX_LEN > bufsize) {
    flushrx();
    RIMESTATS_ADD(toolong);
    RELEASE_LOCK();
    return 0;
  }

  getrxdata(buf, len - AUX_LEN);
#if CC2420_CONF_CHECKSUM
  getrxdata(&checksum, CHECKSUM_LEN);
#endif /* CC2420_CONF_CHECKSUM */
#if CC2420_CONF_TIMESTAMPS
  getrxdata(&t, TIMESTAMP_LEN);
#endif /* CC2420_CONF_TIMESTAMPS */
  getrxdata(footer, FOOTER_LEN);

#if CC2420_CONF_CHECKSUM
  if(checksum != crc16_data(buf, len - AUX_LEN, 0)) {
    PRINTF("checksum failed 0x%04x != 0x%04x\n",
	   checksum, crc16_data(buf, len - AUX_LEN, 0));
  }

  if(footer[1] & FOOTER1_CRC_OK &&
     checksum == crc16_data(buf, len - AUX_LEN, 0)) {
#else
  if(footer[1] & FOOTER1_CRC_OK) {
#endif /* CC2420_CONF_CHECKSUM */
    cc2420_last_rssi = footer[0];
    cc2420_last_correlation = footer[1] & FOOTER1_CORRELATION;


    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, cc2420_last_rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, cc2420_last_correlation);

    RIMESTATS_ADD(llrx);

#if CC2420_CONF_TIMESTAMPS
    if(interrupt_time_set) {
      cc2420_time_of_arrival = interrupt_time;
      cc2420_time_of_departure =
	t.time +
	setup_time_for_transmission +
	(total_time_for_transmission * (len - 2)) / total_transmission_len;
    
      cc2420_authority_level_of_sender = t.authority_level;
      interrupt_time_set = 0;
    } else {
      /* Bypass timesynch */
      cc2420_authority_level_of_sender = timesynch_authority_level();
    }

    packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, t.time);
#endif /* CC2420_CONF_TIMESTAMPS */

  } else {
    RIMESTATS_ADD(badcrc);
    len = AUX_LEN;
  }

  /* Clean up in case of FIFO overflow!  This happens for every full
   * length frame and is signaled by FIFOP = 1 and FIFO = 0.
   */
  if(FIFOP_IS_1 && !FIFO_IS_1) {
    /*    printf("cc2420_read: FIFOP_IS_1 1\n");*/
    flushrx();
  } else if(FIFOP_IS_1) {
    /* Another packet has been received and needs attention. */
    process_poll(&cc2420_process);
  }

  RELEASE_LOCK();

  if(len < AUX_LEN) {
    return 0;
  }

  return len - AUX_LEN;
}
/*---------------------------------------------------------------------------*/
void
cc2420_set_txpower(uint8_t power)
{
  GET_LOCK();
  set_txpower(power);
  RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
int
cc2420_get_txpower(void)
{
  return (int)(getreg(CC2420_TXCTRL) & 0x001f);
}
/*---------------------------------------------------------------------------*/
int
cc2420_rssi(void)
{
  int rssi;
  int radio_was_off = 0;

  if(!receive_on) {
    radio_was_off = 1;
    cc2420_on();
  }
  while(!(status() & BV(CC2420_RSSI_VALID))) {
    /*    printf("cc2420_rssi: RSSI not valid.\n");*/
  }

  rssi = (int)((signed char)getreg(CC2420_RSSI));

  if(radio_was_off) {
    cc2420_off();
  }
  return rssi;
}
/*---------------------------------------------------------------------------*/
