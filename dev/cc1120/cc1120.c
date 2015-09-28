/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 */

/**
 * \file
 *         TI CC1120 driver.
 */

#include "contiki.h"

/* Radio type sanity check */

#include "cc1120.h"
#include "cc1120-arch.h"

#include "cc1120-const.h"
#include "cc1120-config.h"

#include "dev/leds.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#include <string.h>
#include <stdio.h>

#define DEBUG 0

#if DEBUG
#define LEDS_ON(x)  leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#else /* DEBUG */
#define LEDS_ON(x)
#define LEDS_OFF(x)
#endif /* DEBUG */

#define BUSYWAIT_UNTIL(cond, max_time)                                  \
    do {                                                                  \
      rtimer_clock_t t0;                                                  \
      t0 = RTIMER_NOW();                                                  \
      while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) {/*printf(".");*/} \
    } while(0)

#define RSSI_OFFSET 54
#define AUX_LEN 2
#define ACK_LEN 3

#define CC1120_MAX_PAYLOAD 125 /* Note that FIFO also contains length and footer bytes */

/* Flag indicating whether non-interrupt routines are using SPI */
static volatile uint8_t spi_locked = 0;
#define LOCK_SPI() do { spi_locked++; } while(0)
#define SPI_IS_LOCKED() (spi_locked != 0)
#define RELEASE_SPI() do { spi_locked--; } while(0)

/* CRC errors counter */
uint16_t cc1120_crc_errors = 0;

/* Packet buffer for reception */
static uint8_t packet_tx[CC1120_MAX_PAYLOAD];
static uint8_t packet_rx[1 + CC1120_MAX_PAYLOAD + AUX_LEN];
static volatile uint16_t packet_rx_len = 0;

static volatile uint8_t is_transmitting;

static int request_set_channel = -1;

/* If set, all packets are received, even those not intended for us */
static char promiscuous_mode = 1;

#define PERFORM_MANUAL_CALIBRATION 1 /* Manual calibration at init() */
#if PERFORM_MANUAL_CALIBRATION
static void calibrate_manual(void);
#endif /* PERFORM_MANUAL_CALIBRATION */

/* Prototypes: */
static void reset(void);
static uint8_t state(void);
static unsigned char strobe(uint8_t strobe);
static uint8_t single_write(uint16_t addr, uint8_t value);
static void burst_read(uint16_t addr, uint8_t *buffer, uint8_t count);
static void burst_write(uint16_t addr, uint8_t *buffer, uint8_t count);

static void pollhandler(void);

static int on(void);
static int off(void);

static int init(void);

static void restart_input(void);

static int read_packet(void *buf, unsigned short bufsize);
static int send_packet(const void *data, unsigned short len);
static int prepare( const void *data, unsigned short len );
static int transmit(unsigned short len);

static int receiving_packet(void);
static int pending_packet(void);
static int channel_clear(void);
static signed char rssi_dbm(unsigned char temp) ;
static signed char read_rssi(void);

#if 0
static unsigned char read_lqi(void);
#endif /* 0 */
static void channel_set(unsigned char channel_number);

void cc1120_set_promiscuous(char p);

static int current_channel = 0;

#define MIN(a,b) ((a)<(b)?(a):(b))

#define CARRIER_SENSE_VALID 1 << 1
#define CARRIER_SENSE       1 << 2

#define READ_BIT 0x80
#define WRITE_BIT 0x00
#define BURST_BIT 0x40
#define IS_EXTENDED(x) (x & 0x2F00)

#define OFF 0
#define ON  1
static uint8_t radio_on_or_off = OFF;

/*---------------------------------------------------------------------------*/

PROCESS(cc1120_process, "CC1120 driver");

static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    *value = radio_on_or_off == ON ? RADIO_POWER_MODE_ON : RADIO_POWER_MODE_OFF;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
    *value = current_channel;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = 0;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = 49;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = read_rssi();
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}

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
    if(value < 0 || value > 49) {
      return RADIO_RESULT_INVALID_VALUE;
    }
    channel_set(value);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}

static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}

const struct radio_driver cc1120_driver = {
  init,
  prepare,
  transmit,
  send_packet,
  read_packet,
  channel_clear,
  receiving_packet,
  pending_packet,
  on,
  off,
  get_value,
  set_value,
  get_object,
  set_object,
};

