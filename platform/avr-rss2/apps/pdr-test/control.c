// -*- mode: contiki-c-mode; c-basic-offset: 4; c-indent-level: 4 -*-
#include "common.h"
#include "sys/process.h"
#include "dev/serial-line.h"
#include "dev/watchdog.h"
#include "lib/random.h"
#include "sys/node-id.h"
#include "dev/button-sensor.h" 
#include "netstack.h"
#include "dev/radio.h"
#include PLATFORM_RADIO_HEADER
#include "codec.h"
#include "dev/leds.h"
#include <stdlib.h>
#include "core/net/packetbuf.h"
#include <avr/wdt.h>

#include "rime/rime.h"

#define DEBUG 0
#define TRACK_ERRORS 0
#define USE_REAL_NODES 1

#ifndef LOCAL_ONLY
#define LOCAL_ONLY 0
#endif

#define GUARD_TIME           (RTIMER_SECOND / 512)
#define PAUSE_BETWEEN_TESTS  (RTIMER_SECOND / 32)

PROCESS(controlProcess, "PDR test control process");
PROCESS_NAME(samplingProcess);

#define MAX_NIBBLES ((TEST_PACKET_SIZE - HEADER_SIZE) * 2)

#define READY_PRINT_INTERVAL (CLOCK_SECOND * 5)

#if 0

// FOR SIMULATOR
const uint16_t SCHEDULE[] = {
    0x7a8c,
    0x85bf,
    0xbcde,
    0xc087,

    0x8ab7,
    0x8d7e,
    0x8e50,
    0x9755,

    0xa191,
    0xa4c5,
//    0x6c0c,
    0xaf14,

    0xc2b5,
    0xb54f,
    0xc400,
    0xc75d
};

#else

const uint16_t SCHEDULE[] = {
  49771,
  49563,
  49704,
  49617,
  49637,
  49706,
  49740,
  49539
  //  49551 Spare
};

#endif

#define NUM_NODES (sizeof(SCHEDULE) / sizeof(SCHEDULE[0]))

#define IS_INITIATOR(node)  (node == SCHEDULE[0])

#define IS_RELAY(node)  (node == SCHEDULE[1]      \
            || node == SCHEDULE[2]      \
            || node == SCHEDULE[3])

#define RELAY_NR(sender)  \
    (sender == SCHEDULE[0] ? 0 :                 \
            (sender == SCHEDULE[1] ? 1 :         \
                    (sender == SCHEDULE[2] ? 2 : \
                            3)))

// -------------------------------------------------------------

uint8_t packetsReceived[PACKETS_IN_TEST];

uint8_t currentScheduleNr;
int testChannel;

uint8_t currentState;

int8_t currentPacketNumber;

struct rtimer rt;

static struct etimer periodic;

struct {
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
} errors;

uint16_t rssiSum;
uint16_t lqiSum;

// needed to link fastrandom.h
uint32_t fastrandomKey;

int8_t numTestsInSenderRole;

// -------------------------------------------------------------

//
// To simplify the code, all internal RSSI calculations are done using non-negative numbers.
// This function maps the resulting value back to the CC2x20-specific valye range.
//
static inline int16_t platformFixRssi(uint16_t rssi)
{
    // for cc2420 and cc2520 based platfroms, the radio RSSI value is signed 8 bit integer
    rssi -= 128;
    return (int16_t) rssi;
}

// -------------------------------------------------------------

extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

