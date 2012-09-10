/**
 * \file
 *         CC2430 RF driver
 * \author
 *         Zach Shelby <zach@sensinode.com> (Original)
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 *         (port to the netstack API, hexdump output, RX FIFO overflow fixes
 *          code cleanup, ...)
 *
 *  bankable code for cc2430 rf driver.  this code can be placed in any bank.
 *
 */

#include <stdio.h>

#include "contiki.h"
#include "dev/radio.h"
#include "dev/cc2430_rf.h"
#include "cc2430_sfr.h"
#include "sys/clock.h"
#include "sys/rtimer.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"

#define CC2430_RF_TX_POWER_RECOMMENDED 0x5F
#ifdef CC2430_RF_CONF_TX_POWER
#define CC2430_RF_TX_POWER CC2430_RF_CONF_TX_POWER
#else
#define CC2430_RF_TX_POWER CC2430_RF_TX_POWER_RECOMMENDED
#endif

#ifdef CC2430_RF_CONF_CHANNEL
#define CC2430_RF_CHANNEL CC2430_RF_CONF_CHANNEL
#else
#define CC2430_RF_CHANNEL 18
#endif /* CC2430_RF_CONF_CHANNEL */
#define CC2430_CHANNEL_MIN 11
#define CC2430_CHANNEL_MAX 26

#ifdef CC2430_RF_CONF_AUTOACK
#define CC2430_RF_AUTOACK CC2430_RF_CONF_AUTOACK
#else
#define CC2430_RF_AUTOACK 1
#endif

#ifndef CC2430_CONF_CHECKSUM
#define CC2430_CONF_CHECKSUM 0
#endif /* CC2420_CONF_CHECKSUM */

#if CC2430_CONF_CHECKSUM
#include "lib/crc16.h"
#define CHECKSUM_LEN 2
#else
#define CHECKSUM_LEN 2
#endif /* CC2430_CONF_CHECKSUM */
#if DEBUG_LEDS
/* moved leds code to BANK1 to make space for cc2430_rf_process in HOME */
/* can't call code in BANK1 from alternate banks unless it is marked with __banked */
#include "dev/leds.h"
#define RF_RX_LED_ON()    leds_on(LEDS_RED);
#define RF_RX_LED_OFF()   leds_off(LEDS_RED);
#define RF_TX_LED_ON()    leds_on(LEDS_GREEN);
#define RF_TX_LED_OFF()   leds_off(LEDS_GREEN);
#else
#define RF_RX_LED_ON()
#define RF_RX_LED_OFF()
#define RF_TX_LED_ON()
#define RF_TX_LED_OFF()
#endif
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

/* rf_flags bits */
#define RX_ACTIVE   0x80
#define TX_ACK      0x40
#define TX_ON_AIR   0x20
#define WAS_OFF     0x10
#define INITIALISED 0x01

#define RX_NO_DMA
/* Bits of the last byte in the RX FIFO */
#define CRC_BIT_MASK 0x80
#define LQI_BIT_MASK 0x7F

/* 192 ms, radio off -> on interval */
#define ONOFF_TIME                    ((RTIMER_ARCH_SECOND / 3125) + 4)

#if CC2430_RF_CONF_HEXDUMP
#include "uart1.h"
static const uint8_t magic[] = { 0x53, 0x6E, 0x69, 0x66 }; /* Snif */
#endif

#ifdef HAVE_RF_ERROR
uint8_t rf_error = 0;
#endif

/*---------------------------------------------------------------------------*/
#if !NETSTACK_CONF_SHORTCUTS
PROCESS(cc2430_rf_process, "CC2430 RF driver");
#endif
/*---------------------------------------------------------------------------*/
static uint8_t __data rf_flags;
static uint8_t rf_channel;

static int on(void); /* prepare() needs our prototype */
static int off(void); /* transmit() needs our prototype */
static int channel_clear(void); /* transmit() needs our prototype */
/*---------------------------------------------------------------------------*/
/**
 * Execute a single CSP command.
 *
 * \param command command to execute
 *
 */
