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

PROCESS(controlProcess, "PDR test control process");
PROCESS_NAME(samplingProcess);

#define MAX_NIBBLES ((TEST_PACKET_SIZE - HEADER_SIZE) * 2)

#define READY_PRINT_INTERVAL (CLOCK_SECOND * 5)


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
    printf("%u %u %d %u %u\n",
            errors.fine, errors.total, rssi, lqi,
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

    case STATE_TX:
        if (h->packetNumber == 0) {
#if DEBUG
            puts("starting tx...");
#endif
        }
        h->packetNumber++;
        h->crc = crc8(h, sizeof(*h) - 1);

        patternFill((uint16_t *)sendBuffer, TEST_PACKET_SIZE, h->packetNumber, HEADER_SIZE);

        NETSTACK_RADIO.send(sendBuffer, TEST_PACKET_SIZE /*, 0*/);

        if (h->packetNumber >= PACKETS_IN_TEST) {
	  currentState = STATE_IDLE;
        }
	else
	  next += PACKET_SEND_INTERVAL;
        break;

    default:
      break;
    }

    rtimer_set(t, next, 1, rtimerCallback, ptr);
}

// -------------------------------------------------------------

static void inputPacket(void)
{
    void *data = packetbuf_hdrptr();
    struct packetHeader *h = (struct packetHeader *) data;

    uint8_t length = packetbuf_totlen();
//    printf("input packet, length=%d\n", (int16_t) length);

    if (length != TEST_PACKET_SIZE) {
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
	errors.badHeader++;
#endif
        return;
    }

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
    if (currentPacketNumber == PACKETS_IN_TEST) currentPacketNumber = -1;
    rssiSum += (uint8_t) ((int) packetbuf_attr(PACKETBUF_ATTR_RSSI) + 128);
    lqiSum += packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

#if DEBUG || 1
    packetsReceived[h->packetNumber - 1] = 1;
#endif
}

RIME_SNIFFER(printSniffer, inputPacket, NULL);

// -------------------------------------------------------------

static print_help(void)
{
  printf("pdr-test: version=%s", VERSION);
  printf("send -- start tx test\n");
  printf("recv -- start rx test\n");
  printf("upgr -- reboot via bootlaoder\n");
  printf("sch  -- print scheduling table\n");
  printf("help -- print this menu\n");
}

static void handle_serial_input(const char *line)
{
    printf("in: '%s'\n", line);

    if (*line == '\0') return;

    if (!strcmp(line, "send")) {
        puts("command accepted");
        etimer_set(&periodic, READY_PRINT_INTERVAL);

	currentState = STATE_TX;
	radio_set_channel(DEFAULT_CHANNEL);
	rtimer_set(&rt, RTIMER_NOW() + RTIMER_ARCH_SECOND/10, 1, rtimerCallback, NULL);
    }
    else if (!strcmp(line, "recv") || !strcmp(line, "rec")) {
        // XXX always required for the GW server
        puts("command accepted");
        radio_set_channel(DEFAULT_CHANNEL);
        //radio_set_txpower(DEFAULT_TXPOWER);
        etimer_set(&periodic, READY_PRINT_INTERVAL);
    }
    else if (!strcmp(line, "help") || !strcmp(line, "--help")) {
      puts("command accepted");
      print_help();
    }
    else if (!strcmp(line, "stats") || !strcmp(line, "stats")) {
      puts("command accepted");
      printStats();
      clearErrors();
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

print_pgm_info(void)
{
  printf("pdr-test: version=%s", VERSION);
  printf("packet_send_interval=%-d", PACKET_SEND_INTERVAL);
  printf(" preamble_send_interval=%-d", PREAMBLE_SEND_INTERVAL);
  printf(" guard_time=%-d", GUARD_TIME);
  printf(" pause_between_tests=%-d", PAUSE_BETWEEN_TESTS);
  printf(" Local node_id=%u\n", node_id);
}

AUTOSTART_PROCESSES(&controlProcess, &samplingProcess);
PROCESS_THREAD(controlProcess, ev, data)
{
    PROCESS_BEGIN();

    // XXX this is a fix because of invalid ID on a single node
    if (node_id == 17) node_id = 54;

    print_pgm_info();

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