/*---------------------------------------------------------------------------*/
static uint8_t
strobe(uint8_t strobe)
{
  uint8_t ret;
  LOCK_SPI();
  CC1120_ARCH_SPI_ENABLE();
  ret = CC1120_ARCH_SPI_RW_BYTE(strobe);
  CC1120_ARCH_SPI_DISABLE();
  RELEASE_SPI();
  return ret;
}
/*---------------------------------------------------------------------------*/
static uint8_t
single_write(uint16_t addr, uint8_t val)
{
  uint8_t ret;
  LOCK_SPI();
  CC1120_ARCH_SPI_ENABLE();

  if(IS_EXTENDED(addr)) {
    addr &= ~0x2F00;
    CC1120_ARCH_SPI_RW_BYTE(CC1120_EXTENDED_MEMORY_ACCESS | WRITE_BIT);
    CC1120_ARCH_SPI_RW_BYTE(addr);
  } else {
    CC1120_ARCH_SPI_RW_BYTE(addr | WRITE_BIT);
  }

  ret = CC1120_ARCH_SPI_RW_BYTE(val);

  CC1120_ARCH_SPI_DISABLE();
  RELEASE_SPI();
  return ret;
}
/*---------------------------------------------------------------------------*/
static void
burst_read(uint16_t addr, uint8_t *buffer, uint8_t count)
{
  LOCK_SPI();
  CC1120_ARCH_SPI_ENABLE();

  if(IS_EXTENDED(addr)) {
    addr &= ~0x2F00;
    CC1120_ARCH_SPI_RW_BYTE(CC1120_EXTENDED_MEMORY_ACCESS | READ_BIT | BURST_BIT);
    CC1120_ARCH_SPI_RW_BYTE(addr);
  } else {
    CC1120_ARCH_SPI_RW_BYTE(addr | READ_BIT | BURST_BIT);
  }

  CC1120_ARCH_SPI_RW(buffer, NULL, count);

  CC1120_ARCH_SPI_DISABLE();
  RELEASE_SPI();
}
/*---------------------------------------------------------------------------*/
static void
burst_write(uint16_t addr, uint8_t *buffer, uint8_t count)
{
  LOCK_SPI();
  CC1120_ARCH_SPI_ENABLE();

  if(IS_EXTENDED(addr)) {
    addr &= ~0x2F00;
    CC1120_ARCH_SPI_RW_BYTE(CC1120_EXTENDED_MEMORY_ACCESS | WRITE_BIT | BURST_BIT);
    CC1120_ARCH_SPI_RW_BYTE(addr);
  } else {
    CC1120_ARCH_SPI_RW_BYTE(addr | WRITE_BIT | BURST_BIT);
  }

  CC1120_ARCH_SPI_RW(NULL, buffer, count);

  CC1120_ARCH_SPI_DISABLE();
  RELEASE_SPI();
}
/*---------------------------------------------------------------------------*/
static uint8_t
txbytes(void)
{
  uint8_t txbytes;
  burst_read(CC1120_TXBYTES, &txbytes, 1);
  return txbytes;
}
/*---------------------------------------------------------------------------*/
static uint8_t
read_rxbytes(void)
{
  uint8_t rxbytes;
  burst_read(CC1120_RXBYTES, &rxbytes, 1);
  return rxbytes;
}
/*---------------------------------------------------------------------------*/
static void
write_txfifo(uint8_t *data, uint8_t len)
{
  int tosend;
  LOCK_SPI();
  burst_write(CC1120_TXFIFO, &len, 1);

#define FIRST_TX      127
#define SUBSEQUENT_TX 32

  tosend = MIN(len, FIRST_TX);
  burst_write(CC1120_TXFIFO, data, tosend);
  strobe(CC1120_STX);

  do {
    rtimer_clock_t t0;
    t0 = RTIMER_NOW();

    while(txbytes() >= SUBSEQUENT_TX) {
      if(!RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (RTIMER_SECOND / 10))) {
        printf("cc1120: write_txfifo timeout txbytes %d len %d\n", txbytes(), len);
        strobe(CC1120_SFTX);
        strobe(CC1120_SRX);
        break;
      }
      if(txbytes() & 0x80) {
        /* TX FIFO underflow. */
        printf("cc1120.c: write_txfifo txbytes underflow 0x%02x\n", txbytes());
        strobe(CC1120_SFTX);
        strobe(CC1120_SRX);
        break;
      }
    }

    len -= tosend;
    data += tosend;
    tosend = MIN(len, SUBSEQUENT_TX);
    if(tosend > 0) {
      burst_write(CC1120_TXFIFO, data, tosend);
    }
  } while(len > 0);

  RELEASE_SPI();
}
/*---------------------------------------------------------------------------*/
static void
check_txfifo(void)
{
  if(state() == CC1120_STATE_TXFIFO_ERR) {
    /* Acknowledge TX FIFO underflow. */
    printf("cc1120.c: check_txfifo underflow/overflow 0x%02x 0x%02x\n",
           state(), txbytes());
    strobe(CC1120_SFTX);
    strobe(CC1120_SRX);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc1120_process, ev, data)
{

  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

  PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_EXIT);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*
 * process rx packet, when process receives poll
 */
static void
pollhandler(void)
{
  int len;

  do {
    packetbuf_clear();
    len = read_packet(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {
      packetbuf_set_datalen(len);
      /*printf("RDC input %d\n", len);*/
      NETSTACK_RDC.input();
    }

    /* If we were requested to change radio channels but couldn't since the radio
     * was actively sending or receiving data, we try again now. */
    if(request_set_channel >= 0) {
      cc1120_channel_set(request_set_channel);
      request_set_channel = -1;
    }

    /* If we received a packet (or parts thereof) while processing the
       previous packet, we immediately pull it out from the RX
       FIFO. */
    cc1120_rx_interrupt();
  } while(packet_rx_len > 0);
}
/*---------------------------------------------------------------------------*/
static void
flushrx(void)
{
  restart_input();

  LOCK_SPI();
  if(state() == CC1120_STATE_RXFIFO_ERR) {
    strobe(CC1120_SFRX);
  }
  strobe(CC1120_SIDLE);
  BUSYWAIT_UNTIL((state() == CC1120_STATE_IDLE), RTIMER_SECOND / 10);
  strobe(CC1120_SFRX);
  strobe(CC1120_SRX);
  RELEASE_SPI();
}
/*---------------------------------------------------------------------------*/
static void
send_ack(uint8_t seqno)
{
  uint8_t len;
  uint8_t ackdata[ACK_LEN] = {0, 0, 0};

  if(is_transmitting) {
    /* Trying to send an ACK while transmitting - should not be
       possible, so this check is here only to make sure. */
    return;
  }
  ackdata[0] = FRAME802154_ACKFRAME;
  ackdata[1] = 0;
  ackdata[2] = seqno;
  len = ACK_LEN;

  /* Send packet length */

  strobe(CC1120_SIDLE);

  is_transmitting = 1;
  write_txfifo((unsigned char *)ackdata, len);
  check_txfifo();
  BUSYWAIT_UNTIL((state() != CC1120_STATE_TX), RTIMER_SECOND / 10);
  is_transmitting = 0;

#if DEBUG
  printf("^");
#endif /* DEBUG */
}
/*---------------------------------------------------------------------------*/
#define BUFSIZE 256
#define PACKET_LIFETIME (CLOCK_SECOND / 10)
static struct {
  struct pt pt;
  volatile uint8_t receiving;
  int len;
  volatile int ptr;
  uint8_t buf[BUFSIZE];
  struct timer timer;
} rxstate;
/*---------------------------------------------------------------------------*/
static void
restart_input(void)
{
  PT_INIT(&rxstate.pt);
  rxstate.receiving = 0;
}
/*---------------------------------------------------------------------------*/
static int
is_receiving(void)
{
  int receiving;
  LOCK_SPI();
  if(timer_expired(&rxstate.timer)) {
    restart_input();
  }
  receiving = rxstate.receiving;
  RELEASE_SPI();
  return receiving;
}
/*---------------------------------------------------------------------------*/
static int
is_broadcast_addr(uint8_t mode, uint8_t *addr)
{
  int i = mode == FRAME802154_SHORTADDRMODE ? 2 : 8;
  while(i-- > 0) {
    if(addr[i] != 0xff) {
      return 0;
    }
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
input_byte(uint8_t byte)
{
  int crc;

  if(timer_expired(&rxstate.timer)) {
    restart_input();
  }
  PT_BEGIN(&rxstate.pt);

  /* The first incoming byte is the length of the packet. */

  rxstate.receiving = 1;
  rxstate.len = byte;

  if(rxstate.len + AUX_LEN > sizeof(rxstate.buf) || rxstate.len == 0) {
    /*    printf("Bad len %d, state %d rxbytes %d\n", rxstate.len, state(),
          read_rxbytes());*/
    flushrx();
    rxstate.receiving = 0;
    PT_EXIT(&rxstate.pt);
  }

  timer_set(&rxstate.timer, PACKET_LIFETIME);
  for(rxstate.ptr = 0;
      rxstate.ptr < rxstate.len;
      rxstate.ptr++) {

    /* Wait for the next data byte to be received. */
    PT_YIELD(&rxstate.pt);
    if(rxstate.ptr < sizeof(rxstate.buf)) {
      rxstate.buf[rxstate.ptr] = byte;
    }
  }

  /* Receive two more bytes from the FIFO: the RSSI value and the LQI/CRC */
  PT_YIELD(&rxstate.pt);
  rxstate.buf[rxstate.ptr] = byte;
  rxstate.ptr++;
  PT_YIELD(&rxstate.pt);
  rxstate.buf[rxstate.ptr] = byte;
  crc = (byte & 0x80);
  rxstate.ptr++;

  if(crc == 0) {
#if DEBUG
    printf("bad crc\n");
#endif /* DEBUG */

    cc1120_crc_errors++;
    flushrx();
  } else if(rxstate.len < ACK_LEN) {
    /* Drop packets that are way too small: less than ACK_LEN (3) bytes
       long. */
#if DEBUG
    printf("!");
#endif /* DEBUG */

  } else if(packet_rx_len > 0) {

    /* If we have a pending packet in the buffer already, we drop the
       current packet, without sending an ACK. */
    flushrx();
    process_poll(&cc1120_process);

  } else {

    /* Read out the first three bytes to determine if we should send an
       ACK or not. */

#if (THSQ_CONF_NETSTACK & THSQ_CONF_CSL) == THSQ_CONF_CSL
#define IEEE802154E 1
#else /* (THSQ_CONF_NETSTACK & THSQ_CONF_CSL) */
#define IEEE802154E 0
#endif /* (THSQ_CONF_NETSTACK & THSQ_CONF_CSL) */

    if(IEEE802154E) {
      memcpy((void *)packet_rx, rxstate.buf,
             rxstate.len + AUX_LEN);
      packet_rx_len = rxstate.len + AUX_LEN; /* including AUX */
      process_poll(&cc1120_process);

    } else if(rxstate.len >= ACK_LEN) {
      /* Send a link-layer ACK before reading the full packet. */
      /* Try to parse the incoming frame as a 802.15.4 header. */
      frame802154_t info154;
      if(frame802154_parse(rxstate.buf, rxstate.len, &info154) != 0) {
        /* XXX Potential optimization here: we could check if the
           frame is destined for us, or for the broadcast address and
           discard the packet if it isn't for us. */
        if(promiscuous_mode ||
           info154.fcf.frame_type == FRAME802154_ACKFRAME ||
           is_broadcast_addr(FRAME802154_SHORTADDRMODE,
                             (uint8_t *)&info154.dest_addr) ||
           is_broadcast_addr(FRAME802154_LONGADDRMODE,
                             (uint8_t *)&info154.dest_addr) ||
           linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
                        &linkaddr_node_addr)) {

          /* For dataframes that has the ACK request bit set and that
             is destined for us, we send an ack. */
          if(info154.fcf.frame_type == FRAME802154_DATAFRAME &&
             info154.fcf.ack_required != 0 &&
             linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
                          &linkaddr_node_addr)) {
            send_ack(info154.seq);

            /* Make sure that we don't put the radio in the IDLE state
               before the ACK has been fully transmitted. */
            BUSYWAIT_UNTIL((state() != CC1120_STATE_TX), RTIMER_SECOND / 10);
            ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
            ENERGEST_ON(ENERGEST_TYPE_LISTEN);
            if(state() == CC1120_STATE_TX) {
#if DEBUG
              printf("didn't end ack tx (in %d, txbytes %d)\n", state(), txbytes());
#endif /* DEBUG */
              check_txfifo();
              flushrx();
            }
          }

          memcpy((void *)packet_rx, rxstate.buf,
                 rxstate.len + AUX_LEN);
          packet_rx_len = rxstate.len + AUX_LEN; /* including AUX */

          process_poll(&cc1120_process);
#if DEBUG
          printf("#");
#endif /* DEBUG */
        }
      }
    }
  }
  rxstate.receiving = 0;

  PT_END(&rxstate.pt);
}
/*---------------------------------------------------------------------------*/
/**
 * The CC1120 interrupt handler: called by the hardware interrupt
 * handler, which is defined as part of the cc1120-arch interface.
 */
int
cc1120_rx_interrupt(void)
{
  /* NB: This function may be called both from an rx interrupt and
   from cc1120_process */
  uint8_t rxbytes, s;

  if(SPI_IS_LOCKED()) {
#if DEBUG
    printf("/%d", spi_locked);
#endif /* DEBUG */
    process_poll(&cc1120_process);
    return 1;
  }

  /* Lock SPI before we do any SPI operations, in case we are called
     from a non-interrupt context. */
  LOCK_SPI();

  s = state();
  if(s == CC1120_STATE_RXFIFO_ERR) {
    burst_read(CC1120_RXBYTES, &rxbytes, 1);
#if DEBUG
    printf("irqflush\n");
    printf("rxbytes 0x%02x\n", rxbytes);
#endif /* DEBUG */
    flushrx();
    RELEASE_SPI();
    return 1;
  }
  if(s == CC1120_STATE_TXFIFO_ERR) {
#if DEBUG
    printf("irqflushtx\n");
#endif /* DEBUG */
    strobe(CC1120_SFTX);
    strobe(CC1120_SRX);
    RELEASE_SPI();
    return 1;
  }

  if(is_receiving() && timer_expired(&rxstate.timer)) {
#if DEBUG
    printf("Packet expired, flushing fifo\n");
#endif /* DEBUG */
    flushrx();
    RELEASE_SPI();
    return 1;
  }

  /* Read each byte from the RXFIFO and put it into the input_byte()
     function, which takes care of the reception logic. */
  int had_input_bytes = 0;

  do {
    uint8_t byte;
    int i, numbytes;

    numbytes = 0;

    rxbytes = read_rxbytes();

    if(state() == CC1120_STATE_RXFIFO_ERR) {
#if DEBUG
      printf("ovf\n");
#endif /* DEBUG */
      flushrx();
      process_poll(&cc1120_process);
      LEDS_OFF(LEDS_GREEN);
      RELEASE_SPI();
      return 1;
    }

    if(rxbytes == 0) {
      RELEASE_SPI();
      return 1;
    }
    if(rxbytes > 1 + CC1120_MAX_PAYLOAD + AUX_LEN) {
#if DEBUG
      printf("rxbytes too large %d\n", rxbytes);
#endif /* DEBUG */
      flushrx();
      RELEASE_SPI();
      return 1;
    }

    /* Check if we are receiving a packet. If not, we feed the first
       byte of data, which holds the length of the packet, to the
       input function. This will help us later decide how much to read
       from the rx fifo. */
    if(!is_receiving()) {
      burst_read(CC1120_RXFIFO, &byte, 1);
      input_byte(byte);
      rxbytes = read_rxbytes();
    }

    if(is_receiving()) {
      numbytes = rxbytes;
    }

    {
      static uint8_t tmpbuf[CC1120_MAX_PAYLOAD + AUX_LEN + 1];
      if(numbytes > 0 && numbytes <= sizeof(tmpbuf)) {
        burst_read(CC1120_RXFIFO, tmpbuf, numbytes);

        for(i = 0; i < numbytes; i++) {
          byte = tmpbuf[i];
          had_input_bytes = 1;
          input_byte(byte);
        }
      } else if(numbytes != 0) {
#if DEBUG
        printf("numbytes wrong %d\n", numbytes);
#endif /* DEBUG */
        flushrx();
        RELEASE_SPI();
        return 1;
      }
    }

    rxbytes = read_rxbytes();
  } while(rxbytes > 1 && packet_rx_len == 0);

  if(had_input_bytes) {
    process_poll(&cc1120_process);
  }

#if DEBUG
  printf("-");
#endif /* DEBUG */
  RELEASE_SPI();
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Read received packet in RXFIFO.
 */
static int
read_packet(void *buf, unsigned short bufsize)
{
  int len;

  len = 0;

  if(packet_rx_len > 0) {
    signed char rssi;
    signed char lqi;
    signed char crc;
    rssi = rssi_dbm(packet_rx[packet_rx_len - 2]);
    lqi = (packet_rx[packet_rx_len - 1] & 0x7F);
    crc = (packet_rx[packet_rx_len - 1] & 0x80);
    if(crc == 0) {
      packet_rx_len = 0;
#if DEBUG
      printf("Bad crc (0x%02x)\n", lqi | crc);
#endif /* DEBUG */
      cc1120_crc_errors++;
      flushrx();
      return 0;
    }

    len = packet_rx_len - AUX_LEN;
    memcpy(buf, (void *)packet_rx, MIN(len, bufsize));

    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, lqi);

    RIMESTATS_ADD(llrx);

    packet_rx_len = 0;
  }
  return MIN(len, bufsize);
}
/*---------------------------------------------------------------------------*/
/**
 * transmit packet - send packet in buffer
 */
static int
transmit(unsigned short len)
{
  int s;

  if(state() == CC1120_STATE_RXFIFO_ERR) {
    flushrx();
  }

  if(txbytes() != 0) {
    /* There are already bytes in the TXFIFO, which indicates an error somewhere.
     * We flush the TXFIFO and then proceed to send the new packet. */
    strobe(CC1120_SFTX);
    strobe(CC1120_SIDLE);
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 200);
  }

#if DEBUG
  printf("tx len %d\n", len);
#endif /* DEBUG */
  if(len > CC1120_MAX_PAYLOAD) {
#if DEBUG || 1
    printf("cc1120: too big tx %d\n", len);
#endif /* DEBUG */
    return RADIO_TX_ERR;
  }

  RIMESTATS_ADD(lltx);

  LOCK_SPI();
#if DEBUG
  printf(".");
#endif /* DEBUG */
  strobe(CC1120_SIDLE);

  BUSYWAIT_UNTIL(((s = state()) == CC1120_STATE_IDLE), RTIMER_SECOND / 10);
  is_transmitting = 1;
  write_txfifo((unsigned char *)packet_tx, len);
  check_txfifo();
  BUSYWAIT_UNTIL(((s = state()) == CC1120_STATE_TX), RTIMER_SECOND / 10);
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  if(s != CC1120_STATE_TX) {
#if DEBUG
    printf("didn't tx (in %d)\n", state());
#endif /* DEBUG */
    check_txfifo();
    flushrx();
    RELEASE_SPI();
    is_transmitting = 0;
    return RADIO_TX_ERR;
  }
  RELEASE_SPI();
  BUSYWAIT_UNTIL(((s = state()) != CC1120_STATE_TX), RTIMER_SECOND / 10);
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  if(s == CC1120_STATE_TX) {
#if DEBUG
    printf("didn't end tx (in %d, txbytes %d)\n", state(), txbytes());
#endif /* DEBUG */
    check_txfifo();
    flushrx();
    is_transmitting = 0;

    if(radio_on_or_off == OFF) {
      off();
    }

    return RADIO_TX_ERR;
  }
  check_txfifo();
  is_transmitting = 0;

  if(radio_on_or_off == OFF) {
    off();
  }

  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * Prepare function - copy packet into buffer
 */
static int
prepare(const void *payload, unsigned short len)
{
  if(state() == CC1120_STATE_RXFIFO_ERR) {
    flushrx();
  }

  if(len > CC1120_MAX_PAYLOAD) {
#if DEBUG
    printf("CC1120 DEBUG: Too big packet, aborting prepare %d\n", len);
#endif /* DEBUG */
    return RADIO_TX_ERR;
  }

  memcpy(packet_tx, payload, len);
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
/**
 * send function - copy packet into buffer and send.
 */
static int
send_packet(const void *data, unsigned short len)
{
  int ret;
  if(len > CC1120_MAX_PAYLOAD) {
#if DEBUG
    printf("CC1120 DEBUG: Too big packet, aborting send %d\n", len);
#endif /* DEBUG */
    return RADIO_TX_ERR;
  }
  prepare(data, len);
  ret = transmit(len);

  return ret;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  if(SPI_IS_LOCKED()) {
    return 0;
  }

  LOCK_SPI();

  if(radio_on_or_off == ON) {
    RELEASE_SPI();
    return 1;
  }
  radio_on_or_off = ON;

  flushrx();
  strobe(CC1120_SRX);
  RELEASE_SPI();
  BUSYWAIT_UNTIL((state() == CC1120_STATE_RX), RTIMER_SECOND / 100);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  LEDS_ON(LEDS_RED);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  if(SPI_IS_LOCKED()) {
    return 0;
  }

  radio_on_or_off = OFF;

  LOCK_SPI();
  strobe(CC1120_SIDLE);
  BUSYWAIT_UNTIL((state() == CC1120_STATE_IDLE), RTIMER_SECOND / 10);
  strobe(CC1120_SPWD);
  RELEASE_SPI();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  LEDS_OFF(LEDS_RED);

  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * reset radio
 */
static void
reset(void)
{
  LOCK_SPI();
  strobe(CC1120_SRES);
  strobe(CC1120_SNOP); /* XXX needed? */

  /* TODO Move to *-arch.c or smartstudio.c? */
  single_write(CC1120_IOCFG3, CC1120_SETTING_IOCFG3);
  single_write(CC1120_IOCFG2, CC1120_SETTING_IOCFG2);
  single_write(CC1120_IOCFG1, CC1120_SETTING_IOCFG1);
  single_write(CC1120_IOCFG0, CC1120_SETTING_IOCFG0);
  single_write(CC1120_SYNC_CFG1, CC1120_SETTING_SYNC_CFG1);
  single_write(CC1120_DEVIATION_M, CC1120_SETTING_DEVIATION_M);
  single_write(CC1120_MODCFG_DEV_E, CC1120_SETTING_MODCFG_DEV_E);
  single_write(CC1120_DCFILT_CFG, CC1120_SETTING_DCFILT_CFG);
  single_write(CC1120_PREAMBLE_CFG1, CC1120_SETTING_PREAMBLE_CFG1);
  single_write(CC1120_FREQ_IF_CFG, CC1120_SETTING_FREQ_IF_CFG);
  single_write(CC1120_IQIC, CC1120_SETTING_IQIC);
  single_write(CC1120_CHAN_BW, CC1120_SETTING_CHAN_BW);
  single_write(CC1120_MDMCFG0, CC1120_SETTING_MDMCFG0);
#ifdef CC1120_SETTING_DRATE2
  single_write(CC1120_DRATE2, CC1120_SETTING_DRATE2);
  single_write(CC1120_DRATE1, CC1120_SETTING_DRATE1);
  single_write(CC1120_DRATE0, CC1120_SETTING_DRATE0);
#else /* CC1120_SETTING_DRATE2 */
  single_write(CC1120_DRATE2, CC1120_SETTING_SYMBOL_RATE2);
  single_write(CC1120_DRATE1, CC1120_SETTING_SYMBOL_RATE1);
  single_write(CC1120_DRATE0, CC1120_SETTING_SYMBOL_RATE0);
#endif /* CC1120_SETTING_DRATE2 */
  single_write(CC1120_AGC_REF, CC1120_SETTING_AGC_REF);
  single_write(CC1120_AGC_CS_THR, CC1120_SETTING_AGC_CS_THR);
  single_write(CC1120_AGC_CFG3, CC1120_SETTING_AGC_CFG3);
  single_write(CC1120_AGC_CFG2, CC1120_SETTING_AGC_CFG2);
  single_write(CC1120_AGC_CFG1, CC1120_SETTING_AGC_CFG1);
  single_write(CC1120_AGC_CFG0, CC1120_SETTING_AGC_CFG0);
  single_write(CC1120_FIFO_CFG, CC1120_SETTING_FIFO_CFG);
  single_write(CC1120_SETTLING_CFG, CC1120_SETTING_SETTLING_CFG);
  single_write(CC1120_FS_CFG, CC1120_SETTING_FS_CFG);
  single_write(CC1120_PKT_CFG0, CC1120_SETTING_PKT_CFG0);
  single_write(CC1120_RFEND_CFG1, CC1120_SETTING_RFEND_CFG1);
  single_write(CC1120_RFEND_CFG0, CC1120_SETTING_RFEND_CFG0);
  single_write(CC1120_PA_CFG2, CC1120_SETTING_PA_CFG2);
  single_write(CC1120_PA_CFG0, CC1120_SETTING_PA_CFG0);
  single_write(CC1120_PKT_LEN, CC1120_SETTING_PKT_LEN);
  single_write(CC1120_IF_MIX_CFG, CC1120_SETTING_IF_MIX_CFG);
  single_write(CC1120_TOC_CFG, CC1120_SETTING_TOC_CFG);
  single_write(CC1120_FREQ2, CC1120_SETTING_FREQ2);
  single_write(CC1120_FREQ1, CC1120_SETTING_FREQ1);
  single_write(CC1120_FS_DIG1, CC1120_SETTING_FS_DIG1);
  single_write(CC1120_FS_DIG0, CC1120_SETTING_FS_DIG0);
  single_write(CC1120_FS_CAL1, CC1120_SETTING_FS_CAL1);
  single_write(CC1120_FS_CAL0, CC1120_SETTING_FS_CAL0);
  single_write(CC1120_FS_DIVTWO, CC1120_SETTING_FS_DIVTWO);
  single_write(CC1120_FS_DSM0, CC1120_SETTING_FS_DSM0);
  single_write(CC1120_FS_DVC0, CC1120_SETTING_FS_DVC0);
  single_write(CC1120_FS_PFD, CC1120_SETTING_FS_PFD);
  single_write(CC1120_FS_PRE, CC1120_SETTING_FS_PRE);
  single_write(CC1120_FS_REG_DIV_CML, CC1120_SETTING_FS_REG_DIV_CML);
  single_write(CC1120_FS_SPARE, CC1120_SETTING_FS_SPARE);
  single_write(CC1120_FS_VCO0, CC1120_SETTING_FS_VCO0);
  single_write(CC1120_XOSC5, CC1120_SETTING_XOSC5);
  single_write(CC1120_XOSC2, CC1120_SETTING_XOSC2);
  single_write(CC1120_XOSC1, CC1120_SETTING_XOSC1);

  RELEASE_SPI();

  on();
}
/*---------------------------------------------------------------------------*/
/* XXX Placeholders awaiting new radio_driver API */
int
cc1120_channel_get(void)
{
  return current_channel;
}
/*---------------------------------------------------------------------------*/
void
cc1120_channel_set(uint8_t c)
{
  channel_set(c);
}
/*---------------------------------------------------------------------------*/
static void
channel_set(uint8_t c)
{
  if(SPI_IS_LOCKED() || is_transmitting || receiving_packet()) {
#if DEBUG||1
    printf("cannot change channel, radio is busy: spi:%d tx:%d rx:%d\n",
           SPI_IS_LOCKED(), is_transmitting, receiving_packet());
#endif /* DEBUG */
    request_set_channel = c;
    process_poll(&cc1120_process);
    return;
  }
  request_set_channel = -1;

  LOCK_SPI();

  /* XXX Requires FS_CFG.FSD_BANDSELECT == 0010b => LO divider 4 */
  /* XXX Requires SETTLING_CFG.FS_AUTOCAL == 01b */
  uint32_t freq, f_vco, freq_regs;

#define FHSS_ETSI_50    0
#define FHSS_FCC_50     1
#if FHSS_FCC_50 && FHSS_ETSI_50
#error Error: FHSS, both FHSS_ETSI_50 and FHSS_FCC_50 defined. Please set only one.
#endif

#if FHSS_ETSI_50
/* ETSI EN 300 220, 50 channels: Channel 0 863 Mhz, channel 49 869.125MHz */
#define CARRIER_FREQUENCY 13808 /* Channel 0: 863 Mhz (in 16th MHz) */
#define CHANNEL_SPACING 2       /* Channel spacing: 125kHz (in 16th MHz) */
#define FREQ_OSC 512            /* in 16th MHz */
#define LO_DIVIDER 4

#elif FHSS_FCC_50
/* FHSS 902 -- 928 MHz (FCC Part 15.247; 15.249) */
#define CARRIER_FREQUENCY 14432 /* Channel 0: 902 Mhz (in 16th MHz) */
#define CHANNEL_SPACING 2       /* Channel spacing: 125kHz (in 16th MHz) */
#define FREQ_OSC 512            /* in 16th MHz */
#define LO_DIVIDER 4

#else
#error Unknown FHSS frequencies, please define FHSS_ETSI_50 or FHSS_FCC_50
#endif

  freq = CARRIER_FREQUENCY + c*CHANNEL_SPACING; /* Channel -> radio frequency */
  f_vco = freq*LO_DIVIDER; /* Radio frequency -> VCO frequency */
  freq_regs = f_vco<<16; /* Multiply by 2^16 */
  freq_regs /= FREQ_OSC; /* Divide by oscillator frequency -> Frequency registers */

  /*printf("channel %d => freq_regs %02x %02x %02x\n", c,
         ((unsigned char*)&freq_regs)[0],
         ((unsigned char*)&freq_regs)[1],
         ((unsigned char*)&freq_regs)[2]);*/
  single_write(CC1120_FREQ0, ((unsigned char*)&freq_regs)[0]);
  single_write(CC1120_FREQ1, ((unsigned char*)&freq_regs)[1]);
  single_write(CC1120_FREQ2, ((unsigned char*)&freq_regs)[2]);
  strobe(CC1120_SCAL);
  strobe(CC1120_SIDLE);
  BUSYWAIT_UNTIL((state() == CC1120_STATE_IDLE), RTIMER_SECOND / 10);
  single_write(CC1120_FREQ0, ((unsigned char*)&freq_regs)[0]);
  single_write(CC1120_FREQ1, ((unsigned char*)&freq_regs)[1]);
  single_write(CC1120_FREQ2, ((unsigned char*)&freq_regs)[2]);
  strobe(CC1120_SRX);

  current_channel = c;

  RELEASE_SPI();

  if(radio_on_or_off == OFF) {
    BUSYWAIT_UNTIL((state() == CC1120_STATE_RX), RTIMER_SECOND / 10);
    off();
  }
}
/*---------------------------------------------------------------------------*/
static signed char
rssi_dbm(unsigned char raw_rssi)
{
  int16_t dbm = 0;

  if(raw_rssi >= 128) {
    dbm = (int16_t)((int16_t)(raw_rssi - 256) / 2) - RSSI_OFFSET;
  } else {
    dbm = (raw_rssi / 2) - RSSI_OFFSET;
  }

  return dbm;
}
/*---------------------------------------------------------------------------*/
/* XXX Placeholder awaiting new radio_driver API */
signed char
cc1120_read_rssi(void)
{
  return read_rssi();
}
/*---------------------------------------------------------------------------*/
/**
 * read RSSI
 */
static signed char
read_rssi(void)
{
  uint8_t radio_was_off, raw, val;

  if(SPI_IS_LOCKED()) {
    return 1;
  }

  if(state() == CC1120_STATE_IDLE ||
     state() == CC1120_STATE_SLEEP) {
    radio_was_off = 1;
    on();
  } else {
    radio_was_off = 0;
  }
  BUSYWAIT_UNTIL((state() == CC1120_STATE_RX), RTIMER_SECOND / 100);

  /* If we were turned off recently, we need to wait for about 1.2 ms
     before we have a reasonable chance of getting a good CCA
     readout. */
  BUSYWAIT_UNTIL(0, (2L * RTIMER_SECOND) / 1000);

  do {
    burst_read(CC1120_RSSI0, &val, 1);
  } while(!(val & CARRIER_SENSE_VALID));

  burst_read(CC1120_RSSI, &raw, 1);

  if(radio_was_off) {
    off();
  }

  return rssi_dbm(raw);
}
/*---------------------------------------------------------------------------*/
/**
 * rf1a read LQI
 */
#if 0
static unsigned char
read_lqi(void)
{
  unsigned char temp;

  burst_read(CC1120_LQI, &temp, 1);
  return temp;
}
#endif /* 0 */
/*---------------------------------------------------------------------------*/
/**
 * receiving packet function - check Radio SFD
 */
static int
receiving_packet(void)
{
  if(SPI_IS_LOCKED()) {
    return 0;
  }

  if(radio_on_or_off == OFF) {
    return 0;
  }

  return is_receiving();
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  if(packet_rx_len > 0) {
    return 1;
  }

  /* No pending packet. But bytes may still reside in the RXFIFO. We therefor
   * try to empty the FIFO right now. */
#define READ_FIFO_IN_PENDING_PACKET 1
#if READ_FIFO_IN_PENDING_PACKET
  cc1120_rx_interrupt();
  if(packet_rx_len > 0) {
    return 1;
  }
#endif /* READ_FIFO_IN_PENDING_PACKET */

  return 0;
}
/*---------------------------------------------------------------------------*/
static uint8_t
state(void)
{
  uint8_t state;

  burst_read(CC1120_MARCSTATE, &state, 1);
  return state & 0x1f;
}
/*---------------------------------------------------------------------------*/
void
cc1120_set_promiscuous(char p)
{
  promiscuous_mode = p;
}
/*---------------------------------------------------------------------------*/
/**
 * Return CCA bit in packet status register.
 */
static int
channel_clear(void)
{
  uint8_t radio_was_off, cca, val;

  if(SPI_IS_LOCKED()) {
    return 1;
  }

  if(state() == CC1120_STATE_IDLE ||
     state() == CC1120_STATE_SLEEP) {
    radio_was_off = 1;
    on();
    BUSYWAIT_UNTIL((state() == CC1120_STATE_RX), RTIMER_SECOND / 100);
  } else {
    radio_was_off = 0;
  }

  /* If we were turned off recently, we need to wait for about 1.2 ms
     before we have a reasonable chance of getting a good CCA
     readout. */
  BUSYWAIT_UNTIL(0, (12L * RTIMER_SECOND) / 10000);

  burst_read(CC1120_RSSI0, &val, 1);
  if(!(val & CARRIER_SENSE_VALID)) {
    cca = 1;
  } else if (val & CARRIER_SENSE) {
    cca = 0;
  } else {
    cca = 1;
  }

  if(radio_was_off) {
    off();
  }
  return cca;
}
/*---------------------------------------------------------------------------*/
/**
 * Initialize CC1120 radio module.
 */
static int
init(void)
{
  static uint8_t initialized = 0;
  if(initialized) {
    return 1;
  }
  cc1120_arch_init();
  reset();

  process_start(&cc1120_process, NULL);

  off();
  cc1120_arch_interrupt_enable();

#if PERFORM_MANUAL_CALIBRATION
  strobe(CC1120_SIDLE);
  dint(); /* XXX No eint() is performed here! */
  calibrate_manual();
#endif /* PERFORM_MANUAL_CALIBRATION */

  initialized = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
void
cc1120_print_state(void)
{
  printf("state marc: %d\n", state());
  printf("state rxbytes: %d\n", read_rxbytes());
  printf("state txbytes: %d\n", txbytes());
}
/*---------------------------------------------------------------------------*/
#if PERFORM_MANUAL_CALIBRATION
/* Below code is adapted from TI's CC112x/CC1175 ERRATA */
#define cc112xSpiWriteReg burst_write
#define cc112xSpiReadReg burst_read
#define trxSpiCmdStrobe strobe
#define CC112X_FS_VCO2 CC1120_FS_VCO2
#define CC112X_FS_VCO4 CC1120_FS_VCO4
#define CC112X_FS_CHP CC1120_FS_CHP
#define CC112X_FS_CAL2 CC1120_FS_CAL2
#define CC112X_MARCSTATE CC1120_MARCSTATE
#define SCAL CC1120_SCAL
/*---------------------------------------------------------------------------*/
#define VCDAC_START_OFFSET  2
#define FS_VCO2_INDEX       0
#define FS_VCO4_INDEX       1
#define FS_CHP_INDEX        2
/*---------------------------------------------------------------------------*/
static void
calibrate_manual(void)
{
  uint8_t original_fs_cal2;
  uint8_t calResults_for_vcdac_start_high[3];
  uint8_t calResults_for_vcdac_start_mid[3];
  uint8_t marcstate;
  uint8_t writeByte;

  // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
  writeByte = 0x00;
  cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);

  // 2) Start with high VCDAC (original VCDAC_START + 2):
  cc112xSpiReadReg(CC112X_FS_CAL2, &original_fs_cal2, 1);
  writeByte = original_fs_cal2 + VCDAC_START_OFFSET;
  cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);

  // 3) Calibrate and wait for calibration to be done (radio back in IDLE state)
  trxSpiCmdStrobe(SCAL);
  do {
    cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
  } while(marcstate != 0x41);

  // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with high VCDAC_START value
  cc112xSpiReadReg(CC112X_FS_VCO2,
                   &calResults_for_vcdac_start_high[FS_VCO2_INDEX], 1);
  cc112xSpiReadReg(CC112X_FS_VCO4,
                   &calResults_for_vcdac_start_high[FS_VCO4_INDEX], 1);
  cc112xSpiReadReg(CC112X_FS_CHP, &calResults_for_vcdac_start_high[FS_CHP_INDEX],
                   1);

  // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
  writeByte = 0x00;
  cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);

  // 6) Continue with mid VCDAC (original VCDAC_START):
  writeByte = original_fs_cal2;
  cc112xSpiWriteReg(CC112X_FS_CAL2, &writeByte, 1);

  // 7) Calibrate and wait for calibration to be done (radio back in IDLE state)
  trxSpiCmdStrobe(SCAL);
  do {
    cc112xSpiReadReg(CC112X_MARCSTATE, &marcstate, 1);
  } while(marcstate != 0x41);

  // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with mid VCDAC_START value
  cc112xSpiReadReg(CC112X_FS_VCO2, &calResults_for_vcdac_start_mid[FS_VCO2_INDEX],
                   1);
  cc112xSpiReadReg(CC112X_FS_VCO4, &calResults_for_vcdac_start_mid[FS_VCO4_INDEX],
                   1);
  cc112xSpiReadReg(CC112X_FS_CHP, &calResults_for_vcdac_start_mid[FS_CHP_INDEX],
                   1);

  // 9) Write back highest FS_VCO2 and corresponding FS_VCO and FS_CHP result
  if(calResults_for_vcdac_start_high[FS_VCO2_INDEX]
                                     > calResults_for_vcdac_start_mid[FS_VCO2_INDEX]) {
    writeByte = calResults_for_vcdac_start_high[FS_VCO2_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_high[FS_VCO4_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_high[FS_CHP_INDEX];
    cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
  } else {
    writeByte = calResults_for_vcdac_start_mid[FS_VCO2_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO2, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_mid[FS_VCO4_INDEX];
    cc112xSpiWriteReg(CC112X_FS_VCO4, &writeByte, 1);
    writeByte = calResults_for_vcdac_start_mid[FS_CHP_INDEX];
    cc112xSpiWriteReg(CC112X_FS_CHP, &writeByte, 1);
  }
}
#endif /* PERFORM_MANUAL_CALIBRATION */
