/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-rf
 * @{
 *
 * \file
 * Implementation of the cc2538 RF driver
 */
#include "contiki.h"
#include "dev/radio.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/rime/rimeaddr.h"
#include "net/netstack.h"
#include "sys/energest.h"
#include "dev/cc2538-rf.h"
#include "dev/rfcore.h"
#include "dev/sys-ctrl.h"
#include "dev/udma.h"
#include "reg.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
#define CHECKSUM_LEN 2

/* uDMA channel control persistent flags */
#define UDMA_TX_FLAGS (UDMA_CHCTL_ARBSIZE_128 | UDMA_CHCTL_XFERMODE_AUTO \
    | UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_DSTSIZE_8 \
    | UDMA_CHCTL_SRCINC_8 | UDMA_CHCTL_DSTINC_NONE)

#define UDMA_RX_FLAGS (UDMA_CHCTL_ARBSIZE_128 | UDMA_CHCTL_XFERMODE_AUTO \
    | UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_DSTSIZE_8 \
    | UDMA_CHCTL_SRCINC_NONE | UDMA_CHCTL_DSTINC_8)

/*
 * uDMA transfer threshold. DMA will only be used to read an incoming frame
 * if its size is above this threshold
 */
#define UDMA_RX_SIZE_THRESHOLD 3
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/* Local RF Flags */
#define RX_ACTIVE     0x80
#define RF_MUST_RESET 0x40
#define WAS_OFF       0x10
#define RF_ON         0x01

/* Bit Masks for the last byte in the RX FIFO */
#define CRC_BIT_MASK 0x80
#define LQI_BIT_MASK 0x7F
/* RSSI Offset */
#define RSSI_OFFSET    73

/* 192 usec off -> on interval (RX Callib -> SFD Wait). We wait a bit more */
#define ONOFF_TIME                    RTIMER_ARCH_SECOND / 3125
/*---------------------------------------------------------------------------*/
/* Sniffer configuration */
#ifndef CC2538_RF_CONF_SNIFFER_USB
#define CC2538_RF_CONF_SNIFFER_USB 0
#endif

#if CC2538_RF_CONF_SNIFFER
static const uint8_t magic[] = { 0x53, 0x6E, 0x69, 0x66 };      /** Snif */

#if CC2538_RF_CONF_SNIFFER_USB
#include "usb/usb-serial.h"
#define write_byte(b) usb_serial_writeb(b)
#define flush()       usb_serial_flush()
#else
#include "dev/uart.h"
#define write_byte(b) uart_write_byte(b)
#define flush()
#endif

#else /* CC2538_RF_CONF_SNIFFER */
#define write_byte(b)
#define flush()
#endif /* CC2538_RF_CONF_SNIFFER */
/*---------------------------------------------------------------------------*/
#ifdef CC2538_RF_CONF_AUTOACK
#define CC2538_RF_AUTOACK CC2538_RF_CONF_AUTOACK
#else
#define CC2538_RF_AUTOACK 1
#endif
/*---------------------------------------------------------------------------*/
static uint8_t rf_flags;

