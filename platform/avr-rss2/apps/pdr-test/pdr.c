// -*- mode: contiki-c-mode; c-basic-offset: 4; c-indent-level: 4 -*-
#include "pdr.h"
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
#include <string.h>
#include "core/net/packetbuf.h"
#include <avr/wdt.h>
#include "rime/rime.h"
#ifdef CONTIKI_TARGET_SKY
#include "dev/sht11/sht11-sensor.h"
#elif CONTIKI_TARGET_Z1
#include "dev/tmp102.h"
#elif CONTIKI_TARGET_AVR_RSS2
#include "dev/temp-sensor.h"
#endif

#define DEBUG 0
#define TRACK_ERRORS 0
#define USE_REAL_NODES 1

#ifndef LOCAL_ONLY
#define LOCAL_ONLY 0
#endif

PROCESS(controlProcess, "PDR test control process");

#define MAX_NIBBLES ((TEST_PACKET_SIZE - HEADER_SIZE) * 2)

#define READY_PRINT_INTERVAL (CLOCK_SECOND * 5)

uint8_t packetsReceived[PACKETS_IN_TEST];
uint8_t currentState;

uint8_t sendPacketNumber;
uint8_t channel;
uint8_t platform_id;

struct rtimer rt;

static struct etimer periodic;

struct {
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
} stats;

uint16_t rssiSum;
uint16_t lqiSum;

// needed to link fastrandom.h
uint32_t fastrandomKey;
int8_t numTestsInSenderRole;

const char *delim = " \t\r,";

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
    memset(&stats, 0, sizeof(stats));
 }

