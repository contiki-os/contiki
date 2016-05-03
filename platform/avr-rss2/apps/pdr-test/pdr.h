#ifndef PDR_H
#define PDR_H

#define VERSION  "1.4-2016-05-03\n"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_CHANNEL  11

#define TEST_TXPOWER RADIO_POWER_ZERO_DB
#define DEFAULT_TXPOWER RADIO_POWER_ZERO_DB
//#define TEST_TXPOWER RADIO_POWER_MINUS7_DB
//#define TEST_TXPOWER  RADIO_POWER_MINUS15_DB

// test ID (useful e.g. in case of multiple motes having simultaneous tests)
#define TEST_ID 0x1

#define NODES_IN_TEST               8  // Max Nodes
#define TEST_PACKET_SIZE            (6 + 6 * 10)
#define PACKETS_IN_TEST             100u

// pattern with which to fill the test packet
#define FILL_ZERO                   1
#define FILL_SEQUENTIAL             2
#define FILL_RANDOM                 3

#define PATTERN_FILL  FILL_SEQUENTIAL

#ifdef CONTIKI_TARGET_AVR_RSS2
// Approximation for RSS2
#define PACKET_SEND_INTERVAL        (RTIMER_ARCH_SECOND/128)
#else
// good for Z1 and sky
#define PACKET_SEND_INTERVAL        (RTIMER_ARCH_SECOND/200) 
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
// make  TARGET=avr-RSS2

// To upload:
// avrdude -p m256rfr2 -c stk500v2 -P /dev/ttyUSB0 -b 38400 -e -U flash:w:pdr.avr-rss2

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
    uint8_t platform_id;
    uint8_t crc;
};

#define HEADER_SIZE  sizeof(struct packetHeader)
#define MAX_NIBBLES ((TEST_PACKET_SIZE - HEADER_SIZE) * 2)

struct stats_info {
    uint16_t node_id;
    uint8_t channel;
    uint8_t platform_id;
    uint16_t fine;
#if TRACK_ERRORS
    uint16_t zeroLength; // usually signal errors at radio driver level
    uint16_t tooShort;
    uint16_t tooLong;
    uint16_t badHeader;
    uint16_t badContents;
    // // set only when badContents=false, but the PHY level reports a CRC error
    uint16_t badPHYCrcGoodContents;
#endif
    uint16_t total;
    // set by radio driver, not included in the total count
    uint16_t phyCrcErrors;

    // number of bad nibbles...
    uint16_t numBadNibbles;
    uint16_t badNibbles[MAX_NIBBLES];
    uint16_t symbolErrors[MAX_NIBBLES];
    uint16_t confusionMatrix[16 * 16];
    uint32_t correctCounts[16];
};

// -------------------------------------------------------------

#define STATE_RX           0
#define STATE_TX           1

extern bool cc2420_without_send_cca;

#ifdef CONTIKI_TARGET_NONE
#define PLATFORM_ID  0
#endif
#ifdef CONTIKI_TARGET_NATIVE
#define PLATFORM_ID  1
#endif
#ifdef CONTIKI_TARGET_COOJA
#define PLATFORM_ID  2
#endif
#ifdef CONTIKI_TARGET_AVR_RSS2
#define PLATFORM_ID  3
#endif
#ifdef CONTIKI_TARGET_Z1
#define PLATFORM_ID  4
#endif
#ifdef CONTIKI_TARGET_SKY
#define PLATFORM_ID  5
#endif
#ifdef CONTIKI_TARGET_U108 
#define PLATFORM_ID  6
#endif
#ifdef CONTIKI_TARGET_U108DEV
#define PLATFORM_ID  7
#endif

char *platform_list[] = { "none", "native", "cooja", "avr-rss2", "z1", "sky", "u108"};

#include "pattern.h"

#endif /* PDR_H */
