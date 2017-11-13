/*
 * Copyright (c) 2017, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
#include "st7580.h"
#include "stm32_plm01a1.h"
#include "plm_gpio.h"
#include "stm32l1xx.h"
#include "contiki.h"
#include "net/mac/frame802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
#define BUSYWAIT_UNTIL(cond, max_time) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) ; \
  } while(0)

/*---------------------------------------------------------------------------*/
static volatile uint8_t receiving_packet = 0;

/* RX and TX buffers. Since MAX_PACKET_LEN may be larger than the ST7580 frame lenght,
 * we need to buffer outgoing packets. */
#define MAX_PACKET_LEN PACKETBUF_SIZE
#define MAX_PHY_LEN 255

static volatile uint16_t st7580_rxbuf_len = 0;
static uint8_t st7580_rxbuf[MAX_PACKET_LEN];
static volatile uint8_t st7580_txbuf_has_data = 0;
static uint8_t st7580_txbuf[MAX_PACKET_LEN];
static int count = 0;

/*---------------------------------------------------------------------------*/
PROCESS(st7580_radio_process, "ST7580 radio driver");
/*---------------------------------------------------------------------------*/
static int st7580_radio_init(void);
static int st7580_radio_prepare(const void *payload, unsigned short payload_len);
static int st7580_radio_transmit(unsigned short payload_len);
static int st7580_radio_send(const void *data, unsigned short len);
static int st7580_radio_read(void *buf, unsigned short bufsize);
static int st7580_radio_channel_clear(void);
static int st7580_radio_receiving_packet(void);
static int st7580_radio_pending_packet(void);
static int st7580_radio_on(void);
static int st7580_radio_off(void);
/*----------------------------------------------------------------------------*/
const struct radio_driver st7580_radio_driver =
{
  st7580_radio_init,
  st7580_radio_prepare,
  st7580_radio_transmit,
  st7580_radio_send,
  st7580_radio_read,
  st7580_radio_channel_clear,
  st7580_radio_receiving_packet,
  st7580_radio_pending_packet,
  st7580_radio_on,
  st7580_radio_off,
};

/* ---------------------------------------------------------------------------
 * ST7580 Communication Parameters
 * ---------------------------------------------------------------------------*/

static packetbuf_attr_t last_rssi = 0;