static int on(void);
static int off(void);
/*---------------------------------------------------------------------------*/
PROCESS(cc2538_rf_process, "cc2538 RF driver");
/*---------------------------------------------------------------------------*/
uint8_t
cc2538_rf_channel_get()
{
  uint8_t chan = REG(RFCORE_XREG_FREQCTRL) & RFCORE_XREG_FREQCTRL_FREQ;

  return ((chan - CC2538_RF_CHANNEL_MIN) / CC2538_RF_CHANNEL_SPACING
          + CC2538_RF_CHANNEL_MIN);
}
/*---------------------------------------------------------------------------*/
int8_t
cc2538_rf_channel_set(uint8_t channel)
{
  PRINTF("RF: Set Channel\n");

  if((channel < CC2538_RF_CHANNEL_MIN) || (channel > CC2538_RF_CHANNEL_MAX)) {
    return -1;
  }

  /* Changes to FREQCTRL take effect after the next recalibration */
  off();
  REG(RFCORE_XREG_FREQCTRL) = (CC2538_RF_CHANNEL_MIN
      + (channel - CC2538_RF_CHANNEL_MIN) * CC2538_RF_CHANNEL_SPACING);
  on();

  return (int8_t) channel;
}
/*---------------------------------------------------------------------------*/
uint8_t
cc2538_rf_power_set(uint8_t new_power)
{
  PRINTF("RF: Set Power\n");

  REG(RFCORE_XREG_TXPOWER) = new_power;

  return (REG(RFCORE_XREG_TXPOWER) & 0xFF);
}
/*---------------------------------------------------------------------------*/
/* ToDo: Check once we have info on the... infopage */
void
cc2538_rf_set_addr(uint16_t pan)
{
#if RIMEADDR_SIZE==8
  /* EXT_ADDR[7:0] is ignored when using short addresses */
  int i;

  for(i = (RIMEADDR_SIZE - 1); i >= 0; --i) {
    ((uint32_t *)RFCORE_FFSM_EXT_ADDR0)[i] =
        rimeaddr_node_addr.u8[RIMEADDR_SIZE - 1 - i];
  }
#endif

  REG(RFCORE_FFSM_PAN_ID0) = pan & 0xFF;
  REG(RFCORE_FFSM_PAN_ID1) = pan >> 8;

  REG(RFCORE_FFSM_SHORT_ADDR0) = rimeaddr_node_addr.u8[RIMEADDR_SIZE - 1];
  REG(RFCORE_FFSM_SHORT_ADDR1) = rimeaddr_node_addr.u8[RIMEADDR_SIZE - 2];
}
/*---------------------------------------------------------------------------*/
/* Netstack API radio driver functions */
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  int cca;

  PRINTF("RF: CCA\n");

  /* If we are off, turn on first */
  if((REG(RFCORE_XREG_FSMSTAT0) & RFCORE_XREG_FSMSTAT0_FSM_FFCTRL_STATE) == 0) {
    rf_flags |= WAS_OFF;
    on();
  }

  /* Wait on RSSI_VALID */
  while((REG(RFCORE_XREG_RSSISTAT) & RFCORE_XREG_RSSISTAT_RSSI_VALID) == 0);

  if(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_CCA) {
    cca = CC2538_RF_CCA_CLEAR;
  } else {
    cca = CC2538_RF_CCA_BUSY;
  }

  /* If we were off, turn back off */
  if((rf_flags & WAS_OFF) == WAS_OFF) {
    rf_flags &= ~WAS_OFF;
    off();
  }

  return cca;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  PRINTF("RF: On\n");

  if(!(rf_flags & RX_ACTIVE)) {
    CC2538_RF_CSP_ISFLUSHRX();
    CC2538_RF_CSP_ISRXON();

    rf_flags |= RX_ACTIVE;
  }

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  PRINTF("RF: Off\n");

  /* Wait for ongoing TX to complete (e.g. this could be an outgoing ACK) */
  while(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE);

  CC2538_RF_CSP_ISFLUSHRX();

  /* Don't turn off if we are off as this will trigger a Strobe Error */
  if(REG(RFCORE_XREG_RXENABLE) != 0) {
    CC2538_RF_CSP_ISRFOFF();
  }

  rf_flags &= ~RX_ACTIVE;

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  PRINTF("RF: Init\n");

  if(rf_flags & RF_ON) {
    return 0;
  }

  /* Enable clock for the RF Core while Running, in Sleep and Deep Sleep */
  REG(SYS_CTRL_RCGCRFC) = 1;
  REG(SYS_CTRL_SCGCRFC) = 1;
  REG(SYS_CTRL_DCGCRFC) = 1;

  REG(RFCORE_XREG_CCACTRL0) = CC2538_RF_CCA_THRES_USER_GUIDE;

  /*
   * Changes from default values
   * See User Guide, section "Register Settings Update"
   */
  REG(RFCORE_XREG_TXFILTCFG) = 0x09;    /** TX anti-aliasing filter bandwidth */
  REG(RFCORE_XREG_AGCCTRL1) = 0x15;     /** AGC target value */
  REG(ANA_REGS_IVCTRL) = 0x0B;          /** Bias currents */

  /*
   * Defaults:
   * Auto CRC; Append RSSI, CRC-OK and Corr. Val.; CRC calculation;
   * RX and TX modes with FIFOs
   */
  REG(RFCORE_XREG_FRMCTRL0) = RFCORE_XREG_FRMCTRL0_AUTOCRC;