static void printfCrc(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
static void printfCrc(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    static char _print_buf[64];
    vsnprintf(_print_buf, sizeof(_print_buf), format, args);
    
    const char *p = _print_buf;
    while (*p) {
        putchar(*p++);
    }

    uint8_t crc = crc8Str(_print_buf);
    putchar(',');
    putchar(to_hex(crc >> 4));
    putchar(to_hex(crc & 0xf));
    putchar('\n');
}

// -------------------------------------------------------------

static int selectNextChannel(void)
{
    static uint8_t channelIndex;
    static uint8_t seq[NUM_CHANNELS];

    if (channelIndex == 0) {
        // generate a new sequence

        int i;
        for (i = 0; i < NUM_CHANNELS; ++i) {
            seq[i] = i;
        }

        // Fisher-Yates algorithm
        for (i = 0; i < NUM_CHANNELS - 1; ++i) {
            // j ← random integer such that i ≤ j < n
            int j = i + (random_rand() >> 2) % (NUM_CHANNELS - i);

            //  exchange a[j] and a[i]
            uint8_t t = seq[j];
            seq[j] = seq[i];
            seq[i] = t;
        }
    }

    int result = seq[channelIndex] + 11; // 11 is the minimal 802.15.4 channel

    channelIndex++;
    if (channelIndex >= NUM_CHANNELS) {
        channelIndex = 0;
    }

    return result;
}

// -------------------------------------------------------------

void clearErrors(void)
{
    memset(&errors, 0, sizeof(errors));
    currentPacketNumber = -1;
}

void printStats(void)
{
    int rssi;
    uint8_t lqi;
    if (errors.fine == 0) {
        rssi = 0;
        lqi = 0;
    } else {
        rssi = platformFixRssi(rssiSum / errors.fine);
        lqi = lqiSum / errors.fine;
    }
//    printfCrc("> %u %d %u %u %u",
//    printfCrc("%u %u %d %u %u %u",
    printf("%u %u %d %u %u %u\n",
            errors.fine, errors.total, rssi, lqi,
            SCHEDULE[currentScheduleNr],
            testChannel);

#if DEBUG
    int i;
    for (i = 0; i < sizeof(packetsReceived); ++i) {
        printf("%u ", packetsReceived[i]);
    }
    putchar('\n');
    memset(packetsReceived, 0, sizeof(packetsReceived));
#endif

    rssiSum = 0;
    lqiSum = 0;
  
#if TRACK_ERRORS
    puts("Error statistics:");
    //printfCrc(" %u total packets, %u/%u/%u length errors, %u/%u corrupt packets",
    printf(" %u total packets, %u/%u/%u length errors, %u/%u corrupt packets\n",
            errors.fine + errors.zeroLength +
            errors.tooShort + errors.tooLong +
            errors.badHeader + errors.badContents +
            errors.badPHYCrcGoodContents,
            errors.zeroLength,
            errors.tooShort, errors.tooLong,
            errors.badHeader,
            errors.badContents);
    if (errors.phyCrcErrors) {
      //printfCrc(" %u bad PHY CRC",  errors.phyCrcErrors);
      printf(" %u bad PHY CRC\n",  errors.phyCrcErrors);
    }
    if (errors.badPHYCrcGoodContents) {
      //printfCrc(" %u bad PHY CRC with corruption undetected", errors.badPHYCrcGoodContents);
      printf(" %u bad PHY CRC with corruption undetected\n", errors.badPHYCrcGoodContents);
    }
#endif
    clearErrors();
}

// -------------------------------------------------------------

uint8_t selectNextState(void)
{
    watchdog_periodic();

    if (currentState == STATE_RX) {
        printStats();
    } else {
        clearErrors();
        // wait a bit
        rtimer_clock_t wait = RTIMER_NOW() + GUARD_TIME; // RTIMER_SECOND / 64;
        while (RTIMER_CLOCK_LT(RTIMER_NOW(), wait));
    }

    currentScheduleNr++;

    // schedule over?
    if (currentScheduleNr == NUM_NODES) {
        currentScheduleNr = 0;

        currentState = STATE_IDLE;

        // dump the noise floor and channel
        printf("R=%d C=%u\n", radio_get_rssi(), radio_get_channel());

        process_poll(&controlProcess);
        process_poll(&samplingProcess);
        radio_set_channel(DEFAULT_CHANNEL);
        //radio_set_txpower(DEFAULT_TXPOWER);
        printf("ns=%d\n", numTestsInSenderRole);
	numTestsInSenderRole = 0;
        return currentState;
    }

    // decide whether to tx or rx
    if (SCHEDULE[currentScheduleNr] == node_id) {
        currentState = STATE_TX;
    } else {
        currentState = STATE_RX;
    }

    return currentState;
}

// -------------------------------------------------------------

void rtimerCallback(struct rtimer *t, void *ptr)
{
    static uint8_t sendBuffer[TEST_PACKET_SIZE] __attribute__((aligned (2))) = {
        0, 0, 0, 10
    };
    struct packetHeader *h = (struct packetHeader *) sendBuffer;

    rtimer_clock_t next = RTIMER_TIME(t);

    switch (currentState) {
    case STATE_IDLE:
        return;

    case STATE_PREAMBLE_PREPARE:
        h->packetNumber = 0;
        h->sender = node_id;
        h->channel = testChannel;
        currentState = STATE_PREAMBLE_TX;
//        cc2420_without_send_cca = true;
        // fallthrough

    case STATE_PREAMBLE_TX:
        h->packetNumber++;
        h->crc = crc8(h, sizeof(*h) - 1);

        NETSTACK_RADIO.send(sendBuffer, HEADER_SIZE /*, 0 */);
        if (h->packetNumber >= PACKETS_IN_TEST) {
            h->packetNumber = 0;
//            cc2420_without_send_cca = false;
            // switch to the first test channel
            radio_set_channel(h->channel);
            //radio_set_txpower(TEST_TXPOWER);
            testChannel = h->channel;
            next += PACKET_SEND_INTERVAL;
            uint8_t skip = 3 - RELAY_NR(node_id);
            while (skip--) {
                next += PACKETS_IN_TEST * PREAMBLE_SEND_INTERVAL + PACKET_SEND_INTERVAL;
            }
            if (IS_INITIATOR(node_id)) {
                next += GUARD_TIME;
                currentState = STATE_TX;
            } else {
                currentState = STATE_RX;
            }
        } else {
            next += PREAMBLE_SEND_INTERVAL;
        }
        break;

    case STATE_TX:
        if (h->packetNumber == 0) {
            if (numTestsInSenderRole < 0) numTestsInSenderRole = 0;
            numTestsInSenderRole++;
#if DEBUG
            puts("starting tx...");
#endif
        }
        h->packetNumber++;
        h->crc = crc8(h, sizeof(*h) - 1);

        patternFill((uint16_t *)sendBuffer, TEST_PACKET_SIZE, h->packetNumber, HEADER_SIZE);

        NETSTACK_RADIO.send(sendBuffer, TEST_PACKET_SIZE /*, 0*/);

        if (h->packetNumber >= PACKETS_IN_TEST) {
            h->packetNumber = 0;
            selectNextState();
            next += PAUSE_BETWEEN_TESTS - GUARD_TIME;
        }
        next += PACKET_SEND_INTERVAL;
        break;

    case STATE_WAIT:
         clearErrors();
         h->packetNumber = 0;
         currentState = STATE_RX;
         // fallthrough

    case STATE_RX:
        if (h->packetNumber == 0) {
#if DEBUG
            puts("starting rx...");
#endif
        }
        h->packetNumber++;
        if (h->packetNumber > PACKETS_IN_TEST + 1) {
            h->packetNumber = 0;
            if (selectNextState() == STATE_TX) {
                next += PAUSE_BETWEEN_TESTS - PACKET_SEND_INTERVAL + GUARD_TIME;
            } else {
                next += PAUSE_BETWEEN_TESTS - PACKET_SEND_INTERVAL;
            }
        } else {
            next += PACKET_SEND_INTERVAL;
        }

        break;
    }

    rtimer_set(t, next, 1, rtimerCallback, ptr);
}

// -------------------------------------------------------------

static void inputPacket(void)
{
    void *data = packetbuf_hdrptr();
    struct packetHeader *h = (struct packetHeader *) data;

    bool isPreamblePacket = (currentState == STATE_WAIT || currentState == STATE_IDLE || currentState == STATE_PREAMBLE_PREPARE);
    uint16_t expectedLength = isPreamblePacket ? HEADER_SIZE : TEST_PACKET_SIZE;

    if (!isPreamblePacket) errors.total++;

    uint8_t length = packetbuf_totlen();
//    printf("input packet, length=%d\n", (int16_t) length);

    if (length != expectedLength) {
#if DEBUG
        printf("rcvd length=%d\n", length);
#endif

#if TRACK_ERRORS
        if (!isPreamblePacket) {
            if (length <= 2) 
                errors.zeroLength++;
            else if (length < expectedLength) 
                errors.tooShort++;
            else
                errors.tooLong++;
        }
#endif
        return;
    }

    if (crc8(h, sizeof(*h)) != 0) {
#if DEBUG
        puts("header crc bad!");
#endif
#if TRACK_ERRORS
        if (!isPreamblePacket) {
            errors.badHeader++;
        }
#endif
        return;
    }

    if (isPreamblePacket) {

        // if (h->sender != NODE_GROUPS_SINKS[node_id][1]) {
        //     // ignore the packet; not for us
        //     return;
        // }

        /////uint32_t rx_time = packetbuf_attr32(PACKETBUF_ATTR_TIMESTAMP);
      uint32_t rx_time = packetbuf_attr32(PACKETBUF_ATTR_TIMESTAMP);
        //printf("rx_time %u now %u", rx_time, RTIMER_NOW());

        //printf("rx %u\n", h->packetNumber);

        rx_time += (PACKETS_IN_TEST - h->packetNumber) * PREAMBLE_SEND_INTERVAL;
        rx_time += PACKET_SEND_INTERVAL;

        // save the channel, but do not set it yet
        if (h->channel >= 11 && h->channel <= 26) {
            testChannel = h->channel;
        } else {
            printf("invalid test channel: %u\n", h->channel);
        }

        uint8_t sender_nr = RELAY_NR(h->sender);
        uint8_t to_skip;
        if (IS_RELAY(node_id) && sender_nr < RELAY_NR(node_id)) {
            to_skip = RELAY_NR(node_id) - sender_nr - 1;

            currentState = STATE_PREAMBLE_PREPARE;

            rx_time += GUARD_TIME;
        }
        else {
            to_skip = 3 - sender_nr;

            currentState = STATE_WAIT;

            // set the test channel
            radio_set_channel(testChannel);
            //radio_set_txpower(TEST_TXPOWER);
        }

        while (to_skip--) {
            rx_time += PACKETS_IN_TEST * PREAMBLE_SEND_INTERVAL + PACKET_SEND_INTERVAL;
        }

        rtimer_clock_t now = RTIMER_NOW() + 1;
        if (!RTIMER_CLOCK_LT(now, rx_time)) {
            rx_time = now + 1;
        }

        // fix for the case when preamble packets are sent on the same channel as main test packets
        errors.total = errors.fine = 0;

        rtimer_set(&rt, rx_time, 1, rtimerCallback, NULL);
        return;
    }

    cli();
    uint16_t numNibbleErrors = patternCheck(
            (uint16_t *)packetbuf_hdrptr(),
            TEST_PACKET_SIZE,
            h->packetNumber,
            HEADER_SIZE,
#if TRACK_ERRORS
            errors.symbolErrors,
            errors.confusionMatrix,
            errors.correctCounts
#else
            NULL, NULL, NULL
#endif
        );
    sei();

    if (numNibbleErrors) {
#if DEBUG
        printf("some bytes are corrupt, num=%d, crcOk=%d\n",
                numNibbleErrors,
                packetbuf_attr(PACKETBUF_ATTR_CRC_OK));
#endif

#if TRACK_ERRORS
        errors.badContents++;
        errors.numBadNibbles += numNibbleErrors;
        if (numNibbleErrors > MAX_NIBBLES) {
            printf("numNibbleErrors > MAX_NIBBLES\n");
        }
        errors.badNibbles[numNibbleErrors - 1]++;
#endif
        return;
    }
    else if (!packetbuf_attr(PACKETBUF_ATTR_CRC_OK)) {
#if DEBUG
        puts("Corruption not detected, but CRC bad\n");
#endif
        // debugHexdump(packetbuf_hdrptr(), TEST_PACKET_SIZE);
#if TRACK_ERRORS
        errors.badPHYCrcGoodContents++;
#endif
        return;
    }

    if (h->packetNumber > PACKETS_IN_TEST) {
        // whoops. all integrity checks succeeded, but the packet is obviously in error.
        printf("rx a packet with invalid number (%d)\n", h->packetNumber);
#if TRACK_ERRORS
        errors.badHeader++;
#endif
        return;
    }

    if (errors.fine != 0 && h->packetNumber <= currentPacketNumber) {
        // received a packet with out-of-order number
        printf("oo: %u %u\n", h->packetNumber, currentPacketNumber);
        currentPacketNumber = h->packetNumber;
#if TRACK_ERRORS
        errors.badContents++; // XXX: should account as a duplicate
#endif
        return;
    }

#if DEBUG
    printf("rx %u from %u\n", h->packetNumber, h->sender);
#endif
    // print it
    // putchar(to_hex(h->packetNumber >> 4));
    // putchar(to_hex(h->packetNumber & 0xf));
    // putchar('\n');

    errors.fine++;
    currentPacketNumber = h->packetNumber;
    if (currentPacketNumber == PACKETS_IN_TEST) currentPacketNumber = -1;
    rssiSum += (uint8_t) ((int) packetbuf_attr(PACKETBUF_ATTR_RSSI) + 128);
    lqiSum += packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

#if DEBUG || 1
    packetsReceived[h->packetNumber - 1] = 1;
#endif
}

RIME_SNIFFER(printSniffer, inputPacket, NULL);

// -------------------------------------------------------------

static void handle_serial_input(const char *line)
{
    printf("in: '%s'\n", line);

    if (*line == '\0') return;

    if (!strcmp(line, "send")) {
        // XXX always required for the GW server
        puts("command accepted");
        etimer_set(&periodic, READY_PRINT_INTERVAL);

        // if (x-- == 0) {
        //     watchdog_stop();
        //     for (;;);
        // }

	printf("node_id=%u\n", node_id);
        if (IS_INITIATOR(node_id)) {
            currentState = STATE_PREAMBLE_PREPARE;
            numTestsInSenderRole = 0;

            radio_set_channel(DEFAULT_CHANNEL);
            //radio_set_txpower(DEFAULT_TXPOWER);

            // do the selection here, as it may be time consuming
            testChannel = selectNextChannel();

            rtimer_set(&rt, RTIMER_NOW() + RTIMER_ARCH_SECOND, 1, rtimerCallback, NULL);
        }
    }
    else if (!strcmp(line, "recv") || !strcmp(line, "rec")) {
        // XXX always required for the GW server
        puts("command accepted");
        numTestsInSenderRole = -1;
        radio_set_channel(DEFAULT_CHANNEL);
        //radio_set_txpower(DEFAULT_TXPOWER);
        etimer_set(&periodic, READY_PRINT_INTERVAL);
    }
#ifdef CONTIKI_TARGET_AVR_RSS2
    else if (!strcmp(line, "upgr") || !strcmp(line, "upgrade")) {
        puts("command accepted");
	cli();
	wdt_enable(WDTO_15MS);
	while(1);
    }
#endif
}

//-------------------------------------------------------------

AUTOSTART_PROCESSES(&controlProcess, &samplingProcess);
PROCESS_THREAD(controlProcess, ev, data)
{
    PROCESS_BEGIN();

    // XXX this is a fix because of invalid ID on a single node
    if (node_id == 17) node_id = 54;

    printf("pdr-test-marsta, node_id=%u\n", node_id);

#ifndef CONTIKI_TARGET_AVR_RSS2
    // XXX: always disable the CCA checks
    cc2420_without_send_cca = true;

    SENSORS_ACTIVATE(button_sensor);
#endif

#if 1
    NETSTACK_RADIO.off();
    rf230_set_rpc(0x0); /* Disbable all RPC features */
    NETSTACK_RADIO.on();
#endif

    radio_set_channel(DEFAULT_CHANNEL);
    //radio_set_txpower(TEST_TXPOWER);

    rime_sniffer_add(&printSniffer);

    etimer_set(&periodic, CLOCK_SECOND);

    //puts("ready to accept commands");

    for(;;) {
        PROCESS_WAIT_EVENT();

        //printf("event %u (%u) at %u, data %p\n", (uint16_t)ev, (uint16_t)serial_line_event_message, currentState, data);

        switch(currentState) {
        case STATE_IDLE:
            if (numTestsInSenderRole < 0) {
                puts("ns=0");
            }
            numTestsInSenderRole = 0;
            if (etimer_expired(&periodic)) {
	      //puts("ready to accept commands");
                etimer_set(&periodic, READY_PRINT_INTERVAL);
                process_poll(&samplingProcess);
            }
            if (ev == PROCESS_EVENT_POLL) {
	      //puts("ready to accept commands");
                etimer_set(&periodic, READY_PRINT_INTERVAL);
            }
	    else if (ev == serial_line_event_message && data != NULL) {
                handle_serial_input((const char *) data);
            }
#ifndef CONTIKI_TARGET_AVR_RSS2
            else if (ev == sensors_event && data == &button_sensor) {
                puts("click accepted!");
                currentState = STATE_PREAMBLE_PREPARE;
                radio_set_channel(DEFAULT_CHANNEL);
		//radio_set_txpower(DEFAULT_TXPOWER);
                // do the selection here, as it may be time consuming
                testChannel = selectNextChannel();

                rtimer_set(&rt, RTIMER_NOW() + RTIMER_ARCH_SECOND, 1, rtimerCallback, NULL);
            }
#endif
            break;
        case STATE_PREAMBLE_PREPARE:
        case STATE_PREAMBLE_TX:
            break;
        case STATE_WAIT:
            break;
        case STATE_TX:
            break;
        case STATE_RX:
            break;
        }
    }

    PROCESS_END();
}