void
cc2430_rf_command(uint8_t command)
{
  if(command >= 0xE0) { /*immediate strobe*/
    uint8_t fifo_count;
    switch (command) { /*hardware bug workaround*/
    case ISRFOFF:
    case ISRXON:
    case ISTXON:
      fifo_count = RXFIFOCNT;
      RFST = command;
      clock_delay_usec(2);
      if(fifo_count != RXFIFOCNT) {
        RFST = ISFLUSHRX;
        RFST = ISFLUSHRX;
      }
      break;

    default:
      RFST = command;
    }
  } else if(command == SSTART) {
    RFIF &= ~IRQ_CSP_STOP; /*clear IRQ flag*/
    RFST = SSTOP; /*make sure there is a stop in the end*/
    RFST = ISSTART; /*start execution*/
    while((RFIF & IRQ_CSP_STOP) == 0);
  } else {
    RFST = command; /*write command*/
  }
}
/*---------------------------------------------------------------------------*/
static void
flush_rx()
{
  cc2430_rf_command(ISFLUSHRX);
  cc2430_rf_command(ISFLUSHRX);

#if !NETSTACK_CONF_SHORTCUTS
  IEN2 |= RFIE;
#endif
#if CC2430_RFERR_INTERRUPT
  IEN0 |= RFERRIE;
#endif

  RFIF &= ~IRQ_FIFOP;
}
/*---------------------------------------------------------------------------*/
/**
 * Select RF channel.
 *
 * \param channel channel number to select
 *
 * \return channel value or negative (invalid channel number)
 */

 /* channel freqdiv = (2048 + FSCTRL(9:0)) / 4
            freq = (2048 + FSCTRL(9:0)) MHz */

int8_t
cc2430_rf_channel_set(uint8_t channel)
{
  uint16_t freq;

  if((channel < 11) || (channel > 26)) {
    return -1;
  }

  cc2430_rf_command(ISSTOP);  /*make sure CSP is not running*/
  cc2430_rf_command(ISRFOFF);
  /* Channel values: 11-26 */
  freq = (uint16_t) channel - 11;
  freq *= 5;  /*channel spacing*/
  freq += 357; /*correct channel range*/
  freq |= 0x4000; /*LOCK_THR = 1*/
  FSCTRLH = (freq >> 8);
  FSCTRLL = (uint8_t)freq;

  cc2430_rf_command(ISRXON);

  rf_channel = channel;

  return (int8_t) channel;
}
/*---------------------------------------------------------------------------*/
uint8_t
cc2430_rf_channel_get()
{
  return rf_channel;
}
/*---------------------------------------------------------------------------*/
/**
 * Select RF transmit power.
 *
 * \param new_power new power level
 *
 * \return new level
 */
uint8_t
cc2430_rf_power_set(uint8_t new_power)
{
  /* Set transmitter power */
  TXCTRLL = new_power;

  return TXCTRLL;
}
/*---------------------------------------------------------------------------*/
#if 0 /* unused */
/**
 * Enable RF transmitter.
 *
 *
 * \return pdTRUE
 * \return pdFALSE  bus not free
 */
int
cc2430_rf_tx_enable(void)
{
  DMAARM = 0x80 + (1 << 0); /*ABORT + channel bit*/

  return 1;
}
#endif
/*---------------------------------------------------------------------------*/
/**
  * Set MAC addresses
  *
  * \param pan The PAN address to set
  * \param addr The short address to set
  * \param ieee_addr The 64-bit IEEE address to set
  */
void
cc2430_rf_set_addr(unsigned pan, unsigned addr, const uint8_t *ieee_addr)
{
  uint8_t f;
  __xdata unsigned char *ptr;

  PANIDH = pan >> 8;
  PANIDL = pan & 0xff;

  SHORTADDRH = addr >> 8;
  SHORTADDRL = addr & 0xff;

  if(ieee_addr != NULL) {
    ptr = &IEEE_ADDR7;
      /* LSB first, MSB last for 802.15.4 addresses in CC2420 */
    for (f = 0; f < 8; f++) {
      *ptr-- = ieee_addr[f];
    }
  }
}
#if 0 /* currently unused */
/*---------------------------------------------------------------------------*/
/**
 * Channel energy detect.
 *
 * Coordinator use this function detect best channel for PAN-network.
 * \return RSSI-energy level dBm.
 * \return 0  operation failed.
 */

