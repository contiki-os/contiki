#ifndef PDRTEST_COMMON_H
#define PDRTEST_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NUM_CHANNELS 16

#define DEFAULT_CHANNEL  26
//#define DEFAULT_CHANNEL  16

#define TEST_TXPOWER RADIO_POWER_ZERO_DB
#define DEFAULT_TXPOWER RADIO_POWER_ZERO_DB
//#define TEST_TXPOWER RADIO_POWER_MINUS7_DB
//#define TEST_TXPOWER  RADIO_POWER_MINUS15_DB

// test ID (useful e.g. in case of multiple motes having simultaneous tests)
#define TEST_ID 0x1

// --------------------------------------------
// application's user interface: fine-tuning

#define TEST_PACKET_SIZE            (6 + 6 * 10)

#define PACKETS_IN_TEST             100u

// pattern with which to fill the test packet
#define FILL_ZERO                   1
#define FILL_SEQUENTIAL             2
#define FILL_RANDOM                 3

#define PATTERN_FILL  FILL_SEQUENTIAL

#if 0
// good for Z1 and sky
#define PACKET_SEND_INTERVAL      (RTIMER_ARCH_SECOND/200) 
#define PREAMBLE_SEND_INTERVAL      (RTIMER_ARCH_SECOND/512)
#else
// good for RSS2
#define PACKET_SEND_INTERVAL      (RTIMER_ARCH_SECOND/64)
#define PREAMBLE_SEND_INTERVAL      (RTIMER_ARCH_SECOND/256)
#endif


#if CONTIKI_TARGET_Z1 || CONTIKI_TARGET_SKY
// Zolertia Z1 or Sky

#define PLATFORM_RADIO_HEADER "cc2420.h"

#define radio_set_txpower cc2420_set_txpower
#define radio_set_channel cc2420_set_channel
#define radio_get_channel cc2420_get_channel
#define radio_get_rssi    cc2420_rssi

#define RADIO_POWER_MAX        31
#define RADIO_POWER_MIN        0
#define RADIO_POWER_ZERO_DB    31
#define RADIO_POWER_MINUS7_DB  15
#define RADIO_POWER_MINUS15_DB 7

#elif CONTIKI_TARGET_U108 || CONTIKI_TARGET_U108DEV
// U108

// make OPENOCD_CFG=interface/neodb.cfg OPENOCD="sudo openocd" send.upload TARGET=u108dev

#if CONTIKI_TARGET_U108
#define PLATFORM_RADIO_HEADER "cc2520.h"
#else // CONTIKI_TARGET_U108DEV
#define PLATFORM_RADIO_HEADER "cc2520/cc2520-u108.h"
#endif

#define radio_set_txpower cc2520_set_txpower
#define radio_set_channel cc2520_set_channel
#define radio_get_channel cc2520_get_channel
#define radio_get_rssi    cc2520_rssi

// warning: use only these values!
#define RADIO_POWER_MAX        0xF7
#define RADIO_POWER_MIN        0x03
#define RADIO_POWER_ZERO_DB    0x32
#define RADIO_POWER_MINUS7_DB  0x2C
#define RADIO_POWER_MINUS15_DB 0x03 // actually -18 dBm

#elif CONTIKI_TARGET_AVR_RSS2

// To compile:
// make recv.hex TARGET=avr-RSS2

// To upload:
// $ avrdude -p m128rfa1 -c avr109 -P /dev/ttyUSB0 -b 38400 -e -U flash:w:control.avr-RSS2.hex

// Check that avrdude version is correct!

#define PLATFORM_RADIO_HEADER "radio/rf230bb/rf230bb.h"

#define radio_set_txpower rf230_set_txpower
#define radio_set_channel rf230_set_channel
#define radio_get_channel rf230_get_channel
#define radio_get_rssi    rf230_rssi

// see ATmega128RFA1 datasheet page 109
#define RADIO_POWER_MAX        TX_PWR_3DBM
#define RADIO_POWER_MIN        TX_PWR_17_2DBM
#define RADIO_POWER_ZERO_DB    6   // actually 0.5 dBm
#define RADIO_POWER_MINUS7_DB  12  // actually -6.5 dBm
#define RADIO_POWER_MINUS15_DB TX_PWR_17_2DBM  // actually -17.5 dBm


#elif CONTIKI_TARGET_NATIVE || CONTIKI_TARGET_COOJA
// native or Cooja

#define PLATFORM_RADIO_HEADER "cc2420.h"

#define radio_set_txpower(x)
#define radio_set_channel(x)

#define RADIO_POWER_MAX        0x9
#define RADIO_POWER_MIN        0x0
#define RADIO_POWER_ZERO_DB    0x0
#define RADIO_POWER_MINUS7_DB  0x0
#define RADIO_POWER_MINUS15_DB 0x0

#else 
#error No support for your platform!
#endif // CONTIKI_TARGET_xx

// --------------------------------------------
// data structures

struct packetHeader {
    uint16_t sender;
    uint8_t packetNumber;
    uint8_t channel;
    uint8_t __padding;
    uint8_t crc;
};

#define HEADER_SIZE  sizeof(struct packetHeader)

// -------------------------------------------------------------

#define STATE_IDLE         0
#define STATE_PREAMBLE_PREPARE  1
#define STATE_PREAMBLE_TX  2
#define STATE_WAIT         3
#define STATE_TX           4
#define STATE_RX           5

extern bool cc2420_without_send_cca;

// --------------------------------------------
// other generic includes

#include "pattern.h"

#endif
