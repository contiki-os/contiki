/*
 * Copyright (c) 2014, NXP and SICS Swedish ICT.
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
 */

/**
 * \file
 *         Contiki driver for NXP JN516X using MMAC interface
 * \authors
 *         Beshr Al Nahas <beshr@sics.se>
 *         Simon Duquennot <simonduq@sics.se>
 *         Atis Elsts <atis.elsts@sics.se>
 *
 */

#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "sys/rtimer.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "lib/crc16.h"
#include "lib/ringbufindex.h"

#include "AppHardwareApi.h"
#include "MMAC.h"
#include "micromac-radio.h"
#include "JPT.h"
#include "PeripheralRegs.h"

void vMMAC_SetChannelAndPower(uint8 u8Channel, int8 i8power);

/* This driver configures the radio in PHY mode and does address decoding
 * and acknowledging in software. */

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

/* Perform CRC check for received packets in SW,
 * since we use PHY mode which does not calculate CRC in HW */
#define CRC_SW 1

#define CHECKSUM_LEN 2

/* Max packet duration: 5 + 127 + 2 bytes, 32us per byte */
#define MAX_PACKET_DURATION US_TO_RTIMERTICKS((127 + 2) * 32 + RADIO_DELAY_BEFORE_TX)
/* Max ACK duration: 5 + 3 + 2 bytes */
#define MAX_ACK_DURATION    US_TO_RTIMERTICKS((3 + 2) * 32 + RADIO_DELAY_BEFORE_TX)

/* Test-mode pins output on dev-kit */
#define RADIO_TEST_MODE_HIGH_PWR 1
#define RADIO_TEST_MODE_ADVANCED 2
#define RADIO_TEST_MODE_DISABLED 0

#ifndef RADIO_TEST_MODE
#define RADIO_TEST_MODE  RADIO_TEST_MODE_DISABLED
#endif /* RADIO_TEST_MODE */

/* The number of input buffers */
#ifndef MIRCOMAC_CONF_BUF_NUM
#define MIRCOMAC_CONF_BUF_NUM 2
#endif /* MIRCOMAC_CONF_BUF_NUM */

/* Init radio channel */
#ifndef MICROMAC_CONF_CHANNEL
#define MICROMAC_CONF_CHANNEL 26
#endif

/* Default energy level threshold for clear channel detection */
#ifndef MICROMAC_CONF_CCA_THR
#define MICROMAC_CONF_CCA_THR 39 /* approximately -85 dBm */
#endif /* MICROMAC_CONF_CCA_THR */

#if (JENNIC_CHIP == JN5169)
#define OUTPUT_POWER_MAX      10
#define OUTPUT_POWER_MIN      (-32)
#define ABS_OUTPUT_POWER_MIN  (32)
#else
#define OUTPUT_POWER_MAX      0
#define OUTPUT_POWER_MIN      (-32)
#endif

/* Default Tx power [dBm] (between OUTPUT_POWER_MIN and OUTPUT_POWER_MAX) */
#ifndef MICROMAC_CONF_TX_POWER
#define MICROMAC_CONF_TX_POWER 0
#endif

/* Autoack */
#ifndef MICROMAC_CONF_AUTOACK
#define MICROMAC_CONF_AUTOACK 1
#endif /* MICROMAC_CONF_AUTOACK */

/* Set radio always on for now because this is what Contiki MAC layers
 * expect. */
#ifndef MICROMAC_CONF_ALWAYS_ON
#define MICROMAC_CONF_ALWAYS_ON 1
#endif /* MICROMAC_CONF_ALWAYS_ON */

#define BUSYWAIT_UNTIL(cond, max_time) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) ; \
  } while(0)

/* Local variables */
static volatile signed char radio_last_rssi;
static volatile uint8_t radio_last_correlation; /* LQI */

/* Did we miss a request to turn the radio on due to overflow? */
static volatile uint8_t missed_radio_on_request = 0;

