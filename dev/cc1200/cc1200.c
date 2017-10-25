/*
 * Copyright (c) 2015, Weptech elektronik GmbH Germany
 * http://www.weptech.de
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
 * This file is part of the Contiki operating system.
 */

#include "cc1200-const.h"
#include "cc1200-conf.h"
#include "cc1200-arch.h"
#include "cc1200-rf-cfg.h"

#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "dev/watchdog.h"

#include "dev/leds.h"

#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
/* Various implementation specific defines */
/*---------------------------------------------------------------------------*/
/*
 * The debug level to use
 * - 0: No output at all
 * - 1: Print errors (unrecoverable)
 * - 2: Print errors + warnings (recoverable errors)
 * - 3: Print errors + warnings + information (what's going on...)
 */
#define DEBUG_LEVEL                     2
/*
 * RF test mode. Blocks inside "configure()".
 * - Set this parameter to 1 in order to produce an modulated carrier (PN9)
 * - Set this parameter to 2 in order to produce an unmodulated carrier
 * - Set this parameter to 3 in order to switch to rx synchronous mode
 * The channel is set according to CC1200_DEFAULT_CHANNEL
 */
#ifndef CC1200_RF_TESTMODE
#define CC1200_RF_TESTMODE              0
#endif

#if CC1200_RF_TESTMODE
#undef CC1200_RF_CFG
#if CC1200_RF_TESTMODE == 1
#define CC1200_RF_CFG                   cc1200_802154g_863_870_fsk_50kbps
#elif CC1200_RF_TESTMODE == 2
#define CC1200_RF_CFG                   cc1200_802154g_863_870_fsk_50kbps
#elif CC1200_RF_TESTMODE == 3
#define CC1200_RF_CFG                   cc1200_802154g_863_870_fsk_50kbps
#endif
#endif
/*
 * Set this parameter to 1 in order to use the MARC_STATE register when
 * polling the chips's status. Else use the status byte returned when sending
 * a NOP strobe.
 *
 * TODO: Option to be removed upon approval of the driver
 */
#define STATE_USES_MARC_STATE           0
/*
 * Set this parameter to 1 in order to speed up transmission by
 * sending a FSTXON strobe before filling the FIFO.
 *
 * TODO: Option to be removed upon approval of the driver
 */
#define USE_SFSTXON                     1
/*---------------------------------------------------------------------------*/
/* Phy header length */
#if CC1200_802154G
/* Phy header = 2 byte */
#define PHR_LEN                         2
#else
/* Phy header = length byte = 1 byte */
#define PHR_LEN                         1
#endif /* #if CC1200_802154G */
/*---------------------------------------------------------------------------*/
/* Size of appendix (rssi + lqi) appended to the rx pkt */
#define APPENDIX_LEN                    2
/*---------------------------------------------------------------------------*/
/* Verify payload length */
/*---------------------------------------------------------------------------*/
#if CC1200_802154G
#if CC1200_USE_GPIO2
#if CC1200_MAX_PAYLOAD_LEN > (2048 - PHR_LEN)
#error Payload length not supported by this driver
#endif
#else
#if CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN)
/* PHR_LEN = 2 -> we can only place 126 payload bytes bytes in the FIFO */
#error Payload length not supported without GPIO2
#endif
#endif /* #if CC1200_USE_GPIO2 */
#else /* #if CC1200_802154G */
#if CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN)
/* PHR_LEN = 1 -> we can only place 127 payload bytes bytes in the FIFO */
#error Payload length not supported without enabling 802.15.4g mode
#endif
#endif /* #if CC1200_802154G */
/*---------------------------------------------------------------------------*/
/* Main driver configurations settings. Don't touch! */
/*---------------------------------------------------------------------------*/
#if CC1200_USE_GPIO2
/* Use GPIO2 as RX / TX FIFO threshold indicator pin */
#define GPIO2_IOCFG                     CC1200_IOCFG_RXFIFO_THR
/* This is the FIFO threshold we use */
#define FIFO_THRESHOLD                  32
/* Turn on RX after packet reception */
#define RXOFF_MODE_RX                   1
/* Let the CC1200 append RSSI + LQI */
#define APPEND_STATUS                   1
#else
/* Arbitrary configuration for GPIO2 */
#define GPIO2_IOCFG             CC1200_IOCFG_MARC_2PIN_STATUS_0
#if (CC1200_MAX_PAYLOAD_LEN <= (CC1200_FIFO_SIZE - PHR_LEN - APPENDIX_LEN))
/*
 * Read out RX FIFO at the end of the packet (GPIO0 falling edge). RX restarts
 * automatically
 */
#define RXOFF_MODE_RX                   1
/* Let the CC1200 append RSSI + LQI */
#define APPEND_STATUS                   1
#else
/*
 * Read out RX FIFO at the end of the packet (GPIO0 falling edge). RX has
 * to be started manually in this case
 */
#define RXOFF_MODE_RX                   0
/* No space for appendix in the RX FIFO. Read it out by hand */
#define APPEND_STATUS                   0
#endif /* #if CC1200_MAX_PAYLOAD_LEN <= 125 */
#endif /* #if CC1200_USE_GPIO2 */

/* Read out packet on falling edge of GPIO0 */
#define GPIO0_IOCFG                     CC1200_IOCFG_PKT_SYNC_RXTX
/* Arbitrary configuration for GPIO3 */
#define GPIO3_IOCFG                     CC1200_IOCFG_MARC_2PIN_STATUS_0
/* Turn on RX automatically after TX */
#define TXOFF_MODE_RX                   1
#if APPEND_STATUS
/* CC1200 places two bytes in the RX FIFO */
#define CC_APPENDIX_LEN                 2
#else
/* CC1200 doesn't add appendix to RX FIFO */
#define CC_APPENDIX_LEN                 0
#endif /* #if APPEND_STATUS */
/*---------------------------------------------------------------------------*/
/* RF configuration */
/*---------------------------------------------------------------------------*/
/* Import the rf configuration set by CC1200_RF_CFG */
extern const cc1200_rf_cfg_t CC1200_RF_CFG;
/*---------------------------------------------------------------------------*/
/* This defines the way we calculate the frequency registers */
/*---------------------------------------------------------------------------*/
/* XTAL frequency in kHz */
#define XTAL_FREQ_KHZ                   40000
/*
 * Divider + multiplier for calculation of FREQ registers
 * f * 2^16 * 4 / 40000 = f * 2^12 / 625 (no overflow up to frequencies of
 * 1048.576 MHz using uint32_t)
 */
#define LO_DIVIDER                      4
#if (XTAL_FREQ_KHZ == 40000) && (LO_DIVIDER == 4)
#define FREQ_DIVIDER                    625
#define FREQ_MULTIPLIER                 4096
#else
#error Invalid settings for frequency calculation
#endif
/*---------------------------------------------------------------------------*/
#if STATE_USES_MARC_STATE
/* We use the MARC_STATE register to poll the chip's status */
#define STATE_IDLE                      CC1200_MARC_STATE_IDLE
#define STATE_RX                        CC1200_MARC_STATE_RX
#define STATE_TX                        CC1200_MARC_STATE_TX
#define STATE_RX_FIFO_ERROR             CC1200_MARC_STATE_RX_FIFO_ERR
#define STATE_TX_FIFO_ERROR             CC1200_MARC_STATE_TX_FIFO_ERR
#else
/* We use the status byte read out using a NOP strobe */
#define STATE_IDLE                      CC1200_STATUS_BYTE_IDLE
#define STATE_RX                        CC1200_STATUS_BYTE_RX
#define STATE_TX                        CC1200_STATUS_BYTE_TX
#define STATE_FSTXON                    CC1200_STATUS_BYTE_FSTXON
#define STATE_CALIBRATE                 CC1200_STATUS_BYTE_CALIBRATE
#define STATE_SETTLING                  CC1200_STATUS_BYTE_SETTLING
#define STATE_RX_FIFO_ERR               CC1200_STATUS_BYTE_RX_FIFO_ERR
#define STATE_TX_FIFO_ERR               CC1200_STATUS_BYTE_TX_FIFO_ERR
#endif /* #if STATE_USES_MARC_STATE */
/*---------------------------------------------------------------------------*/
/* Return values for addr_check_auto_ack() */
/*---------------------------------------------------------------------------*/
/* Frame cannot be parsed / is to short */
#define INVALID_FRAME                   0
/* Address check failed */
#define ADDR_CHECK_FAILED               1
/* Address check succeeded */
#define ADDR_CHECK_OK                   2
/* Address check succeeded and ACK was send */
#define ADDR_CHECK_OK_ACK_SEND          3
/*---------------------------------------------------------------------------*/
/* Return values for set_channel() */
/*---------------------------------------------------------------------------*/
/* Channel update was performed */
#define CHANNEL_UPDATE_SUCCEEDED        0
/* Busy, channel update postponed */
#define CHANNEL_UPDATE_POSTPONED        1
/* Invalid channel */
#define CHANNEL_OUT_OF_LIMITS           2
/*---------------------------------------------------------------------------*/
/* Various flags indicating the operating state of the radio. See rf_flags */
/*---------------------------------------------------------------------------*/
/* Radio was initialized (= init() was called) */
#define RF_INITIALIZED                  0x01
/* The radio is on (= not in standby) */
#define RF_ON                           0x02
/* An incoming packet was detected (at least payload length was received */
#define RF_RX_PROCESSING_PKT            0x04
/* TX is ongoing */
#define RF_TX_ACTIVE                    0x08
/* Channel update required */
#define RF_UPDATE_CHANNEL               0x10
/* SPI was locked when calling RX interrupt, let the pollhandler do the job */
#define RF_POLL_RX_INTERRUPT            0x20
/* Force calibration in case we don't use CC1200 AUTOCAL + timeout */
#if !CC1200_AUTOCAL
#if CC1200_CAL_TIMEOUT_SECONDS
#define RF_FORCE_CALIBRATION            0x40
#endif
#endif
/*---------------------------------------------------------------------------*/
/* Length of 802.15.4 ACK. We discard packets with a smaller size */
#define ACK_LEN                         3
/*---------------------------------------------------------------------------*/
/* This is the way we handle the LEDs */
/*---------------------------------------------------------------------------*/
#ifdef CC1200_TX_LEDS
#define TX_LEDS_ON()                    leds_on(CC1200_TX_LEDS)
#define TX_LEDS_OFF()                   leds_off(CC1200_TX_LEDS)
#else
#define TX_LEDS_ON()
#define TX_LEDS_OFF()
#endif /* #ifdef CC1200_TX_LEDS */