#if CC2538_RF_AUTOACK
  REG(RFCORE_XREG_FRMCTRL0) |= RFCORE_XREG_FRMCTRL0_AUTOACK;
#endif

  /* If we are a sniffer, turn off frame filtering */
#if CC2538_RF_CONF_SNIFFER
  REG(RFCORE_XREG_FRMFILT0) &= ~RFCORE_XREG_FRMFILT0_FRAME_FILTER_EN;
#endif

  /* Disable source address matching and autopend */
  REG(RFCORE_XREG_SRCMATCH) = 0;

  /* MAX FIFOP threshold */
  REG(RFCORE_XREG_FIFOPCTRL) = CC2538_RF_MAX_PACKET_LEN;

  cc2538_rf_power_set(CC2538_RF_TX_POWER);
  cc2538_rf_channel_set(CC2538_RF_CHANNEL);

  /* Acknowledge RF interrupts, FIFOP only */
  REG(RFCORE_XREG_RFIRQM0) |= RFCORE_XREG_RFIRQM0_FIFOP;
  nvic_interrupt_enable(NVIC_INT_RF_RXTX);

  /* Acknowledge all RF Error interrupts */
  REG(RFCORE_XREG_RFERRM) = RFCORE_XREG_RFERRM_RFERRM;
  nvic_interrupt_enable(NVIC_INT_RF_ERR);

  if(CC2538_RF_CONF_TX_USE_DMA) {
    /* Disable peripheral triggers for the channel */
    udma_channel_mask_set(CC2538_RF_CONF_TX_DMA_CHAN);

    /*
     * Set the channel's DST. SRC can not be set yet since it will change for
     * each transfer
     */
    udma_set_channel_dst(CC2538_RF_CONF_TX_DMA_CHAN, RFCORE_SFR_RFDATA);
  }

  if(CC2538_RF_CONF_RX_USE_DMA) {
    /* Disable peripheral triggers for the channel */
    udma_channel_mask_set(CC2538_RF_CONF_RX_DMA_CHAN);

    /*
     * Set the channel's SRC. DST can not be set yet since it will change for
     * each transfer
     */
    udma_set_channel_src(CC2538_RF_CONF_RX_DMA_CHAN, RFCORE_SFR_RFDATA);
  }

  process_start(&cc2538_rf_process, NULL);

  rf_flags |= RF_ON;

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  uint8_t i;

  PRINTF("RF: Prepare 0x%02x bytes\n", payload_len + CHECKSUM_LEN);

  /*
   * When we transmit in very quick bursts, make sure previous transmission
   * is not still in progress before re-writing to the TX FIFO
   */
  while(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE);

  if((rf_flags & RX_ACTIVE) == 0) {
    on();
  }

  CC2538_RF_CSP_ISFLUSHTX();

  PRINTF("RF: data = ");
  /* Send the phy length byte first */
  REG(RFCORE_SFR_RFDATA) = payload_len + CHECKSUM_LEN;

  if(CC2538_RF_CONF_TX_USE_DMA) {
    PRINTF("<uDMA payload>");

    /* Set the transfer source's end address */
    udma_set_channel_src(CC2538_RF_CONF_TX_DMA_CHAN,
                         (uint32_t)(payload) + payload_len - 1);

    /* Configure the control word */
    udma_set_channel_control_word(CC2538_RF_CONF_TX_DMA_CHAN,
                                  UDMA_TX_FLAGS | udma_xfer_size(payload_len));

    /* Enabled the RF TX uDMA channel */
    udma_channel_enable(CC2538_RF_CONF_TX_DMA_CHAN);

    /* Trigger the uDMA transfer */
    udma_channel_sw_request(CC2538_RF_CONF_TX_DMA_CHAN);

    /*
     * No need to wait for this to end. Even if transmit() gets called
     * immediately, the uDMA controller will stream the frame to the TX FIFO
     * faster than transmit() can empty it
     */
  } else {
    for(i = 0; i < payload_len; i++) {
      REG(RFCORE_SFR_RFDATA) = ((unsigned char *)(payload))[i];
      PRINTF("%02x", ((unsigned char *)(payload))[i]);
    }
  }
  PRINTF("\n");

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  uint8_t counter;
  int ret = RADIO_TX_ERR;
  rtimer_clock_t t0;

  PRINTF("RF: Transmit\n");

  if(!(rf_flags & RX_ACTIVE)) {
    t0 = RTIMER_NOW();
    on();
    rf_flags |= WAS_OFF;
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + ONOFF_TIME));
  }

  if(channel_clear() == CC2538_RF_CCA_BUSY) {
    RIMESTATS_ADD(contentiondrop);
    return RADIO_TX_COLLISION;
  }

  /*
   * prepare() double checked that TX_ACTIVE is low. If SFD is high we are
   * receiving. Abort transmission and bail out with RADIO_TX_COLLISION
   */
  if(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_SFD) {
    RIMESTATS_ADD(contentiondrop);
    return RADIO_TX_COLLISION;
  }

  /* Start the transmission */
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  CC2538_RF_CSP_ISTXON();

  counter = 0;
  while(!((REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE))
        && (counter++ < 3)) {
    clock_delay_usec(6);
  }

  if(!(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE)) {
    PRINTF("RF: TX never active.\n");
    CC2538_RF_CSP_ISFLUSHTX();
    ret = RADIO_TX_ERR;
  } else {
    /* Wait for the transmission to finish */
    while(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_TX_ACTIVE);
    ret = RADIO_TX_OK;
  }
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  if(rf_flags & WAS_OFF) {
    rf_flags &= ~WAS_OFF;
    off();
  }

  RIMESTATS_ADD(lltx);

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
read(void *buf, unsigned short bufsize)
{
  uint8_t i;
  uint8_t len;
  uint8_t crc_corr;
  int8_t rssi;

  PRINTF("RF: Read\n");

  if((REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFOP) == 0) {
    return 0;
  }

  /* Check the length */
  len = REG(RFCORE_SFR_RFDATA);

  /* Check for validity */
  if(len > CC2538_RF_MAX_PACKET_LEN) {
    /* Oops, we must be out of sync. */
    PRINTF("RF: bad sync\n");

    RIMESTATS_ADD(badsynch);
    CC2538_RF_CSP_ISFLUSHRX();
    return 0;
  }

  if(len <= CC2538_RF_MIN_PACKET_LEN) {
    PRINTF("RF: too short\n");

    RIMESTATS_ADD(tooshort);
    CC2538_RF_CSP_ISFLUSHRX();
    return 0;
  }

  if(len - CHECKSUM_LEN > bufsize) {
    PRINTF("RF: too long\n");

    RIMESTATS_ADD(toolong);
    CC2538_RF_CSP_ISFLUSHRX();
    return 0;
  }

  /* If we reach here, chances are the FIFO is holding a valid frame */
  PRINTF("RF: read (0x%02x bytes) = ", len);
  len -= CHECKSUM_LEN;

  /* Don't bother with uDMA for short frames (e.g. ACKs) */
  if(CC2538_RF_CONF_RX_USE_DMA && len > UDMA_RX_SIZE_THRESHOLD) {
    PRINTF("<uDMA payload>");

    /* Set the transfer destination's end address */
    udma_set_channel_dst(CC2538_RF_CONF_RX_DMA_CHAN,
                         (uint32_t)(buf) + len - 1);

    /* Configure the control word */
    udma_set_channel_control_word(CC2538_RF_CONF_RX_DMA_CHAN,
                                  UDMA_RX_FLAGS | udma_xfer_size(len));

    /* Enabled the RF RX uDMA channel */
    udma_channel_enable(CC2538_RF_CONF_RX_DMA_CHAN);

    /* Trigger the uDMA transfer */
    udma_channel_sw_request(CC2538_RF_CONF_RX_DMA_CHAN);

    /* Wait for the transfer to complete. */
    while(udma_channel_get_mode(CC2538_RF_CONF_RX_DMA_CHAN));
  } else {
    for(i = 0; i < len; ++i) {
      ((unsigned char *)(buf))[i] = REG(RFCORE_SFR_RFDATA);
      PRINTF("%02x", ((unsigned char *)(buf))[i]);
    }
  }

  /* Read the RSSI and CRC/Corr bytes */
  rssi = ((int8_t)REG(RFCORE_SFR_RFDATA)) - RSSI_OFFSET;
  crc_corr = REG(RFCORE_SFR_RFDATA);

  PRINTF("%02x%02x\n", (uint8_t)rssi, crc_corr);

  /* MS bit CRC OK/Not OK, 7 LS Bits, Correlation value */
  if(crc_corr & CRC_BIT_MASK) {
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, crc_corr & LQI_BIT_MASK);
    RIMESTATS_ADD(llrx);
  } else {
    RIMESTATS_ADD(badcrc);
    PRINTF("RF: Bad CRC\n");
    CC2538_RF_CSP_ISFLUSHRX();
    return 0;
  }