/* Poll mode disabled by default */
static uint8_t poll_mode = 0;
/* (Software) frame filtering enabled by default */
static uint8_t frame_filtering = 1;
/* (Software) autoack */
static uint8_t autoack_enabled = MICROMAC_CONF_AUTOACK;
/* CCA before sending? Disabled by default. */
static uint8_t send_on_cca = 0;

/* Current radio channel */
static int current_channel = MICROMAC_CONF_CHANNEL;

/* Current set point tx power
   Actual tx power may be different. Use get_txpower() for actual power */
static int current_tx_power = MICROMAC_CONF_TX_POWER;

/* an integer between 0 and 255, used only with cca() */
static uint8_t cca_thershold = MICROMAC_CONF_CCA_THR;

/* Tx in progress? */
static volatile uint8_t tx_in_progress = 0;
/* Are we currently listening? */
static volatile uint8_t listen_on = 0;

/* Is the driver currently transmitting a software ACK? */
static uint8_t in_ack_transmission = 0;

/* TX frame buffer */
static tsPhyFrame tx_frame_buffer;

/* RX frame buffer */
static tsPhyFrame *rx_frame_buffer;

/* Frame buffer pointer to read from */
static tsPhyFrame *input_frame_buffer = NULL;

/* Ringbuffer for received packets in interrupt enabled mode */
static struct ringbufindex input_ringbuf;
static tsPhyFrame input_array[MIRCOMAC_CONF_BUF_NUM];

/* SFD timestamp in RTIMER ticks */
static volatile uint32_t last_packet_timestamp = 0;

/* Local functions prototypes */
static int on(void);
static int off(void);
static int is_packet_for_us(uint8_t *buf, int len, int do_send_ack);
static void set_frame_filtering(uint8_t enable);
static rtimer_clock_t get_packet_timestamp(void);
static void set_txpower(int8_t power);
void set_channel(int c);
static void radio_interrupt_handler(uint32 mac_event);
static int get_detected_energy(void);
static int get_rssi(void);
static void read_last_rssi(void);

/*---------------------------------------------------------------------------*/
PROCESS(micromac_radio_process, "micromac_radio_driver");
/*---------------------------------------------------------------------------*/

/* Custom Radio parameters */
#ifndef RADIO_RX_MODE_POLL_MODE
#define RADIO_PARAM_LAST_RSSI 0x80
#define RADIO_PARAM_LAST_PACKET_TIMESTAMP 0x81
#define RADIO_RX_MODE_POLL_MODE        (1 << 2)
#endif /* RADIO_RX_MODE_POLL_MODE */