#ifdef CC1200_RX_LEDS
#define RX_LEDS_ON()                    leds_on(CC1200_RX_LEDS)
#define RX_LEDS_OFF()                   leds_off(CC1200_RX_LEDS)
#else
#define RX_LEDS_ON()
#define RX_LEDS_OFF()
#endif /* #ifdef CC1200_RX_LEDS */
/*---------------------------------------------------------------------------*/
/*
 * We have to prevent duplicate SPI access.
 * We therefore LOCK SPI in order to prevent the rx interrupt to
 * interfere.
 */
#define LOCK_SPI()                      do { spi_locked++; } while(0)
#define SPI_IS_LOCKED()                 (spi_locked != 0)
#define RELEASE_SPI()                   do { spi_locked--; } while(0)
/*---------------------------------------------------------------------------*/
#define BUSYWAIT_UNTIL(cond, max_time) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) { \
      watchdog_periodic(); \
    } \
  } while(0)
/*---------------------------------------------------------------------------*/
#if CC1200_USE_GPIO2
/* Configure GPIO interrupts. GPIO0: falling, GPIO2: rising edge */
#define SETUP_GPIO_INTERRUPTS() \
  do { \
    cc1200_arch_gpio0_setup_irq(0); \
    cc1200_arch_gpio2_setup_irq(1); \
  } while(0)
#define ENABLE_GPIO_INTERRUPTS() \
  do { \
    cc1200_arch_gpio0_enable_irq(); \
    cc1200_arch_gpio2_enable_irq(); \
  } while(0)
#define DISABLE_GPIO_INTERRUPTS() \
  do { \
    cc1200_arch_gpio0_disable_irq(); \
    cc1200_arch_gpio2_disable_irq(); \
  } while(0)
#else
#define SETUP_GPIO_INTERRUPTS()         cc1200_arch_gpio0_setup_irq(0)
#define ENABLE_GPIO_INTERRUPTS()        cc1200_arch_gpio0_enable_irq()
#define DISABLE_GPIO_INTERRUPTS()       cc1200_arch_gpio0_disable_irq()
#endif /* #if CC1200_USE_GPIO2 */
/*---------------------------------------------------------------------------*/
/* Debug macros */
/*---------------------------------------------------------------------------*/
#if DEBUG_LEVEL > 0
/* Show all kind of errors e.g. when passing invalid payload length */
#define ERROR(...) printf(__VA_ARGS__)
#else
#define ERROR(...)
#endif

#if DEBUG_LEVEL > 0
/* This macro is used to check if the radio is in a valid state */
#define RF_ASSERT(condition) \
  do { \
    if(!(condition)) { \
      printf("RF: Assertion failed in line %d\n", __LINE__); \
    } \
  } while(0)
#else
#define RF_ASSERT(condition)
#endif

#if DEBUG_LEVEL > 1
/* Show warnings e.g. for FIFO errors */
#define WARNING(...) printf(__VA_ARGS__)
#else
#define WARNING(...)
#endif

#if DEBUG_LEVEL > 2
/* We just print out what's going on */
#define INFO(...) printf(__VA_ARGS__)
#else
#define INFO(...)
#endif

#if DEBUG_LEVEL > 0
/*
 * As BUSYWAIT_UNTIL was mainly used to test for a state transition,
 * we define a separate macro for this adding the possibility to
 * throw an error message when the timeout exceeds
 */
#define BUSYWAIT_UNTIL_STATE(s, t) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while((state() != s) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (t))) {} \
    if(!(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (t)))) { \
      printf("RF: Timeout exceeded in line %d!\n", __LINE__); \
    } \
  } while(0)
#else
#define BUSYWAIT_UNTIL_STATE(s, t) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while((state() != s) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (t))) {} \
  } while(0)
#endif
/*---------------------------------------------------------------------------*/
/* Variables */
/*---------------------------------------------------------------------------*/
/* Flag indicating whether non-interrupt routines are using SPI */
static volatile uint8_t spi_locked = 0;
/* Packet buffer for transmission, filled within prepare() */
static uint8_t tx_pkt[CC1200_MAX_PAYLOAD_LEN];
/* The number of bytes waiting in tx_pkt */
static uint16_t tx_pkt_len;
/* Packet buffer for reception */
static uint8_t rx_pkt[CC1200_MAX_PAYLOAD_LEN + APPENDIX_LEN];
/* The number of bytes placed in rx_pkt */
static volatile uint16_t rx_pkt_len = 0;
/*
 * The current channel in the range CC1200_RF_CHANNEL_MIN
 * to CC1200_RF_CHANNEL_MAX
 */
static uint8_t rf_channel;
/* The next channel requested */
static uint8_t new_rf_channel;
/* RADIO_PARAM_RX_MODE. Initialized in init() */
static radio_value_t rx_mode_value;
/* RADIO_PARAM_RX_MODE. Initialized in init() */
static radio_value_t tx_mode_value;
/* RADIO_PARAM_TXPOWER in dBm. Initialized in init() */
static int8_t txpower;
static int8_t new_txpower;
/* RADIO_PARAM_CCA_THRESHOLD. Initialized in init() */
static int8_t cca_threshold;
static int8_t new_cca_threshold;
/* The radio drivers state */
static uint8_t rf_flags = 0;
#if !CC1200_AUTOCAL && CC1200_CAL_TIMEOUT_SECONDS
/* Use a timeout to decide when to calibrate */
static unsigned long cal_timer;
#endif
#if CC1200_USE_RX_WATCHDOG
/* Timer used for RX watchdog */
static struct etimer et;
#endif /* #if CC1200_USE_RX_WATCHDOG */
/*---------------------------------------------------------------------------*/
/* Prototypes for Netstack API radio driver functions */
/*---------------------------------------------------------------------------*/
/* Init the radio. */
static int
init(void);
/* Prepare the radio with a packet to be sent. */
static int
prepare(const void *payload, unsigned short payload_len);
/* Send the packet that has previously been prepared. */
static int
transmit(unsigned short payload_len);
/* Prepare & transmit a packet. */
static int
send(const void *payload, unsigned short payload_len);
/* Read a received packet into a buffer. */
static int
read(void *buf, unsigned short bufsize);
/*
 * Perform a Clear-Channel Assessment (CCA) to find out if there is
 * a packet in the air or not.
 */