int8_t
cc2430_rf_analyze_rssi(void)
{
  int8_t retval = -128;
  /*pause_us(128);*/

  retval = (int8_t)RSSIL;
  retval -= 45;
  return retval;
}
#endif /* currently unused */
/*---------------------------------------------------------------------------*/
/**
 * Send ACK.
 *
 *\param pending set up pending flag if pending > 0.
 */
void
cc2430_rf_send_ack(uint8_t pending)
{
  if(pending) {
    cc2430_rf_command(ISACKPEND);
  } else {
    cc2430_rf_command(ISACK);
  }
}
/*---------------------------------------------------------------------------*/
/* Netstack API radio driver functions */
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  if(rf_flags & INITIALISED) {
    return 0;
  }

  PRINTF("cc2430_rf_init called\n");

  RFPWR &= ~RREG_RADIO_PD;  /*make sure it's powered*/
  while((RFPWR & ADI_RADIO_PD) == 1);
  while((RFIF & IRQ_RREG_ON) == 0); /*wait for power up*/
  SLEEP &= ~OSC_PD; /*Osc on*/
  while((SLEEP & XOSC_STB) == 0); /*wait for power up*/

  rf_flags = 0;

  FSMTC1 = 1; /*don't abort reception, if enable called, accept ack, auto rx after tx*/

  MDMCTRL0H = 0x0A;  /* Generic client, standard hysteresis, decoder on 0x0a */
  MDMCTRL0L = 0xE2;  /* automatic CRC, standard CCA and preamble 0xE2 */
#if CC2430_RF_AUTOACK
  MDMCTRL0L |= 0x10;
#endif

  MDMCTRL1H = 0x30;     /* Defaults */
  MDMCTRL1L = 0x0;

  RXCTRL0H = 0x32;      /* RX tuning optimized */
  RXCTRL0L = 0xf5;

  cc2430_rf_channel_set(CC2430_RF_CHANNEL);
  cc2430_rf_command(ISFLUSHTX);
  cc2430_rf_command(ISFLUSHRX);

  /* Temporary values, main() will sort this out later on */
  cc2430_rf_set_addr(0xffff, 0x0000, NULL);

  RFIM = IRQ_FIFOP;
  RFIF &= ~(IRQ_FIFOP);

  S1CON &= ~(RFIF_0 | RFIF_1);
#if !NETSTACK_CONF_SHORTCUTS
  IEN2 |= RFIE;
#endif

  /* If contiki-conf.h turns on the RFERR interrupt, enable it here */
#if CC2430_RFERR_INTERRUPT
  IEN0 |= RFERRIE;
#endif

  RF_TX_LED_OFF();
  RF_RX_LED_OFF();

  rf_flags |= INITIALISED;

#if !NETSTACK_CONF_SHORTCUTS
  process_start(&cc2430_rf_process, NULL);