#if CC2538_RF_CONF_SNIFFER
  write_byte(magic[0]);
  write_byte(magic[1]);
  write_byte(magic[2]);
  write_byte(magic[3]);
  write_byte(len + 2);
  for(i = 0; i < len; ++i) {
    write_byte(((unsigned char *)(buf))[i]);
  }
  write_byte(rssi);
  write_byte(crc_corr);
  flush();
#endif

  /* If FIFOP==1 and FIFO==0 then we had a FIFO overflow at some point. */
  if(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFOP) {
    if(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFO) {
      process_poll(&cc2538_rf_process);
    } else {
      CC2538_RF_CSP_ISFLUSHRX();
    }
  }

  return (len);
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  PRINTF("RF: Receiving\n");

  /*
   * SFD high while transmitting and receiving.
   * TX_ACTIVE high only when transmitting
   *
   * FSMSTAT1 & (TX_ACTIVE | SFD) == SFD <=> receiving
   */
  return ((REG(RFCORE_XREG_FSMSTAT1)
           & (RFCORE_XREG_FSMSTAT1_TX_ACTIVE | RFCORE_XREG_FSMSTAT1_SFD))
          == RFCORE_XREG_FSMSTAT1_SFD);
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  PRINTF("RF: Pending\n");

  return (REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFOP);
}
/*---------------------------------------------------------------------------*/
const struct radio_driver cc2538_rf_driver = {
  init,
  prepare,
  transmit,
  send,
  read,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
};
/*---------------------------------------------------------------------------*/
/**
 * \brief Implementation of the cc2538 RF driver process
 *
 *        This process is started by init(). It simply sits there waiting for
 *        an event. Upon frame reception, the RX ISR will poll this process.
 *        Subsequently, the contiki core will generate an event which will
 *        call this process so that the received frame can be picked up from
 *        the RF RX FIFO
 *
 */
