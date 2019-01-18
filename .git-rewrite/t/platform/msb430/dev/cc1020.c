/*
Copyright 2006, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universität Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.

For documentation and questions please use the web site
http://scatterweb.mi.fu-berlin.de and the mailinglist
scatterweb@lists.spline.inf.fu-berlin.de (subscription via the Website).
Berlin, 2006
*/


/**
 * @file	cc1020.c
 * @author	FUB ScatterWeb Developers, Michael Baar, Nicolas Tsiftes
 *
 * Taken from ScatterWeb² 1.1 and modified/reformatted for Contiki 2.0
 **/

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "contiki.h"
#include "contiki-msb430.h"
#include "cc1020-internal.h"
#include "cc1020.h"
#include "lib/random.h"
#include "net/rime/rimestats.h"
#include "dev/irq.h"
#include "dev/dma.h"
#include "energest.h"

static int cc1020_calibrate(void);
static int cc1020_setupTX(int);
static int cc1020_setupRX(int);
static void cc1020_setupPD(void);
static void cc1020_wakeupTX(int);
static void cc1020_wakeupRX(int);
static uint8_t cc1020_read_reg(uint8_t addr);
static void cc1020_write_reg(uint8_t addr, uint8_t adata);
static void cc1020_load_config(const uint8_t *);
static void cc1020_reset(void);

// current mode of cc1020 chip
static volatile enum cc1020_state cc1020_state = CC1020_OFF;
static volatile uint8_t cc1020_rxbuf[HDRSIZE + CC1020_BUFFERSIZE];
static uint8_t cc1020_txbuf[PREAMBLESIZE + SYNCWDSIZE + HDRSIZE +
			   CC1020_BUFFERSIZE + TAILSIZE];
//static volatile enum cc1020_rxstate cc1020_rxstate = CC1020_RX_SEARCHING;

// number of bytes in receive and transmit buffers respectively.
static uint16_t cc1020_rxlen;
static uint16_t cc1020_txlen;

// received signal strength indicator reading for last received packet
static volatile uint8_t rssi;

// callback when a packet has been received
static uint8_t cc1020_pa_power = PA_POWER;

static volatile int dma_done;

static void (*receiver_callback)(const struct radio_driver *);

const struct radio_driver cc1020_driver =
  {
    cc1020_send,
    cc1020_read,
    cc1020_set_receiver,
    cc1020_on,
    cc1020_off
  };

#define MS_DELAY(x) clock_delay(354 * (x))

PROCESS(cc1020_receiver_process, "CC1020 receiver");

static void
dma_callback(void)
{
  dma_done = 1;
}

void
cc1020_init(const uint8_t *config)
{
  cc1020_setupPD();
  cc1020_reset();
  cc1020_load_config(config);

  // init tx buffer with preamble + syncword
  memset(cc1020_txbuf, PREAMBLE, PREAMBLESIZE);
  memcpy((char *)cc1020_txbuf + PREAMBLESIZE, &syncword, SYNCWDSIZE);

  // calibrate receiver
  cc1020_wakeupRX(RX_CURRENT);
  if (!cc1020_calibrate())
    printf("rx calibration failed\n");

  // calibrate transmitter
  cc1020_wakeupTX(TX_CURRENT);
  if (!cc1020_calibrate())
    printf("tx calibration failed\n");

  // power down
  cc1020_setupPD();

  process_start(&cc1020_receiver_process, NULL);
  dma_subscribe(0, dma_callback);
}

