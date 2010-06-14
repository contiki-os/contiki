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
 * @(#)$Id: cc2420.c,v 1.53 2010/06/14 19:19:16 adamdunkels Exp $
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

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"

#include "sys/timetable.h"

#define WITH_SEND_CCA 1

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

#define AUX_LEN (CHECKSUM_LEN + FOOTER_LEN)


#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define DEBUG_LEDS DEBUG
#undef LEDS_ON
#undef LEDS_OFF
#if DEBUG_LEDS
#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#else
#define LEDS_ON(x)
#define LEDS_OFF(x)
#endif

void cc2420_arch_init(void);

/* XXX hack: these will be made as Chameleon packet attributes */
rtimer_clock_t cc2420_time_of_arrival, cc2420_time_of_departure;

int cc2420_authority_level_of_sender;

int cc2420_packets_seen, cc2420_packets_read;

static uint8_t volatile pending;

/*---------------------------------------------------------------------------*/
PROCESS(cc2420_process, "CC2420 driver");
/*---------------------------------------------------------------------------*/


int cc2420_on(void);
int cc2420_off(void);

static int cc2420_read(void *buf, unsigned short bufsize);

static int cc2420_prepare(const void *data, unsigned short len);
static int cc2420_transmit(unsigned short len);
static int cc2420_send(const void *data, unsigned short len);

static int cc2420_receiving_packet(void);
static int pending_packet(void);
static int cc2420_cca(void);

signed char cc2420_last_rssi;
uint8_t cc2420_last_correlation;

const struct radio_driver cc2420_driver =
  {
    cc2420_init,
    cc2420_prepare,
    cc2420_transmit,
    cc2420_send,
    cc2420_read,
    cc2420_cca,
    cc2420_receiving_packet,
    pending_packet,
    cc2420_on,
    cc2420_off,
  };

static uint8_t receive_on;

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
  /*  PRINTF("on\n");*/
  receive_on = 1;

  ENABLE_FIFOP_INT();
  strobe(CC2420_SRXON);
  while(!(status() & (BV(CC2420_XOSC16M_STABLE))));
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  LEDS_ON(LEDS_GREEN);
}
static void
off(void)
{
  /*  PRINTF("off\n");*/
  receive_on = 0;

  /* Wait for transmission to end before turning radio off. */
  while(status() & BV(CC2420_TX_ACTIVE));

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  strobe(CC2420_SRFOFF);
  DISABLE_FIFOP_INT();
  LEDS_OFF(LEDS_GREEN);

  if(!FIFOP_IS_1) {
    flushrx();
  }
}
/*---------------------------------------------------------------------------*/
#define GET_LOCK() locked++
static void RELEASE_LOCK(void) {
  if(locked == 1) {
    if(lock_on) {
      on();
      lock_on = 0;
    }
    if(lock_off) {
      LEDS_OFF(LEDS_BLUE);
      off();
      lock_off = 0;
    }
  }
  locked--;
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
int
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

  /* Set transmission turnaround time to the lower setting (8 symbols
     = 0.128 ms) instead of the default (12 symbols = 0.192 ms). */
  /*  reg = getreg(CC2420_TXCTRL);
  reg &= ~(1 << 13);
  setreg(CC2420_TXCTRL, reg);*/

  
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

  flushrx();
  
  process_start(&cc2420_process, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
cc2420_transmit(unsigned short payload_len)
{
  int i, txpower;
  uint8_t total_len;
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

  total_len = payload_len + AUX_LEN;
  
  /* The TX FIFO can only hold one packet. Make sure to not overrun
   * FIFO by waiting for transmission to start here and synchronizing
   * with the CC2420_TX_ACTIVE check in cc2420_send.
   *
   * Note that we may have to wait up to 320 us (20 symbols) before
   * transmission starts.
   */
#ifdef TMOTE_SKY
#define LOOP_20_SYMBOLS 800	/* 326us (msp430 @ 2.4576MHz) */
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
        /* SFD went high but we are not transmitting. This means that
           we just started receiving a packet, so we drop the
           transmission. */
        RELEASE_LOCK();
        return RADIO_TX_COLLISION;
      }
      if(receive_on) {
	ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
      }
      ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

      /* We wait until transmission has ended so that we get an
	 accurate measurement of the transmission time.*/
      while(status() & BV(CC2420_TX_ACTIVE));

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
  return RADIO_TX_COLLISION;
}
/*---------------------------------------------------------------------------*/
static int
cc2420_prepare(const void *payload, unsigned short payload_len)
{
  uint8_t total_len;
#if CC2420_CONF_CHECKSUM
  uint16_t checksum;
#endif /* CC2420_CONF_CHECKSUM */
  GET_LOCK();

  PRINTF("cc2420: sending %d bytes\n", payload_len);

  RIMESTATS_ADD(lltx);

  /* Wait for any previous transmission to finish. */
  /*  while(status() & BV(CC2420_TX_ACTIVE));*/

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

  RELEASE_LOCK();
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
cc2420_send(const void *payload, unsigned short payload_len)
{
  cc2420_prepare(payload, payload_len);
  return cc2420_transmit(payload_len);
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
    /*    printf("Off when locked (%d)\n", locked);*/
    LEDS_ON(LEDS_GREEN + LEDS_BLUE);
    lock_off = 1;
    return 1;
  }

  GET_LOCK();
  /* If we are currently receiving a packet (indicated by SFD == 1),
     we don't actually switch the radio off now, but signal that the
     driver should switch off the radio once the packet has been
     received and processed, by setting the 'lock_off' variable. */
  if(status() & BV(CC2420_TX_ACTIVE)) {
    lock_off = 1;
  } else {
    off();
  }
  RELEASE_LOCK();
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
    LEDS_ON(LEDS_GREEN + LEDS_RED);
    lock_on = 1;
    return 1;
  }

  GET_LOCK();
  on();
  RELEASE_LOCK();
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

  GET_LOCK();
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

  RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
void
cc2420_set_pan_addr(unsigned pan,
                    unsigned addr,
                    const uint8_t *ieee_addr)
{
  uint16_t f = 0;
  uint8_t tmp[2];

  GET_LOCK();
  
  /*
   * Writing RAM requires crystal oscillator to be stable.
   */
  while(!(status() & (BV(CC2420_XOSC16M_STABLE))));

  tmp[0] = pan & 0xff;
  tmp[1] = pan >> 8;
  FASTSPI_WRITE_RAM_LE(&tmp, CC2420RAM_PANID, 2, f);

  tmp[0] = addr & 0xff;
  tmp[1] = addr >> 8;
  FASTSPI_WRITE_RAM_LE(&tmp, CC2420RAM_SHORTADDR, 2, f);
  if(ieee_addr != NULL) {
    uint8_t tmp_addr[8];
    /* LSB first, MSB last for 802.15.4 addresses in CC2420 */
    for (f = 0; f < 8; f++) {
      tmp_addr[7 - f] = ieee_addr[f];
    }
    FASTSPI_WRITE_RAM_LE(tmp_addr, CC2420RAM_IEEEADDR, 8, f);
  }
  RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
/*
 * Interrupt leaves frame intact in FIFO.
 */
#if CC2420_TIMETABLE_PROFILING
#define cc2420_timetable_size 16
TIMETABLE(cc2420_timetable);
TIMETABLE_AGGREGATE(aggregate_time, 10);
#endif /* CC2420_TIMETABLE_PROFILING */
int
cc2420_interrupt(void)
{
  CLEAR_FIFOP_INT();
  process_poll(&cc2420_process);
#if CC2420_TIMETABLE_PROFILING
  timetable_clear(&cc2420_timetable);
  TIMETABLE_TIMESTAMP(cc2420_timetable, "interrupt");
#endif /* CC2420_TIMETABLE_PROFILING */

  pending++;
  
  cc2420_packets_seen++;
  return 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2420_process, ev, data)
{
  int len;
  PROCESS_BEGIN();

  PRINTF("cc2420_process: started\n");

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
#if CC2420_TIMETABLE_PROFILING
    TIMETABLE_TIMESTAMP(cc2420_timetable, "poll");
#endif /* CC2420_TIMETABLE_PROFILING */
    
    PRINTF("cc2420_process: calling receiver callback\n");

    packetbuf_clear();
    len = cc2420_read(packetbuf_dataptr(), PACKETBUF_SIZE);
    if(len > 0) {
      packetbuf_set_datalen(len);

      NETSTACK_RDC.input();
#if CC2420_TIMETABLE_PROFILING
      TIMETABLE_TIMESTAMP(cc2420_timetable, "end");
      timetable_aggregate_compute_detailed(&aggregate_time,
                                           &cc2420_timetable);
      timetable_clear(&cc2420_timetable);
#endif /* CC2420_TIMETABLE_PROFILING */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
cc2420_read(void *buf, unsigned short bufsize)
{
  uint8_t footer[2];
  uint8_t len;
#if CC2420_CONF_CHECKSUM
  uint16_t checksum;
#endif /* CC2420_CONF_CHECKSUM */

  if(!FIFOP_IS_1) {
    return 0;
  }
  /*  if(!pending) {
    return 0;
    }*/
  
  pending = 0;
  
  GET_LOCK();

  cc2420_packets_read++;

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

  } else {
    RIMESTATS_ADD(badcrc);
    len = AUX_LEN;
  }

  if(FIFOP_IS_1) {
    if(!FIFO_IS_1) {
      /* Clean up in case of FIFO overflow!  This happens for every
       * full length frame and is signaled by FIFOP = 1 and FIFO =
       * 0. */
      flushrx();
    } else {
      /* Another packet has been received and needs attention. */
      process_poll(&cc2420_process);
    }
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
  int power;
  GET_LOCK();
  power = (int)(getreg(CC2420_TXCTRL) & 0x001f);
  RELEASE_LOCK();
  return power;
}
/*---------------------------------------------------------------------------*/
int
cc2420_rssi(void)
{
  int rssi;
  int radio_was_off = 0;

  GET_LOCK();

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
  RELEASE_LOCK();
  return rssi;
}
/*---------------------------------------------------------------------------*/
int
cc2420_cca_valid(void)
{
  int valid;
  if(locked) {
    return 1;
  }
  GET_LOCK();
  valid = !!(status() & BV(CC2420_RSSI_VALID));
  RELEASE_LOCK();
  return valid;
}
/*---------------------------------------------------------------------------*/
static int
cc2420_cca(void)
{
  int cca;
  int radio_was_off = 0;

  /* If the radio is locked by an underlying thread (because we are
     being invoked through an interrupt), we preted that the coast is
     clear (i.e., no packet is currently being transmitted by a
     neighbor). */
  if(locked) {
    return 1;
  }

  GET_LOCK();
  if(!receive_on) {
    radio_was_off = 1;
    cc2420_on();
  }

  /* Make sure that the radio really got turned on. */
  if(!receive_on) {
    RELEASE_LOCK();
    return 1;
  }

  while(!(status() & BV(CC2420_RSSI_VALID))) {
    /*    printf("cc2420_rssi: RSSI not valid.\n"); */
  }

  cca = CCA_IS_1;

  if(radio_was_off) {
    cc2420_off();
  }
  RELEASE_LOCK();
  return cca;
}
/*---------------------------------------------------------------------------*/
int
cc2420_receiving_packet(void)
{
  return SFD_IS_1;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return FIFOP_IS_1;
}
/*---------------------------------------------------------------------------*/
void
cc2420_set_cca_threshold(int value)
{
  uint16_t shifted = value << 8;
  GET_LOCK();
  setreg(CC2420_RSSI, shifted);
  RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
