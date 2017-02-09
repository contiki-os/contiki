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
#include "net/linkaddr.h"
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
#define RF_ON         0x01

/* Bit Masks for the last byte in the RX FIFO */
#define CRC_BIT_MASK 0x80
#define LQI_BIT_MASK 0x7F
/* RSSI Offset */
#define RSSI_OFFSET    73

/* 192 usec off -> on interval (RX Callib -> SFD Wait). We wait a bit more */
#define ONOFF_TIME                    RTIMER_ARCH_SECOND / 3125
/*---------------------------------------------------------------------------*/
#ifdef CC2538_RF_CONF_AUTOACK
#define CC2538_RF_AUTOACK CC2538_RF_CONF_AUTOACK
#else
#define CC2538_RF_AUTOACK 1
#endif
/*---------------------------------------------------------------------------
 * MAC timer
 *---------------------------------------------------------------------------*/
/* Timer conversion */
#define RADIO_TO_RTIMER(X) ((uint32_t)((uint64_t)(X) * RTIMER_ARCH_SECOND / SYS_CTRL_32MHZ))

#define CLOCK_STABLE() do {															\
			while ( !(REG(SYS_CTRL_CLOCK_STA) & (SYS_CTRL_CLOCK_STA_XOSC_STB)));	\
		} while(0)
/*---------------------------------------------------------------------------*/
/* Are we currently in poll mode? Disabled by default */
static uint8_t volatile poll_mode = 0;
/* Do we perform a CCA before sending? Enabled by default. */
static uint8_t send_on_cca = 1;
static int8_t rssi;
static uint8_t crc_corr;
/*---------------------------------------------------------------------------*/
static uint8_t rf_flags;
static uint8_t rf_channel = CC2538_RF_CHANNEL;

static int on(void);
static int off(void);
/*---------------------------------------------------------------------------*/
/* TX Power dBm lookup table. Values from SmartRF Studio v1.16.0 */
typedef struct output_config {
  radio_value_t power;
  uint8_t txpower_val;
} output_config_t;

static const output_config_t output_power[] = {
  {  7, 0xFF },
  {  5, 0xED },
  {  3, 0xD5 },
  {  1, 0xC5 },
  {  0, 0xB6 },
  { -1, 0xB0 },
  { -3, 0xA1 },
  { -5, 0x91 },
  { -7, 0x88 },
  { -9, 0x72 },
  {-11, 0x62 },
  {-13, 0x58 },
  {-15, 0x42 },
  {-24, 0x00 },
};

#define OUTPUT_CONFIG_COUNT (sizeof(output_power) / sizeof(output_config_t))

/* Max and Min Output Power in dBm */
#define OUTPUT_POWER_MIN    (output_power[OUTPUT_CONFIG_COUNT - 1].power)
#define OUTPUT_POWER_MAX    (output_power[0].power)
/*---------------------------------------------------------------------------*/
PROCESS(cc2538_rf_process, "cc2538 RF driver");
/*---------------------------------------------------------------------------*/
/**
 * \brief Get the current operating channel
 * \return Returns a value in [11,26] representing the current channel
 */