void
cc1020_set_rx(void)
{
  int s;

  s = splhigh();

  // Reset SEL for P3[1-3] (CC DIO, DIO, DCLK) and P3[4-5] (Camera Rx+Tx)
  P3SEL &= ~0x3E;
  IFG1 &= ~(UTXIE0 | URXIE0);	// Clear interrupt flags
  ME1 &= ~(UTXE0 | URXE0);	// Disable Uart0 Tx + Rx
  UCTL0 = SWRST;		// U0 into reset state.
  UCTL0 |= CHAR | SYNC;		// 8-bit character, SPI, Slave mode

  // CKPH works also, but not CKPH+CKPL or none of them!!
  UTCTL0 = CKPL | STC;
  URCTL0 = 0x00;
  UBR00 = 0x00;			// No baudrate divider 
  UBR10 = 0x00;			// settings for a spi
  UMCTL0 = 0x00;		// slave.
  ME1 |= USPIE0;		// Enable USART0 TXD/RXD, disabling does not yield any powersavings
  P3SEL |= 0x0A;		// Select rx line and clk
  UCTL0 &= ~SWRST;		// Clear reset bit
  splx(s);

  // configure driver
  cc1020_rxlen = 0;		// receive buffer position to start
  CC1020_SET_OPSTATE(CC1020_RX | CC1020_RX_SEARCHING); // driver state to receive mode

  // configure radio
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  cc1020_wakeupRX(RX_CURRENT);
  cc1020_setupRX(RX_CURRENT);
  LNA_POWER_ON();		// enable amplifier

  // activate
  IE1 |= URXIE0;		// enable interrupt
}

void
cc1020_set_tx(void)
{
  int s;

  // configure radio rx
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  LNA_POWER_OFF();		// power down LNA
  s = splhigh();
  DISABLE_RX_IRQ();
  P3SEL &= ~0x02;		// Ensure Rx line is off
  splx(s);

  // configure radio tx
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
  cc1020_wakeupTX(TX_CURRENT);
  cc1020_setupTX(TX_CURRENT);
  P3SEL |= 0x0C;		// select Tx line and clk
  U0CTL |= SWRST;		// UART to reset mode
  IFG1 &= ~UTXIFG0;		// Reset IFG.

  // configure driver
  CC1020_SET_OPSTATE(CC1020_TX);
}

void
cc1020_set_power(uint8_t pa_power)
{
  cc1020_pa_power = pa_power;
}

int
cc1020_send(const void *buf, unsigned short len)
{
  int try;

  if (cc1020_state == CC1020_OFF)
    return -2;

  if (len > CC1020_BUFFERSIZE)
    return -1;

  /* The preamble and the sync word are already in buffer. */
  cc1020_txlen = PREAMBLESIZE + SYNCWDSIZE;

  // header
  cc1020_txbuf[cc1020_txlen++] = 0x00;
  cc1020_txbuf[cc1020_txlen++] = HDRSIZE + len;

  // data to send
  memcpy((char *)cc1020_txbuf + cc1020_txlen, buf, len);
  cc1020_txlen += len;

  // suffix
  cc1020_txbuf[cc1020_txlen++] = TAIL;
  cc1020_txbuf[cc1020_txlen++] = TAIL;

  // Wait for the medium to become idle.
  if (cc1020_carrier_sense()) {
    for (try = 0; try < CC1020_CONF_CCA_TIMEOUT; try++) {
      MS_DELAY(1);
      if (!cc1020_carrier_sense()) {
	break;
      }
    }
    if (try == CC1020_CONF_CCA_TIMEOUT) {
      return -3;
    }

    // Then wait for a short pseudo-random time before sending.
    clock_delay(100 * ((random_rand() + 1) & 0xf));
  }

  // Switch to transceive mode.
  cc1020_set_tx();

  // Initiate radio transfer.
  dma_done = 0;
  dma_transfer(&TXBUF0, cc1020_txbuf, cc1020_txlen);
  while (!dma_done);

  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  RIMESTATS_ADD(lltx);

  // clean up
  cc1020_txlen = 0;
  if (cc1020_state & CC1020_TURN_OFF) {
    cc1020_off();
  } else {
    cc1020_set_rx();
  }

  return len;
}

int
cc1020_read(void *buf, unsigned short size)
{
  unsigned len;

  if (cc1020_rxlen <= HDRSIZE)
    return 0;

  len = cc1020_rxlen - HDRSIZE;
  if (len > size) {
    RIMESTATS_ADD(toolong);
    return -1;
  }

  memcpy(buf, (char *)cc1020_rxbuf + HDRSIZE, len);
  RIMESTATS_ADD(llrx);

  // reset receiver
  cc1020_rxlen = 0;

  if ((cc1020_state & CC1020_TURN_OFF) && (cc1020_txlen == 0)) {
    cc1020_off();
  } else {
    CC1020_SET_OPSTATE(CC1020_RX | CC1020_RX_SEARCHING);
    ENABLE_RX_IRQ();
  }

  return len;
}