PROCESS_THREAD(cc2538_rf_process, ev, data)
{
  int len;
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    packetbuf_clear();
    len = read(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {
      packetbuf_set_datalen(len);

      NETSTACK_RDC.input();
    }

    /* If we were polled due to an RF error, reset the transceiver */
    if(rf_flags & RF_MUST_RESET) {
      rf_flags = 0;

      off();
      init();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief The cc2538 RF RX/TX ISR
 *
 *        This is the interrupt service routine for all RF interrupts relating
 *        to RX and TX. Error conditions are handled by cc2538_rf_err_isr().
 *        Currently, we only acknowledge the FIFOP interrupt source.
 */
void
cc2538_rf_rx_tx_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  process_poll(&cc2538_rf_process);

  /* We only acknowledge FIFOP so we can safely wipe out the entire SFR */
  REG(RFCORE_SFR_RFIRQF0) = 0;

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief The cc2538 RF Error ISR
 *
 *        This is the interrupt service routine for all RF errors. We
 *        acknowledge every error type and instead of trying to be smart and
 *        act differently depending on error condition, we simply reset the
 *        transceiver. RX FIFO overflow is an exception, we ignore this error
 *        since read() handles it anyway.
 *
 *        However, we don't want to reset within this ISR. If the error occurs
 *        while we are reading a frame out of the FIFO, trashing the FIFO in
 *        the middle of read(), would result in further errors (RX underflows).
 *
 *        Instead, we set a flag and poll the driver process. The process will
 *        reset the transceiver without any undesirable consequences.
 */
void
cc2538_rf_err_isr(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  PRINTF("RF Error: 0x%08lx\n", REG(RFCORE_SFR_RFERRF));

  /* If the error is not an RX FIFO overflow, set a flag */
  if(REG(RFCORE_SFR_RFERRF) != RFCORE_SFR_RFERRF_RXOVERF) {
    rf_flags |= RF_MUST_RESET;
  }

  REG(RFCORE_SFR_RFERRF) = 0;

  process_poll(&cc2538_rf_process);

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/

/** @} */