static int
channel_clear(void);
/* Check if the radio driver is currently receiving a packet. */
static int
receiving_packet(void);
/* Check if the radio driver has just received a packet. */
static int
pending_packet(void);
/* Turn the radio on. */
static int
on(void);
/* Turn the radio off. */
static int
off(void);
/* Get a radio parameter value. */
static radio_result_t
get_value(radio_param_t param, radio_value_t *value);
/* Set a radio parameter value. */
static radio_result_t
set_value(radio_param_t param, radio_value_t value);
/* Get a radio parameter object. */
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size);
/* Set a radio parameter object. */
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size);
/*---------------------------------------------------------------------------*/
/* The radio driver exported to contiki */
/*---------------------------------------------------------------------------*/
const struct radio_driver cc1200_driver = {
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
/* Prototypes for CC1200 low level function. All of these functions are
   called by the radio driver functions or the rx interrupt,
   so there is no need to lock SPI within these functions */
/*---------------------------------------------------------------------------*/
/* Send a command strobe. */
static uint8_t
strobe(uint8_t strobe);
/* Reset CC1200. */
static void
reset(void);
/* Write a single byte to the specified address. */
static uint8_t
single_write(uint16_t addr, uint8_t value);
/* Read a single byte from the specified address. */
static uint8_t
single_read(uint16_t addr);
/* Write a burst of bytes starting at the specified address. */
static void
burst_write(uint16_t addr, const uint8_t *data, uint8_t data_len);
/* Read a burst of bytes starting at the specified address. */
static void
burst_read(uint16_t addr, uint8_t *data, uint8_t data_len);
/* Write a list of register settings. */
static void
write_reg_settings(const registerSetting_t *reg_settings,
                   uint16_t sizeof_reg_settings);
/* Configure the radio (write basic configuration). */
static void
configure(void);
/* Return the radio's state. */
static uint8_t
state(void);
#if !CC1200_AUTOCAL
/* Perform manual calibration. */
static void
calibrate(void);
#endif
/* Enter IDLE state. */
static void
idle(void);
/* Enter RX state. */
static void
idle_calibrate_rx(void);
/* Restart RX from within RX interrupt. */
static void
rx_rx(void);
/* Fill TX FIFO, start TX and wait for TX to complete (blocking!). */
static int
idle_tx_rx(const uint8_t *payload, uint16_t payload_len);
/* Update TX power */
static void
update_txpower(int8_t txpower_dbm);
/* Update CCA threshold */
static void
update_cca_threshold(int8_t threshold_dbm);
/* Calculate FREQ register from channel */
static uint32_t
calculate_freq(uint8_t channel);
/* Update rf channel if possible, else postpone it (-> pollhandler). */
static int
set_channel(uint8_t channel);
/* Validate address and send ACK if requested. */
static int
addr_check_auto_ack(uint8_t *frame, uint16_t frame_len);
/*---------------------------------------------------------------------------*/
/* Handle tasks left over from rx interrupt or because SPI was locked */
static void pollhandler(void);
/*---------------------------------------------------------------------------*/
PROCESS(cc1200_process, "CC1200 driver");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc1200_process, ev, data)
{

  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();

#if CC1200_USE_RX_WATCHDOG
  while(1) {

    if((rf_flags & (RF_ON | RF_TX_ACTIVE)) == RF_ON) {

      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      etimer_reset(&et);

      /*
       * We are on and not in TX. As every function of this driver
       * assures that we are in RX mode
       * (using BUSYWAIT_UNTIL_STATE(STATE_RX, ...) construct) in
       * either rx_rx(), idle_calibrate_rx() or transmit(),
       * something probably went wrong in the rx interrupt handler
       * if we are not in RX at this point.
       */

      if(cc1200_arch_gpio0_read_pin() == 0) {

        /*
         * GPIO de-asserts as soon as we leave RX for what reason ever. No
         * reason to check RX as long as it is asserted (we are receiving a
         * packet). We should never interfere with the rx interrupt if we
         * check GPIO0 in advance...
         */

        LOCK_SPI();
        if(state() != STATE_RX) {
          WARNING("RF: RX watchdog triggered!\n");
          rx_rx();
        }
        RELEASE_SPI();

      }

    } else {
      PROCESS_YIELD();
    }

  }
#endif /* #if CC1200_USE_RX_WATCHDOG */

  PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_EXIT);

  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
/* Handle tasks left over from rx interrupt or because SPI was locked */
static void
pollhandler(void)
{

  if((rf_flags & (RF_ON + RF_POLL_RX_INTERRUPT)) ==
     (RF_ON + RF_POLL_RX_INTERRUPT)) {
    cc1200_rx_interrupt();
  }

  if(rf_flags & RF_UPDATE_CHANNEL) {
    /* We couldn't set the channel because we were busy. Try again now. */
    set_channel(new_rf_channel);
  }

  if(rx_pkt_len > 0) {

    int len;

    /*
     * We received a valid packet. CRC was checked before,
     * address filtering was performed (if configured)
     * and ACK was send (if configured)
     */

    packetbuf_clear();
    len = read(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {
      packetbuf_set_datalen(len);
      NETSTACK_RDC.input();
    }

  }

}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * Netstack API radio driver functions
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* Initialize radio. */
static int
init(void)
{

  INFO("RF: Init (%s)\n", CC1200_RF_CFG.cfg_descriptor);

  if(!(rf_flags & RF_INITIALIZED)) {

    LOCK_SPI();

    /* Perform low level initialization */
    cc1200_arch_init();

    /* Configure GPIO interrupts */
    SETUP_GPIO_INTERRUPTS();

    /* Write initial configuration */
    configure();

    /* Enable address filtering + auto ack */
    rx_mode_value = (RADIO_RX_MODE_AUTOACK | RADIO_RX_MODE_ADDRESS_FILTER);

    /* Enable CCA */
    tx_mode_value = (RADIO_TX_MODE_SEND_ON_CCA);

    /* Set output power */
    new_txpower = CC1200_RF_CFG.max_txpower;
    update_txpower(new_txpower);

    /* Adjust CAA threshold */
    new_cca_threshold = CC1200_RF_CFG.cca_threshold;
    update_cca_threshold(new_cca_threshold);

    process_start(&cc1200_process, NULL);

    /* We are on + initialized at this point */
    rf_flags |= (RF_INITIALIZED + RF_ON);

    RELEASE_SPI();

    /* Set default channel. This will also force initial calibration! */
    set_channel(CC1200_DEFAULT_CHANNEL);

    /*
     * We have to call off() before on() because on() relies on the
     * configuration of the GPIO0 pin
     */
    off();
  }

  return 1;

}
/*---------------------------------------------------------------------------*/
/* Prepare the radio with a packet to be sent. */
static int
prepare(const void *payload, unsigned short payload_len)
{

  INFO("RF: Prepare (%d)\n", payload_len);

  if((payload_len < ACK_LEN) ||
     (payload_len > CC1200_MAX_PAYLOAD_LEN)) {
    ERROR("RF: Invalid payload length!\n");
    return RADIO_TX_ERR;
  }

  tx_pkt_len = payload_len;
  memcpy(tx_pkt, payload, tx_pkt_len);

  return RADIO_TX_OK;

}
/*---------------------------------------------------------------------------*/
/* Send the packet that has previously been prepared. */
static int
transmit(unsigned short transmit_len)
{

  uint8_t was_off = 0;
  int ret = RADIO_TX_OK;

  INFO("RF: Transmit (%d)\n", transmit_len);

  if(transmit_len != tx_pkt_len) {
    ERROR("RF: TX length mismatch!\n");
    return RADIO_TX_ERR;
  }

  /* TX ongoing. Inhibit channel update & ACK as soon as possible */
  rf_flags |= RF_TX_ACTIVE;

  if(!(rf_flags & RF_ON)) {
    /* Radio is off - turn it on */
    was_off = 1;
    on();
    /* Radio is in RX now (and calibrated...) */
  }

  if(tx_mode_value & RADIO_TX_MODE_SEND_ON_CCA) {
    /* Perform clear channel assessment */
    if(!channel_clear()) {
      /* Channel occupied */
      RIMESTATS_ADD(contentiondrop);
      if(was_off) {
        off();
      }
      rf_flags &= ~RF_TX_ACTIVE;
      return RADIO_TX_COLLISION;
    }
  }

  /*
   * Lock SPI here because "on()" and "channel_clear()"
   * won't work if SPI is locked!
   */
  LOCK_SPI();

  /*
   * Make sure we start from a sane state. idle() also disables
   * the GPIO interrupt(s).
   */
  idle();

  /* Update output power */
  if(new_txpower != txpower) {
    update_txpower(new_txpower);
  }

#if !CC1200_AUTOCAL
  /* Perform manual calibration unless just turned on */
  if(!was_off) {
    calibrate();
  }
#endif

  RIMESTATS_ADD(lltx);

  /* Send data using TX FIFO */
  if(idle_tx_rx((const uint8_t *)tx_pkt, tx_pkt_len) == RADIO_TX_OK) {

    /*
     * TXOFF_MODE is set to RX,
     * let's wait until we are in RX and turn on the GPIO IRQs
     * again as they were turned off in idle()
     */

    BUSYWAIT_UNTIL_STATE(STATE_RX,
        CC1200_RF_CFG.tx_rx_turnaround);

    ENABLE_GPIO_INTERRUPTS();

  } else {

    /*
     * Something went wrong during TX, idle_tx_rx() returns in IDLE
     * state in this case.
     * Turn on RX again unless we turn off anyway
     */

    ret = RADIO_TX_ERR;
    if(!was_off) {
#ifdef RF_FORCE_CALIBRATION
      rf_flags |= RF_FORCE_CALIBRATION;
#endif
      idle_calibrate_rx();
    }
  }

  /* Release SPI here because "off()" won't work if SPI is locked! */
  RELEASE_SPI();

  if(was_off) {
    off();
  }

  /* TX completed */
  rf_flags &= ~RF_TX_ACTIVE;

  return ret;

}
/*---------------------------------------------------------------------------*/
/* Prepare & transmit a packet. */
static int
send(const void *payload, unsigned short payload_len)
{

  int ret;

  INFO("RF: Send (%d)\n", payload_len);

  /* payload_len checked within prepare() */
  if((ret = prepare(payload, payload_len)) == RADIO_TX_OK) {
    ret = transmit(payload_len);
  }

  return ret;

}
/*---------------------------------------------------------------------------*/
/* Read a received packet into a buffer. */
static int
read(void *buf, unsigned short buf_len)
{

  int len = 0;

  if(rx_pkt_len > 0) {

    int8_t rssi = rx_pkt[rx_pkt_len - 2];
    /* CRC is already checked */
    uint8_t crc_lqi = rx_pkt[rx_pkt_len - 1];

    len = rx_pkt_len - APPENDIX_LEN;

    if(len > buf_len) {

      ERROR("RF: Failed to read packet (too big)!\n");

    } else {

      INFO("RF: Read (%d bytes, %d dBm)\n", len, rssi);

      memcpy((void *)buf, (const void *)rx_pkt, len);

      /* Release rx_pkt */
      rx_pkt_len = 0;

      packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
      /* Mask out CRC bit */
      packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY,
                         crc_lqi & ~(1 << 7));

      RIMESTATS_ADD(llrx);
    }

  }

  return len;

}
/*---------------------------------------------------------------------------*/
/*
 * Perform a Clear-Channel Assessment (CCA) to find out if there is a
 * packet in the air or not.
 */