void
cc1020_set_receiver(void (*recv)(const struct radio_driver *))
{
  receiver_callback = recv;
}

int
cc1020_on(void)
{
  cc1020_state &= ~CC1020_TURN_OFF;
  // Switch to receive mode
  cc1020_set_rx();

  return 1;
}

int
cc1020_off(void)
{
  int s;

  if (cc1020_state & CC1020_RX_SEARCHING) {
    // Discard the current read buffer when the radio is shutting down.
    cc1020_rxlen = 0;

    LNA_POWER_OFF();		// power down lna
    s = splhigh();
    DISABLE_RX_IRQ();
    cc1020_state = CC1020_OFF;
    splx(s);
    cc1020_setupPD();		// power down radio
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    cc1020_state = CC1020_OFF;
  } else {
    cc1020_state |= CC1020_TURN_OFF;
  }
  return 1;
}

uint8_t
cc1020_get_rssi(void)
{
  rssi = cc1020_read_reg(CC1020_RSS);
  return rssi;
}

int
cc1020_carrier_sense(void)
{
  return !!(cc1020_read_reg(CC1020_STATUS) & CARRIER_SENSE);
}

PROCESS_THREAD(cc1020_receiver_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    ev = PROCESS_EVENT_NONE;
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    if(receiver_callback != NULL) {
      receiver_callback(&cc1020_driver);
    }
  }

  PROCESS_END();
}

interrupt(UART0RX_VECTOR) cc1020_rxhandler(void)
{
  static signed char syncbs;
  static union {
    struct {
      uint8_t b2;
      uint8_t b1;
      uint8_t b4;
      uint8_t b3;
    };
    struct {
      uint16_t i1;
      uint16_t i2;
    };
  } shiftbuf;
  static unsigned char pktlen;

  if (cc1020_state & CC1020_RX_SEARCHING) {
    shiftbuf.b1 = shiftbuf.b2;
    shiftbuf.b2 = shiftbuf.b3;
    shiftbuf.b3 = shiftbuf.b4;
    shiftbuf.b4 = RXBUF0;
    if (shiftbuf.i1 == 0xAAD3 && shiftbuf.b3 == 0x91) {
      // 0  AA D3 91 00 | FF 00 |
      syncbs = 0;
      cc1020_rxbuf[cc1020_rxlen++] = shiftbuf.b4;
    } else if (shiftbuf.i1 == 0x5569 && shiftbuf.i2 == 0xC880) {
      // 1  55 69 C8 80 | 7F 80 |
      syncbs = -1;
    } else if (shiftbuf.i1 == 0xAAB4 && shiftbuf.i2 == 0xE440) {
      // 2  AA B4 E4 40 | 3F C0 |
      syncbs = -2;
    } else if (shiftbuf.i1 == 0x555A && shiftbuf.i2 == 0x7220) {
      // 3  55 5A 72 20 | 1F E0 |
      syncbs = -3;
    } else if (shiftbuf.i1 == 0xAAAD && shiftbuf.i2 == 0x3910) {
      // 4  AA AD 39 10 | 0F F0 |
      syncbs = -4;
    } else if (shiftbuf.i1 == 0x5556 && shiftbuf.i2 == 0x9C88) {
      // 5  55 56 9C 88 | 07 F8 |
      syncbs = +3;
    } else if (shiftbuf.i1 == 0xAAAB && shiftbuf.i2 == 0x4E44) {
      // 6  AA AB 4E 44 | 03 FC |
      syncbs = +2;
    } else if (shiftbuf.i1 == 0x5555 && shiftbuf.i2 == 0xA722) {
      // 7  55 55 A7 22 | 01 FE |
      syncbs = +1;
    } else {
      return;
    }
    // Update RSSI.
    rssi = cc1020_read_reg(CC1020_RSS);
    CC1020_SET_OPSTATE(CC1020_RX | CC1020_RX_RECEIVING);
  } else if (cc1020_state & CC1020_RX_RECEIVING) {
    if (syncbs == 0) {
      cc1020_rxbuf[cc1020_rxlen] = RXBUF0;
    } else {
      shiftbuf.b3 = shiftbuf.b4;
      shiftbuf.b4 = RXBUF0;
      if (syncbs < 0) {
        shiftbuf.i1 = shiftbuf.i2 << -syncbs;
        cc1020_rxbuf[cc1020_rxlen] = shiftbuf.b1;
      } else {
        shiftbuf.i1 = shiftbuf.i2 >> syncbs;
        cc1020_rxbuf[cc1020_rxlen] = shiftbuf.b2;
      }
    }

    cc1020_rxlen++;
    if (cc1020_rxlen == HDRSIZE) {
      pktlen = ((struct cc1020_header *)cc1020_rxbuf)->length;
      if (pktlen == 0 || pktlen > sizeof (cc1020_rxbuf)) {
	cc1020_rxlen = 0;
	CC1020_SET_OPSTATE(CC1020_RX | CC1020_RX_SEARCHING);
      }
    } else if (cc1020_rxlen > HDRSIZE) {
      if (cc1020_rxlen == pktlen) {
        /* Disable interrupts while processing the packet. */
        DISABLE_RX_IRQ();
	CC1020_SET_OPSTATE(CC1020_RX | CC1020_RX_PROCESSING);
	_BIC_SR_IRQ(LPM3_bits);
        process_poll(&cc1020_receiver_process);
      }
    }
  } 
}