/*---------------------------------------------------------------------------*/
static rtimer_clock_t
get_packet_timestamp(void)
{
  /* Wait for an edge */
  uint32_t t = u32MMAC_GetTime();
  while(u32MMAC_GetTime() == t);
  /* Save SFD timestamp, converted from radio timer to RTIMER */
  last_packet_timestamp = RTIMER_NOW() -
    RADIO_TO_RTIMER((uint32_t)(u32MMAC_GetTime() - (u32MMAC_GetRxTime() - 1)));
  /* The remaining measured error is typically in range 0..16 usec.
   * Center it around zero, in the -8..+8 usec range. */
  last_packet_timestamp -= US_TO_RTIMERTICKS(8);
  return last_packet_timestamp;
}
/*---------------------------------------------------------------------------*/
static int
init_software(void)
{
  int put_index;
  /* Initialize ring buffer and first input packet pointer */
  ringbufindex_init(&input_ringbuf, MIRCOMAC_CONF_BUF_NUM);
  /* get pointer to next input slot */
  put_index = ringbufindex_peek_put(&input_ringbuf);
  if(put_index == -1) {
    rx_frame_buffer = NULL;
    printf("micromac_radio init:! no buffer available. Abort init.\n");
    off();
    return 0;
  } else {
    rx_frame_buffer = &input_array[put_index];
  }
  input_frame_buffer = rx_frame_buffer;

  process_start(&micromac_radio_process, NULL);

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  int ret = 1;
  tsExtAddr node_long_address;
  uint16_t node_short_address;
  static uint8_t is_initialized;

  tx_in_progress = 0;

  u32JPT_Init();
  vMMAC_Enable();

  /* Enable/disable interrupts */
  if(poll_mode) {
    vMMAC_EnableInterrupts(NULL);
    vMMAC_ConfigureInterruptSources(0);
  } else {
    vMMAC_EnableInterrupts(&radio_interrupt_handler);
  }
  vMMAC_ConfigureRadio();
  set_channel(current_channel);
  set_txpower(current_tx_power);

  vMMAC_GetMacAddress(&node_long_address);
  /* Short addresses are disabled by default */
  node_short_address = (uint16_t)node_long_address.u32L;
  vMMAC_SetRxAddress(frame802154_get_pan_id(), node_short_address, &node_long_address);

  /* Disable hardware backoff */
  vMMAC_SetTxParameters(1, 0, 0, 0);
  vMMAC_SetCutOffTimer(0, FALSE);

#if RADIO_TEST_MODE == RADIO_TEST_MODE_HIGH_PWR
  /* Enable high power mode.
   * In this mode DIO2 goes high during RX
   * and DIO3 goes high during TX
   **/
  vREG_SysWrite(REG_SYS_PWR_CTRL,
                u32REG_SysRead(REG_SYS_PWR_CTRL)
                | REG_SYSCTRL_PWRCTRL_RFRXEN_MASK
                | REG_SYSCTRL_PWRCTRL_RFTXEN_MASK);
#elif RADIO_TEST_MODE == RADIO_TEST_MODE_ADVANCED
  /* output internal radio status on IO pins.
   * See Chris@NXP email */
  vREG_SysWrite(REG_SYS_PWR_CTRL,
                u32REG_SysRead(REG_SYS_PWR_CTRL) | (1UL << 26UL));
#endif /* TEST_MODE */

  if(!is_initialized) {
    is_initialized = 1;
    ret = init_software();
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  /* No address matching or frame decoding */
  if(rx_frame_buffer != NULL) {
    vMMAC_StartPhyReceive(rx_frame_buffer,
                          (uint16_t)(E_MMAC_RX_START_NOW
                                     | E_MMAC_RX_NO_FCS_ERROR) /* means: reject FCS errors */
                          );
  } else {
    missed_radio_on_request = 1;
  }
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  listen_on = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  listen_on = 0;
  tx_in_progress = 0;

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  /* The following would be needed with delayed Tx/Rx functions
   * vMMAC_SetCutOffTimer(0, FALSE);*/
  vMMAC_RadioOff();

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short payload_len)
{
  if(tx_in_progress) {
    return RADIO_TX_COLLISION;
  }
  tx_in_progress = 1;

  /* Energest */
  if(listen_on) {
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  }
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  /* Transmit and wait */
  vMMAC_StartPhyTransmit(&tx_frame_buffer,
                         E_MMAC_TX_START_NOW |
                         (send_on_cca ? E_MMAC_TX_USE_CCA : E_MMAC_TX_NO_CCA));

  if(poll_mode) {
    BUSYWAIT_UNTIL(u32MMAC_PollInterruptSource(E_MMAC_INT_TX_COMPLETE), MAX_PACKET_DURATION);
  } else {
    if(in_ack_transmission) {
      /* as nested interupts are not possible, the tx flag will never be cleared */
      BUSYWAIT_UNTIL(FALSE, MAX_ACK_DURATION);
    } else {
      /* wait until the tx flag is cleared */
      BUSYWAIT_UNTIL(!tx_in_progress, MAX_PACKET_DURATION);
    }
  }

  /* Energest */
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  if(listen_on) {
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  }
  tx_in_progress = 0;

  /* Check error code */
  int ret;
  uint32_t tx_error = u32MMAC_GetTxErrors();
  if(tx_error == 0) {
    ret = RADIO_TX_OK;
    RIMESTATS_ADD(acktx);
  } else if(tx_error & E_MMAC_TXSTAT_ABORTED) {
    ret = RADIO_TX_ERR;
    RIMESTATS_ADD(sendingdrop);
  } else if(tx_error & E_MMAC_TXSTAT_CCA_BUSY) {
    ret = RADIO_TX_COLLISION;
    RIMESTATS_ADD(contentiondrop);
  } else if(tx_error & E_MMAC_TXSTAT_NO_ACK) {
    ret = RADIO_TX_NOACK;
    RIMESTATS_ADD(noacktx);
  } else {
    ret = RADIO_TX_ERR;
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  uint8_t i;
  uint16_t checksum;

  RIMESTATS_ADD(lltx);

  if(tx_in_progress) {
    return 1;
  }
  if(payload_len > 127 || payload == NULL) {
    return 1;
  }
  /* Copy payload to (soft) Ttx buffer */
  memcpy(tx_frame_buffer.uPayload.au8Byte, payload, payload_len);
  i = payload_len;
#if CRC_SW
  /* Compute CRC */
  checksum = crc16_data(payload, payload_len, 0);
  tx_frame_buffer.uPayload.au8Byte[i++] = checksum;
  tx_frame_buffer.uPayload.au8Byte[i++] = (checksum >> 8) & 0xff;
  tx_frame_buffer.u8PayloadLength = payload_len + CHECKSUM_LEN;
#else
  tx_frame_buffer.u8PayloadLength = payload_len;
#endif

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
send(const void *payload, unsigned short payload_len)
{
  if(prepare(payload, payload_len) == 0) {
    return transmit(payload_len);
  } else {
    return RADIO_TX_ERR;
  }
}
/*---------------------------------------------------------------------------*/
int
get_channel(void)
{
  return current_channel;
}
/*---------------------------------------------------------------------------*/
void
set_channel(int c)
{
  current_channel = c;
  /* will fine tune TX power as well */
  vMMAC_SetChannel(current_channel);
}
/*---------------------------------------------------------------------------*/
static int
is_broadcast_addr(uint8_t mode, uint8_t *addr)
{
  int i = ((mode == FRAME802154_SHORTADDRMODE) ? 2 : 8);
  while(i-- > 0) {
    if(addr[i] != 0xff) {
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/* Send an ACK */
static void
send_ack(const frame802154_t *frame)
{
  uint8_t buffer[3];
  /* FCF: 2 octets */
  buffer[0] = FRAME802154_ACKFRAME;
  buffer[1] = 0;
  /* Seqnum: 1 octets */
  buffer[2] = frame->seq;
  in_ack_transmission = 1;
  send(&buffer, sizeof(buffer));
  in_ack_transmission = 0;
}
/*---------------------------------------------------------------------------*/
/* Check if a packet is for us */
static int
is_packet_for_us(uint8_t *buf, int len, int do_send_ack)
{
  frame802154_t frame;
  int result;
  uint8_t parsed = frame802154_parse(buf, len, &frame);
  if(parsed) {
    if(frame.fcf.dest_addr_mode) {
      int has_dest_panid;
      frame802154_has_panid(&frame.fcf, NULL, &has_dest_panid);
      if(has_dest_panid
         && frame802154_get_pan_id() != FRAME802154_BROADCASTPANDID
         && frame.dest_pid != frame802154_get_pan_id()
         && frame.dest_pid != FRAME802154_BROADCASTPANDID) {
        /* Packet to another PAN */
        return 0;
      }
      if(!is_broadcast_addr(frame.fcf.dest_addr_mode, frame.dest_addr)) {
        result = linkaddr_cmp((linkaddr_t *)frame.dest_addr, &linkaddr_node_addr);
        if(autoack_enabled && result && do_send_ack) {
          /* this is a unicast frame and sending ACKs is enabled */
          send_ack(&frame);
        }
        return result;
      }
    }
    return 1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static int
read(void *buf, unsigned short bufsize)
{
  int len = 0;
  uint16_t radio_last_rx_crc;
  uint8_t radio_last_rx_crc_ok = 1;

  len = input_frame_buffer->u8PayloadLength;

  if(len <= CHECKSUM_LEN) {
    input_frame_buffer->u8PayloadLength = 0;
    return 0;
  } else {
    len -= CHECKSUM_LEN;
    /* Check CRC */
#if CRC_SW
    uint16_t checksum = crc16_data(input_frame_buffer->uPayload.au8Byte, len, 0);
    radio_last_rx_crc =
      (uint16_t)(input_frame_buffer->uPayload.au8Byte[len + 1] << (uint16_t)8)
      | input_frame_buffer->uPayload.au8Byte[len];
    radio_last_rx_crc_ok = (checksum == radio_last_rx_crc);
    if(!radio_last_rx_crc_ok) {
      RIMESTATS_ADD(badcrc);
    }
#endif /* CRC_SW */
    if(radio_last_rx_crc_ok) {
      /* If we are in poll mode we need to check the frame here */
      if(poll_mode) {
        if(frame_filtering &&
           !is_packet_for_us(input_frame_buffer->uPayload.au8Byte, len, 0)) {
          len = 0;
        } else {
          read_last_rssi();
        }
      }
      if(len != 0) {
        bufsize = MIN(len, bufsize);
        memcpy(buf, input_frame_buffer->uPayload.au8Byte, bufsize);
        RIMESTATS_ADD(llrx);
        if(!poll_mode) {
          /* Not in poll mode: packetbuf should not be accessed in interrupt context */
          packetbuf_set_attr(PACKETBUF_ATTR_RSSI, radio_last_rssi);
          packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, radio_last_correlation);
        }
      }
    } else {
      len = 0;
    }
    /* Disable further read attempts */
    input_frame_buffer->u8PayloadLength = 0;
  }

  return len;
}
/*---------------------------------------------------------------------------*/
static void
set_txpower(int8_t power)
{
  if(power > OUTPUT_POWER_MAX) {
    current_tx_power = OUTPUT_POWER_MAX;
  } else {
    if(power < OUTPUT_POWER_MIN) {
      current_tx_power = OUTPUT_POWER_MIN;
    } else {
      current_tx_power = power;
    }
  }
  vMMAC_SetChannelAndPower(current_channel, current_tx_power);
}
/*--------------------------------------------------------------------------*/
static int
get_txpower(void)
{
  int actual_tx_power;
#if (JENNIC_CHIP == JN5169)
  /* Actual tx power value rounded to nearest integer number */
  const static int8 power_table [] = {
    -32, -30, -29, -29,   /* -32 .. -29 */ 
    -28, -28, -28, -28,   /* -28 .. -25 */
    -21, -21, -21,  -2,   /* -24 .. -21 */
    -20, -19, -18, -17,   /* -20 .. -17 */
    -17, -17, -17, -10,   /* -16 .. -13 */
    -10, -10, -10,  -9,   /* -12 .. -09 */
     -8,  -7,  -6,  -6,   /* -08 .. -05 */
     -6,  -6,   1,   1,   /* -04 .. -01 */
      1,   1,   2,   3,   /*  00 ..  03 */
      4,   5,   6,   7,   /*  04 ..  07 */
      9,   9,  10 };      /*  08 ..  10 */
  if(current_tx_power > OUTPUT_POWER_MAX) {
    actual_tx_power = OUTPUT_POWER_MAX;
  } else if(current_tx_power < OUTPUT_POWER_MIN) {
    actual_tx_power = OUTPUT_POWER_MIN;
  } else {
    actual_tx_power = power_table[current_tx_power + ABS_OUTPUT_POWER_MIN];
  }
#else
  /* Other JN516x chips */
  if(current_tx_power < (-24)) {
    actual_tx_power = OUTPUT_POWER_MIN;
  } else if(current_tx_power < (-12)) {
    actual_tx_power = (-20);
  } else if(current_tx_power < 0) {
    actual_tx_power = (-9);
  } else {
    actual_tx_power = OUTPUT_POWER_MAX;
  }
#endif
  return (int)actual_tx_power;
}
/*---------------------------------------------------------------------------*/
static int
get_detected_energy(void)
{
  const uint32 u32Samples = 8;
  return u8JPT_EnergyDetect(current_channel, u32Samples);
}
/*---------------------------------------------------------------------------*/
static int
get_rssi(void)
{
  /* this approximate formula for RSSI is taken from NXP internal docs */
  return (7 * get_detected_energy() - 1970) / 20;
}
/*---------------------------------------------------------------------------*/
static void
read_last_rssi(void)
{
  uint8_t radio_last_rx_energy;
  radio_last_rx_energy = u8MMAC_GetRxLqi((uint8_t *)&radio_last_correlation);
  radio_last_rssi = i16JPT_ConvertEnergyTodBm(radio_last_rx_energy);
}
/*---------------------------------------------------------------------------*/
int
receiving_packet(void)
{
  return bMMAC_RxDetected();
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  if(!poll_mode) {
    return ringbufindex_peek_get(&input_ringbuf) != -1;
  } else {
    return u32MMAC_PollInterruptSource(
             E_MMAC_INT_RX_COMPLETE | E_MMAC_INT_RX_HEADER);
  }
}
/*---------------------------------------------------------------------------*/
static int
cca(void)
{
  bool_t is_channel_busy = bJPT_CCA(current_channel,
                                    E_JPT_CCA_MODE_CARRIER_OR_ENERGY,
                                    cca_thershold);
  return is_channel_busy == FALSE;
}
/*---------------------------------------------------------------------------*/
static void
radio_interrupt_handler(uint32 mac_event)
{
  uint32_t rx_status;
  uint8_t overflow = 0;
  int get_index;
  int put_index;
  int packet_for_me = 0;

  if(mac_event & E_MMAC_INT_TX_COMPLETE) {
    /* Transmission attempt has finished */
    tx_in_progress = 0;
  } else if(mac_event & E_MMAC_INT_RX_COMPLETE) {
    rx_status = u32MMAC_GetRxErrors();
    /* If rx is successful */
    if(rx_status == 0) {
      /* Save SFD timestamp */
      last_packet_timestamp = get_packet_timestamp();

      if(!poll_mode && (mac_event & E_MMAC_INT_RX_COMPLETE)) {
        if(rx_frame_buffer->u8PayloadLength > CHECKSUM_LEN) {
          if(frame_filtering) {
            /* Check RX address */
            packet_for_me = is_packet_for_us(rx_frame_buffer->uPayload.au8Byte, rx_frame_buffer->u8PayloadLength - CHECKSUM_LEN, 1);
          } else if(!frame_filtering) {
            packet_for_me = 1;
          }
        }
        if(!packet_for_me) {
          /* Prevent reading */
          rx_frame_buffer->u8PayloadLength = 0;
        } else {
          /* read and cache RSSI and LQI values */
          read_last_rssi();
          /* Put received frame in queue */
          ringbufindex_put(&input_ringbuf);

          if((get_index = ringbufindex_peek_get(&input_ringbuf)) != -1) {
            input_frame_buffer = &input_array[get_index];
          }
          process_poll(&micromac_radio_process);

          /* get pointer to next input slot */
          put_index = ringbufindex_peek_put(&input_ringbuf);
          /* is there space? */
          if(put_index != -1) {
            /* move rx_frame_buffer to next empty slot */
            rx_frame_buffer = &input_array[put_index];
          } else {
            overflow = 1;
            rx_frame_buffer = NULL;
          }
        }
      }
    } else { /* if rx is not successful */
      if(rx_status & E_MMAC_RXSTAT_ABORTED) {
        RIMESTATS_ADD(badsynch);
      } else if(rx_status & E_MMAC_RXSTAT_ERROR) {
        RIMESTATS_ADD(badcrc);
      } else if(rx_status & E_MMAC_RXSTAT_MALFORMED) {
        RIMESTATS_ADD(toolong);
      }
    }
  }
  if(overflow) {
    off();
  } else if(MICROMAC_CONF_ALWAYS_ON
            && (mac_event & (E_MMAC_INT_TX_COMPLETE | E_MMAC_INT_RX_COMPLETE))) {
    on();
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(micromac_radio_process, ev, data)
{
  PROCESS_BEGIN();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    /* Pass received packets to upper layer */
    int16_t read_index;
    /* Loop on accessing (without removing) a pending input packet */
    while((read_index = ringbufindex_peek_get(&input_ringbuf)) != -1) {
      input_frame_buffer = &input_array[read_index];
      /* Put packet into packetbuf for input callback */
      packetbuf_clear();
      int len = read(packetbuf_dataptr(), PACKETBUF_SIZE);
      /* is packet valid? */
      if(len > 0) {
        packetbuf_set_datalen(len);
        NETSTACK_RDC.input();
      }
      /* Remove packet from ringbuf */
      ringbufindex_get(&input_ringbuf);
      /* Disable further read attempts */
      input_frame_buffer->u8PayloadLength = 0;
    }

    /* Are we recovering from overflow? */
    if(rx_frame_buffer == NULL) {
      /* get pointer to next input slot */
      int put_index = ringbufindex_peek_put(&input_ringbuf);
      /* is there space? */
      if(put_index != -1) {
        /* move rx_frame_buffer to next empty slot */
        rx_frame_buffer = &input_array[put_index];
        /* do we need to turn radio on? */
        if(MICROMAC_CONF_ALWAYS_ON || missed_radio_on_request) {
          missed_radio_on_request = 0;
          on();
        }
      } else {
        rx_frame_buffer = NULL;
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
set_frame_filtering(uint8_t enable)
{
  frame_filtering = enable;
}
/*---------------------------------------------------------------------------*/
static void
set_autoack(uint8_t enable)
{
  autoack_enabled = enable;
}
/*---------------------------------------------------------------------------*/
static void
set_poll_mode(uint8_t enable)
{
  poll_mode = enable;
  if(poll_mode) {
    /* Disable interrupts */
    vMMAC_EnableInterrupts(NULL);
    vMMAC_ConfigureInterruptSources(0);
  } else {
    /* Initialize and enable interrupts */
    /* TODO: enable E_MMAC_INT_RX_HEADER & filter out frames after header rx */
    vMMAC_ConfigureInterruptSources(
      E_MMAC_INT_RX_COMPLETE | E_MMAC_INT_TX_COMPLETE);
    vMMAC_EnableInterrupts(&radio_interrupt_handler);
  }
}
/* Enable or disable CCA before sending */
static void
set_send_on_cca(uint8_t enable)
{
  send_on_cca = enable;
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
    *value = listen_on || tx_in_progress ? RADIO_POWER_MODE_ON : RADIO_POWER_MODE_OFF;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = get_channel();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    *value = 0;
    if(frame_filtering) {
      *value |= RADIO_RX_MODE_ADDRESS_FILTER;
    }
    if(autoack_enabled) {
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
    *value = get_txpower();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = get_rssi();
    return RADIO_RESULT_OK;
  case RADIO_PARAM_LAST_RSSI:
    *value = radio_last_rssi;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    *value = cca_thershold;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = 11;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = 26;
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
    if(value < 11 || value > 26) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    set_channel(value);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RX_MODE:
    if(value & ~(RADIO_RX_MODE_ADDRESS_FILTER |
                 RADIO_RX_MODE_AUTOACK | RADIO_RX_MODE_POLL_MODE)) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    set_frame_filtering((value & RADIO_RX_MODE_ADDRESS_FILTER) != 0);
    set_autoack((value & RADIO_RX_MODE_AUTOACK) != 0);
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
      /* Find the closest higher PA_LEVEL for the desired output power */
    }
    set_txpower(value);
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CCA_THRESHOLD:
    cca_thershold = value;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}
/*---------------------------------------------------------------------------*/
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  if(param == RADIO_PARAM_LAST_PACKET_TIMESTAMP) {
    if(size != sizeof(rtimer_clock_t) || !dest) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    *(rtimer_clock_t *)dest = get_packet_timestamp();

    return RADIO_RESULT_OK;
  }
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver micromac_radio_driver = {
  init,
  prepare,
  transmit,
  send,
  read,
  cca,
  receiving_packet,
  pending_packet,
  on,
  off,
  get_value,
  set_value,
  get_object,
  set_object
};