static uint8_t
get_channel()
{
  uint8_t chan = REG(RFCORE_XREG_FREQCTRL) & RFCORE_XREG_FREQCTRL_FREQ;

  return (chan - CC2538_RF_CHANNEL_MIN) / CC2538_RF_CHANNEL_SPACING
         + CC2538_RF_CHANNEL_MIN;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the current operating channel
 * \param channel The desired channel as a value in [11,26]
 * \return Returns a value in [11,26] representing the current channel
 *         or a negative value if \e channel was out of bounds
 */
static int8_t
set_channel(uint8_t channel)
{
  uint8_t was_on = 0;

  PRINTF("RF: Set Channel\n");

  if((channel < CC2538_RF_CHANNEL_MIN) || (channel > CC2538_RF_CHANNEL_MAX)) {
    return CC2538_RF_CHANNEL_SET_ERROR;
  }

  /* Changes to FREQCTRL take effect after the next recalibration */

  /* If we are off, save state, otherwise switch off and save state */
  if((REG(RFCORE_XREG_FSMSTAT0) & RFCORE_XREG_FSMSTAT0_FSM_FFCTRL_STATE) != 0) {
    was_on = 1;
    off();
  }
  REG(RFCORE_XREG_FREQCTRL) = CC2538_RF_CHANNEL_MIN +
    (channel - CC2538_RF_CHANNEL_MIN) * CC2538_RF_CHANNEL_SPACING;

  /* switch radio back on only if radio was on before - otherwise will turn on radio foor sleepy nodes */
  if(was_on) {
    on();
  }

  rf_channel = channel;

  return (int8_t)channel;
}
/*---------------------------------------------------------------------------*/
static radio_value_t
get_pan_id(void)
{
  return (radio_value_t)(REG(RFCORE_FFSM_PAN_ID1) << 8 | REG(RFCORE_FFSM_PAN_ID0));
}
/*---------------------------------------------------------------------------*/
static void
set_pan_id(uint16_t pan)
{
  REG(RFCORE_FFSM_PAN_ID0) = pan & 0xFF;
  REG(RFCORE_FFSM_PAN_ID1) = pan >> 8;
}
/*---------------------------------------------------------------------------*/
static radio_value_t
get_short_addr(void)
{
  return (radio_value_t)(REG(RFCORE_FFSM_SHORT_ADDR1) << 8 | REG(RFCORE_FFSM_SHORT_ADDR0));
}
/*---------------------------------------------------------------------------*/
static void
set_short_addr(uint16_t addr)
{
  REG(RFCORE_FFSM_SHORT_ADDR0) = addr & 0xFF;
  REG(RFCORE_FFSM_SHORT_ADDR1) = addr >> 8;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Reads the current signal strength (RSSI)
 * \return The current RSSI in dBm
 *
 * This function reads the current RSSI on the currently configured
 * channel.
 */
static radio_value_t
get_rssi(void)
{
  int8_t rssi;
  uint8_t was_off = 0;

  /* If we are off, turn on first */
  if((REG(RFCORE_XREG_FSMSTAT0) & RFCORE_XREG_FSMSTAT0_FSM_FFCTRL_STATE) == 0) {
    was_off = 1;
    on();
  }

  /* Wait on RSSI_VALID */
  while((REG(RFCORE_XREG_RSSISTAT) & RFCORE_XREG_RSSISTAT_RSSI_VALID) == 0);

  rssi = (int8_t)(REG(RFCORE_XREG_RSSI) & RFCORE_XREG_RSSI_RSSI_VAL) - RSSI_OFFSET;

  /* If we were off, turn back off */
  if(was_off) {
    off();
  }

  return rssi;
}
/*---------------------------------------------------------------------------*/
/* Returns the current CCA threshold in dBm */
static radio_value_t
get_cca_threshold(void)
{
  return (int8_t)(REG(RFCORE_XREG_CCACTRL0) & RFCORE_XREG_CCACTRL0_CCA_THR) - RSSI_OFFSET;
}
/*---------------------------------------------------------------------------*/
/* Sets the CCA threshold in dBm */
static void
set_cca_threshold(radio_value_t value)
{
  REG(RFCORE_XREG_CCACTRL0) = (value & 0xFF) + RSSI_OFFSET;
}
/*---------------------------------------------------------------------------*/
/* Returns the current TX power in dBm */
static radio_value_t
get_tx_power(void)
{
  int i;
  uint8_t reg_val = REG(RFCORE_XREG_TXPOWER) & 0xFF;

  /*
   * Find the TXPOWER value in the lookup table
   * If the value has been written with set_tx_power, we should be able to
   * find the exact value. However, in case the register has been written in
   * a different fashion, we return the immediately lower value of the lookup
   */
  for(i = 0; i < OUTPUT_CONFIG_COUNT; i++) {
    if(reg_val >= output_power[i].txpower_val) {
      return output_power[i].power;
    }
  }
  return OUTPUT_POWER_MIN;
}
/*---------------------------------------------------------------------------*/
/*
 * Set TX power to 'at least' power dBm
 * This works with a lookup table. If the value of 'power' does not exist in
 * the lookup table, TXPOWER will be set to the immediately higher available
 * value
 */
static void
set_tx_power(radio_value_t power)
{
  int i;

  for(i = OUTPUT_CONFIG_COUNT - 1; i >= 0; --i) {
    if(power <= output_power[i].power) {
      REG(RFCORE_XREG_TXPOWER) = output_power[i].txpower_val;
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_frame_filtering(uint8_t enable)
{
  if(enable) {
    REG(RFCORE_XREG_FRMFILT0) |= RFCORE_XREG_FRMFILT0_FRAME_FILTER_EN;
  } else {
    REG(RFCORE_XREG_FRMFILT0) &= ~RFCORE_XREG_FRMFILT0_FRAME_FILTER_EN;
  }
}
/*---------------------------------------------------------------------------*/
static void
mac_timer_init(void)
{
  CLOCK_STABLE();
  REG(RFCORE_SFR_MTCTRL) |= RFCORE_SFR_MTCTRL_SYNC;
  REG(RFCORE_SFR_MTCTRL) |= RFCORE_SFR_MTCTRL_RUN;
  while(!(REG(RFCORE_SFR_MTCTRL) & RFCORE_SFR_MTCTRL_STATE));
  REG(RFCORE_SFR_MTCTRL) &= ~RFCORE_SFR_MTCTRL_RUN;
  while(REG(RFCORE_SFR_MTCTRL) & RFCORE_SFR_MTCTRL_STATE);
  REG(RFCORE_SFR_MTCTRL) |= RFCORE_SFR_MTCTRL_SYNC;
  REG(RFCORE_SFR_MTCTRL) |= (RFCORE_SFR_MTCTRL_RUN);
  while(!(REG(RFCORE_SFR_MTCTRL) & RFCORE_SFR_MTCTRL_STATE));
}
/*---------------------------------------------------------------------------*/
static void
set_poll_mode(uint8_t enable)
{
  poll_mode = enable;

  if(enable) {
    mac_timer_init();
    REG(RFCORE_XREG_RFIRQM0) &= ~RFCORE_XREG_RFIRQM0_FIFOP; /* mask out FIFOP interrupt source */
    REG(RFCORE_SFR_RFIRQF0) &= ~RFCORE_SFR_RFIRQF0_FIFOP;   /* clear pending FIFOP interrupt */
    NVIC_DisableIRQ(RF_TX_RX_IRQn);                         /* disable RF interrupts */
  } else {
    REG(RFCORE_XREG_RFIRQM0) |= RFCORE_XREG_RFIRQM0_FIFOP;  /* enable FIFOP interrupt source */
    NVIC_EnableIRQ(RF_TX_RX_IRQn);                          /* enable RF interrupts */
  }
}
/*---------------------------------------------------------------------------*/
static void
set_send_on_cca(uint8_t enable)
{
  send_on_cca = enable;
}
/*---------------------------------------------------------------------------*/
static void
set_auto_ack(uint8_t enable)
{
  if(enable) {
    REG(RFCORE_XREG_FRMCTRL0) |= RFCORE_XREG_FRMCTRL0_AUTOACK;
  } else {
    REG(RFCORE_XREG_FRMCTRL0) &= ~RFCORE_XREG_FRMCTRL0_AUTOACK;
  }
}
/*---------------------------------------------------------------------------*/
static uint32_t
get_sfd_timestamp(void)
{
  uint64_t sfd, timer_val, buffer;

  REG(RFCORE_SFR_MTMSEL) = (REG(RFCORE_SFR_MTMSEL) & ~RFCORE_SFR_MTMSEL_MTMSEL) | 0x00000000;
  REG(RFCORE_SFR_MTCTRL) |= RFCORE_SFR_MTCTRL_LATCH_MODE;
  timer_val = REG(RFCORE_SFR_MTM0) & RFCORE_SFR_MTM0_MTM0;
  timer_val |= ((REG(RFCORE_SFR_MTM1) & RFCORE_SFR_MTM1_MTM1) << 8);
  REG(RFCORE_SFR_MTMSEL) = (REG(RFCORE_SFR_MTMSEL) & ~RFCORE_SFR_MTMSEL_MTMOVFSEL) | 0x00000000;
  timer_val |= ((REG(RFCORE_SFR_MTMOVF0) & RFCORE_SFR_MTMOVF0_MTMOVF0) << 16);
  timer_val |= ((REG(RFCORE_SFR_MTMOVF1) & RFCORE_SFR_MTMOVF1_MTMOVF1) << 24);
  buffer = REG(RFCORE_SFR_MTMOVF2) & RFCORE_SFR_MTMOVF2_MTMOVF2;
  timer_val |= (buffer << 32);

  REG(RFCORE_SFR_MTMSEL) = (REG(RFCORE_SFR_MTMSEL) & ~RFCORE_SFR_MTMSEL_MTMSEL) | 0x00000001;
  REG(RFCORE_SFR_MTCTRL) |= RFCORE_SFR_MTCTRL_LATCH_MODE;
  sfd = REG(RFCORE_SFR_MTM0) & RFCORE_SFR_MTM0_MTM0;
  sfd |= ((REG(RFCORE_SFR_MTM1) & RFCORE_SFR_MTM1_MTM1) << 8);
  REG(RFCORE_SFR_MTMSEL) = (REG(RFCORE_SFR_MTMSEL) & ~RFCORE_SFR_MTMSEL_MTMOVFSEL) | 0x00000010;
  sfd |= ((REG(RFCORE_SFR_MTMOVF0) & RFCORE_SFR_MTMOVF0_MTMOVF0) << 16);
  sfd |= ((REG(RFCORE_SFR_MTMOVF1) & RFCORE_SFR_MTMOVF1_MTMOVF1) << 24);
  buffer = REG(RFCORE_SFR_MTMOVF2) & RFCORE_SFR_MTMOVF2_MTMOVF2;
  sfd |= (buffer << 32);

  return RTIMER_NOW() - RADIO_TO_RTIMER(timer_val - sfd);
}
/*---------------------------------------------------------------------------*/
/* Netstack API radio driver functions */
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  int cca;
  uint8_t was_off = 0;

  PRINTF("RF: CCA\n");

  /* If we are off, turn on first */
  if((REG(RFCORE_XREG_FSMSTAT0) & RFCORE_XREG_FSMSTAT0_FSM_FFCTRL_STATE) == 0) {
    was_off = 1;
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
  if(was_off) {
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

  if(!(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFOP)) {
    CC2538_RF_CSP_ISFLUSHRX();
  }

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

  REG(RFCORE_XREG_CCACTRL0) = CC2538_RF_CCA_THRES;

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

  /* Disable source address matching and autopend */
  REG(RFCORE_XREG_SRCMATCH) = 0;

  /* MAX FIFOP threshold */
  REG(RFCORE_XREG_FIFOPCTRL) = CC2538_RF_MAX_PACKET_LEN;

  /* Set TX Power */
  REG(RFCORE_XREG_TXPOWER) = CC2538_RF_TX_POWER;

  set_channel(rf_channel);

  /* Acknowledge all RF Error interrupts */
  REG(RFCORE_XREG_RFERRM) = RFCORE_XREG_RFERRM_RFERRM;
  NVIC_EnableIRQ(RF_ERR_IRQn);

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

  set_poll_mode(poll_mode);

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
  uint8_t was_off = 0;

  PRINTF("RF: Transmit\n");

  if(!(rf_flags & RX_ACTIVE)) {
    t0 = RTIMER_NOW();
    on();
    was_off = 1;
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + ONOFF_TIME));
  }

  if(send_on_cca) {
    if(channel_clear() == CC2538_RF_CCA_BUSY) {
      RIMESTATS_ADD(contentiondrop);
      return RADIO_TX_COLLISION;
    }
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

  if(was_off) {
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

  if(!poll_mode) {
    /* If FIFOP==1 and FIFO==0 then we had a FIFO overflow at some point. */
    if(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFOP) {
      if(REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFO) {
        process_poll(&cc2538_rf_process);
      } else {
        CC2538_RF_CSP_ISFLUSHRX();
      }
    }
  }

  return len;
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
  return (REG(RFCORE_XREG_FSMSTAT1)
          & (RFCORE_XREG_FSMSTAT1_TX_ACTIVE | RFCORE_XREG_FSMSTAT1_SFD))
         == RFCORE_XREG_FSMSTAT1_SFD;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  PRINTF("RF: Pending\n");

  return REG(RFCORE_XREG_FSMSTAT1) & RFCORE_XREG_FSMSTAT1_FIFOP;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    *value = (REG(RFCORE_XREG_RXENABLE) && RFCORE_XREG_RXENABLE_RXENMASK) == 0
      ? RADIO_POWER_MODE_OFF : RADIO_POWER_MODE_ON;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = (radio_value_t)get_channel();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_PAN_ID:
    *value = get_pan_id();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_16BIT_ADDR:
    *value = get_short_addr();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    *value = 0;
    if(REG(RFCORE_XREG_FRMFILT0) & RFCORE_XREG_FRMFILT0_FRAME_FILTER_EN) {
      *value |= RADIO_RX_MODE_ADDRESS_FILTER;
    }
    if(REG(RFCORE_XREG_FRMCTRL0) & RFCORE_XREG_FRMCTRL0_AUTOACK) {
      *value |= RADIO_RX_MODE_AUTOACK;
    }
    if(poll_mode) {
      *value |= RADIO_RX_MODE_POLL_MODE;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
    *value = 0;
    if(send_on_cca) {
      *value |= RADIO_TX_MODE_SEND_ON_CCA;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    *value = get_tx_power();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    *value = get_cca_threshold();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = get_rssi();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_LAST_RSSI:
    *value = rssi;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_LAST_LINK_QUALITY:
    *value = crc_corr & LQI_BIT_MASK;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = CC2538_RF_CHANNEL_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = CC2538_RF_CHANNEL_MAX;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MIN:
    *value = OUTPUT_POWER_MIN;
    return RADIO_RESULT_OK;
  case RADIO_CONST_TXPOWER_MAX:
    *value = OUTPUT_POWER_MAX;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    if(value == RADIO_POWER_MODE_ON) {
      on();
      return RADIO_RESULT_OK;
    }
    if(value == RADIO_POWER_MODE_OFF) {
      off();
      return RADIO_RESULT_OK;
    }
    return RADIO_RESULT_INVALID_VALUE;
  case RADIO_PARAM_CHANNEL:
    if(value < CC2538_RF_CHANNEL_MIN ||
       value > CC2538_RF_CHANNEL_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    if(set_channel(value) == CC2538_RF_CHANNEL_SET_ERROR) {
      return RADIO_RESULT_ERROR;
    }
    return RADIO_RESULT_OK;
  case RADIO_PARAM_PAN_ID:
    set_pan_id(value & 0xffff);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_16BIT_ADDR:
    set_short_addr(value & 0xffff);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER |
                 RADIO_RX_MODE_AUTOACK |
                 RADIO_RX_MODE_POLL_MODE)) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    set_frame_filtering((value & RADIO_RX_MODE_ADDRESS_FILTER) != 0);
    set_auto_ack((value & RADIO_RX_MODE_AUTOACK) != 0);
    set_poll_mode((value & RADIO_RX_MODE_POLL_MODE) != 0);

    return RADIO_RESULT_OK;
  case RADIO_PARAM_TX_MODE:
    if(value & ~(RADIO_TX_MODE_SEND_ON_CCA)) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    set_send_on_cca((value & RADIO_TX_MODE_SEND_ON_CCA) != 0);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_TXPOWER:
    if(value < OUTPUT_POWER_MIN || value > OUTPUT_POWER_MAX) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    set_tx_power(value);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    set_cca_threshold(value);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  uint8_t *target;
  int i;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    target = dest;
    for(i = 0; i < 8; i++) {
      target[i] = ((uint32_t *)RFCORE_FFSM_EXT_ADDR0)[7 - i] & 0xFF;
    }

    return RADIO_RESULT_OK;
  }

  if(param == RADIO_PARAM_LAST_PACKET_TIMESTAMP) {
    if(size != sizeof(rtimer_clock_t) || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    *(rtimer_clock_t *)dest = get_sfd_timestamp();
    return RADIO_RESULT_OK;
  }

  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  int i;

  if(param == RADIO_PARAM_64BIT_ADDR) {
    if(size != 8 || !src) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    for(i = 0; i < 8; i++) {
      ((uint32_t *)RFCORE_FFSM_EXT_ADDR0)[i] = ((uint8_t *)src)[7 - i];
    }

    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
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
  get_value,
  set_value,
  get_object,
  set_object
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
    /* Only if we are not in poll mode oder we are in poll mode and transceiver has to be reset */
    PROCESS_YIELD_UNTIL((!poll_mode || (poll_mode && (rf_flags & RF_MUST_RESET))) && (ev == PROCESS_EVENT_POLL));

    if(!poll_mode) {
      packetbuf_clear();
      len = read(packetbuf_dataptr(), PACKETBUF_SIZE);

      if(len > 0) {
        packetbuf_set_datalen(len);

        NETSTACK_RDC.input();
      }
    }

    /* If we were polled due to an RF error, reset the transceiver */
    if(rf_flags & RF_MUST_RESET) {
      uint8_t was_on;
      rf_flags = 0;

      /* save state so we know if to switch on again after re-init */
      if((REG(RFCORE_XREG_FSMSTAT0) & RFCORE_XREG_FSMSTAT0_FSM_FFCTRL_STATE) == 0) {
        was_on = 0;
      } else {
        was_on = 1;
      }
      off();
      init();
      if(was_on) {
        /* switch back on */
        on();
      }
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

  if(!poll_mode) {
    process_poll(&cc2538_rf_process);
  }

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
void
cc2538_rf_set_promiscous_mode(char p)
{
  set_frame_filtering(p);
}
/*---------------------------------------------------------------------------*/
/** @} */