static void
cc1020_write_reg(uint8_t addr, uint8_t adata)
{
  unsigned i;
  unsigned char data;

  PSEL_OFF;
  data = addr << 1;
  PSEL_ON;

  // Send address bits 
  for (i = 0; i < 7; i++) {
    nop();
    PCLK_LOW;
    nop();
    if (data & 0x80)
      PDI_HIGH;
    else
      PDI_LOW;
    data = data << 1;
    PCLK_HIGH;
  }

  // Send read/write bit 
  // Ignore bit in data, always use 1 
  nop();
  PCLK_LOW;
  PDI_HIGH;
  nop();
  PCLK_HIGH;
  nop();
  PCLK_LOW;
  data = adata;

  // Send data bits 
  for (i = 0; i < 8; i++) {
    nop();
    PCLK_LOW;
    nop();
    if (data & 0x80)
      PDI_HIGH;
    else
      PDI_LOW;
    data = data << 1;
    PCLK_HIGH;
  }

  nop();
  PCLK_LOW;
  nop();
  PSEL_OFF;
}

static uint8_t
cc1020_read_reg(uint8_t addr)
{
  unsigned i;
  unsigned char data = 0;

  PSEL_OFF;
  data = addr << 1;
  PSEL_ON;
  nop();

  // Send address bits 
  for (i = 0; i < 7; i++) {
    nop();
    PCLK_LOW;
    nop();
    if (data & 0x80)
      PDI_HIGH;
    else
      PDI_LOW;
    data = data << 1;
    PCLK_HIGH;
  }

  // Send read/write bit 
  // Ignore bit in data, always use 0 
  nop();
  PCLK_LOW;
  PDI_LOW;
  nop();
  PCLK_HIGH;
  nop();
  PCLK_LOW;

  // Receive data bits       
  for (i = 0; i < 8; i++) {
    nop();
    PCLK_HIGH;
    nop();
    data = data << 1;
    if (PDO)
      data++;
    nop();
    PCLK_LOW;
  }

  nop();
  PSEL_OFF;

  return data;
}

static void
cc1020_load_config(const uint8_t * config)
{
  int i;

  for (i = 0; i < 0x28; i++)
    cc1020_write_reg(i, config[i]);
}

static void
cc1020_reset(void)
{
  // Reset CC1020
  cc1020_write_reg(CC1020_MAIN, 0x0FU & ~0x01U);

  // Bring CC1020 out of reset
  cc1020_write_reg(CC1020_MAIN, 0x1F);
}

