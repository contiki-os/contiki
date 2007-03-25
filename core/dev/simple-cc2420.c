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
 * @(#)$Id: simple-cc2420.c,v 1.4 2007/03/25 17:15:30 adamdunkels Exp $
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

/* #include "dev/leds.h" */

#include "dev/spi.h"
#include "dev/simple-cc2420.h"
#include "dev/cc2420_const.h"

#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f

#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif
/*---------------------------------------------------------------------------*/
PROCESS(simple_cc2420_process, "CC2420 driver");
/*---------------------------------------------------------------------------*/

static void (* receiver_callback)(void);

signed char simple_cc2420_last_rssi;
u8_t simple_cc2420_last_correlation;

static u8_t receive_on;
/* Radio stuff in network byte order. */
static u16_t pan_id;
/*---------------------------------------------------------------------------*/
static unsigned
getreg(enum cc2420_register regname)
{
  unsigned reg;
  int s = splhigh();
  FASTSPI_GETREG(regname, reg);
  splx(s);
  return reg;
}
/*---------------------------------------------------------------------------*/
static void
setreg(enum cc2420_register regname, unsigned value)
{
  int s = splhigh();
  FASTSPI_SETREG(regname, value);
  splx(s);
}
/*---------------------------------------------------------------------------*/
static void
strobe(enum cc2420_register regname)
{
  int s = splhigh();
  FASTSPI_STROBE(regname);
  splx(s);
}
/*---------------------------------------------------------------------------*/
static unsigned
status(void)
{
  u8_t status;
  int s = splhigh();
  FASTSPI_UPD_STATUS(status);
  splx(s);
  return status;
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
simple_cc2420_set_receiver(void (* recv)(void))
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
    __cc2420_arch_init();		/* Initalize ports and SPI. */
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
simple_cc2420_send(const u8_t *payload, u8_t payload_len)
{
  u8_t spiStatusByte;
  int s, i;
  
  /* struct hdr_802_15::len shall *not* be counted, thus the -1.
   * 2 == sizeof(footer).
   */
  /*  if(((hdr_len - 1) + payload_len + 2) > MAX_PACKET_LEN) {
    return -1;
    }*/

  /* This code uses the CC2420 CCA (Clear Channel Assessment) to
   * implement Carrier Sense Multiple Access with Collision Avoidance
   * (CSMA-CA) and requires the receiver to be enabled and ready.
   */
  if(!receive_on) {
    return -2;
  }

  /* Wait for previous transmission to finish and RSSI. */
  do {
    spiStatusByte = status();
    if(!(spiStatusByte & BV(CC2420_RSSI_VALID))) { /* RSSI needed by CCA */
      continue;
    }
  } while(spiStatusByte & BV(CC2420_TX_ACTIVE));

#if 0
  hdr->dst_pan = pan_id;	/* Not at fixed position! xxx/bg */
  last_correspondent = hdr->dst; /* Not dst either. */
  last_used_seq++;
  hdr->seq = last_used_seq;
  cc2420_ack_received = 0;
#endif
  
  /* Write packet to TX FIFO, appending FCS if AUTOCRC is enabled. */
  strobe(CC2420_SFLUSHTX); /* Cancel send that never started. */
  s = splhigh();
  /*  FASTSPI_WRITE_FIFO(hdr, hdr_len);*/
  {
    u8_t total_len = payload_len + 2; /* 2 bytes footer. */
    FASTSPI_WRITE_FIFO(&total_len, 1);
  }
  FASTSPI_WRITE_FIFO(payload, payload_len);
  splx(s);
  PRINTF("simple_cc2420_send: wrote %d bytes\n", payload_len);

  /*  if(hdr->dst == 0xffff) {
    int i;
    for(i = 1; i < 3; i++) {
      if(do_send() >= 0) {
	return 0;
      }
      clock_delay(i*256);
    }
    }*/

  if(FIFOP_IS_1 && !FIFO_IS_1) {
    /* RXFIFO overflow, send on retransmit. */
    PRINTF("rxfifo overflow!\n");
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
      PRINTF("simple_cc2420: do_send() transmission has started\n");
      return 0;			/* Transmission has started. */
    }
  }

  PRINTF("simple_cc2420: do_send() transmission never started\n");
  return -3;			/* Transmission never started! */
}
/*---------------------------------------------------------------------------*/
void
simple_cc2420_off(void)
{
  u8_t spiStatusByte;

  if (receive_on == 0)
    return;
  receive_on = 0;
  /* Wait for transmission to end before turning radio off. */
  do {
    spiStatusByte = status();
  } while(spiStatusByte & BV(CC2420_TX_ACTIVE));

  strobe(CC2420_SRFOFF);
  DISABLE_FIFOP_INT();
}
/*---------------------------------------------------------------------------*/
void
simple_cc2420_on(void)
{
  if(receive_on) {
    return;
  }
  receive_on = 1;

  strobe(CC2420_SRXON);
  strobe(CC2420_SFLUSHRX);
  ENABLE_FIFOP_INT();
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
  int s;
        
  f = 5*(f - 11) + 357 + 0x4000;
  /*
   * Writing RAM requires crystal oscillator to be stable.
   */
  do {
    spiStatusByte = status();
  } while(!(spiStatusByte & (BV(CC2420_XOSC16M_STABLE))));

  pan_id = pan;
  setreg(CC2420_FSCTRL, f);
  s = splhigh();
  FASTSPI_WRITE_RAM_LE(&pan, CC2420RAM_PANID, 2, f);
  FASTSPI_WRITE_RAM_LE(&addr, CC2420RAM_SHORTADDR, 2, f);
  if(ieee_addr != NULL) {
    FASTSPI_WRITE_RAM_LE(ieee_addr, CC2420RAM_IEEEADDR, 8, f);
  }
  splx(s);
}
/*---------------------------------------------------------------------------*/
static volatile u8_t rx_fifo_remaining_bytes;
    
/*
 * Interrupt either leaves frame intact in FIFO or reads *only* the
 * MAC header and sets rx_fifo_remaining_bytes.
 *
 * In order to quickly empty the FIFO ack processing is done at
 * interrupt priority rather than poll priority.
 */
int
__cc2420_intr(void)
{
  u8_t length;
  /*  const u8_t *const ack_footer = (u8_t *)&h.dst_pan;*/


  CLEAR_FIFOP_INT();

  if(spi_busy || rx_fifo_remaining_bytes > 0) {
    /* SPI bus hardware is currently used elsewhere (UART0 or I2C bus)
     * or we already have a packet in the works and will have to defer
     * interrupt processing of this packet in a fake interrupt.
     */
    process_poll(&simple_cc2420_process);
    return 1;
  }

  FASTSPI_READ_FIFO_BYTE(length);
  if(length > SIMPLE_CC2420_MAX_PACKET_LEN) {
    /* Oops, we must be out of sync. */
    FASTSPI_STROBE(CC2420_SFLUSHRX);
    FASTSPI_STROBE(CC2420_SFLUSHRX);

    return 0;
  }


  /* The payload and footer is now left in the RX FIFO and will be
   * picked up asynchronously at poll priority in the cc2420_process
   * below.
   */
  rx_fifo_remaining_bytes = length;
  process_poll(&simple_cc2420_process);

  return 1;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(simple_cc2420_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD();

    if(receiver_callback != NULL) {
      receiver_callback();
    } else {
      PRINTF("simple_cc2420_process dropping %d bytes\n",
	     rx_fifo_remaining_bytes);
      if(rx_fifo_remaining_bytes > 0) {
	int s;
	s = splhigh();
	FASTSPI_READ_FIFO_GARBAGE(rx_fifo_remaining_bytes);
	rx_fifo_remaining_bytes = 0; /* RX FIFO emptied! */
	splx(s);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
simple_cc2420_read(u8_t *buf, u8_t bufsize)
{
  u8_t footer[2];
  int len;
  int s;
  
  len = rx_fifo_remaining_bytes;
  
  if(len > 0) {
    /* Read payload and two bytes of footer */
    if(len > bufsize) {
      PRINTF("simple_cc2420_poll too big len=%d\n", len);
      s = splhigh();
      FASTSPI_READ_FIFO_GARBAGE(len);
      rx_fifo_remaining_bytes = 0; /* RX FIFO emptied! */
      splx(s);
      len = 2; /* We eventually return len - 2 */
    } else {
      s = splhigh();
      FASTSPI_READ_FIFO_NO_WAIT(buf, len - 2);
      FASTSPI_READ_FIFO_NO_WAIT(footer, 2);
      rx_fifo_remaining_bytes = 0; /* RX FIFO emptied! */
      splx(s);
      if(footer[1] & FOOTER1_CRC_OK) {
	simple_cc2420_last_rssi = footer[0];
	simple_cc2420_last_correlation = footer[1] & FOOTER1_CORRELATION;
	/*	if((h.fc0 & FC0_TYPE_MASK) == FC0_TYPE_DATA) {
	  uip_len = len - 2;
	  }*/
      }
    }
  }
  
  /* Clean up in case of FIFO overflow!  This happens for every full
   * length frame and is signaled by FIFOP = 1 and FIFO = 0.
   */
  if(FIFOP_IS_1 && !FIFO_IS_1) {
    strobe(CC2420_SFLUSHRX);
    strobe(CC2420_SFLUSHRX);
  }
  
  if(FIFOP_IS_1) {
    s = splhigh();
    __cc2420_intr();		/* Fake interrupt! */
    splx(s);
  }

  return len - 2; /* Remove two bytes for the footer. */
}
/*---------------------------------------------------------------------------*/