static int
channel_clear(void)
{

  uint8_t cca, was_off = 0;

  if(SPI_IS_LOCKED()) {
    /* Probably locked in rx interrupt. Return "channel occupied" */
    return 0;
  }

  if(!(rf_flags & RF_ON)) {
    /* We are off */
    was_off = 1;
    on();
  }

  LOCK_SPI();

  RF_ASSERT(state() == STATE_RX);

  /*
   * At this point we should be in RX. If GPIO0 is set, we are currently
   * receiving a packet, no need to check the RSSI. Or is there any situation
   * when we want access the channel even if we are currently receiving a
   * packet???
   */

  if(cc1200_arch_gpio0_read_pin() == 1) {
    /* Channel occupied */
    INFO("RF: CCA (0)\n");
    cca = 0;
  } else {

    uint8_t rssi0;

    /* Update CCA threshold */
    if(new_cca_threshold != cca_threshold) {
      update_cca_threshold(new_cca_threshold);
    }

    /* Wait for CARRIER_SENSE_VALID signal */
    BUSYWAIT_UNTIL(((rssi0 = single_read(CC1200_RSSI0))
                    & CC1200_CARRIER_SENSE_VALID),
                   RTIMER_SECOND / 100);
    RF_ASSERT(rssi0 & CC1200_CARRIER_SENSE_VALID);

    if(rssi0 & CC1200_CARRIER_SENSE) {
      /* Channel occupied */
      INFO("RF: CCA (0)\n");
      cca = 0;
    } else {
      /* Channel clear */
      INFO("RF: CCA (1)\n");
      cca = 1;
    }

  }

  RELEASE_SPI();

  if(was_off) {
    off();
  }

  return cca;

}
/*---------------------------------------------------------------------------*/
/*
 * Check if the radio driver is currently receiving a packet.
 *
 * nullrdc uses this function
 * - to detect a collision before transmit()
 * - to detect an incoming ACK
 */