#endif

  cc2430_rf_power_set(CC2430_RF_TX_POWER);

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  uint8_t i;
  /*
   * When we transmit in very quick bursts, make sure previous transmission
   * is not still in progress before re-writing in the TX FIFO
   */
  while(RFSTATUS & TX_ACTIVE);

  if(rf_flags & TX_ACK) {
    return -1;
  }

  if((rf_flags & RX_ACTIVE) == 0) {
    on();
  }

  PRINTF("cc2430_rf: sending %u byte payload\n", payload_len);

  cc2430_rf_command(ISFLUSHTX);
  PRINTF("cc2430_rf: data = ");
  /* Send the phy length byte first */
  RFD = payload_len + CHECKSUM_LEN; /* Payload plus FCS */
  PRINTF("(%d)", payload_len+CHECKSUM_LEN);
  for(i = 0; i < payload_len; i++) {
    RFD = ((unsigned char*) (payload))[i];
    PRINTF("%02X", ((unsigned char*)(payload))[i]);
  }
  PRINTF("\n");

  /* Leave space for the FCS */
  RFD = 0;
  RFD = 0;

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  uint8_t counter;
  int ret = RADIO_TX_ERR;

  if(!(rf_flags & RX_ACTIVE)) {
    on();
    rf_flags |= WAS_OFF;
  }

  if(channel_clear() == CC2430_CCA_BUSY) {
    RIMESTATS_ADD(contentiondrop);
    return RADIO_TX_COLLISION;
  }

  /*
   * prepare() double checked that TX_ACTIVE is low. If SFD is high we are
   * receiving. Abort transmission and bail out with RADIO_TX_COLLISION
   */
  if(RFSTATUS & SFD) {
    RIMESTATS_ADD(contentiondrop);
    return RADIO_TX_COLLISION;
  }

  /* Start the transmission */
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  cc2430_rf_command(ISTXON);
  counter = 0;
  while(!(RFSTATUS & TX_ACTIVE) && (counter++ < 3)) {
    clock_delay_usec(6);
  }

  if(!(RFSTATUS & TX_ACTIVE)) {
    PRINTF("cc2430_rf: TX never active.\n");
    cc2430_rf_command(ISFLUSHTX);
    ret = RADIO_TX_ERR;
  } else {
    /* Wait for the transmission to finish */
    while(RFSTATUS & TX_ACTIVE);
    RF_RX_LED_OFF();
    RF_TX_LED_ON();
    ret = RADIO_TX_OK;
    // rf_flags |= TX_ON_AIR;
  }
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  if(rf_flags & WAS_OFF){
    off();
  }

  RIMESTATS_ADD(lltx);
  /* OK, sent. We are now ready to send more */
  return ret;
}
/*---------------------------------------------------------------------------*/
static int
send(void *payload, unsigned short payload_len)
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
#if CC2420_CONF_CHECKSUM
  uint16_t checksum;
#endif /* CC2420_CONF_CHECKSUM */

  /* Don't interrupt us while emptying the FIFO */
#if !NETSTACK_CONF_SHORTCUTS
  IEN2 &= ~RFIE;
#endif
#if CC2430_RFERR_INTERRUPT
  IEN0 &= ~RFERRIE;
#endif

  /* RX interrupt polled the cc2430_rf_process, now read the RX FIFO */
  /* Check the length */
  len = RFD;

  /* Check for validity */
  if(len > CC2430_MAX_PACKET_LEN) {
    /* Oops, we must be out of sync. */
    PRINTF("error: bad sync\n");

    RIMESTATS_ADD(badsynch);
    flush_rx();
    return 0;
  }

  if(len <= CC2430_MIN_PACKET_LEN) {
  PRINTF("error: too short\n");

    RIMESTATS_ADD(tooshort);
    flush_rx();
    return 0;
  }

  if(len - CHECKSUM_LEN > bufsize) {
  PRINTF("error: too long\n");

    RIMESTATS_ADD(toolong);
    flush_rx();
    return 0;
  }

#if CC2430_RF_CONF_HEXDUMP
  /* If we reach here, chances are the FIFO is holding a valid frame */
  uart1_writeb(magic[0]);
  uart1_writeb(magic[1]);
  uart1_writeb(magic[2]);
  uart1_writeb(magic[3]);
  uart1_writeb(len);
#endif

  PRINTF("cc2430_rf: read = ");
  PRINTF("(%d)", len);
  len -= CHECKSUM_LEN;
  for(i = 0; i < len; ++i) {
      ((unsigned char*)(buf))[i] = RFD;
#if CC2430_RF_CONF_HEXDUMP
      uart1_writeb(((unsigned char*)(buf))[i]);
#endif
      PRINTF("%02X", ((unsigned char*)(buf))[i]);
  }
  PRINTF("\n");

