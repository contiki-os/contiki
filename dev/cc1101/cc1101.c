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
 *         TI CC1101 driver
 */

#include "contiki.h"

#include "cc1101.h"
#include "cc1101-arch.h"
#include "cc1101-const.h"
#include "cc1101-config.h"

#include "dev/leds.h"

#include "lib/random.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#include <string.h>
#include <stdio.h>

#define DEBUG 0

#define BUSYWAIT_UNTIL(cond, max_time)                                    \
    do {                                                                  \
      rtimer_clock_t t0;                                                  \
      t0 = RTIMER_NOW();                                                  \
      while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) {}  \
    } while(0)

#define RSSI_OFFSET 74  /* This seems to need to be tweaked for every
                           hw design, according to the data sheet. */

/* 2 bytes for RSSI and LQI */
#define AUX_LEN 2
#define ACK_LEN 3

#define CC1101_MAX_PAYLOAD CC1101_SETTING_PKTLEN

/* Flag indicating whether non-interrupt routines are using SPI */
static volatile uint8_t spi_locked = 0;
#define LOCK_SPI() do { spi_locked++; } while(0)
#define RELEASE_SPI() do { spi_locked--; } while(0)
#define SPI_IS_LOCKED() (spi_locked != 0)

/* Packet buffers. */
static uint8_t packet_tx[CC1101_MAX_PAYLOAD];
/* AUX_LEN bytes for RSSI and LQI */
static uint8_t packet_rx[CC1101_MAX_PAYLOAD + AUX_LEN];
static volatile uint16_t packet_rx_len = 0;

static volatile uint8_t is_transmitting;

/* Initial CC1101 Register configuration settings */
static const unsigned char cc1101_register_config[CONF_REG_SIZE] = {
  /* IOCFG2: GDO2 signals on RF_RDYn */
  CC1101_SETTING_IOCFG2,

  /* IOCFG1: GDO1 signals on RSSI_VALID */
  CC1101_SETTING_IOCFG1,

  /* IOCFG0: GDO0 signals on PA power down signal to control RX/TX
     switch */
  CC1101_SETTING_IOCFG0,

  /* FIFOTHR: RX/TX FIFO Threshold: 33 bytes in TX, 32 bytes in RX */
  CC1101_SETTING_FIFOTHR,

  /* SYNC1: high byte of Sync Word */
  CC1101_SETTING_SYNC1,

  /* SYNC0: low byte of Sync Word */
  CC1101_SETTING_SYNC0,

  /* PKTLEN: Packet Length in fixed mode, Maximum Length in
     variable-length mode */
  CC1101_SETTING_PKTLEN,

  /* PKTCTRL1: No status bytes appended to the packet */
  CC1101_SETTING_PKTCTRL1,

  /* PKTCTRL0: Fixed-Length Mode, No CRC */
  CC1101_SETTING_PKTCTRL0,

  /* ADDR: Address for packet filtration */
  CC1101_SETTING_ADDR,

  /* CHANNR: 8-bit channel number, freq = base freq + CHANNR * channel
     spacing */
  CC1101_SETTING_CHANNR,

  /* FSCTRL1: Frequency Synthesizer Control (refer to User's
     Guide/RF1A Studio) */
  CC1101_SETTING_FSCTRL1,

  /* FSCTRL0: Frequency Synthesizer Control (refer to User's
     Guide/RF1A Studio) */
  CC1101_SETTING_FSCTRL0,

  /* FREQ2: base frequency, high byte */
  CC1101_SETTING_FREQ2,

  /* FREQ1: base frequency, middle byte */
  CC1101_SETTING_FREQ1,

  /* FREQ0: base frequency, low byte*/
  CC1101_SETTING_FREQ0,

  /* MDMCFG4: modem configuration (refer to User's Guide/RF1A Studio)*/
  CC1101_SETTING_MDMCFG4,

  /* MDMCFG3:                "                      " */
  CC1101_SETTING_MDMCFG3,

  /* MDMCFG2:                "                      " */
  CC1101_SETTING_MDMCFG2,

  /* MDMCFG1:                "                      " */
  CC1101_SETTING_MDMCFG1,

  /* MDMCFG0:                "                      " */
  CC1101_SETTING_MDMCFG0,

  /* DEVIATN: modem deviation setting (refer to User's Guide/RF1A
     Studio) */
  CC1101_SETTING_DEVIATN,

  /* MCSM2: Main Radio Control State Machine Conf. : timeout for sync
     word search disabled */
  CC1101_SETTING_MCSM2,

  /* MCSM1: CCA signals when RSSI below threshold, stay in RX after
     packet has been received */
  CC1101_SETTING_MCSM1,

  /* MCSM0: Auto-calibrate when going from IDLE to RX or TX (or
     FSTXON) */
  CC1101_SETTING_MCSM0,

  /* FOCCFG: Frequency Offset Compensation Conf. */
  CC1101_SETTING_FOCCFG,

  /* BSCFG: Bit Synchronization Conf. */
  CC1101_SETTING_BSCFG,

  /* AGCCTRL2: AGC Control */
  CC1101_SETTING_AGCCTRL2,

  /* AGCCTRL1:     " */
  CC1101_SETTING_AGCCTRL1,

  /* AGCCTRL0:     " */
  CC1101_SETTING_AGCCTRL0,

  /* WOREVT1: High Byte Event0 Timeout */
  CC1101_SETTING_WOREVT1,

  /* WOREVT0: High Byte Event0 Timeout */
  CC1101_SETTING_WOREVT0,

  /* WORCTL: Wake On Radio Control ****Feature unavailable in
     PG0.6**** */
  CC1101_SETTING_WORCTRL,

  /* FREND1: Front End RX Conf. */
  CC1101_SETTING_FREND1,

  /* FREND0: Front End TX Conf. */
  CC1101_SETTING_FREND0,

  /* FSCAL3: Frequency Synthesizer Calibration (refer to User's
     Guide/RF1A Studio) */
  CC1101_SETTING_FSCAL3,

  /* FSCAL2:              " */
  CC1101_SETTING_FSCAL2,

  /* FSCAL1:              " */
  CC1101_SETTING_FSCAL1,

  /* FSCAL0:              " */
  CC1101_SETTING_FSCAL0,

  /* RC Oscillator Config*/
  CC1101_SETTING_RCCTRL1,

  /* RC Oscillator Config */
  CC1101_SETTING_RCCTRL0,

  /* FSTEST: For test only, irrelevant for normal use case */
  CC1101_SETTING_FSTEST,

  /* PTEST: For test only, irrelevant for normal use case */
  CC1101_SETTING_PTEST,

  /* AGCTEST: For test only, irrelevant for normal use case */
  CC1101_SETTING_AGCTEST,

  /* TEST2  : For test only, irrelevant for normal use case */
  CC1101_SETTING_TEST2,

  /* TEST1  : For test only, irrelevant for normal use case */
  CC1101_SETTING_TEST1,

  /* TEST0  : For test only, irrelevant for normal use case */
  CC1101_SETTING_TEST0
};