static int
receiving_packet(void)
{

  int ret = 0;

  if((rf_flags & (RF_ON | RF_TX_ACTIVE)) == RF_ON) {
    /* We are on and not in TX */
    if((cc1200_arch_gpio0_read_pin() == 1) || (rx_pkt_len != 0)) {

      /*
       * SYNC word found or packet just received. Changing the criteria
       * for this event might make it necessary to review the MAC timing
       * parameters! Instead of (or in addition to) using GPIO0 we could also
       * read out MODEM_STATUS1 (e.g. PQT reached), but this would not change
       * the situation at least for nullrdc as it uses two "blocking" timers
       * (does not perform polling...). Therefore the overall timing
       * of the ACK handling wouldn't change. It would just allow to detect an
       * incoming packet a little bit earlier and help us with respect to
       * collision avoidance (why not use channel_clear() in nullrdc
       * at this point?).
       */

      ret = 1;

    }
  }

  INFO("RF: Receiving (%d)\n", ret);
  return ret;

}
/*---------------------------------------------------------------------------*/
/* Check if the radio driver has just received a packet. */
static int
pending_packet(void)
{

  INFO("RF: Pending (%d)\n", ((rx_pkt_len != 0) ? 1 : 0));
  return (rx_pkt_len != 0) ? 1 : 0;

}
/*---------------------------------------------------------------------------*/
/* Turn the radio on. */
static int
on(void)
{

  INFO("RF: On\n");

  /* Don't turn on if we are on already */
  if(!(rf_flags & RF_ON)) {

    if(SPI_IS_LOCKED()) {
      return 0;
    }

    LOCK_SPI();

    /* Wake-up procedure. Wait for GPIO0 to de-assert (CHIP_RDYn) */
    cc1200_arch_spi_select();
    BUSYWAIT_UNTIL((cc1200_arch_gpio0_read_pin() == 0),
                   RTIMER_SECOND / 100);
    RF_ASSERT((cc1200_arch_gpio0_read_pin() == 0));
    cc1200_arch_spi_deselect();

    rf_flags |= RF_ON;

    /* Radio is IDLE now, re-configure GPIO0 (modified inside off()) */
    single_write(CC1200_IOCFG0, GPIO0_IOCFG);

    /* Turn on RX */
    idle_calibrate_rx();

    RELEASE_SPI();

#if CC1200_USE_RX_WATCHDOG
    PROCESS_CONTEXT_BEGIN(&cc1200_process);
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_CONTEXT_END(&cc1200_process);
#endif /* #if CC1200_USE_RX_WATCHDOG */

  } else {
    INFO("RF: Already on\n");
  }

  return 1;

}
/*---------------------------------------------------------------------------*/
/* Turn the radio off. */
static int
off(void)
{

  INFO("RF: Off\n");

  /* Don't turn off if we are off already */
  if(rf_flags & RF_ON) {

    if(SPI_IS_LOCKED()) {
      return 0;
    }

    LOCK_SPI();

    idle();

    /*
     * As we use GPIO as CHIP_RDYn signal on wake-up / on(),
     * we re-configure it for CHIP_RDYn.
     */
    single_write(CC1200_IOCFG0, CC1200_IOCFG_RXFIFO_CHIP_RDY_N);

    /* Say goodbye ... */
    strobe(CC1200_SPWD);

    /* Clear all but the initialized flag */
    rf_flags = RF_INITIALIZED;

    RELEASE_SPI();

#if CC1200_USE_RX_WATCHDOG
    etimer_stop(&et);
#endif /* #if CC1200_USE_RX_WATCHDOG */

  } else {
    INFO("RF: Already off\n");
  }

  return 1;

}
/*---------------------------------------------------------------------------*/
/* Get a radio parameter value. */
static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{

  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }

  switch(param) {
  case RADIO_PARAM_POWER_MODE:

    if(rf_flags & RF_ON) {
      *value = (radio_value_t)RADIO_POWER_MODE_ON;
    } else {
      *value = (radio_value_t)RADIO_POWER_MODE_OFF;
    }
    return RADIO_RESULT_OK;

  case RADIO_PARAM_CHANNEL:

    *value = (radio_value_t)rf_channel;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_PAN_ID:
  case RADIO_PARAM_16BIT_ADDR:

    return RADIO_RESULT_NOT_SUPPORTED;

  case RADIO_PARAM_RX_MODE:

    *value = (radio_value_t)rx_mode_value;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_TX_MODE:

    *value = (radio_value_t)tx_mode_value;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_TXPOWER:

    *value = (radio_value_t)txpower;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_CCA_THRESHOLD:

    *value = (radio_value_t)cca_threshold;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_RSSI:
  case RADIO_PARAM_64BIT_ADDR:

    return RADIO_RESULT_NOT_SUPPORTED;

  case RADIO_CONST_CHANNEL_MIN:

    *value = (radio_value_t)CC1200_RF_CFG.min_channel;
    return RADIO_RESULT_OK;

  case RADIO_CONST_CHANNEL_MAX:

    *value = (radio_value_t)CC1200_RF_CFG.max_channel;
    return RADIO_RESULT_OK;

  case RADIO_CONST_TXPOWER_MIN:

    *value = (radio_value_t)CC1200_CONST_TX_POWER_MIN;
    return RADIO_RESULT_OK;

  case RADIO_CONST_TXPOWER_MAX:

    *value = (radio_value_t)CC1200_RF_CFG.max_txpower;
    return RADIO_RESULT_OK;

  default:

    return RADIO_RESULT_NOT_SUPPORTED;

  }

}
/*---------------------------------------------------------------------------*/
/* Set a radio parameter value. */
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

    if(set_channel(value) == CHANNEL_OUT_OF_LIMITS) {
      return RADIO_RESULT_INVALID_VALUE;
    }

    /*
     * We always return OK here even if the channel update was
     * postponed. rf_channel is NOT updated in this case until
     * the channel update was performed. So reading back
     * the channel using get_value() might return the "old" channel
     * until the channel was actually changed
     */

    return RADIO_RESULT_OK;

  case RADIO_PARAM_PAN_ID:
  case RADIO_PARAM_16BIT_ADDR:

    return RADIO_RESULT_NOT_SUPPORTED;

  case RADIO_PARAM_RX_MODE:

    rx_mode_value = value;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_TX_MODE:

    tx_mode_value = value;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_TXPOWER:

    if(value > (radio_value_t)CC1200_RF_CFG.max_txpower) {
      value = (radio_value_t)CC1200_RF_CFG.max_txpower;
    }

    if(value < (radio_value_t)CC1200_CONST_TX_POWER_MIN) {
      value = (radio_value_t)CC1200_CONST_TX_POWER_MIN;
    }

    /* We update the output power as soon as we transmit the next packet */
    new_txpower = (int8_t)value;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_CCA_THRESHOLD:

    if(value > (radio_value_t)CC1200_CONST_CCA_THRESHOLD_MAX) {
      value = (radio_value_t)CC1200_CONST_CCA_THRESHOLD_MAX;
    }

    if(value < (radio_value_t)CC1200_CONST_CCA_THRESHOLD_MIN) {
      value = (radio_value_t)CC1200_CONST_CCA_THRESHOLD_MIN;
    }

    /* When to update the threshold? Let's do it in channel_clear() ... */
    new_cca_threshold = (int8_t)value;
    return RADIO_RESULT_OK;

  case RADIO_PARAM_RSSI:
  case RADIO_PARAM_64BIT_ADDR:

  default:

    return RADIO_RESULT_NOT_SUPPORTED;

  }

}
/*---------------------------------------------------------------------------*/
/* Get a radio parameter object. */
static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{

  return RADIO_RESULT_NOT_SUPPORTED;

}
/*---------------------------------------------------------------------------*/
/* Set a radio parameter object. */
static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{

  return RADIO_RESULT_NOT_SUPPORTED;

}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * CC1200 low level functions
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* Send a command strobe. */
static uint8_t
strobe(uint8_t strobe)
{

  uint8_t ret;

  cc1200_arch_spi_select();
  ret = cc1200_arch_spi_rw_byte(strobe);
  cc1200_arch_spi_deselect();

  return ret;

}
/*---------------------------------------------------------------------------*/
/* Reset CC1200. */
static void
reset(void)
{

  cc1200_arch_spi_select();
  cc1200_arch_spi_rw_byte(CC1200_SRES);
  /*
   * Here we should wait for SO to go low again.
   * As we don't have access to this pin we just wait for 100µs.
   */
  clock_delay(100);
  cc1200_arch_spi_deselect();

}
/*---------------------------------------------------------------------------*/
/* Write a single byte to the specified address. */
static uint8_t
single_write(uint16_t addr, uint8_t val)
{

  uint8_t ret;

  cc1200_arch_spi_select();
  if(CC1200_IS_EXTENDED_ADDR(addr)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_WRITE_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)addr);
  } else {
    cc1200_arch_spi_rw_byte(addr | CC1200_WRITE_BIT);
  }
  ret = cc1200_arch_spi_rw_byte(val);
  cc1200_arch_spi_deselect();

  return ret;

}
/*---------------------------------------------------------------------------*/
/* Read a single byte from the specified address. */
static uint8_t
single_read(uint16_t addr)
{

  uint8_t ret;

  cc1200_arch_spi_select();
  if(CC1200_IS_EXTENDED_ADDR(addr)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_READ_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)addr);
  } else {
    cc1200_arch_spi_rw_byte(addr | CC1200_READ_BIT);
  }
  ret = cc1200_arch_spi_rw_byte(0);
  cc1200_arch_spi_deselect();

  return ret;

}
/*---------------------------------------------------------------------------*/
/* Write a burst of bytes starting at the specified address. */
static void
burst_write(uint16_t addr, const uint8_t *data, uint8_t data_len)
{

  cc1200_arch_spi_select();
  if(CC1200_IS_EXTENDED_ADDR(addr)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_BURST_WRITE_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)addr);
  } else {
    cc1200_arch_spi_rw_byte(addr | CC1200_WRITE_BIT | CC1200_BURST_BIT);
  }
  cc1200_arch_spi_rw(NULL, data, data_len);
  cc1200_arch_spi_deselect();

}
/*---------------------------------------------------------------------------*/
/* Read a burst of bytes starting at the specified address. */
static void
burst_read(uint16_t addr, uint8_t *data, uint8_t data_len)
{

  cc1200_arch_spi_select();
  if(CC1200_IS_EXTENDED_ADDR(addr)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_BURST_READ_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)addr);
  } else {
    cc1200_arch_spi_rw_byte(addr | CC1200_READ_BIT | CC1200_BURST_BIT);
  }
  cc1200_arch_spi_rw(data, NULL, data_len);
  cc1200_arch_spi_deselect();

}
/*---------------------------------------------------------------------------*/
/* Write a list of register settings. */
static void
write_reg_settings(const registerSetting_t *reg_settings,
                   uint16_t sizeof_reg_settings)
{

  int i = sizeof_reg_settings / sizeof(registerSetting_t);

  if(reg_settings != NULL) {
    while(i--) {
      single_write(reg_settings->addr,
                   reg_settings->val);
      reg_settings++;
    }
  }

}
/*---------------------------------------------------------------------------*/
/* Configure the radio (write basic configuration). */
static void
configure(void)
{

  uint8_t reg;
#if CC1200_RF_TESTMODE
  uint32_t freq;
#endif

  /*
   * As we only write registers which are different from the chip's reset
   * state, let's assure that the chip is in a clean state
   */
  reset();

  /* Write the configuration as exported from SmartRF Studio */
  write_reg_settings(CC1200_RF_CFG.register_settings,
                     CC1200_RF_CFG.size_of_register_settings);

  /* Write frequency offset */
#if CC1200_FREQ_OFFSET
  /* MSB */
  single_write(CC1200_FREQOFF1, (uint8_t)(CC1200_FREQ_OFFSET >> 8));
  /* LSB */
  single_write(CC1200_FREQOFF0, (uint8_t)(CC1200_FREQ_OFFSET));
#endif

  /* RSSI offset */
  single_write(CC1200_AGC_GAIN_ADJUST, (int8_t)CC1200_RF_CFG.rssi_offset);

  /***************************************************************************
   * RF test modes needed during hardware development
   **************************************************************************/

#if (CC1200_RF_TESTMODE == 1) || (CC1200_RF_TESTMODE == 2)

  strobe(CC1200_SFTX);
  single_write(CC1200_TXFIRST, 0);
  single_write(CC1200_TXLAST, 0xFF);
  update_txpower(CC1200_CONST_TX_POWER_MAX);
  single_write(CC1200_PKT_CFG2, 0x02);
  freq = calculate_freq(CC1200_DEFAULT_CHANNEL - CC1200_RF_CFG.min_channel);
  single_write(CC1200_FREQ0, ((uint8_t *)&freq)[0]);
  single_write(CC1200_FREQ1, ((uint8_t *)&freq)[1]);
  single_write(CC1200_FREQ2, ((uint8_t *)&freq)[2]);

  printf("RF: Freq0 0x%02x\n",  ((uint8_t *)&freq)[0]);
  printf("RF: Freq1 0x%02x\n",  ((uint8_t *)&freq)[1]);
  printf("RF: Freq2 0x%02x\n",  ((uint8_t *)&freq)[2]);

#if (CC1200_RF_TESTMODE == 1)
  single_write(CC1200_SYNC_CFG1, 0xE8);
  single_write(CC1200_PREAMBLE_CFG1, 0x00);
  single_write(CC1200_MDMCFG1, 0x46);
  single_write(CC1200_PKT_CFG0, 0x40);
  single_write(CC1200_FS_DIG1, 0x07);
  single_write(CC1200_FS_DIG0, 0xAA);
  single_write(CC1200_FS_DVC1, 0xFF);
  single_write(CC1200_FS_DVC0, 0x17);
#endif

#if (CC1200_RF_TESTMODE == 2)
  single_write(CC1200_SYNC_CFG1, 0xE8);
  single_write(CC1200_PREAMBLE_CFG1, 0x00);
  single_write(CC1200_MDMCFG1, 0x06);
  single_write(CC1200_PA_CFG1, 0x3F);
  single_write(CC1200_MDMCFG2, 0x03);
  single_write(CC1200_FS_DIG1, 0x07);
  single_write(CC1200_FS_DIG0, 0xAA);
  single_write(CC1200_FS_DVC0, 0x17);
  single_write(CC1200_SERIAL_STATUS, 0x08);
#endif

  strobe(CC1200_STX);

  while(1) {
#if (CC1200_RF_TESTMODE == 1)
    watchdog_periodic();
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 10);
    leds_off(LEDS_YELLOW);
    leds_on(LEDS_RED);
    watchdog_periodic();
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 10);
    leds_off(LEDS_RED);
    leds_on(LEDS_YELLOW);