/*----------------------------------------------------------------------------*/
static int
st7580_radio_init(void)
{

  PRINTF("RADIO INIT IN\n");
  /* Physical Reset st7580 */
  PRINTF("Resetting ST7580\n");
  BSP_PLM_Init();

  /* Modem configuration */
  PRINTF("Writing MIB Modem configuration\n");
  BSP_PLM_Mib_Write(MIB_MODEM_CONF, modem_config, sizeof(modem_config));
  /* Phy Configuration */
  PRINTF("Writing MIB Phy configuration\n");
  BSP_PLM_Mib_Write(MIB_PHY_CONF, phy_config, sizeof(phy_config));

  process_start(&st7580_radio_process, NULL);
  initialized = 1;

  PRINTF("ST7580 init done\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_prepare(const void *payload, unsigned short payload_len)
{
  PRINTF("ST7580: prep %u\n", payload_len);
  PRINTF("count: %d\n", count);

  /* Checks if the payload length is supported */
  if(payload_len > MAX_PACKET_LEN) {
    return RADIO_TX_ERR;
  }

  /* Buffer outgoing data */
  memcpy(st7580_txbuf, payload, payload_len);
  st7580_txbuf_has_data = 1;

  /* collision test */
  if(!(st7580_radio_channel_clear())) {
    PRINTF("COLLISION!\n");
    return RADIO_TX_COLLISION;
  }
  /* end collision test */

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_transmit(unsigned short payload_len)
{

  /* This function blocks until the packet has been transmitted */
  int left = payload_len;
  int written = 0;
  int ret;
  static int tot = 0;

  PRINTF("TRANSMIT IN\n");

  while(left > 0) {
    tot += left;
    if(left < MAX_PHY_LEN) {
      PRINTF("min left %d max_phy_len %d tot %d\n", left, MAX_PHY_LEN, tot);
      ret = BSP_PLM_Send_Data(DATA_OPT, &st7580_txbuf[written], (uint8_t)left, NULL);
      if(ret == 0) {
        written += left;
        left = 0;
      } else {
        st7580_txbuf_has_data = 0;
        PRINTF("collision code %d!!!\n\r", ret);
        return RADIO_TX_COLLISION;
      }
    } else {
      PRINTF("mag left %d max_phy_len %d tot %d \n", left, MAX_PHY_LEN, tot);
      ret = BSP_PLM_Send_Data(DATA_OPT, &st7580_txbuf[written], MAX_PHY_LEN, NULL);
      if(ret == 0) {
        written += MAX_PHY_LEN;
        left -= MAX_PHY_LEN;
      } else {
        st7580_txbuf_has_data = 0;
        PRINTF("collision code %d!!!\n", ret);
        return RADIO_TX_COLLISION;
      }
    }
  }
  st7580_txbuf_has_data = 0; /* clear TX buf */

  /* Reset radio - needed for immediate RX of ack */

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_send(const void *payload, unsigned short payload_len)
{
  int ret;

  /* collision test */
  if(!(st7580_radio_channel_clear())) {
    PRINTF("channel not clear!\n");
    return RADIO_TX_COLLISION;
  }
  /* end collision test */

  ret = st7580_radio_prepare(payload, payload_len);
  if((ret == RADIO_TX_ERR) | (ret == RADIO_TX_COLLISION)) {
    return ret;
  }

  return st7580_radio_transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_read(void *buf, unsigned short bufsize)
{
  int len;

  /* Checks if the RX buffer is empty */
  if(st7580_rxbuf_len == 0) {
    PRINTF("READ OUT RX BUF EMPTY\n");
    return 0;
  }

  if(bufsize < st7580_rxbuf_len) {
    st7580_rxbuf_len = 0; /* clear RX buf */
    return 0;
  } else {
    len = st7580_rxbuf_len;
    memcpy(buf, st7580_rxbuf, st7580_rxbuf_len);

    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);
    PRINTF("st7580_radio_read SNR %d\n", last_rssi);

    st7580_rxbuf_len = 0; /* clear RX buf */
  }
  return len;
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_channel_clear(void)
{
  return !(PlmGpioGetLevel(PLM_PL_RX_ON));
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_receiving_packet(void)
{
  return receiving_packet;
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_pending_packet(void)
{
  return st7580_rxbuf_len != 0;
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_off(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
st7580_radio_on(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
void
wrapped_callback(ST7580Frame *frame)
{
  static uint16_t lastCheckSum = 0, dup = 0;

  PRINTF("Inside wrapped callback\n");
  if(frame == NULL) {
    PRINTF("wrapped callback frame null\n");
    return;
  }
  if(!((frame->command == CMD_PHY_DATA_IND) || (frame->command == CMD_DL_DATA_IND))) {
    PRINTF("wrong command %u\n", frame->command);
    return;
  }

  PRINTF("checksum:%d LASTchecksum %d DUP %d\n", frame->checksum, lastCheckSum, dup);

  /* Check to avoid duplicated packets */
  if((frame->stx == ST7580_STX_03) && (lastCheckSum == frame->checksum)) {
    PRINTF("DUPLICATED Deteched!!!\n");
    dup++;
    return;
  } else {
    lastCheckSum = frame->checksum;
  }

  PRINTF("mod: %d\n", frame->data[0]);

  if(st7580_rxbuf_len > 0) {
    /* We have an unprocessed received packet and have to drop the new packet */
    /* Note that this is normal behavior for long packets: we expect to get a
     * RX_DATA_READY interrupt after the RX_FIFO_ALMOST_FULL. */
    process_poll(&st7580_radio_process);
    return;
  }

  receiving_packet = 1;

  memcpy(st7580_rxbuf, &(frame->data[4]), (frame->length - 4));
  st7580_rxbuf_len = frame->length - 4;

  last_rssi = (packetbuf_attr_t)frame->data[1];
  PRINTF("Wrapped callback RSSI %d\n", frame->data[1]);

  if(st7580_rxbuf_len == 0) {
    /* We didn't read the last byte, but aborted prematurely. Discard packet. */
  }

  /* INTPRINTF("RX OK %u %u\n", st7580_rxbuf_len, read); */
  process_poll(&st7580_radio_process);
  receiving_packet = 0;

  return;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(st7580_radio_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("ST7580: process started\n");

  while(1) {
    int len;

    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    PRINTF("ST7580: polled\n");

    packetbuf_clear();

    PRINTF("ST7580: calling st7580_radio_read\n");
    len = st7580_radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {
      packetbuf_set_datalen(len);
      NETSTACK_RDC.input();
    }
    if(st7580_rxbuf_len != 0) {
      process_poll(&st7580_radio_process);
    }
  }

  PROCESS_END();
}

void
st7580_interrupt_callback()
{
  count++;
}