void printStats(void)
{
  int16_t temp;
  int rssi;
  uint8_t lqi;

  if (stats.fine == 0) {
    rssi = 0;
    lqi = 0;
  } else {
    rssi = platformFixRssi(rssiSum / stats.fine);
    lqi = lqiSum / stats.fine;
  }
  
  temp = temp_sensor.value(0);

  printf("%s %5u %s %5u %u %i ",
	 platform_list[platform_id], node_id, platform_list[stats.platform_id], 
	 stats.node_id,  stats.channel, temp);

  printf("%u %u %u %u\n",
	 stats.fine, stats.total, rssi, lqi);

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
            stats.fine + stats.zeroLength +
            stats.tooShort + stats.tooLong +
            stats.badHeader + stats.badContents +
            stats.badPHYCrcGoodContents,
            stats.zeroLength,
            stats.tooShort, stats.tooLong,
            stats.badHeader,
            stats.badContents);
    if (stats.phyCrcErrors) {
      //printfCrc(" %u bad PHY CRC",  stats.phyCrcErrors);
      printf(" %u bad PHY CRC\n",  stats.phyCrcErrors);
    }
    if (stats.badPHYCrcGoodContents) {
      //printfCrc(" %u bad PHY CRC with corruption undetected", stats.badPHYCrcGoodContents);
      printf(" %u bad PHY CRC with corruption undetected\n", stats.badPHYCrcGoodContents);
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
    case STATE_RX:
        return;

    case STATE_TX:
        if (h->packetNumber == 0) {
#if DEBUG
            puts("starting tx...");
#endif
        }
	sendPacketNumber++;
	h->sender = node_id;
	h->channel = radio_get_channel();
	h->platform_id = platform_id;
        h->packetNumber = sendPacketNumber;
        h->crc = crc8(h, sizeof(*h) - 1);

        patternFill((uint16_t *)sendBuffer, TEST_PACKET_SIZE, h->packetNumber, HEADER_SIZE);

        NETSTACK_RADIO.send(sendBuffer, TEST_PACKET_SIZE /*, 0*/);

        if (h->packetNumber >= PACKETS_IN_TEST) {
	  currentState = STATE_RX;
	  printf("send done\n");
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

    stats.total++;

    if (length != TEST_PACKET_SIZE) {
#if DEBUG
        printf("rcvd length=%d\n", length);
#endif

#if TRACK_ERRORS
	if (length <= 2) 
	  stats.zeroLength++;
	else if (length < expectedLength) 
	  stats.tooShort++;
	else
	  stats.tooLong++;
#endif
        return;
    }

    if (crc8(h, sizeof(*h)) != 0) {
#if DEBUG
        puts("header crc bad!");
#endif
#if TRACK_ERRORS
	stats.badHeader++;
#endif
        return;
    }

    uint16_t numNibbleErrors = patternCheck(
            (uint16_t *)packetbuf_hdrptr(),
            TEST_PACKET_SIZE,
            h->packetNumber,
            HEADER_SIZE,
#if TRACK_ERRORS
            stats.symbolErrors,
            stats.confusionMatrix,
            stats.correctCounts
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
        stats.badContents++;
        stats.numBadNibbles += numNibbleErrors;
        if (numNibbleErrors > MAX_NIBBLES) {
            printf("numNibbleErrors > MAX_NIBBLES\n");
        }
        stats.badNibbles[numNibbleErrors - 1]++;
#endif
        return;
    }
    else if (!packetbuf_attr(PACKETBUF_ATTR_CRC_OK)) {
#if DEBUG
        puts("Corruption not detected, but CRC bad\n");
#endif
        // debugHexdump(packetbuf_hdrptr(), TEST_PACKET_SIZE);
#if TRACK_ERRORS
        stats.badPHYCrcGoodContents++;
#endif
        return;
    }

    if (h->packetNumber > PACKETS_IN_TEST) {
        // whoops. all integrity checks succeeded, but the packet is obviously in error.
        printf("rx a packet with invalid number (%d)\n", h->packetNumber);
#if TRACK_ERRORS
        stats.badHeader++;
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

    stats.fine++;
    rssiSum += (uint8_t) ((int) packetbuf_attr(PACKETBUF_ATTR_RSSI) + 128);
    lqiSum += packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

    /* sender is a "key" */
    if (h->sender != stats.node_id) {
      stats.node_id = h->sender;
      stats.platform_id = h->platform_id;
      stats.channel = h->channel;
      printf("received from new sender %u\n", h->sender);
    }

#if DEBUG || 1
    packetsReceived[h->packetNumber - 1] = 1;
#endif
}

RIME_SNIFFER(printSniffer, inputPacket, NULL);

// -------------------------------------------------------------

static void print_help(void)
{
  printf("pdr-test: version=%s", VERSION);
  printf("tx [11-26]   -- send on chan\n");
  printf("rx [11-26]   -- receive on chan\n");
  printf("upgr         -- reboot via bootlaoder\n");
  printf("stat         -- report & clear stat\n");
  printf("ch [11-26]   -- chan read/set\n");
  printf("te           -- board temp\n");
  printf("help         -- this menu\n");
}

static int cmd_chan(uint8_t verbose)
{
  uint8_t tmp;
  char *p = strtok(NULL, delim);
  
  if(p) {
    tmp  =  atoi((char *) p);
    if( tmp >= 11 && tmp <= 26) {
      channel = tmp;
      radio_set_channel(channel);
    }
    else {
	printf("Invalid chan=%d\n", tmp);
	return 0;
    }
  }
  if(verbose) 
    printf("chan=%d\n", channel);
  return 1;
}

static void handle_serial_input(const char *line)
{
     char *p;
     //printf("in: '%s'\n", line);
     p = strtok((char *)&line[0], (const char *) delim);
  
    if (!p) 
      return;

    if (!strcmp(p, "tx") || !strcmp(line, "TX")) {
        puts("command accepted");
        if( !cmd_chan(0))
	  return;

        etimer_set(&periodic, READY_PRINT_INTERVAL);
	currentState = STATE_TX;
	sendPacketNumber = 0;
	rtimer_set(&rt, RTIMER_NOW() + RTIMER_ARCH_SECOND/10, 1, rtimerCallback, NULL);
    }
    else if (!strcmp(p, "rx") || !strcmp(line, "RX")) {
        // XXX always required for the GW server
        puts("command accepted");
        if( !cmd_chan(0))
	  return;

	/* report before new run */
	printStats();
	clearErrors();

        etimer_set(&periodic, READY_PRINT_INTERVAL);
    }
    else if (!strcmp(p, "ch") || !strcmp(line, "chan")) {
        puts("command accepted");
        cmd_chan(1);
    }
    else if (!strcmp(p, "help") || !strcmp(line, "h")) {
      puts("command accepted");
      print_help();
    }
    else if (!strcmp(p, "stat") || !strcmp(line, "stats")) {
      puts("command accepted");
      printStats();
      clearErrors();
    }
    else if (!strcmp(line, "te") || !strcmp(line, "temp")) {
      puts("command accepted");
      printf("temp=%i\n", temp_sensor.value(0));
    }
#ifdef CONTIKI_TARGET_AVR_RSS2
    else if (!strcmp(p, "upgr") || !strcmp(line, "upgrade")) {
        puts("command accepted");
	cli();
	wdt_enable(WDTO_15MS);
	while(1);
    }
#endif
}

//-------------------------------------------------------------

static void print_pgm_info(void)
{
  printf("pdr-test: version=%s", VERSION);
  printf(" Local node_id=%u\n", node_id);
  printf(" platform_id=%u\n", platform_id);
  printf(" temp=%i\n", temp_sensor.value(0));
}

AUTOSTART_PROCESSES(&controlProcess);
PROCESS_THREAD(controlProcess, ev, data)
{
    PROCESS_BEGIN();

    SENSORS_ACTIVATE(temp_sensor);
    SENSORS_ACTIVATE(button_sensor);

#ifdef CONTIKI_TARGET_AVR_RSS2
    NETSTACK_RADIO.off();
    rf230_set_rpc(0x0); /* Disbable reduced power (RPC) features */
    NETSTACK_RADIO.on();
#endif

    platform_id = PLATFORM_ID;
    channel = DEFAULT_CHANNEL;
    radio_set_channel(channel);
    //radio_set_txpower(TEST_TXPOWER);

    print_pgm_info();
    rime_sniffer_add(&printSniffer);
    etimer_set(&periodic, CLOCK_SECOND);

    for(;;) {
        PROCESS_WAIT_EVENT();

        //printf("event %u (%u) at %u, data %p\n", (uint16_t)ev, (uint16_t)serial_line_event_message, currentState, data);

        switch(currentState) {
        case STATE_RX:
            if (numTestsInSenderRole < 0) {
                puts("ns=0");
            }
            numTestsInSenderRole = 0;
            if (etimer_expired(&periodic)) {
                etimer_set(&periodic, READY_PRINT_INTERVAL);
            }
            if (ev == PROCESS_EVENT_POLL) {
                etimer_set(&periodic, READY_PRINT_INTERVAL);
            }
	    else if (ev == serial_line_event_message && data != NULL) {
                handle_serial_input((const char *) data);
            }
        case STATE_TX:
            break;
        }
    }
    PROCESS_END();
}