#else
    watchdog_periodic();
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 10);
    leds_off(LEDS_GREEN);
    leds_on(LEDS_RED);
    watchdog_periodic();
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 10);
    leds_off(LEDS_RED);
    leds_on(LEDS_GREEN);
#endif
  }

#elif (CC1200_RF_TESTMODE == 3)

  /* CS on GPIO3 */
  single_write(CC1200_IOCFG3, CC1200_IOCFG_CARRIER_SENSE);
  single_write(CC1200_IOCFG2, CC1200_IOCFG_SERIAL_CLK);
  single_write(CC1200_IOCFG0, CC1200_IOCFG_SERIAL_RX);
  update_cca_threshold(CC1200_RF_CFG.cca_threshold);
  freq = calculate_freq(CC1200_DEFAULT_CHANNEL - CC1200_RF_CFG.min_channel);
  single_write(CC1200_FREQ0, ((uint8_t *)&freq)[0]);
  single_write(CC1200_FREQ1, ((uint8_t *)&freq)[1]);
  single_write(CC1200_FREQ2, ((uint8_t *)&freq)[2]);
  strobe(CC1200_SRX);

  while(1) {

    watchdog_periodic();
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 10);
    leds_off(LEDS_GREEN);
    leds_on(LEDS_YELLOW);
    watchdog_periodic();
    BUSYWAIT_UNTIL(0, RTIMER_SECOND / 10);
    leds_off(LEDS_YELLOW);
    leds_on(LEDS_GREEN);
    clock_delay_usec(1000);

    /* CS on GPIO3 */
    if(cc1200_arch_gpio3_read_pin() == 1) {
      leds_on(LEDS_RED);
    } else {
      leds_off(LEDS_RED);
    }

  }

#endif /* #if CC1200_RF_TESTMODE == ... */

  /***************************************************************************
   * Set the stuff we need for this driver to work. Don't touch!
   **************************************************************************/

  /* GPIOx configuration */
  single_write(CC1200_IOCFG3, GPIO3_IOCFG);
  single_write(CC1200_IOCFG2, GPIO2_IOCFG);
  single_write(CC1200_IOCFG0, GPIO0_IOCFG);

  reg = single_read(CC1200_SETTLING_CFG);
  /*
   * Turn of auto calibration. This gives us better control
   * over the timing (RX/TX & TX /RX turnaround!). We calibrate manually:
   * - Upon wake-up (on())
   * - Before going to TX (transmit())
   * - When setting an new channel (set_channel())
   */
  reg &= ~(3 << 3);
#if CC1200_AUTOCAL
  /* We calibrate when going from idle to RX or TX */
  reg |= (1 << 3);
#endif
  single_write(CC1200_SETTLING_CFG, reg);

  /* Configure RXOFF_MODE */
  reg = single_read(CC1200_RFEND_CFG1);
  reg &= ~(3 << 4);                         /* RXOFF_MODE = IDLE */
#if RXOFF_MODE_RX
  reg |= (3 << 4);                          /* RXOFF_MODE = RX */
#endif
  reg |= 0x0F;                              /* Disable RX timeout */
  single_write(CC1200_RFEND_CFG1, reg);

  /* Configure TXOFF_MODE */
  reg = single_read(CC1200_RFEND_CFG0);
  reg &= ~(3 << 4);                         /* TXOFF_MODE = IDLE */
#if TXOFF_MODE_RX
  reg |= (3 << 4);                          /* TXOFF_MODE = RX */
#endif
  single_write(CC1200_RFEND_CFG0, reg);

  /*
   * CCA Mode 0: Always give clear channel indication.
   * CCA is done "by hand". Keep in mind: automatic CCA would also
   * affect the transmission of the ACK and is not implemented yet!
   */
#if CC1200_802154G
  single_write(CC1200_PKT_CFG2, (1 << 5));
#else
  single_write(CC1200_PKT_CFG2, 0x00);
#endif

  /* Configure appendix */
  reg = single_read(CC1200_PKT_CFG1);
#if APPEND_STATUS
  reg |= (1 << 0);
#else
  reg &= ~(1 << 0);
#endif
  single_write(CC1200_PKT_CFG1, reg);

  /* Variable packet length mode */
  reg = single_read(CC1200_PKT_CFG0);
  reg &= ~(3 << 5);
  reg |= (1 << 5);
  single_write(CC1200_PKT_CFG0, reg);

#ifdef FIFO_THRESHOLD
  /* FIFO threshold */
  single_write(CC1200_FIFO_CFG, FIFO_THRESHOLD);
