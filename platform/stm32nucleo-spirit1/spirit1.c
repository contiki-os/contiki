/*
 * Copyright (c) 2012, STMicroelectronics.
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
#include "spirit1.h"
#include "spirit1-arch.h"
#include "stm32l1xx.h"
#include "contiki.h"
#include "net/mac/frame802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "spirit1-arch.h"
#include <stdio.h>
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
/* MGR extern st_lib_spirit_irqs st_lib_x_irq_status; */
extern volatile st_lib_spirit_flag_status rx_timeout;
/*---------------------------------------------------------------------------*/
#define XXX_ACK_WORKAROUND 1
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
#define CLEAR_TXBUF()           (spirit_txbuf[0] = 0)
#define CLEAR_RXBUF()           (spirit_rxbuf[0] = 0)
#define IS_TXBUF_EMPTY()        (spirit_txbuf[0] == 0)
#define IS_RXBUF_EMPTY()        (spirit_rxbuf[0] == 0)
#define IS_RXBUF_FULL()         (spirit_rxbuf[0] != 0)
/*---------------------------------------------------------------------------*/
/* transceiver state. */
#define ON     0
#define OFF    1
/*---------------------------------------------------------------------------*/
static volatile unsigned int spirit_on = OFF;
static volatile uint8_t receiving_packet = 0;
static packetbuf_attr_t last_rssi = 0;  /* MGR */
static packetbuf_attr_t last_lqi = 0;  /* MGR */
/*---------------------------------------------------------------------------*/
/*
 * The buffers which hold incoming data.
 * The +1 because of the first byte,
 * which will contain the length of the packet.
 */