static int
cc1020_calibrate(void)
{
  unsigned int timeout_cnt;

  // Turn off PA to avoid spurs during calibration in TX mode
  cc1020_write_reg(CC1020_PA_POWER, 0x00);

  // Start calibration
  cc1020_write_reg(CC1020_CALIBRATE, 0xB5);
  MS_DELAY(3);
  while ((cc1020_read_reg(CC1020_STATUS) & CAL_COMPLETE) == 0);
  MS_DELAY(2);

  // Monitor lock
  for (timeout_cnt = LOCK_TIMEOUT; timeout_cnt > 0; timeout_cnt--) {
    if (cc1020_read_reg(CC1020_STATUS) & LOCK_CONTINUOUS)
    	break;
  }

  // Restore PA_POWER
  cc1020_write_reg(CC1020_PA_POWER, cc1020_pa_power);

  // Return state of LOCK_CONTINUOUS bit
  return (cc1020_read_reg(CC1020_STATUS) & LOCK_CONTINUOUS) == LOCK_CONTINUOUS;
}

static int
cc1020_lock(void)
{
  char lock_status;
  int i;

  // Monitor LOCK, lasts 420 - 510 cycles @ 4505600 = 93 us - 113 us
  for (i = LOCK_TIMEOUT; i > 0; i--) {
    lock_status = cc1020_read_reg(CC1020_STATUS) & LOCK_CONTINUOUS;
    if (lock_status)
      break;
  }

  if (lock_status == LOCK_CONTINUOUS) {
    return LOCK_OK;
  }

  return cc1020_calibrate() ? LOCK_RECAL_OK : LOCK_NOK;
}

static int
cc1020_setupRX(int analog)
{
  char lock_status;

  // Switch into RX, switch to freq. reg A
  cc1020_write_reg(CC1020_MAIN, 0x11);

  // Setup bias current adjustment
  cc1020_write_reg(CC1020_ANALOG, analog);
  MS_DELAY(1);
  lock_status = cc1020_lock();

  // Switch RX part of CC1020 on
  cc1020_write_reg(CC1020_MAIN, 0x01);
  cc1020_write_reg(CC1020_INTERFACE, 0x02);

  // Return LOCK status to application
  return lock_status;
}

static int
cc1020_setupTX(int analog)
{
  char lock_status;

  // Setup bias current adjustment
  cc1020_write_reg(CC1020_ANALOG, analog);

  // Switch into TX, switch to freq. reg B
  cc1020_write_reg(CC1020_MAIN, 0xC1);
  MS_DELAY(1);
  lock_status = cc1020_lock();

  // Restore PA_POWER
  cc1020_write_reg(CC1020_PA_POWER, cc1020_pa_power);

  // Turn OFF DCLK squelch in TX
  cc1020_write_reg(CC1020_INTERFACE, 0x01);

  // Return LOCK status to application
  return lock_status;
}

static void
cc1020_setupPD(void)
{
  /*
   *  Power down components an reset all registers except MAIN
   *  to their default values.
   */
  cc1020_write_reg(CC1020_MAIN,
        RESET_N | BIAS_PD | FS_PD | XOSC_PD | PD_MODE_1);

  /* Turn off the power amplifier. */
  cc1020_write_reg(CC1020_PA_POWER, 0x00);

  cc1020_write_reg(CC1020_POWERDOWN, 0x1F);
}

static void
cc1020_wakeupRX(int analog)
{
  // Turn on crystal oscillator core.
  cc1020_write_reg(CC1020_MAIN, 0x1B);

  // Setup bias current adjustment.
  cc1020_write_reg(CC1020_ANALOG, analog);

  /*
   * Wait for the crystal oscillator to stabilize.
   * This typically takes 2-5 ms.
   */
  MS_DELAY(5);

  // Turn on bias generator.
  cc1020_write_reg(CC1020_MAIN, 0x19);

  // Turn on frequency synthesizer.
  cc1020_write_reg(CC1020_MAIN, 0x11);
}

static void
cc1020_wakeupTX(int analog)
{
  // Turn on crystal oscillator core.
  cc1020_write_reg(CC1020_MAIN, 0xDB);

  // Setup bias current adjustment.
  cc1020_write_reg(CC1020_ANALOG, analog);

  /*
   * Wait for the crystal oscillator to stabilize.
   * This typically takes 2-5 ms.
   */
  MS_DELAY(5);

  // Turn on bias generator.
  cc1020_write_reg(CC1020_MAIN, 0xD9);

  // Turn on frequency synthesizer.
  MS_DELAY(1);
  cc1020_write_reg(CC1020_MAIN, 0xD1);
}