#endif

}
/*---------------------------------------------------------------------------*/
/* Return the radio's state. */
static uint8_t
state(void)
{

#if STATE_USES_MARC_STATE
  return single_read(CC1200_MARCSTATE) & 0x1f;
#else
  return strobe(CC1200_SNOP) & 0x70;
#endif

}
/*---------------------------------------------------------------------------*/
#if !CC1200_AUTOCAL
/* Perform manual calibration. */
static void
calibrate(void)
{

#ifdef RF_FORCE_CALIBRATION
  if (!(rf_flags & RF_FORCE_CALIBRATION) 
      && ((clock_seconds() - cal_timer) < CC1200_CAL_TIMEOUT_SECONDS)) {
    /* Timeout not reached, defer calibration... */
    return;
  }
  rf_flags &= ~RF_FORCE_CALIBRATION;
#endif

  INFO("RF: Calibrate\n");

  strobe(CC1200_SCAL);
  BUSYWAIT_UNTIL_STATE(STATE_CALIBRATE, RTIMER_SECOND / 100);
  BUSYWAIT_UNTIL_STATE(STATE_IDLE, RTIMER_SECOND / 100);

#if CC1200_CAL_TIMEOUT_SECONDS
  cal_timer = clock_seconds();
#endif

}
#endif
/*---------------------------------------------------------------------------*/
/* Enter IDLE state. */
static void
idle(void)
{

  uint8_t s;

  DISABLE_GPIO_INTERRUPTS();

  TX_LEDS_OFF();
  RX_LEDS_OFF();

  ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);

  s = state();

  if(s == STATE_IDLE) {
    return;
  } else if(s == STATE_RX_FIFO_ERR) {
    WARNING("RF: RX FIFO error!\n");
    strobe(CC1200_SFRX);
  } else if(s == STATE_TX_FIFO_ERR) {
    WARNING("RF: TX FIFO error!\n");
    strobe(CC1200_SFTX);
  }

  strobe(CC1200_SIDLE);
  BUSYWAIT_UNTIL_STATE(STATE_IDLE, RTIMER_SECOND / 100);

} /* idle(), 21.05.2015 */
/*---------------------------------------------------------------------------*/
/* Enter RX state. */
static void
idle_calibrate_rx(void)
{

  RF_ASSERT(state() == STATE_IDLE);

#if !CC1200_AUTOCAL
  calibrate();
#endif

  rf_flags &= ~RF_RX_PROCESSING_PKT;
  strobe(CC1200_SFRX);
  strobe(CC1200_SRX);
  BUSYWAIT_UNTIL_STATE(STATE_RX, RTIMER_SECOND / 100);

  ENABLE_GPIO_INTERRUPTS();

  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

}
/*---------------------------------------------------------------------------*/
/* Restart RX from within RX interrupt. */
static void
rx_rx(void)
{

  uint8_t s = state();

  if(s == STATE_IDLE) {
    /* Proceed to rx */
  } else if(s == STATE_RX_FIFO_ERR) {
    WARNING("RF: RX FIFO error!\n");
    strobe(CC1200_SFRX);
  } else if(s == STATE_TX_FIFO_ERR) {
    WARNING("RF: TX FIFO error!\n");
    strobe(CC1200_SFTX);
  } else {
    strobe(CC1200_SIDLE);
    BUSYWAIT_UNTIL_STATE(STATE_IDLE,
                         RTIMER_SECOND / 100);
  }

  RX_LEDS_OFF();
  rf_flags &= ~RF_RX_PROCESSING_PKT;

  /* Clear pending GPIO interrupts */
  ENABLE_GPIO_INTERRUPTS();

  strobe(CC1200_SFRX);
  strobe(CC1200_SRX);
  BUSYWAIT_UNTIL_STATE(STATE_RX, RTIMER_SECOND / 100);

}
/*---------------------------------------------------------------------------*/
/* Fill TX FIFO, start TX and wait for TX to complete (blocking!). */
static int
idle_tx_rx(const uint8_t *payload, uint16_t payload_len)
{

#if (CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN))
  uint16_t bytes_left_to_write;
  uint8_t to_write;
  const uint8_t *p;
#endif

#if CC1200_802154G
  /* Prepare PHR for 802.15.4g frames */
  struct {
    uint8_t phra;
    uint8_t phrb;
  } phr;
#if CC1200_802154G_CRC16
  payload_len += 2;
#else
  payload_len += 4;
#endif
  /* Frame length */
  phr.phrb = (uint8_t)(payload_len & 0x00FF);
  phr.phra = (uint8_t)((payload_len >> 8) & 0x0007);
#if CC1200_802154G_WHITENING
  /* Enable Whitening */
  phr.phra |= (1 << 3);
#endif /* #if CC1200_802154G_WHITENING */
#if CC1200_802154G_CRC16
  /* FCS type 1, 2 Byte CRC */
  phr.phra |= (1 << 4);
#endif /* #if CC1200_802154G_CRC16 */
#endif /* #if CC1200_802154G */

  /* Prepare for RX */
  rf_flags &= ~RF_RX_PROCESSING_PKT;
  strobe(CC1200_SFRX);

  /* Flush TX FIFO */
  strobe(CC1200_SFTX);

#if USE_SFSTXON
  /*
   * Enable synthesizer. Saves us a few µs especially if it takes
   * long enough to fill the FIFO. This strobe must not be
   * send before SFTX!
   */
  strobe(CC1200_SFSTXON);
#endif

  /* Configure GPIO0 to detect TX state */
  single_write(CC1200_IOCFG0, CC1200_IOCFG_MARC_2PIN_STATUS_0);

#if (CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN))
  /*
   * We already checked that GPIO2 is used if
   * CC1200_MAX_PAYLOAD_LEN > 127 / 126 in the header of this file
   */
  single_write(CC1200_IOCFG2, CC1200_IOCFG_TXFIFO_THR);
#endif

#if CC1200_802154G
  /* Write PHR */
  burst_write(CC1200_TXFIFO, (uint8_t *)&phr, PHR_LEN);
#else
  /* Write length byte */
  burst_write(CC1200_TXFIFO, (uint8_t *)&payload_len, PHR_LEN);
#endif /* #if CC1200_802154G */

  /*
   * Fill FIFO with data. If SPI is slow it might make sense
   * to divide this process into several chunks.
   * The best solution would be to perform TX FIFO refill
   * using an interrupt, but we are blocking here (= in TX) anyway...
   */

#if (CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN))
  to_write = MIN(payload_len, (CC1200_FIFO_SIZE - PHR_LEN));
  burst_write(CC1200_TXFIFO, payload, to_write);
  bytes_left_to_write = payload_len - to_write;
  p = payload + to_write;
#else
  burst_write(CC1200_TXFIFO, payload, payload_len);
#endif

#if USE_SFSTXON
  /* Wait for synthesizer to be ready */
  BUSYWAIT_UNTIL_STATE(STATE_FSTXON, RTIMER_SECOND / 100);
#endif

  /* Start TX */
  strobe(CC1200_STX);

  /* Wait for TX to start. */
  BUSYWAIT_UNTIL((cc1200_arch_gpio0_read_pin() == 1), RTIMER_SECOND / 100);

  /* Turned off at the latest in idle() */
  TX_LEDS_ON();

  /* Turned off at the latest in idle() */
  ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

  if((cc1200_arch_gpio0_read_pin() == 0) &&
     (single_read(CC1200_NUM_TXBYTES) != 0)) {

    /*
     * TX didn't start in time. We also check NUM_TXBYES
     * in case we missed the rising edge of the GPIO signal
     */

    ERROR("RF: TX doesn't start!\n");
#if (CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN))
    single_write(CC1200_IOCFG2, GPIO2_IOCFG);
#endif
    idle();

    /* Re-configure GPIO0 */
    single_write(CC1200_IOCFG0, GPIO0_IOCFG);

    return RADIO_TX_ERR;

  }

#if (CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN))
  if(bytes_left_to_write != 0) {
    rtimer_clock_t t0;
    uint8_t s;
    t0 = RTIMER_NOW();
    do {
      if((bytes_left_to_write != 0) &&
         (cc1200_arch_gpio2_read_pin() == 0)) {
        /* TX TIFO is drained below FIFO_THRESHOLD. Re-fill... */
        to_write = MIN(bytes_left_to_write, FIFO_THRESHOLD);
        burst_write(CC1200_TXFIFO, p, to_write);
        bytes_left_to_write -= to_write;
        p += to_write;
        t0 += CC1200_RF_CFG.tx_pkt_lifetime;
      }
    } while((cc1200_arch_gpio0_read_pin() == 1) &&
            RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + CC1200_RF_CFG.tx_pkt_lifetime));

    /*
     * At this point we either left TX or a timeout occurred. If all went
     * well, we are in RX (or at least settling) now.
     * If we didn't manage to refill the TX FIFO, an underflow might
     * have occur-ed - the radio might be still in TX here!
     */

    s = state();
    if((s != STATE_RX) && (s != STATE_SETTLING)) {

      /*
       * Something bad happened. Wait for radio to enter a
       * stable state (in case of an error we are in TX here)
       */

      INFO("RF: TX failure!\n");
      BUSYWAIT_UNTIL((state() != STATE_TX), RTIMER_SECOND / 100);
      /* Re-configure GPIO2 */
      single_write(CC1200_IOCFG2, GPIO2_IOCFG);
      idle();

      /* Re-configure GPIO0 */
      single_write(CC1200_IOCFG0, GPIO0_IOCFG);

      return RADIO_TX_ERR;

    }

  } else {
    /* Wait for TX to complete */
    BUSYWAIT_UNTIL((cc1200_arch_gpio0_read_pin() == 0),
                   CC1200_RF_CFG.tx_pkt_lifetime);
  }
#else
  /* Wait for TX to complete */
  BUSYWAIT_UNTIL((cc1200_arch_gpio0_read_pin() == 0),
                 CC1200_RF_CFG.tx_pkt_lifetime);
#endif

  if(cc1200_arch_gpio0_read_pin() == 1) {
    /* TX takes to long - abort */
    ERROR("RF: TX takes to long!\n");
#if (CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN))
    /* Re-configure GPIO2 */
    single_write(CC1200_IOCFG2, GPIO2_IOCFG);
#endif
    idle();

    /* Re-configure GPIO0 */
    single_write(CC1200_IOCFG0, GPIO0_IOCFG);

    return RADIO_TX_ERR;

  }

#if (CC1200_MAX_PAYLOAD_LEN > (CC1200_FIFO_SIZE - PHR_LEN))
  /* Re-configure GPIO2 */
  single_write(CC1200_IOCFG2, GPIO2_IOCFG);
#endif

  /* Re-configure GPIO0 */
  single_write(CC1200_IOCFG0, GPIO0_IOCFG);

  TX_LEDS_OFF();

  ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
  ENERGEST_ON(ENERGEST_TYPE_LISTEN);

  return RADIO_TX_OK;

}
/*---------------------------------------------------------------------------*/
/* Update TX power */
static void
update_txpower(int8_t txpower_dbm)
{

  uint8_t reg = single_read(CC1200_PA_CFG1);

  reg &= ~0x3F;
  /* Up to now we don't handle the special power levels PA_POWER_RAMP < 3 */
  reg |= ((((txpower_dbm + 18) * 2) - 1) & 0x3F);
  single_write(CC1200_PA_CFG1, reg);

  txpower = txpower_dbm;

}
/*---------------------------------------------------------------------------*/
/* Update CCA threshold */
static void
update_cca_threshold(int8_t threshold_dbm)
{

  single_write(CC1200_AGC_CS_THR, (uint8_t)threshold_dbm);
  cca_threshold = threshold_dbm;

}
/*---------------------------------------------------------------------------*/
/* Calculate FREQ register from channel */
static uint32_t
calculate_freq(uint8_t channel)
{

  uint32_t freq;

  freq = CC1200_RF_CFG.chan_center_freq0 + (channel * CC1200_RF_CFG.chan_spacing) / 1000 /* /1000 because chan_spacing is in Hz */;
  freq *= FREQ_MULTIPLIER;
  freq /= FREQ_DIVIDER;

  return freq;

}
/*---------------------------------------------------------------------------*/
/* Update rf channel if possible, else postpone it (->pollhandler) */
static int
set_channel(uint8_t channel)
{

  uint8_t was_off = 0;
  uint32_t freq;

#if 0
  /*
   * We explicitly allow a channel update even if the channel does not change.
   * This feature can be used to manually force a calibration.
   */
  if(channel == rf_channel) {
    return rf_channel;
  }
#endif

  if(channel < CC1200_RF_CFG.min_channel ||
     channel > CC1200_RF_CFG.max_channel) {
    /* Invalid channel */
    return CHANNEL_OUT_OF_LIMITS;
  }

  if(SPI_IS_LOCKED() || (rf_flags & RF_TX_ACTIVE) || receiving_packet()) {

    /* We are busy, postpone channel update */

    new_rf_channel = channel;
    rf_flags |= RF_UPDATE_CHANNEL;
    process_poll(&cc1200_process);
    INFO("RF: Channel update postponed\n");

    return CHANNEL_UPDATE_POSTPONED;

  }
  rf_flags &= ~RF_UPDATE_CHANNEL;

  INFO("RF: Channel update (%d)\n", channel);

  if(!(rf_flags & RF_ON)) {
    was_off = 1;
    on();
  }

  LOCK_SPI();

  idle();

  freq = calculate_freq(channel - CC1200_RF_CFG.min_channel);
  single_write(CC1200_FREQ0, ((uint8_t *)&freq)[0]);
  single_write(CC1200_FREQ1, ((uint8_t *)&freq)[1]);
  single_write(CC1200_FREQ2, ((uint8_t *)&freq)[2]);

  rf_channel = channel;

  /* Turn on RX again unless we turn off anyway */
  if(!was_off) {
#ifdef RF_FORCE_CALIBRATION
    rf_flags |= RF_FORCE_CALIBRATION;
#endif
    idle_calibrate_rx();
  }

  RELEASE_SPI();

  if(was_off) {
    off();
  }

  return CHANNEL_UPDATE_SUCCEEDED;

}
/*---------------------------------------------------------------------------*/
/* Check broadcast address. */
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
addr_check_auto_ack(uint8_t *frame, uint16_t frame_len)
{

  frame802154_t info154;

  if(frame802154_parse(frame, frame_len, &info154) != 0) {

    /* We received a valid 802.15.4 frame */

    if(!(rx_mode_value & RADIO_RX_MODE_ADDRESS_FILTER) ||
       info154.fcf.frame_type == FRAME802154_ACKFRAME ||
       is_broadcast_addr(info154.fcf.dest_addr_mode,
                         (uint8_t *)&info154.dest_addr) ||
       linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
                    &linkaddr_node_addr)) {

      /* 
       * Address check succeeded or address filter disabled. 
       * We send an ACK in case a corresponding data frame
       * is received even in promiscuous mode (if auto-ack is
       * enabled)!
       */

      if((rx_mode_value & RADIO_RX_MODE_AUTOACK) &&
         info154.fcf.frame_type == FRAME802154_DATAFRAME &&
         info154.fcf.ack_required != 0 &&
         (!(rx_mode_value & RADIO_RX_MODE_ADDRESS_FILTER) || 
          linkaddr_cmp((linkaddr_t *)&info154.dest_addr,
                       &linkaddr_node_addr))) {

        /* 
         * Data frame destined for us & ACK request bit set -> send ACK.
         * Make sure the preamble length is configured accordingly as
         * MAC timing parameters rely on this!
         */

        uint8_t ack[ACK_LEN] = { FRAME802154_ACKFRAME, 0, info154.seq };

#if (RXOFF_MODE_RX == 1)
        /*
         * This turns off GPIOx interrupts. Make sure they are turned on
         * in rx_rx() later on!
         */
        idle();
#endif
        
        idle_tx_rx((const uint8_t *)ack, ACK_LEN);
        
        /* rx_rx() will follow */
        
        return ADDR_CHECK_OK_ACK_SEND;
        
      }

      return ADDR_CHECK_OK;
      
    } else {

      return ADDR_CHECK_FAILED;

    }

  }
  
  return INVALID_FRAME;

}
/*---------------------------------------------------------------------------*/
/*
 * The CC1200 interrupt handler: called by the hardware interrupt
 * handler, which is defined as part of the cc1200-arch interface.
 */
