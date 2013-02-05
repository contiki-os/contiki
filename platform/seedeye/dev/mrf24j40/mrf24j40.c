/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
 *
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
 */

/**
 * \addtogroup mrf24j40 MRF24J40 Driver
 *
 * @{
 */

/**
 * \file   mrf24j40.c
 * 
 * \brief  MRF24J40 Driver
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#include "contiki.h"

#include "mrf24j40.h"
#include "mrf24j40_arch.h"

#include <pic32_spi.h>
#include <pic32_irq.h>

#include "net/packetbuf.h"
#include "net/netstack.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
PROCESS(mrf24j40_process, "MRF24J40 driver");
/*---------------------------------------------------------------------------*/

static volatile uint8_t mrf24j40_last_lqi;
static volatile uint8_t mrf24j40_last_rssi;
static volatile uint8_t status_tx;
static volatile uint8_t pending;
static volatile uint8_t receive_on;

/*---------------------------------------------------------------------------*/
static void
set_short_add_mem(uint8_t addr, uint8_t val)
{
  const uint8_t tmp = MRF24J40_INTERRUPT_ENABLE_STAT();
  uint8_t msg[2];

  msg[0] = (addr << 1) | 0x01;
  msg[1] = val;

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_CLR();
  }

  MRF24J40_CSn_LOW();
  MRF24J40_SPI_PORT_WRITE(msg, 2);
  MRF24J40_CSn_HIGH();

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_SET();
  }
}
/*---------------------------------------------------------------------------*/
static void
set_long_add_mem(uint16_t addr, uint8_t val)
{
  const uint8_t tmp = MRF24J40_INTERRUPT_ENABLE_STAT();
  uint8_t msg[3];

  msg[0] = (((uint8_t)(addr >> 3)) & 0x7F) | 0x80;
  msg[1] = (((uint8_t)(addr << 5)) & 0xE0) | 0x10;
  msg[2] = val;

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_CLR();
  }

  MRF24J40_CSn_LOW();
  MRF24J40_SPI_PORT_WRITE(msg, 3);
  MRF24J40_CSn_HIGH();

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_SET();
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t
get_short_add_mem(uint8_t addr)
{
  const uint8_t tmp = MRF24J40_INTERRUPT_ENABLE_STAT();
  uint8_t ret_val;
  
  addr <<= 1;

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_CLR();
  }

  MRF24J40_CSn_LOW();
  MRF24J40_SPI_PORT_WRITE(&addr, 1);
  MRF24J40_SPI_PORT_READ(&ret_val, 1);
  MRF24J40_CSn_HIGH();

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_SET();
  }

  return ret_val;
}
/*---------------------------------------------------------------------------*/
static uint8_t
get_long_add_mem(uint16_t addr)
{
  const uint8_t tmp = MRF24J40_INTERRUPT_ENABLE_STAT();
  uint8_t ret_val;
  uint8_t msg[2];

  msg[0] = (((uint8_t)(addr >> 3)) & 0x7F) | 0x80;
  msg[1] = ((uint8_t)(addr << 5)) & 0xE0;

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_CLR();
  }

  MRF24J40_CSn_LOW();
  MRF24J40_SPI_PORT_WRITE(msg, 2);
  MRF24J40_SPI_PORT_READ(&ret_val, 1);
  MRF24J40_CSn_HIGH();

  if(tmp) {
    MRF24J40_INTERRUPT_ENABLE_SET();
  }

  return ret_val;
}
/*---------------------------------------------------------------------------*/
static void
reset_rf_state_machine(void)
{
  /*
   * Reset RF state machine
   */

  const uint8_t rfctl = get_short_add_mem(MRF24J40_RFCTL);

  set_short_add_mem(MRF24J40_RFCTL, rfctl | 0b00000100);
  set_short_add_mem(MRF24J40_RFCTL, rfctl & 0b11111011);
  
  clock_delay_usec(2500);
}
/*---------------------------------------------------------------------------*/
void
flush_rx_fifo(void)
{
  set_short_add_mem(MRF24J40_RXFLUSH, get_short_add_mem(MRF24J40_RXFLUSH) | 0b00000001);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the channel
 *
 *        This routine sets the rx/tx channel
 */
void
mrf24j40_set_channel(uint16_t ch)
{
  set_long_add_mem(MRF24J40_RFCON0, ((ch - 11) << 4) | 0b00000011);

  reset_rf_state_machine();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Store MAC PAN ID
 *
 *        This routine sets the MAC PAN ID in the MRF24J40.
 */
void
mrf24j40_set_panid(uint16_t id)
{
  set_short_add_mem(MRF24J40_PANIDL, (uint8_t)id);
  set_short_add_mem(MRF24J40_PANIDH, (uint8_t)(id >> 8));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Store short MAC address
 *
 *        This routine sets the short MAC address in the MRF24J40.
 */
void
mrf24j40_set_short_mac_addr(uint16_t addr)
{
  set_short_add_mem(MRF24J40_SADRL, (uint8_t)addr);
  set_short_add_mem(MRF24J40_SADRH, (uint8_t)(addr >> 8));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Store extended MAC address
 *
 *        This routine sets the extended MAC address in the MRF24J40.
 */
void
mrf24j40_set_extended_mac_addr(uint64_t addr)
{
  set_short_add_mem(MRF24J40_EADR7, (uint8_t)addr);
  set_short_add_mem(MRF24J40_EADR6, (uint8_t)(addr >> 8));
  set_short_add_mem(MRF24J40_EADR5, (uint8_t)(addr >> 16));
  set_short_add_mem(MRF24J40_EADR4, (uint8_t)(addr >> 24));
  set_short_add_mem(MRF24J40_EADR3, (uint8_t)(addr >> 32));
  set_short_add_mem(MRF24J40_EADR2, (uint8_t)(addr >> 40));
  set_short_add_mem(MRF24J40_EADR1, (uint8_t)(addr >> 48));
  set_short_add_mem(MRF24J40_EADR0, (uint8_t)(addr >> 56));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get short MAC address
 *
 *        This routine gets the short MAC address stored in the MRF24J40.
 */
void
mrf24j40_get_short_mac_addr(uint16_t *addr)
{
  *(((uint8_t *)& addr)) = get_short_add_mem(MRF24J40_SADRH);
  *(((uint8_t *)& addr) + 1) = get_short_add_mem(MRF24J40_SADRL);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Gets extended MAC address
 *
 *        This routine gets the extended MAC address stored in the MRF24J40.
 */
void
mrf24j40_get_extended_mac_addr(uint64_t *addr)
{
  *(((uint8_t *)& addr)) = get_short_add_mem(MRF24J40_EADR7);
  *(((uint8_t *)& addr) + 1) = get_short_add_mem(MRF24J40_EADR6);
  *(((uint8_t *)& addr) + 2) = get_short_add_mem(MRF24J40_EADR5);
  *(((uint8_t *)& addr) + 3) = get_short_add_mem(MRF24J40_EADR4);
  *(((uint8_t *)& addr) + 4) = get_short_add_mem(MRF24J40_EADR3);
  *(((uint8_t *)& addr) + 5) = get_short_add_mem(MRF24J40_EADR2);
  *(((uint8_t *)& addr) + 6) = get_short_add_mem(MRF24J40_EADR1);
  *(((uint8_t *)& addr) + 7) = get_short_add_mem(MRF24J40_EADR0);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set TX power
 *
 *        This routine sets the transmission power of the MRF24J40.
 */
void
mrf24j40_set_tx_power(uint8_t pwr)
{
  set_long_add_mem(MRF24J40_RFCON3, pwr);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get radio status
 *
 *        This routine returns the MRF24J40 status.
 */
uint8_t
mrf24j40_get_status(void)
{
  return get_long_add_mem(MRF24J40_RFSTATE);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Get the RSSI
 *
 *        This routine returns the rssi value mesured in dbm
 *        Note: to convert the returned value to dBm, use the table 3-8 available
 *        in the MRF24J40 datasheet.  
 */
uint8_t
mrf24j40_get_rssi(void)
{
  /*
   * 3.6.1 RSSI FIRMWARE REQUEST (RSSI MODE1)
   * In this mode, the host microcontroller sends a request
   * to calculate RSSI, then waits until it is done and then
   * reads the RSSI value. The steps are:
   *
   *  1.
   *    Set RSSIMODE1 0x3E<7> – Initiate RSSI
   *    calculation.
   * 
   *  2.
   *    Wait until RSSIRDY 0x3E<0> is set to ‘1’ – RSSI
   *    calculation is complete.
   * 
   *  3.
   *    Read RSSI 0x210<7:0> – The RSSI register
   *    contains the averaged RSSI received power
   *    level for 8 symbol periods.
   */

  /* Initiate RSSI calculation */
  set_short_add_mem(MRF24J40_BBREG6, get_short_add_mem(MRF24J40_BBREG6) | 0b10000000);

  /* Wait until RSSI calculation is done */
  while(!(get_short_add_mem(MRF24J40_BBREG6) & 0b00000001)) {
    ;
  }

  mrf24j40_last_rssi = get_long_add_mem(MRF24J40_RSSI);

  return mrf24j40_last_rssi;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get the last read RSSI
 *
 *        This routine returns the last rssi value mesured in dbm
 *        Note: to convert the returned value to dBm, use the table 3-8 available
 *        in the MRF24J40 datasheet.
 */
uint8_t
mrf24j40_get_last_rssi(void)
{
  return mrf24j40_last_rssi;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get the last read LQI
 *
 *        This routine returns the last lqi
 */
uint8_t
mrf24j40_get_last_lqi(void)
{
  return mrf24j40_last_lqi;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Store message
 *
 *        This routine stores a buffer of buf_len bytes in the TX_FIFO
 *        buffer of the MRF24J40.
 */
int32_t
mrf24j40_set_txfifo(const uint8_t *buf, uint8_t buf_len)
{
  uint8_t i;

  if((buf_len == 0) || (buf_len > 128)) {
    return -1;
  }

  set_long_add_mem(MRF24J40_NORMAL_TX_FIFO, 0);

  set_long_add_mem(MRF24J40_NORMAL_TX_FIFO + 1, buf_len);

  for(i = 0; i < buf_len; ++i) {
    set_long_add_mem(MRF24J40_NORMAL_TX_FIFO + 2 + i, buf[i]);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Get message
 *
 *        This routine is used to retrieve a message stored in the RX_FIFO
 */
int32_t
mrf24j40_get_rxfifo(uint8_t *buf, uint8_t buf_len)
{
  uint8_t i, len;

  MRF24J40_INTERRUPT_ENABLE_CLR();

  /* Disable packet reception */
  set_short_add_mem(MRF24J40_BBREG1, 0b00000100);

  /* Get packet length discarding 2 bytes (LQI, RSSI) */
  len = get_long_add_mem(MRF24J40_RX_FIFO) - 2;
  
  if(len <= buf_len) {
    /* Get the packet */
    for(i = 0; i < len; ++i) {
      buf[i] = get_long_add_mem(MRF24J40_RX_FIFO + i + 1);
    }

    /*
     * packet len includes = header + paylod + LQI + RSSI
     */
#ifdef ADD_RSSI_AND_LQI_TO_PACKET
    mrf24j40_last_lqi = get_long_add_mem(MRF24J40_RX_FIFO + len + 3);
    mrf24j40_last_rssi = get_long_add_mem(MRF24J40_RX_FIFO + len + 4);
#endif
  } else {
    len = 0;
  }

  /* Enable packet reception */
  set_short_add_mem(MRF24J40_BBREG1, 0b00000000);
  
  pending = 0;
  
#ifdef MRF24J40_PROMISCUOUS_MODE
  /*
   * Flush RX FIFO as suggested by the work around 1 in
   * MRF24J40 Silicon Errata.
   */
  flush_rx_fifo();
#endif
  
  MRF24J40_INTERRUPT_ENABLE_SET();

  return len == 0 ? -1 : len;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Start sleep
 *
 *        This routine puts the radio in sleep mode.
 */
static void
put_to_sleep(void)
{
  /* Prepare WAKE pin: */
  MRF24J40_WAKE = 0;

  /* Enable Immediate Wake-up mode */
  set_short_add_mem(MRF24J40_WAKECON, 0b10000000);

  set_short_add_mem(MRF24J40_SOFTRST, 0b00000100);

  set_short_add_mem(MRF24J40_RXFLUSH, 0b01100000);
    
  /* Put to sleep */
  set_short_add_mem(MRF24J40_SLPACK, 0b10000000);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Awake the radio
 *
 *        This routine turns on and sets the radio on receiving mode.
 *        Note: After performing this routine the radio is in the receiving state.
 */
static void
wake(void)
{
  /* Wake-up */
  MRF24J40_WAKE = 1;

  /* RF State Machine reset */
  set_short_add_mem(MRF24J40_RFCTL, 0b00000100);
  set_short_add_mem(MRF24J40_RFCTL, 0b00000000);
    
  clock_delay_usec(2500);
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_on(void)
{
  if(!receive_on) {
    wake();
  
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
    receive_on = 1;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_off(void)
{
  if(receive_on) {
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    receive_on = 0;
  
    put_to_sleep();
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Init transceiver
 *
 *        This routine initializes the radio transceiver
 */
int
mrf24j40_init(void)
{
  uint8_t i;
  
  /* Set the IO pins direction */
  MRF24J40_PINDIRECTION_INIT();
  
  /* Set interrupt registers and reset flags */
  MRF24J40_INTERRUPT_INIT(6, 3);

  if(MRF24J40_SPI_PORT_INIT(10000000, SPI_DEFAULT) < 0)
    return -1;

  PRINTF("MRF24J40 Initialization started\n");

  MRF24J40_HARDRESET_LOW();

  clock_delay_usec(2500);
  
  MRF24J40_HARDRESET_HIGH();
  
  clock_delay_usec(2500);

  /*
   * bit 7:3 reserved: Maintain as ‘0’
   * bit 2   RSTPWR: Power Management Reset bit
   *         1 = Reset power management circuitry (bit is automatically cleared to ‘0’ by hardware)
   * bit 1   RSTBB: Baseband Reset bit
   *         1 = Reset baseband circuitry (bit is automatically cleared to ‘0’ by hardware)
   * bit 0   RSTMAC: MAC Reset bit
   *         1 = Reset MAC circuitry (bit is automatically cleared to ‘0’ by hardware)
   */
  set_short_add_mem(MRF24J40_SOFTRST, 0b00000111);

  /*
   * wait until the radio reset is completed
   */
  do {
    i = get_short_add_mem(MRF24J40_SOFTRST);
  } while((i & 0b0000111) != 0);
  
  clock_delay_usec(2500);


  /*
   * bit 7   FIFOEN: FIFO Enable bit 1 = Enabled (default). Always maintain this bit as a ‘1’.
   * bit 6   reserved: Maintain as ‘0’
   * bit 5:2 TXONTS<3:0>: Transmitter Enable On Time Symbol bits(1)
   *         Transmitter on time before beginning of packet. Units: symbol period (16 μs).
   *         Minimum value: 0x1. Default value: 0x2 (2 * 16 μs = 32 μs). Recommended value: 0x6 (6 * 16 μs = 96 μs).
   * bit 1:0 TXONT<8:7>: Transmitter Enable On Time Tick bits(1)
   *         Transmitter on time before beginning of packet. TXONT is a 9-bit value. TXONT<6:0> bits are located
   *         in SYMTICKH<7:1>. Units: tick (50 ns). Default value = 0x028 (40 * 50 ns = 2 μs).
   */
  set_short_add_mem(MRF24J40_PACON2, 0b10011000);

  mrf24j40_set_channel(MRF24J40_DEFAULT_CHANNEL);

  set_long_add_mem(MRF24J40_RFCON1, 0b00000010);      /* program the RF and Baseband Register */
                                                      /* as suggested by the datasheet */

  set_long_add_mem(MRF24J40_RFCON2, 0b10000000);      /* enable PLL */

  mrf24j40_set_tx_power(0b00000000);                  /* set power 0dBm (plus 20db power amplifier 20dBm)*/

  /* 
   * Set up 
   * 
   * bit 7   '1' as suggested by the datasheet
   * bit 6:5 '00' reserved
   * bit 4   '1' recovery from sleep 1 usec
   * bit 3   '0' battery monitor disabled
   * bit 2:0 '000' reserved
   */
  set_long_add_mem(MRF24J40_RFCON6, 0b10010000);

  set_long_add_mem(MRF24J40_RFCON7, 0b10000000);      /* Sleep clock = 100kHz */
  set_long_add_mem(MRF24J40_RFCON8, 0b00000010);      /* as suggested by the datasheet */

  set_long_add_mem(MRF24J40_SLPCON1, 0b00100001);     /* as suggested by the datasheet */

  /* Program CCA, RSSI threshold values */
  set_short_add_mem(MRF24J40_BBREG2, 0b01111000);     /* Recommended value by the datashet */
  set_short_add_mem(MRF24J40_CCAEDTH, 0b01100000);    /* Recommended value by the datashet */

#ifdef MRF24J40MB
  /* Activate the external amplifier needed by the MRF24J40MB */
  set_long_add_mem(MRF24J40_TESTMODE, 0b0001111);
  PRINTF("MRF24J40 Init Amplifier activated \n");
#endif

#ifdef ADD_RSSI_AND_LQI_TO_PACKET
  /* Enable the packet RSSI */
  set_short_add_mem(MRF24J40_BBREG6, 0b01000000);
  PRINTF("MRF24J40 Init append RSSI and LQI to packet\n");
#endif

  /*
   * Wait until the radio state machine is not on rx mode
   */
  do {
    i = get_long_add_mem(MRF24J40_RFSTATE);
  } while((i & 0xA0) != 0xA0);

  i = 0;

#ifdef MRF24J40_DISABLE_AUTOMATIC_ACK
  i = i | 0b00100000;
  PRINTF("MRF24J40 Init NO_AUTO_ACK\n");
#endif

#ifdef MRF24J40_PAN_COORDINATOR
  i = i | 0b00001000;
  PRINTF("MRF24J40 Init PAN COORD\n");
  set_short_add_mem(MRF24J40_ORDER, 0b11111111);
#endif

#ifdef MRF24J40_COORDINATOR
  i = i | 0b00000100;
  PRINTF("MRF24J40 Init COORD\n");
#endif

#ifdef MRF24J40_ACCEPT_WRONG_CRC_PKT
  i = i | 0b00000010;
  PRINTF("MRF24J40 Init Accept Wrong CRC\n");
#endif

#ifdef MRF24J40_PROMISCUOUS_MODE
  i = i | 0b00000001;
  PRINTF("MRF24J40 Init PROMISCUOUS MODE\n");
#endif

  /*
   * Set the RXMCR register.
   * Default setting i = 0x00, which means:
   * - Automatic ACK;
   * - Device is not a PAN coordinator;
   * - Device is not a coordinator;
   * - Accept only packets with good CRC
   * - Discard packet when there is a MAC address mismatch,
   *   illegal frame type, dPAN/sPAN or MAC short address mismatch.
   */
  set_short_add_mem(MRF24J40_RXMCR, i);
  PRINTF("RXMCR 0x%X\n", i);

  /*
   * Set the TXMCR register.
   * bit 7   '0' Enable No Carrier Sense Multiple Access (CSMA) Algorithm.
   * bit 6   '0' Disable Battery Life Extension Mode bit.
   * bit 5   '0' Disable Slotted CSMA-CA Mode bit.
   * bit 4:3 '11' MAC Minimum Backoff Exponent bits (macMinBE).
   * bit 2:0 '100' CSMA Backoff bits (macMaxCSMABackoff)
   */
  set_short_add_mem(MRF24J40_TXMCR, 0b00011100);

  i = get_short_add_mem(MRF24J40_TXMCR);
  PRINTF("TXMCR 0x%X\n", i);
  
	/*
	 * Set TX turn around time as defined by IEEE802.15.4 standard
	 */
  set_short_add_mem(MRF24J40_TXSTBL, 0b10010101);
  set_short_add_mem(MRF24J40_TXTIME, 0b00110000);

#ifdef INT_POLARITY_HIGH
  /* Set interrupt edge polarity high */
  set_long_add_mem(MRF24J40_SLPCON0, 0b00000011);
  PRINTF("MRF24J40 Init INT Polarity High\n");
#else
  set_long_add_mem(MRF24J40_SLPCON0, 0b00000001);
  PRINTF("MRF24J40 Init INT Polarity Low\n");
#endif

  PRINTF("MRF24J40 Inititialization completed\n");
  
  mrf24j40_last_lqi = 0;
  mrf24j40_last_rssi = 0;
  status_tx = MRF24J40_TX_ERR_NONE;
  pending = 0;

  receive_on = 1;
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  
  reset_rf_state_machine();

  /* Flush RX FIFO */
  flush_rx_fifo();

  process_start(&mrf24j40_process, NULL);

  /*
   *
   * Setup interrupts.
   *
   * set INTCON
   * bit 7 '1' Disables the sleep alert interrupt
   * bit 6 '1' Disables the wake-up alert interrupt
   * bit 5 '1' Disables the half symbol timer interrupt
   * bit 4 '1' Disables the security key request interrupt
   * bit 3 '0' Enables the RX FIFO reception interrupt
   * bit 2 '1' Disables the TX GTS2 FIFO transmission interrupt
   * bit 1 '1' Disables the TX GTS1 FIFO transmission interrupt
   * bit 0 '0' Enables the TX Normal FIFO transmission interrupt
   */
  set_short_add_mem(MRF24J40_INTCON, 0b11110110);
  
  return 0;
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_prepare(const void *data, unsigned short len)
{
  PRINTF("PREPARE %u bytes\n", len);

  uint8_t receive_was_on = receive_on;

  mrf24j40_on();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  mrf24j40_set_txfifo(data, len);

  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  
  if(!receive_was_on) {
    mrf24j40_off();
  } else {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_transmit(unsigned short len)
{
  PRINTF("TRANSMIT %u bytes\n", len);

  uint8_t receive_was_on = receive_on;

  mrf24j40_on();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
  
  status_tx = MRF24J40_TX_WAIT;

  set_short_add_mem(MRF24J40_TXNCON, 0b00000001);

  /* Wait until the transmission has finished. */
  while(status_tx == MRF24J40_TX_WAIT) {
    ;
  }
  
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);

  if(!receive_was_on) {
    mrf24j40_off();
  } else {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }
  
  switch(status_tx) {
  case MRF24J40_TX_ERR_NONE:
    return RADIO_TX_OK;
  case MRF24J40_TX_ERR_COLLISION:
    return RADIO_TX_COLLISION;
  case MRF24J40_TX_ERR_MAXRETRY:
    return RADIO_TX_NOACK;
  default:
    return RADIO_TX_ERR;
  }
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_write(const void *data, uint16_t len)
{
  int ret = -1;
  
  PRINTF("PREPARE & TRANSMIT %u bytes\n", len);

  if(mrf24j40_prepare(data, len))
    return ret;

  ret = mrf24j40_transmit(len);
  
  return ret;
}

/*---------------------------------------------------------------------------*/
int
mrf24j40_read(void *data, uint16_t len)
{
  return mrf24j40_get_rxfifo(data, len);
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_cca(void)
{
  uint8_t ret;

  uint8_t receive_was_on = receive_on;

  mrf24j40_on();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
  
  ret = mrf24j40_get_rssi() <= 95; /* -69dbm */
  
  if(!receive_was_on) {
    mrf24j40_off();
  } else {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_receiving_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int
mrf24j40_pending_packet(void)
{
  return pending;
}
/*---------------------------------------------------------------------------*/
MRF24J40_ISR()
{
  INT_status int_status;
  TX_status tx_status;
  
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  int_status.val = get_short_add_mem(MRF24J40_INTSTAT);

  if(!int_status.val) {
    return;
  }

  if(int_status.bits.RXIF) {
  
    pending = 1;
    
    process_poll(&mrf24j40_process);

  }
  
  if(int_status.bits.TXNIF) {

    tx_status.val = get_short_add_mem(MRF24J40_TXSTAT);

    if(tx_status.bits.TXNSTAT) {
      if(tx_status.bits.CCAFAIL) {
        status_tx = MRF24J40_TX_ERR_COLLISION;
      } else {
        status_tx = MRF24J40_TX_ERR_MAXRETRY;
      }
    } else {
      status_tx = MRF24J40_TX_ERR_NONE;
    }
  }
  
  MRF24J40_INTERRUPT_FLAG_CLR();
  
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mrf24j40_process, ev, data)
{
  PROCESS_BEGIN();
  
  uint8_t ret;

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    
    if(!pending) {
      continue;
    }
    
    packetbuf_clear();
    
    ret = mrf24j40_read(packetbuf_dataptr(), PACKETBUF_SIZE);
    
    packetbuf_set_datalen(ret);
    
#ifdef ADD_RSSI_AND_LQI_TO_PACKET
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, mrf24j40_last_rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, mrf24j40_last_lqi);
#endif

    NETSTACK_RDC.input();
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
const struct radio_driver mrf24j40_driver = {
  mrf24j40_init,
  mrf24j40_prepare,
  mrf24j40_transmit,
  mrf24j40_write,
  mrf24j40_read,
  mrf24j40_cca,
  mrf24j40_receiving_packet,
  mrf24j40_pending_packet,
  mrf24j40_on,
  mrf24j40_off
};
/*---------------------------------------------------------------------------*/

/** @} */