#if CC2430_CONF_CHECKSUM
    /* Deal with the checksum */
    checksum = RFD * 256;
    checksum += RFD;
#endif /* CC2430_CONF_CHECKSUM */

  /* Read the RSSI and CRC/Corr bytes */
  rssi = ((int8_t) RFD) - 45;
  crc_corr = RFD;

#if CC2430_RF_CONF_HEXDUMP
  uart1_writeb(rssi);
  uart1_writeb(crc_corr);
#endif

  /* MS bit CRC OK/Not OK, 7 LS Bits, Correlation value */
  if(crc_corr & CRC_BIT_MASK) {
    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, crc_corr & LQI_BIT_MASK);
    RIMESTATS_ADD(llrx);
  } else {
    RIMESTATS_ADD(badcrc);
    flush_rx();
    return 0;
}

  /* If FIFOP==1 and FIFO==0 then we had a FIFO overflow at some point. */
  if((RFSTATUS & (FIFO | FIFOP)) == FIFOP) {
    /*
     * If we reach here means that there might be more intact packets in the
     * FIFO despite the overflow. This can happen with bursts of small packets.
 *
     * Only flush if the FIFO is actually empty. If not, then next pass we will
     * pick up one more packet or flush due to an error.
 */
    if(!RXFIFOCNT) {
      flush_rx();
    }
  }

  RF_RX_LED_OFF();

#if !NETSTACK_CONF_SHORTCUTS
  IEN2 |= RFIE;
#endif
#if CC2430_RFERR_INTERRUPT
  IEN0 |= RFERRIE;
#endif

  RFIF &= ~IRQ_FIFOP;

  return (len);
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  if(!(RFSTATUS & CCA)) {
    return CC2430_CCA_BUSY;
  }
  return CC2430_CCA_CLEAR;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  /*
   * SFD high while transmitting and receiving.
   * TX_ACTIVE high only when transmitting
   *
   * RFSTATUS & (TX_ACTIVE | SFD) == SFD <=> receiving
   */
  return (RFSTATUS & (TX_ACTIVE | SFD) == SFD);
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return (RFSTATUS & FIFOP);
}
/*---------------------------------------------------------------------------*/
/**
 * Enable RF receiver.
 *
 *
 * \return pdTRUE
 * \return pdFALSE  bus not free
 */
static int
on(void)
{
  rtimer_clock_t t0;
  PRINTF("cc2430_rf_rx_enable called\n");
  if(!(rf_flags & RX_ACTIVE)) {
    t0 = RTIMER_NOW();
    rf_flags |= RX_ACTIVE;
    IOCFG0 = 0x7f; /* Set the FIFOP threshold 127 */
    RSSIH = 0xd2; /* -84dbm = 0xd2 default, 0xe0 -70 dbm */

    RFPWR &= ~RREG_RADIO_PD; /* make sure it's powered */
    while ((RFIF & IRQ_RREG_ON) == 0); /* wait for power up */

    /* Make sure the RREG On Interrupt Flag is 0 next time we get called */
    RFIF &= ~IRQ_RREG_ON;

    cc2430_rf_command(ISRXON);
    cc2430_rf_command(ISFLUSHRX);
    while (RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + ONOFF_TIME));

  }
  PRINTF("cc2430_rf_rx_enable done\n");
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Disable RF receiver.
 *
 *
 * \return pdTRUE
 * \return pdFALSE  bus not free
 */

static int
off(void)
{
  cc2430_rf_command(ISSTOP);  /* make sure CSP is not running */
  cc2430_rf_command(ISRFOFF);

  RFPWR |= RREG_RADIO_PD;   /* RF powerdown */

  /* Clear the RREG On Interrupt Flag */
  RFIF &= ~IRQ_RREG_ON;

  rf_flags &= ~RX_ACTIVE;
  rf_flags &= ~WAS_OFF;
  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  return 1;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver cc2430_rf_driver =
{
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
#if !NETSTACK_CONF_SHORTCUTS
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc2430_rf_process, ev, data)
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
  }

  PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