int
cc1200_rx_interrupt(void)
{

  /* The radio's state */
  uint8_t s;
  /* The number of bytes in the RX FIFO waiting for read-out */
  uint8_t num_rxbytes;
  /* The payload length read as the first byte from the RX FIFO */
  static uint16_t payload_len;
  /*
   * The number of bytes already read out and placed in the
   * intermediate buffer
   */
  static uint16_t bytes_read;
  /*
   * We use an intermediate buffer for the packet before
   * we pass it to the next upper layer. We also place RSSI +
   * LQI in this buffer
   */
  static uint8_t buf[CC1200_MAX_PAYLOAD_LEN + APPENDIX_LEN];

  if(SPI_IS_LOCKED()) {

    /*
     * SPI is in use. Exit and make sure this
     * function is called from the poll handler as soon
     * as SPI is available again
     */

    rf_flags |= RF_POLL_RX_INTERRUPT;
    process_poll(&cc1200_process);
    return 1;

  }
  rf_flags &= ~RF_POLL_RX_INTERRUPT;

  LOCK_SPI();

  /*
   * If CC1200_USE_GPIO2 is enabled, we come here either once RX FIFO
   * threshold is reached (GPIO2 rising edge)
   * or at the end of the packet (GPIO0 falling edge).
   */

  /* Make sure we are in a sane state. Sane means: either RX or IDLE */
  s = state();
  if((s == STATE_RX_FIFO_ERR) || (s == STATE_TX_FIFO_ERR)) {

    rx_rx();
    RELEASE_SPI();
    return 0;

  }

  num_rxbytes = single_read(CC1200_NUM_RXBYTES);

  if(num_rxbytes == 0) {

    /*
     * This might happen from time to time because
     * this function is also called by the pollhandler and / or
     * from TWO interrupts which can occur at the same time.
     */

    INFO("RF: RX FIFO empty!\n");
    RELEASE_SPI();
    return 0;

  }

  if(!(rf_flags & RF_RX_PROCESSING_PKT)) {

#if CC1200_802154G
    struct {
      uint8_t phra;
      uint8_t phrb;
    }
    phr;

    if(num_rxbytes < PHR_LEN) {

      WARNING("RF: PHR incomplete!\n");
      rx_rx();
      RELEASE_SPI();
      return 0;

    }

    burst_read(CC1200_RXFIFO,
               &phr,
               PHR_LEN);
    payload_len = (phr.phra & 0x07);
    payload_len <<= 8;
    payload_len += phr.phrb;

    if(phr.phra & (1 << 4)) {
      /* CRC16, payload_len += 2 */
      payload_len -= 2;
    } else {
      /* CRC16, payload_len += 4 */
      payload_len -= 4;
    }
#else
    /* Read first byte in RX FIFO (payload length) */
    burst_read(CC1200_RXFIFO,
               (uint8_t *)&payload_len,
               PHR_LEN);
#endif

    if(payload_len < ACK_LEN) {
      /* Packet to short. Discard it */
      WARNING("RF: Packet too short!\n");
      RIMESTATS_ADD(tooshort);
      rx_rx();
      RELEASE_SPI();
      return 0;
    }

    if(payload_len > CC1200_MAX_PAYLOAD_LEN) {
      /* Packet to long. Discard it */
      WARNING("RF: Packet to long!\n");
      RIMESTATS_ADD(toolong);
      rx_rx();
      RELEASE_SPI();
      return 0;
    }

    RX_LEDS_ON();
    bytes_read = 0;
    num_rxbytes -= PHR_LEN;

    rf_flags |= RF_RX_PROCESSING_PKT;

    /* Fall through... */

  }

  if(rf_flags & RF_RX_PROCESSING_PKT) {

    /*
     * Read out remaining bytes unless FIFO is empty.
     * We have at least num_rxbytes in the FIFO to be read out.
     */

    if((num_rxbytes + bytes_read) > (payload_len + CC_APPENDIX_LEN)) {

      /*
       * We have a mismatch between the number of bytes in the RX FIFO
       * and the payload_len. This would lead to an buffer overflow,
       * so we catch this error here.
       */

      WARNING("RF: RX length mismatch %d %d %d!\n", num_rxbytes,
              bytes_read,
              payload_len);
      rx_rx();
      RELEASE_SPI();
      return 0;

    }

    burst_read(CC1200_RXFIFO,
               &buf[bytes_read],
               num_rxbytes);

    bytes_read += num_rxbytes;
    num_rxbytes = 0;

    if(bytes_read == (payload_len + CC_APPENDIX_LEN)) {

      /*
       * End of packet. Read appendix (if available), check CRC
       * and copy the data from temporary buffer to rx_pkt
       * RSSI offset already set using AGC_GAIN_ADJUST.GAIN_ADJUSTMENT
       */

#if APPEND_STATUS
      uint8_t crc_lqi = buf[bytes_read - 1];
#else
      int8_t rssi = single_read(CC1200_RSSI1);
      uint8_t crc_lqi = single_read(CC1200_LQI_VAL);
#endif

      if(!(crc_lqi & (1 << 7))) {
        /* CRC error. Drop the packet */
        INFO("RF: CRC error!\n");
        RIMESTATS_ADD(badcrc);
      } else if(rx_pkt_len != 0) {
        /* An old packet is pending. Drop the packet */
        WARNING("RF: Packet pending!\n");
      } else {

        int ret = addr_check_auto_ack(buf, bytes_read);

        if((ret == ADDR_CHECK_OK) ||
           (ret == ADDR_CHECK_OK_ACK_SEND)) {
#if APPEND_STATUS
          /* RSSI + LQI already read out and placed into buf */
#else
          buf[bytes_read++] = (uint8_t)rssi;
          buf[bytes_read++] = crc_lqi;
#endif
          rx_pkt_len = bytes_read;
          memcpy((void *)rx_pkt, buf, rx_pkt_len);
          rx_rx();
          process_poll(&cc1200_process);
          RELEASE_SPI();
          return 1;

        } else {
          /* Invalid address. Drop the packet */
        }

      }

      /* Buffer full, address or CRC check failed */
      rx_rx();
      RELEASE_SPI();
      return 0;

    } /* if (bytes_read == payload_len) */

  }

  RELEASE_SPI();
  return 0;

}
/*---------------------------------------------------------------------------*/