static uint8_t spirit_rxbuf[MAX_PACKET_LEN + 1];
static uint8_t spirit_txbuf[MAX_PACKET_LEN + 1 - SPIRIT_MAX_FIFO_LEN];
void st_lib_spirit_management_set_frequency_base(uint32_t);
/*---------------------------------------------------------------------------*/
static int just_got_an_ack = 0; /* Interrupt callback just detected an ack */
#if NULLRDC_CONF_802154_AUTOACK
#define ACK_LEN 3
static int wants_an_ack = 0; /* The packet sent expects an ack */
/* static int just_got_an_ack = 0; / * Interrupt callback just detected an ack * / */
/* #define ACKPRINTF printf */
#define ACKPRINTF(...)
#endif /* NULLRDC_CONF_802154_AUTOACK */
/*---------------------------------------------------------------------------*/
static int packet_is_prepared = 0;
/*---------------------------------------------------------------------------*/
PROCESS(spirit_radio_process, "SPIRIT radio driver");
/*---------------------------------------------------------------------------*/
static int spirit_radio_init(void);
static int spirit_radio_prepare(const void *payload, unsigned short payload_len);
static int spirit_radio_transmit(unsigned short payload_len);
static int spirit_radio_send(const void *data, unsigned short len);
static int spirit_radio_read(void *buf, unsigned short bufsize);
static int spirit_radio_channel_clear(void);
static int spirit_radio_receiving_packet(void);
static int spirit_radio_pending_packet(void);
static int spirit_radio_on(void);
static int spirit_radio_off(void);
/*---------------------------------------------------------------------------*/
const struct radio_driver spirit_radio_driver =
{
  spirit_radio_init,
  spirit_radio_prepare,
  spirit_radio_transmit,
  spirit_radio_send,
  spirit_radio_read,
  spirit_radio_channel_clear,
  spirit_radio_receiving_packet,
  spirit_radio_pending_packet,
  spirit_radio_on,
  spirit_radio_off,
};
/*---------------------------------------------------------------------------*/
void
spirit1_printstatus(void)
{
  int s = SPIRIT1_STATUS();
  if(s == SPIRIT1_STATE_STANDBY) {
    printf("spirit1: SPIRIT1_STATE_STANDBY\n");
  } else if(s == SPIRIT1_STATE_READY) {
    printf("spirit1: SPIRIT1_STATE_READY\n");
  } else if(s == SPIRIT1_STATE_TX) {
    printf("spirit1: SPIRIT1_STATE_TX\n");
  } else if(s == SPIRIT1_STATE_RX) {
    printf("spirit1: SPIRIT1_STATE_RX\n");
  } else {
    printf("spirit1: status: %d\n", s);
  }
}
/*---------------------------------------------------------------------------*/
/* Strobe a command. The rationale for this is to clean up the messy legacy code. */
static void
spirit1_strobe(uint8_t s)
{
  st_lib_spirit_cmd_strobe_command(s);
}
/*---------------------------------------------------------------------------*/
void
spirit_set_ready_state(void)
{
  PRINTF("READY IN\n");

  st_lib_spirit_irq_clear_status();
  IRQ_DISABLE();

  if(SPIRIT1_STATUS() == SPIRIT1_STATE_STANDBY) {
    spirit1_strobe(SPIRIT1_STROBE_READY);
  } else if(SPIRIT1_STATUS() == SPIRIT1_STATE_RX) {
    spirit1_strobe(SPIRIT1_STROBE_SABORT);
    st_lib_spirit_irq_clear_status();
  }

  IRQ_ENABLE();

  PRINTF("READY OUT\n");
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_init(void)
{

  PRINTF("RADIO INIT IN\n");

  st_lib_spirit_spi_init();

  /* Configure radio shut-down (SDN) pin and activate radio */
  st_lib_radio_gpio_init(RADIO_GPIO_SDN, RADIO_MODE_GPIO_OUT);

  /* Configures the SPIRIT1 library */
  st_lib_spirit_radio_set_xtal_frequency(XTAL_FREQUENCY);
  st_lib_spirit_management_set_frequency_base(XTAL_FREQUENCY);

  /* wake up to READY state */
  /* weirdly enough, this *should* actually *set* the pin, not clear it! The pins is declared as GPIO_pin13 == 0x2000 */
  RADIO_GPIO_SDN_PORT->BSRR = RADIO_GPIO_SDN_PIN;
  st_lib_hal_gpio_write_pin(RADIO_GPIO_SDN_PORT, RADIO_GPIO_SDN_PIN, GPIO_PIN_RESET);

  /* wait minimum 1.5 ms to allow SPIRIT1 a proper boot-up sequence */
  BUSYWAIT_UNTIL(0, 3 * RTIMER_SECOND / 2000);

  /* Soft reset of core */
  spirit1_strobe(SPIRIT1_STROBE_SRES);

  /* Configures the SPIRIT1 radio part */
  st_lib_s_radio_init x_radio_init = {
    /* XTAL_FREQUENCY, */
    XTAL_OFFSET_PPM,
    BASE_FREQUENCY,
    CHANNEL_SPACE,
    CHANNEL_NUMBER,
    MODULATION_SELECT,
    DATARATE,
    FREQ_DEVIATION,
    BANDWIDTH
  };
  st_lib_spirit_radio_init(&x_radio_init);
  st_lib_spirit_radio_set_xtal_frequency(XTAL_FREQUENCY);
  st_lib_spirit_radio_set_pa_level_dbm(0, POWER_DBM);
  st_lib_spirit_radio_set_pa_level_max_index(0);

  /* Configures the SPIRIT1 packet handler part*/
  st_lib_pkt_basic_init x_basic_init = {
    PREAMBLE_LENGTH,
    SYNC_LENGTH,
    SYNC_WORD,
    LENGTH_TYPE,
    LENGTH_WIDTH,
    CRC_MODE,
    CONTROL_LENGTH,
    EN_ADDRESS,
    EN_FEC,
    EN_WHITENING
  };
  st_lib_spirit_pkt_basic_init(&x_basic_init);

  /* Enable the following interrupt sources, routed to GPIO */
  st_lib_spirit_irq_de_init(NULL);
  st_lib_spirit_irq_clear_status();
  st_lib_spirit_irq(TX_DATA_SENT, S_ENABLE);
  st_lib_spirit_irq(RX_DATA_READY, S_ENABLE);
  st_lib_spirit_irq(VALID_SYNC, S_ENABLE);
  st_lib_spirit_irq(RX_DATA_DISC, S_ENABLE);
  st_lib_spirit_irq(TX_FIFO_ERROR, S_ENABLE);
  st_lib_spirit_irq(RX_FIFO_ERROR, S_ENABLE);

  /* Configure Spirit1 */
  st_lib_spirit_radio_persisten_rx(S_ENABLE);
  st_lib_spirit_qi_set_sqi_threshold(SQI_TH_0);
  st_lib_spirit_qi_sqi_check(S_ENABLE);
  st_lib_spirit_qi_set_rssi_threshold_dbm(CCA_THRESHOLD);
  st_lib_spirit_timer_set_rx_timeout_stop_condition(SQI_ABOVE_THRESHOLD);
  SET_INFINITE_RX_TIMEOUT();
  st_lib_spirit_radio_afc_freeze_on_sync(S_ENABLE);

  /* Puts the SPIRIT1 in STANDBY mode (125us -> rx/tx) */
  spirit1_strobe(SPIRIT1_STROBE_STANDBY);
  spirit_on = OFF;
  CLEAR_RXBUF();
  CLEAR_TXBUF();

  /* Initializes the mcu pin as input, used for IRQ */
  st_lib_radio_gpio_init(RADIO_GPIO_IRQ, RADIO_MODE_EXTI_IN);

  /* Configure the radio to route the IRQ signal to its GPIO 3 */
  st_lib_spirit_gpio_init(&(st_lib_s_gpio_init){SPIRIT_GPIO_IRQ, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP, SPIRIT_GPIO_DIG_OUT_IRQ });

  process_start(&spirit_radio_process, NULL);

  PRINTF("Spirit1 init done\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_prepare(const void *payload, unsigned short payload_len)
{
  PRINTF("Spirit1: prep %u\n", payload_len);
  packet_is_prepared = 0;

  /* Checks if the payload length is supported */
  if(payload_len > MAX_PACKET_LEN) {
    return RADIO_TX_ERR;
  }

  /* Should we delay for an ack? */
#if NULLRDC_CONF_802154_AUTOACK
  frame802154_t info154;
  wants_an_ack = 0;
  if(payload_len > ACK_LEN
     && frame802154_parse((char *)payload, payload_len, &info154) != 0) {
    if(info154.fcf.frame_type == FRAME802154_DATAFRAME
       && info154.fcf.ack_required != 0) {
      wants_an_ack = 1;
    }
  }
#endif /* NULLRDC_CONF_802154_AUTOACK */

  /* Sets the length of the packet to send */
  IRQ_DISABLE();
  spirit1_strobe(SPIRIT1_STROBE_FTX);
  st_lib_spirit_pkt_basic_set_payload_length(payload_len);
  st_lib_spirit_spi_write_linear_fifo(payload_len, (uint8_t *)payload);
  IRQ_ENABLE();

  PRINTF("PREPARE OUT\n");

  packet_is_prepared = 1;
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_transmit(unsigned short payload_len)
{
  /* This function blocks until the packet has been transmitted */
  rtimer_clock_t rtimer_txdone, rtimer_rxack;

  PRINTF("TRANSMIT IN\n");
  if(!packet_is_prepared) {
    return RADIO_TX_ERR;
  }

  /* Stores the length of the packet to send */
  /* Others spirit_radio_prepare will be in hold */
  spirit_txbuf[0] = payload_len;

  /* Puts the SPIRIT1 in TX state */
  receiving_packet = 0;
  spirit_set_ready_state();
  spirit1_strobe(SPIRIT1_STROBE_TX);
  just_got_an_ack = 0;
  BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_TX, 1 * RTIMER_SECOND / 1000);
  /* BUSYWAIT_UNTIL(SPIRIT1_STATUS() != SPIRIT1_STATE_TX, 4 * RTIMER_SECOND/1000); //For GFSK with high data rate */
  BUSYWAIT_UNTIL(SPIRIT1_STATUS() != SPIRIT1_STATE_TX, 50 * RTIMER_SECOND / 1000); /* For FSK with low data rate */

  /* Reset radio - needed for immediate RX of ack */
  CLEAR_TXBUF();
  CLEAR_RXBUF();
  IRQ_DISABLE();
  st_lib_spirit_irq_clear_status();
  spirit1_strobe(SPIRIT1_STROBE_SABORT);
  BUSYWAIT_UNTIL(0, RTIMER_SECOND / 2500);
  spirit1_strobe(SPIRIT1_STROBE_READY);
  BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_READY, 1 * RTIMER_SECOND / 1000);
  spirit1_strobe(SPIRIT1_STROBE_RX);
  BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_RX, 1 * RTIMER_SECOND / 1000);
  IRQ_ENABLE();

#if XXX_ACK_WORKAROUND
  just_got_an_ack = 1;
#endif /* XXX_ACK_WORKAROUND */

#if NULLRDC_CONF_802154_AUTOACK
  if(wants_an_ack) {
    rtimer_txdone = RTIMER_NOW();
    BUSYWAIT_UNTIL(just_got_an_ack, 2 * RTIMER_SECOND / 1000);
    rtimer_rxack = RTIMER_NOW();

    if(just_got_an_ack) {
      ACKPRINTF("debug_ack: ack received after %u/%u ticks\n",
                (uint32_t)(rtimer_rxack - rtimer_txdone), 2 * RTIMER_SECOND / 1000);
    } else {
      ACKPRINTF("debug_ack: no ack received\n");
    }
  }
#endif /* NULLRDC_CONF_802154_AUTOACK */

  PRINTF("TRANSMIT OUT\n");

  CLEAR_TXBUF();

  packet_is_prepared = 0;

  clock_wait(1);

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_send(const void *payload, unsigned short payload_len)
{
  if(spirit_radio_prepare(payload, payload_len) == RADIO_TX_ERR) {
    return RADIO_TX_ERR;
  }
  return spirit_radio_transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_read(void *buf, unsigned short bufsize)
{
  PRINTF("READ IN\n");

  /* Checks if the RX buffer is empty */
  if(IS_RXBUF_EMPTY()) {
    IRQ_DISABLE();
    CLEAR_RXBUF();
    spirit1_strobe(SPIRIT1_STROBE_SABORT);
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 2500);
    spirit1_strobe(SPIRIT1_STROBE_READY);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_READY, 1 * RTIMER_SECOND / 1000);
    spirit1_strobe(SPIRIT1_STROBE_RX);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_RX, 1 * RTIMER_SECOND / 1000);
    PRINTF("READ OUT RX BUF EMPTY\n");
    IRQ_ENABLE();
    return 0;
  }

  if(bufsize < spirit_rxbuf[0]) {
    /* If buf has the correct size */
    PRINTF("TOO SMALL BUF\n");
    return 0;
  } else {
    /* Copies the packet received */
    memcpy(buf, spirit_rxbuf + 1, spirit_rxbuf[0]);

    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);        /* MGR */
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, last_lqi); /* MGR */
    bufsize = spirit_rxbuf[0];
    CLEAR_RXBUF();

    PRINTF("READ OUT\n");

    return bufsize;
  }
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_channel_clear(void)
{
  float rssi_value;
  /* Local variable used to memorize the SPIRIT1 state */
  uint8_t spirit_state = ON;

  PRINTF("CHANNEL CLEAR IN\n");

  if(spirit_on == OFF) {
    /* Wakes up the SPIRIT1 */
    spirit_radio_on();
    spirit_state = OFF;
  }

  /*  */
  IRQ_DISABLE();
  spirit1_strobe(SPIRIT1_STROBE_SABORT);
/*  SpiritCmdStrobeSabort();*/
  st_lib_spirit_irq_clear_status();
  IRQ_ENABLE();
  {
    rtimer_clock_t timeout = RTIMER_NOW() + 5 * RTIMER_SECOND / 1000;
    do {
      st_lib_spirit_refresh_status();
    } while((st_lib_g_x_status.MC_STATE != MC_STATE_READY) && (RTIMER_NOW() < timeout));
    if(RTIMER_NOW() < timeout) {
      return 1;
    }
  }

  /* Stores the RSSI value */
  rssi_value = st_lib_spirit_qi_get_rssi_dbm();

  /* Puts the SPIRIT1 in its previous state */
  if(spirit_state == OFF) {
    spirit_radio_off();
  } else {
    spirit1_strobe(SPIRIT1_STROBE_RX);
/*    SpiritCmdStrobeRx();*/
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_RX, 5 * RTIMER_SECOND / 1000);
  }

  PRINTF("CHANNEL CLEAR OUT\n");

  /* Checks the RSSI value with the threshold */
  if(rssi_value < CCA_THRESHOLD) {
    return 0;
  } else {
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_receiving_packet(void)
{
  return receiving_packet;
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_pending_packet(void)
{
  PRINTF("PENDING PACKET\n");
  return !IS_RXBUF_EMPTY();
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_off(void)
{
  PRINTF("Spirit1: ->off\n");
  if(spirit_on == ON) {
    /* Disables the mcu to get IRQ from the SPIRIT1 */
    IRQ_DISABLE();

    /* first stop rx/tx */
    spirit1_strobe(SPIRIT1_STROBE_SABORT);

    /* Clear any pending irqs */
    st_lib_spirit_irq_clear_status();

    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_READY, 5 * RTIMER_SECOND / 1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_READY) {
      PRINTF("Spirit1: failed off->ready\n");
      return 1;
    }

    /* Puts the SPIRIT1 in STANDBY */
    spirit1_strobe(SPIRIT1_STROBE_STANDBY);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_STANDBY, 5 * RTIMER_SECOND / 1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_STANDBY) {
      PRINTF("Spirit1: failed off->stdby\n");
      return 1;
    }

    spirit_on = OFF;
    CLEAR_TXBUF();
    CLEAR_RXBUF();
  }
  PRINTF("Spirit1: off.\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_on(void)
{

  PRINTF("Spirit1: on\n");
  spirit1_strobe(SPIRIT1_STROBE_SABORT);
  BUSYWAIT_UNTIL(0, RTIMER_SECOND / 2500);
  if(spirit_on == OFF) {
    IRQ_DISABLE();
    /* ensure we are in READY state as we go from there to Rx */
    spirit1_strobe(SPIRIT1_STROBE_READY);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_READY, 5 * RTIMER_SECOND / 1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_READY) {
      PRINTF("Spirit1: failed to turn on\n");
      while(1) ;
      /* return 1; */
    }

    /* now we go to Rx */
    spirit1_strobe(SPIRIT1_STROBE_RX);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_RX, 5 * RTIMER_SECOND / 1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_RX) {
      PRINTF("Spirit1: failed to enter rx\n");
      while(1) ;
      /* return 1; */
    }

    /* Enables the mcu to get IRQ from the SPIRIT1 */
    IRQ_ENABLE();
    spirit_on = ON;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int interrupt_callback_in_progress = 0;
static int interrupt_callback_wants_poll = 0;
PROCESS_THREAD(spirit_radio_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Spirit1: process started\n");

  while(1) {
    int len;

    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    PRINTF("Spirit1: polled\n");

    packetbuf_clear();
    len = spirit_radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {

#if NULLRDC_CONF_802154_AUTOACK
      /* Check if the packet has an ACK request */
      frame802154_t info154;
      if(len > ACK_LEN &&
         frame802154_parse((char *)packetbuf_dataptr(), len, &info154) != 0) {
        if(info154.fcf.frame_type == FRAME802154_DATAFRAME &&
           info154.fcf.ack_required != 0 &&
           linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
                        &linkaddr_node_addr)) {

#if !XXX_ACK_WORKAROUND
          /* Send an ACK packet */
          uint8_t ack_frame[ACK_LEN] = {
            FRAME802154_ACKFRAME,
            0x00,
            info154.seq
          };
          IRQ_DISABLE();
          spirit1_strobe(SPIRIT1_STROBE_FTX);
          st_lib_spirit_pkt_basic_set_payload_length((uint16_t)ACK_LEN);
          st_lib_spirit_spi_write_linear_fifo((uint16_t)ACK_LEN, (uint8_t *)ack_frame);

          spirit_set_ready_state();
          IRQ_ENABLE();
          spirit1_strobe(SPIRIT1_STROBE_TX);
          BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_TX, 1 * RTIMER_SECOND / 1000);
          BUSYWAIT_UNTIL(SPIRIT1_STATUS() != SPIRIT1_STATE_TX, 1 * RTIMER_SECOND / 1000);
          ACKPRINTF("debug_ack: sent ack %d\n", ack_frame[2]);
#endif /* !XXX_ACK_WORKAROUND */
        }
      }
#endif /* NULLRDC_CONF_802154_AUTOACK */

      packetbuf_set_datalen(len);
      NETSTACK_RDC.input();
    }

    if(!IS_RXBUF_EMPTY()) {
      process_poll(&spirit_radio_process);
    }

    if(interrupt_callback_wants_poll) {
      spirit1_interrupt_callback();

      if(SPIRIT1_STATUS() == SPIRIT1_STATE_READY) {
        spirit1_strobe(SPIRIT1_STROBE_RX);
        BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_RX, 1 * RTIMER_SECOND / 1000);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
spirit1_interrupt_callback(void)
{
#define INTPRINTF(...) /* PRINTF */
  st_lib_spirit_irqs x_irq_status;
  if(spirit_spi_busy() || interrupt_callback_in_progress) {
    process_poll(&spirit_radio_process);
    interrupt_callback_wants_poll = 1;
    return;
  }

  interrupt_callback_wants_poll = 0;
  interrupt_callback_in_progress = 1;

  /* get interrupt source from radio */
  st_lib_spirit_irq_get_status(&x_irq_status);
  st_lib_spirit_irq_clear_status();

  if(x_irq_status.IRQ_RX_FIFO_ERROR) {
    receiving_packet = 0;
    interrupt_callback_in_progress = 0;
    spirit1_strobe(SPIRIT1_STROBE_FRX);
    return;
  }

  if(x_irq_status.IRQ_TX_FIFO_ERROR) {
    receiving_packet = 0;
    interrupt_callback_in_progress = 0;
    spirit1_strobe(SPIRIT1_STROBE_FTX);
    return;
  }

  /* The IRQ_VALID_SYNC is used to notify a new packet is coming */
  if(x_irq_status.IRQ_VALID_SYNC) {
    INTPRINTF("SYNC\n");
    receiving_packet = 1;
  }

  /* The IRQ_TX_DATA_SENT notifies the packet received. Puts the SPIRIT1 in RX */
  if(x_irq_status.IRQ_TX_DATA_SENT) {
    spirit1_strobe(SPIRIT1_STROBE_RX);
/*    SpiritCmdStrobeRx();*/
    INTPRINTF("SENT\n");
    CLEAR_TXBUF();
    interrupt_callback_in_progress = 0;
    return;
  }

  /* The IRQ_RX_DATA_READY notifies a new packet arrived */
  if(x_irq_status.IRQ_RX_DATA_READY) {
    st_lib_spirit_spi_read_linear_fifo(st_lib_spirit_linear_fifo_read_num_elements_rx_fifo(),
                                       &spirit_rxbuf[1]);
    spirit_rxbuf[0] = st_lib_spirit_pkt_basic_get_received_pkt_length();
    spirit1_strobe(SPIRIT1_STROBE_FRX);

    INTPRINTF("RECEIVED\n");

    process_poll(&spirit_radio_process);

    last_rssi = (packetbuf_attr_t)st_lib_spirit_qi_get_rssi();  /* MGR */
    last_lqi = (packetbuf_attr_t)st_lib_spirit_qi_get_lqi();    /* MGR */

    receiving_packet = 0;

#if NULLRDC_CONF_802154_AUTOACK
    if(spirit_rxbuf[0] == ACK_LEN) {
      /* For debugging purposes we assume this is an ack for us */
      just_got_an_ack = 1;
    }
#endif /* NULLRDC_CONF_802154_AUTOACK */

    interrupt_callback_in_progress = 0;
    return;
  }

  if(x_irq_status.IRQ_RX_DATA_DISC) {
    /* RX command - to ensure the device will be ready for the next reception */
    if(x_irq_status.IRQ_RX_TIMEOUT) {
      st_lib_spirit_cmd_strobe_flush_rx_fifo();
      rx_timeout = SET;
    }
  }

  interrupt_callback_in_progress = 0;
}
/*---------------------------------------------------------------------------*/