#define CS_BIT (1 << 6)
#define CCA_BIT (1 << 4)

/* Prototypes: */
static void reset(void);
static uint8_t state(void);
static unsigned char strobe(uint8_t strobe);
#if 0
static unsigned char single_read(uint8_t addr);
#endif /* 0 */
static uint8_t single_write(uint8_t addr, uint8_t value);
static void burst_read(uint8_t addr, uint8_t *buffer,
                       uint8_t count);
static void burst_write(uint8_t addr, uint8_t *buffer,
                        uint8_t count);
static void pa_table_write(uint8_t pa_value);

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
signed char cc1101_read_rssi(void);
#if 0
static unsigned char read_lqi(void);
static unsigned char channel_get(void);
#endif /* 0 */

static int current_channel;

#define MIN(a,b) ((a)<(b)?(a):(b))

#define OFF 0
#define ON  1
static uint8_t radio_on_or_off = OFF;

/*---------------------------------------------------------------------------*/
PROCESS(cc1101_process, "CC1101 driver");

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
    *value = cc1101_channel_get();
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = 0;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = 49;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_RSSI:
    *value = cc1101_read_rssi();
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
    cc1101_channel_set(value);
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

const struct radio_driver cc1101_driver = {
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
  cc1101_arch_enable();
  ret = cc1101_arch_write_command(strobe);
  cc1101_arch_disable();
  RELEASE_SPI();
  return ret;
}
/*---------------------------------------------------------------------------*/
#if 0
static uint8_t
single_read(uint8_t addr)
{
  uint8_t ret;

  if(addr >= CC1101_PARTNUM &&
     addr <= CC1101_RCCTRL0_STATUS) {
    addr |= 0xc0;
    /* for status registers, burst bit needs to be set. */
  } else if(addr == 0x2f) {
    return 0;
    /* invalid register address (see cc1101 datasheet) */
  }
  LOCK_SPI();
  cc1101_arch_enable();
  cc1101_arch_write_command(addr | 0x80);
  ret = cc1101_arch_read_data();
  cc1101_arch_disable();
  RELEASE_SPI();
  return ret;
}
#endif /* 0 */
/*---------------------------------------------------------------------------*/
static uint8_t
single_write(uint8_t addr, uint8_t value)
{
  uint8_t ret;
  if(addr >= 0x2f) {
    return 0;
  }

  LOCK_SPI();
  cc1101_arch_enable();
  cc1101_arch_write_command(addr);
  ret = cc1101_arch_write_data(value);
  cc1101_arch_disable();
  RELEASE_SPI();
  return ret;
}
/*---------------------------------------------------------------------------*/
uint8_t
cc1101_single_write(uint8_t addr, uint8_t value)
{
  return single_write(addr, value);
}
/*---------------------------------------------------------------------------*/
static void
burst_read(uint8_t addr,
     uint8_t *data,
     uint8_t len)
{
  LOCK_SPI();
  cc1101_arch_enable();
  cc1101_arch_write_command(addr | 0xc0);
  cc1101_arch_read_databuf(data, len);
  cc1101_arch_disable();
  RELEASE_SPI();
}
/*---------------------------------------------------------------------------*/
static void
burst_write(uint8_t addr,
      uint8_t *data,
      uint8_t len)
{
  LOCK_SPI();
  cc1101_arch_enable();
  cc1101_arch_write_command(addr | 0x40);
  cc1101_arch_write_databuf(data, len);
  cc1101_arch_disable();
  RELEASE_SPI();
}
/*---------------------------------------------------------------------------*/
static uint8_t
txbytes(void)
{
  uint8_t txbytes1, txbytes2;

  do {
    burst_read(CC1101_TXBYTES, &txbytes1, 1);
    burst_read(CC1101_TXBYTES, &txbytes2, 1);
    if(txbytes1 - 1 == txbytes2 || txbytes1 - 2 == txbytes2) {
      /* XXX Workaround for slow CPU/SPI */
      return txbytes2;
    }
  } while(txbytes1 != txbytes2);

  return txbytes2;
}
/*---------------------------------------------------------------------------*/
static uint8_t
read_rxbytes(void)
{
  uint8_t rxbytes1, rxbytes2;

  do {
    burst_read(CC1101_RXBYTES, &rxbytes1, 1);
    burst_read(CC1101_RXBYTES, &rxbytes2, 1);
    if(rxbytes1 + 1 == rxbytes2 || rxbytes1 + 2 == rxbytes2) {
      /* XXX Workaround for slow CPU/SPI */
      return rxbytes2;
    }
  } while(rxbytes1 != rxbytes2);

  return rxbytes1;
}
/*---------------------------------------------------------------------------*/
static void
write_txfifo(uint8_t *data, uint8_t len)
{
#define BURST_BIT 0x40
  uint8_t status;
  int i;
#define CC1101_STATUS_STATE_MASK             0x70
#define CC1101_STATUS_STATE_TXFIFO_UNDERFLOW 0x70

  /* ensure FIFO is empty before tx */
  strobe(CC1101_SIDLE);
  strobe(CC1101_SFTX);
  is_transmitting = 1;

  LOCK_SPI();
  burst_write(CC1101_TXFIFO, &len, 1);

#define FIRST_TX 60 /* Must not be 62 or above! */
  i = MIN(len, FIRST_TX);
  burst_write(CC1101_TXFIFO, data, i);


  strobe(CC1101_STX);

  if(len > i) {
    cc1101_arch_enable();
    cc1101_arch_write_command(CC1101_TXFIFO | BURST_BIT);
    for(; i < len; i++) {

      status = cc1101_arch_write_data(data[i]);

      if((status & CC1101_STATUS_STATE_MASK) ==
         CC1101_STATUS_STATE_TXFIFO_UNDERFLOW) {
        cc1101_arch_disable();
        /* TX FIFO underflow, acknowledge it with an SFTX (else the
           CC1101 becomes completely unresponsive) followed by an SRX,
           and break the transmission. */
        strobe(CC1101_SFTX);
        strobe(CC1101_SRX);
        break;
#define MIN_TXFIFO_AVAILABLE 2
      } else if((status & 0x0f) < MIN_TXFIFO_AVAILABLE) {
        cc1101_arch_disable();
        BUSYWAIT_UNTIL(txbytes() < 60 || (txbytes() & 0x80) != 0,
                       RTIMER_SECOND / 10);
        if(txbytes() & 0x80) {
/*          printf("TxUF2\n");*/
          /* TX FIFO underflow. */
          strobe(CC1101_SFTX);
          strobe(CC1101_SRX);
          break;
        }
        /* If there are more bytes to write, we should issue another
           TXFIFO write command. */
        if(i < len - 1) {
          cc1101_arch_enable();
          cc1101_arch_write_command(CC1101_TXFIFO | BURST_BIT);
        }
      }
    }
    cc1101_arch_disable();
  }
  RELEASE_SPI();

  is_transmitting = 0;
}
/*---------------------------------------------------------------------------*/
static void
check_txfifo(void)
{
  uint8_t b;
  b = txbytes();
  if((b & 0x80) != 0 ||
    state() == CC1101_STATUS_STATE_TXFIFO_UNDERFLOW) {
    /* Acknowledge TX FIFO underflow. */
    strobe(CC1101_SFTX);
    strobe(CC1101_SRX);
  }
}
/*---------------------------------------------------------------------------*/
static void
pa_table_write(uint8_t val)
{
  uint8_t table[8];
  int i;

  for(i = 0; i < sizeof(table); i++) {
    table[i] = val;
  }

  burst_write(CC1101_PATABLE, table, 8);
}
/*---------------------------------------------------------------------------*/
static void
calibrate(void)
{
  if(CC1101_SETTING_MCSM0 & FS_AUTOCAL_ALWAYS) {
    return;
  }
  if(radio_on_or_off == ON) {
    off();
    strobe(CC1101_SCAL);
    BUSYWAIT_UNTIL((state() == CC1101_STATE_IDLE), RTIMER_SECOND / 10);
    on();
  } else {
    strobe(CC1101_SCAL);
    BUSYWAIT_UNTIL((state() == CC1101_STATE_IDLE), RTIMER_SECOND / 10);
    off();
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc1101_process, ev, data)
{

  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

#if 0
  while(1) {
    static struct etimer et;
    uint8_t rxbytes, txbytes;
    etimer_set(&et, CLOCK_SECOND * 4);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    //    cc1101_rx_interrupt();
    burst_read(CC1101_RXBYTES, &rxbytes, 1);
    burst_read(CC1101_TXBYTES, &txbytes, 1);
    printf("state 0x%02x rxbytes 0x%02x txbytes 0x%02x\n",
     state(), rxbytes, txbytes);
    on();
  }
#endif /* 0 */
  while(1) {
    static struct etimer et;
    etimer_set(&et, 10 * CLOCK_SECOND * 60);
    calibrate();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  //  PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_EXIT);

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
/*        printf("p(");*/
    packetbuf_clear();
    len = read_packet(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {
      packetbuf_set_datalen(len);
/*            printf("RDC input %d\n", len);*/
      NETSTACK_RDC.input();
    }

    /* If we received a packet (or parts thereof) while processing the
       previous packet, we immediately pull it out from the RX
       FIFO. */
    /*    printf("[");*/
    if(radio_on_or_off == ON) {
      cc1101_rx_interrupt();
    }
    /*    printf("]");*/
    //    printf(")\n");
  } while(packet_rx_len > 0);
}
/*---------------------------------------------------------------------------*/
static void
flushrx(void)
{
  int was_on;

  LOCK_SPI();

  if(radio_on_or_off == ON) {
    was_on = 1;
  } else {
    was_on = 0;
  }

  restart_input();

  if(state() == CC1101_STATE_RXFIFO_OVERFLOW) {
    strobe(CC1101_SFRX);
  }
  strobe(CC1101_SIDLE);
  BUSYWAIT_UNTIL((state() == CC1101_STATE_IDLE), RTIMER_SECOND / 10);
  strobe(CC1101_SFRX);
  if(was_on) {
    strobe(CC1101_SRX);
  } else {
    strobe(CC1101_SPWD);
  }
  RELEASE_SPI();
}
/*---------------------------------------------------------------------------*/
static void
flushtx(void)
{
  int was_on;

  LOCK_SPI();
  if(radio_on_or_off == ON) {
    was_on = 1;
  } else {
    was_on = 0;
  }

  strobe(CC1101_SIDLE);
  BUSYWAIT_UNTIL((state() == CC1101_STATE_IDLE), RTIMER_SECOND / 10);
  strobe(CC1101_SFTX);
  if(was_on) {
    strobe(CC1101_SRX);
  } else {
    strobe(CC1101_SPWD);
  }
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

  write_txfifo((unsigned char *)ackdata, len);
  check_txfifo();
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

  if(rxstate.len == 0 || rxstate.len > (sizeof(packet_rx) - AUX_LEN)) {
#if DEBUG
    printf("Bad len %u, state %d rxbytes %d\n", len, cc1101_get_state(), read_rxbytes());
#endif /* DEBUG */
    flushrx();
    PT_EXIT(&rxstate.pt);
  }

  timer_set(&rxstate.timer, PACKET_LIFETIME);
  for(rxstate.ptr = 0;
      rxstate.ptr < rxstate.len;
      rxstate.ptr++) {

    /* Wait for the next data byte to be received. */
    PT_YIELD(&rxstate.pt);
    rxstate.buf[rxstate.ptr] = byte;
  }

  /* Receive two more bytes from the FIFO: the RSSI value and the LQI/CRC */
  PT_YIELD(&rxstate.pt);
  rxstate.buf[rxstate.ptr] = byte;
  //printf("rssi: %u\n", byte);
  rxstate.ptr++;
  PT_YIELD(&rxstate.pt);
  rxstate.buf[rxstate.ptr] = byte;
  crc = (byte & 0x80);
  rxstate.ptr++;

  if(crc == 0) {
//#if DEBUG
//    printf("bad crc\n");
//#endif /* DEBUG */
    flushrx();
  } else if(packet_rx_len > 0) {
//#if DEBUG
    printf("Packet in the buffer (%d), dropping %d bytes\n", packet_rx_len, rxstate.len);
//#endif /* DEBUG */
    flushrx();
    process_poll(&cc1101_process);
  } else if(rxstate.len < ACK_LEN) {
    /* Drop packets that are way too small: less than ACK_LEN (3) bytes
       long. */
//#if DEBUG
    printf("!");
//#endif /* DEBUG */
  } else {
    //printf("k\n");
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
      process_poll(&cc1101_process);

    } else if(rxstate.len >= ACK_LEN) {
      /* Send a link-layer ACK before reading the full packet. */
      /* Try to parse the incoming frame as a 802.15.4 header. */
      frame802154_t info154;
      if(frame802154_parse(rxstate.buf, rxstate.len, &info154) != 0) {

        /* XXX Potential optimization here: we could check if the
           frame is destined for us, or for the broadcast address and
           discard the packet if it isn't for us. */
        if(1) {

          /* For dataframes that has the ACK request bit set and that
       is destined for us, we send an ack. */
          if(info154.fcf.frame_type == FRAME802154_DATAFRAME &&
              info154.fcf.ack_required != 0 &&
              linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
                           &linkaddr_node_addr)) {
            send_ack(info154.seq);

            /* Make sure that we don't put the radio in the IDLE state
         before the ACK has been fully transmitted. */
            BUSYWAIT_UNTIL((state() != CC1101_STATE_TX), RTIMER_SECOND / 10);
            ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
            ENERGEST_ON(ENERGEST_TYPE_LISTEN);
            if(state() == CC1101_STATE_TX) {
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

          process_poll(&cc1101_process);
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
 * The CC1101 interrupt handler: called by the hardware interrupt
 * handler, which is defined as part of the cc1101-arch interface.
 */
int
cc1101_rx_interrupt(void)
{
  /* NB: This function may be called both from an rx interrupt and
     from cc1101_process */
  uint8_t rxbytes, s;

  /*  cooja_debug("cc1101_rx_interrupt\n");*/
  /*  if(radio_on_or_off == OFF) {
    cooja_debug("cc1101_rx_interrupt when off\n");
    printf("cc1101_rx_interrupt when off\n");
    return 0;
    }*/
#if DEBUG
  printf("-");
#endif /* DEBUG */

  if(SPI_IS_LOCKED()) {
#if DEBUG
    printf("/%d", spi_locked);
#endif /* DEBUG */
    process_poll(&cc1101_process);
    return 1;
  }

  LOCK_SPI();
  s = state();
  if(s == CC1101_STATE_RXFIFO_OVERFLOW) {
    burst_read(CC1101_RXBYTES, &rxbytes, 1);
#if DEBUG
    printf("irqflush\n");
    printf("rxbytes 0x%02x\n", rxbytes);
#endif /* DEBUG */
    flushrx();
    RELEASE_SPI();
    return 0;
  }
  if(s == CC1101_STATE_TXFIFO_UNDERFLOW) {
#if DEBUG
    printf("irqflushtx\n");
#endif /* DEBUG */
    flushtx();
    RELEASE_SPI();
    return 0;
  }

  if(is_receiving() && timer_expired(&rxstate.timer)) {
#if DEBUG
    printf("Packet expired, flushing fifo\n");
#endif /* DEBUG */
    flushrx();
    RELEASE_SPI();
    //   cooja_debug("packet expired\n");
    return 0;
  }
#define RX_OVERFLOW 0x80

  /* Read each byte from the RXFIFO and put it into the input_byte()
     function, which takes care of the reception logic. */
  if(packet_rx_len == 0) {
    do {
      uint8_t byte;
      int i, numbytes;

      rxbytes = read_rxbytes();

      if(rxbytes & RX_OVERFLOW) {
#if DEBUG
        printf("ovf\n");
        leds_off(LEDS_GREEN);
#endif /* DEBUG */
        flushrx();
        process_poll(&cc1101_process);
        RELEASE_SPI();
        return 1;
      }

      if(rxbytes == 0) {
        RELEASE_SPI();
        return 0;
      }
      if(rxbytes >= 64) {
#if DEBUG
        printf("rxbytes too large %d\n", rxbytes);
#endif /* DEBUG */
        flushrx();
        RELEASE_SPI();
        return 0;
      }

    /* Check if we are receiving a packet. If not, we feed the first
       byte of data, which hold the length of the packet, to the input
       function. This will help us later decide how much to read from
       the rx fifo. */
      if(!is_receiving()) {
        burst_read(CC1101_RXFIFO, &byte, 1);
        input_byte(byte);
        rxbytes = read_rxbytes();
        //   cooja_debug("starting a new packet\n");
      }


      numbytes = 0;
      if(is_receiving()) {
        if(rxbytes < (rxstate.len + AUX_LEN) - rxstate.ptr) {
          /* If the fifo contains only a part of the packet, we leave
             one byte behind. */
          numbytes = rxbytes - 1;
          //          printf("one byte behind\n");
        } else {
          /* If the full packet can be found in the fifo, we read it out
             in full. */
          numbytes = rxbytes;
        }
        //   cooja_debug("burst read\n");
        /*        LOCK_SPI();*/
        /*       cc1101_arch_enable();
                 cc1101_arch_write_command(CC1101_RXFIFO | 0xc0);*/

        for(i = 0; i < numbytes && is_receiving(); i++) {
          burst_read(CC1101_RXFIFO, &byte, 1);
          input_byte(byte);
                              /*          input_byte(cc1101_arch_read_data());*/
        }
        /*        cc1101_arch_disable();
                  RELEASE_SPI();*/
      }
      rxbytes = read_rxbytes();

    } while(is_receiving() && rxbytes > 0 && packet_rx_len == 0);
  }

  //   cooja_debug("return from interrupt\n");
#if DEBUG
  printf("-");
#endif /* DEBUG */
  RELEASE_SPI();
  return 1;
}

/*---------------------------------------------------------------------------*/
/**
 * Read received packet in rx FIFO.
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
  if(state() == CC1101_STATE_RXFIFO_OVERFLOW) {
    flushrx();
  }

#if DEBUG
  printf("tx len %d\n", len);
#endif /* DEBUG */
  if(len > CC1101_MAX_PAYLOAD) {
#if DEBUG
    printf("Too big %d\n", len);
#endif /* DEBUG */
    return RADIO_TX_ERR;
  }

  LOCK_SPI();
#if DEBUG
  printf(".");
#endif /* DEBUG */
  strobe(CC1101_SIDLE);

  write_txfifo((unsigned char *)packet_tx, len);

  BUSYWAIT_UNTIL((state() == CC1101_STATE_TX), RTIMER_SECOND / 10);

  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);

  if(state() != CC1101_STATE_TX) {
#if DEBUG
    printf("didn't tx (in %d)\n", state());
#endif /* DEBUG */
    check_txfifo();
    flushrx();
    RELEASE_SPI();
    return RADIO_TX_ERR;
  }
  BUSYWAIT_UNTIL((state() != CC1101_STATE_TX), RTIMER_SECOND / 10);
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  if(state() == CC1101_STATE_TX) {
#if DEBUG
    printf("didn't end tx (in %d, txbytes %d)\n", state(), txbytes());
#endif /* DEBUG */
    check_txfifo();
    flushrx();
    RELEASE_SPI();
    if(radio_on_or_off == OFF) {
      off();
    }

    return RADIO_TX_ERR;
  }
  check_txfifo();
  RELEASE_SPI();
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
  if(state() == CC1101_STATE_RXFIFO_OVERFLOW) {
    flushrx();
  }

  if(len > CC1101_MAX_PAYLOAD) {
#if DEBUG
    printf("CC1101 DEBUG: Too big packet, aborting prepare %d\n", len);
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
  if(len > CC1101_MAX_PAYLOAD) {
#if DEBUG
    printf("CC1101 DEBUG: Too big packet, aborting send %d\n", len);
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
    printf("Z");
    return 0;
  }
  LOCK_SPI();

  if(radio_on_or_off == ON) {
    RELEASE_SPI();
    return 1;
  }
  radio_on_or_off = ON;
  if(state() == CC1101_STATE_RX) {
    RELEASE_SPI();
    return 1;
  }

  restart_input();
  BUSYWAIT_UNTIL(
      (state() == CC1101_STATE_IDLE || state() == CC1101_STATE_RXFIFO_OVERFLOW) || state() == CC1101_STATE_SLEEP,
      RTIMER_SECOND / 100);
  strobe(CC1101_SFRX);
  strobe(CC1101_SRX);
  BUSYWAIT_UNTIL((state() == CC1101_STATE_RX), RTIMER_SECOND / 10);
  /*  cooja_debug("is on CC1101_SRX");*/
  RELEASE_SPI();

  /*  cooja_debug("on\n");*/
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
#if DEBUG
  printf("(");
  leds_on(LEDS_RED);
#endif /* DEBUG */

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  if(SPI_IS_LOCKED()) {
    //    printf("Z\n");
    return 0;
  }

  radio_on_or_off = OFF;
  LOCK_SPI();
  strobe(CC1101_SIDLE);
  BUSYWAIT_UNTIL((state() == CC1101_STATE_IDLE), RTIMER_SECOND / 10);
  strobe(CC1101_SPWD);
  RELEASE_SPI();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
#if DEBUG
  leds_off(LEDS_RED);
  printf(")");
#endif /* DEBUG */
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
  strobe(CC1101_SRES);
  BUSYWAIT_UNTIL((strobe(CC1101_SNOP) & 0x80) == 0, RTIMER_SECOND / 100); /* wait for CHIP_RDY */

  burst_write(CC1101_IOCFG2, (unsigned char *)cc1101_register_config,
        CONF_REG_SIZE);
  pa_table_write(CC1101_PA_11);

#if DEBUG
  {
    uint8_t data;
    uint8_t dummydata[] = {0, 1, 2, 3, 4, 5};
    uint8_t dummydata_readback[6];
    uint8_t i;

    burst_read(CC1101_VERSION, &data, 1);
    printf("CC1101 DEBUG: Version %d\n", data);
    burst_read(CC1101_PARTNUM, &data, 1);
    printf("CC1101 DEBUG: Partnum %d\n", data);
    burst_read(CC1101_PKTLEN, &data, 1);
    printf("CC1101 DEBUG: Variable packet length up to %d\n", data);

    single_write(0, 0x19);
    printf("CC1101 DEBUG: single write, read, should be 0x19 0x%02x\n", single_read(0));
    single_write(0, 0x25);
    printf("CC1101 DEBUG: single write, read, should be 0x25 0x%02x\n", single_read(0));
    single_write(0, 0x33);
    printf("CC1101 DEBUG: single write, read, should be 0x33 0x%02x\n", single_read(0));

    printf("CC1101 DEBUG: testing burst write and read...\n");
    burst_write(0, dummydata, 6);
    burst_read(0, dummydata_readback, 6);
    for(i = 0; i < 6; i += 1) {
      if(dummydata_readback[i] != dummydata[i]) {
        printf("Burst write/read failed on index %d; got %u expected %u\n",
               i, dummydata_readback[i], dummydata[i]);
      }
    }
    printf("CC1101 DEBUG: ...done debug tests.\n");

    strobe(CC1101_SRES);
    strobe(CC1101_SNOP); /* XXX needed? */
    burst_write(CC1101_IOCFG2, (unsigned char *)cc1101_register_config,
      CONF_REG_SIZE);
    pa_table_write(CC1101_PA_11);

  }
#endif

  RELEASE_SPI();

  on();
}
/*---------------------------------------------------------------------------*/
uint8_t
cc1101_channel_get(void)
{
  return current_channel;
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
/**
 * read RSSI
 */
signed char
cc1101_read_rssi(void)
{
  int s;
  uint8_t raw;
  uint8_t cca, cca_bit, radio_was_off;

  if(SPI_IS_LOCKED()) {
    return 0;
  }

  radio_was_off = (radio_on_or_off == OFF);
  if(radio_was_off) {
    on();
    /*    cooja_debug("wait for on");*/
    BUSYWAIT_UNTIL((state() == CC1101_STATE_RX), RTIMER_SECOND / 100);
    /*    cooja_debug("wait for on - is now on");*/
  }

  /* Read the PKTSTATUS register until either the Carrier Sense (CS)
     or the Clear Channel Assessment (CCA) pins are non-zero. */
  rtimer_clock_t t0;
  t0 = RTIMER_NOW();
  do {
    burst_read(CC1101_PKTSTATUS, &cca_bit, 1);
  } while((cca_bit & CS_BIT) == 0 && (cca_bit & CCA_BIT) == 0 &&
          RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + RTIMER_SECOND / 10));
  burst_read(CC1101_RSSI, &raw, 1);
  if(radio_was_off) {
    off();
  }
  return rssi_dbm(raw);
}
/*---------------------------------------------------------------------------*/
#if 0
static unsigned char
read_lqi(void)
{
  unsigned char temp;

  burst_read(CC1101_LQI, &temp, 1);
  return temp;
}
#endif /* 0 */
/*---------------------------------------------------------------------------*/
/**
 * cc1101 receiving packet function - check Radio SFD
 */
static int
receiving_packet(void)
{
  if(SPI_IS_LOCKED()) {
    return 0;
  }

  if(radio_on_or_off == OFF) {
    //    cooja_debug("off, not receiving\n");
    return 0;
  }

#if 0
  uint8_t rxbytes;
  rxbytes = read_rxbytes();

  if(rxbytes & 0x80) {
    /* RXFIFO overflow - flush it. */
    flushrx();
#if DEBUG
    printf("recv overflow - nuke\n");
#endif /* DEBUG */
  } else if(rxbytes > 0 && !timer_expired(&rxstate.timer)) {
    //   cooja_debug("rxbytes > 0, poll\n");
    /* If we detect a reception here, we'll poll the process just in
       case somehow the interrupt missed it... */
#if DEBUG
    printf("r");
#endif /* DEBUG */
    process_poll(&cc1101_process);
    //    return 1;
  }
#endif /* 0 */
  return is_receiving();

}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  if(packet_rx_len > 0) {
#if DEBUG
    printf("p");
#endif /* DEBUG */
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static uint8_t
state(void)
{
  uint8_t state;

  burst_read(CC1101_MARCSTATE, &state, 1);
  return state & 0x1f;
}
/*---------------------------------------------------------------------------*/
/**
 * Return CCA bit in packet status register.
 */
static int
channel_clear(void)
{
  uint8_t cca, cca_bit, radio_was_off;
  int s;

  if(SPI_IS_LOCKED()) {
    return 0;
  }

  radio_was_off = (radio_on_or_off == OFF);
  if(radio_was_off) {
    on();
    /*    cooja_debug("wait for on");*/
    BUSYWAIT_UNTIL((state() == CC1101_STATE_RX), RTIMER_SECOND / 100);
    /*    cooja_debug("wait for on - is now on");*/
  }


  /* Wait for the CCA to stabilize itself, should take about 1 ms */
  BUSYWAIT_UNTIL(0, RTIMER_SECOND / 1000);
  /* Read the PKTSTATUS register until either the Carrier Sense (CS)
     or the Clear Channel Assessment (CCA) pins are non-zero. */
  rtimer_clock_t t0;
  t0 = RTIMER_NOW();
  do {
    burst_read(CC1101_PKTSTATUS, &cca_bit, 1);
  } while((cca_bit & CS_BIT) == 0 && (cca_bit & CCA_BIT) == 0 &&
          RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + RTIMER_SECOND / 10));

  if((cca_bit & CS_BIT) != 0) {
    cca = 0;
  } else if((cca_bit & CCA_BIT) != 0) {
    cca = 1;
  } else {
    cca = -1;
  }

  if(radio_was_off) {
    off();
  }

  if(cca == 0) {
#if DEBUG
    printf("+");
#endif /* DEBUG */
  } else {
    //    printf(" ");
  }
  return cca;
}
/*---------------------------------------------------------------------------*/
/**
 * Initialise CC1101 radio module.
 */
static int
init(void)
{
  static uint8_t initialized = 0;

  if(initialized) {
    return 0;
  }
  cc1101_arch_init();
  reset();

  process_start(&cc1101_process, NULL);

  on();
  int rssi = 0;
  int i;
  for(i = 0; i < 50; i++) {
    cc1101_channel_set(i);
    channel_clear();
    rssi += cc1101_read_rssi();
  }
  random_init(rssi);
  off();
  cc1101_arch_interrupt_enable();

  printf("cc1101 init\n");
  cc1101_channel_set(0);

  initialized = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
void
cc1101_print_state(void)
{
  printf("cc1101 state %d, read_rxbytes %d, cc1101_read_rssi %d\n", state(),
         read_rxbytes(), cc1101_read_rssi());
}
/*---------------------------------------------------------------------------*/
/* XXX Placeholder awaiting new radio_driver API */
void
cc1101_channel_set(uint8_t c)
{
  current_channel = c;
  single_write(CC1101_CHANNR, c);
  /* Calibration must be done after switching to a new channel */
  calibrate();
}
/*---------------------------------------------------------------------------*/
/* XXX Placeholder awaiting new radio_driver API */
void
cc1101_powerlevel_set(int powerlevel)
{
  uint8_t target_pa = powerlevel;

  off();
  LOCK_SPI();
  strobe(CC1101_SRES);
  strobe(CC1101_SNOP); /* XXX needed? */

  pa_table_write(target_pa);

  RELEASE_SPI();

  on();
}
/*---------------------------------------------------------------------------*/
