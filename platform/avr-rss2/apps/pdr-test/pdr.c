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
#include PLATFORM_TEMP_SENSOR_HEADER
#include "codec.h"
#include "dev/leds.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "rime/rime.h"
#include "net/packetbuf.h"

#if CONTIKI_TARGET_AVR_RSS2
#include <avr/wdt.h>
#endif

#define DEBUG 0
#define TRACK_ERRORS 0
#define USE_REAL_NODES 1

#ifndef LOCAL_ONLY
#define LOCAL_ONLY 0
#endif

PROCESS(controlProcess, "PDR test control process");

#define READY_PRINT_INTERVAL (CLOCK_SECOND * 5)

uint8_t packetsReceived[PACKETS_IN_TEST];
uint8_t currentState;
uint8_t sendPacketNumber;
uint8_t channel;
uint8_t platform_id;

struct rtimer rt;
static struct etimer periodic;

struct stats_info stats[NODES_IN_TEST];
int8_t currentStatsIdx;

// needed to link fastrandom.h
uint32_t fastrandomKey;
int8_t numTestsInSenderRole;

const char *delim = " \t\r,";

#define END_OF_FILE 26
uint8_t eof = END_OF_FILE;

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

void clearStats(void)
{
    memset(&stats, 0, sizeof(stats));
    currentStatsIdx = 0;
}

void printStats(struct stats_info *s)
{
    int16_t temp = 0;
    int rssi;
    uint8_t lqi;
    
    if (s->node_id == 0) return;
    
    if (s->fine == 0) {
        rssi = 0;
        lqi = 0;
    } else {
        rssi = platformFixRssi(s->rssiSum / s->fine);
        lqi = s->lqiSum / s->fine;
    }
    
    temp = temp_sensor.value(0);
    
    printf("%s %5u %s %5u %u %i ",
           platform_list[platform_id],
           node_id,
           platform_list[s->platform_id],
           s->node_id,
           s->channel,
           temp);
    
    printf("%u %u %u %u\n",
           s->fine,
           s->total,
           rssi,
           lqi);
    
#if DEBUG
    uint8_t i;
    for (i = 0; i < sizeof(packetsReceived); ++i) {
        printf("%u ", packetsReceived[i]);
    }
    putchar('\n');
    memset(packetsReceived, 0, sizeof(packetsReceived));
#endif
    
#if TRACK_ERRORS
    puts("Error statistics:");
    //printfCrc(" %u total packets, %u/%u/%u length errors, %u/%u corrupt packets",
    printf(" %u total packets, %u/%u/%u length errors, %u/%u corrupt packets\n",
           s->fine + s->zeroLength +
           s->tooShort + s->tooLong +
           s->badHeader + s->badContents +
           s->badPHYCrcGoodContents,
           s->zeroLength,
           s->tooShort, s->tooLong,
           s->badHeader,
           s->badContents);
    if (s->phyCrcErrors) {
        //printfCrc(" %u bad PHY CRC",  s->phyCrcErrors);
        printf(" %u bad PHY CRC\n",  s->phyCrcErrors);
    }
    if (s->badPHYCrcGoodContents) {
        //printfCrc(" %u bad PHY CRC with corruption undetected", s->badPHYCrcGoodContents);
        printf(" %u bad PHY CRC with corruption undetected\n", stats.badPHYCrcGoodContents);
    }
#endif
}

void rtimerCallback(struct rtimer *t, void *ptr)
{
    static uint8_t sendBuffer[TEST_PACKET_SIZE] __attribute__((aligned (2))) =  { 0, 0, 0, 10 };
    struct packetHeader *h = (struct packetHeader *) sendBuffer;
    
    rtimer_clock_t next = RTIMER_TIME(t);
    
    switch (currentState) {
        case STATE_RX:
            return;
            
        case STATE_TX:
#if DEBUG
            if (h->packetNumber == 0) {
                puts("starting tx...");
            }
#endif
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
                printf("send done: pkts=%d channel=%d, platform_txp=%d\n", h->packetNumber, 
		       radio_get_channel(), radio_get_txpower());
            }
            else {
                next += PACKET_SEND_INTERVAL;
            }
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
    struct stats_info *s;
    int8_t findIdx;
    int8_t lastIdx;
    uint8_t rssi;
    uint8_t i;
    
    if (currentStatsIdx < 0 || currentStatsIdx >= NODES_IN_TEST) return;
    // TODO: better handling when stats memory full.
    // at the moment: ignoring new packets.
    
    s = &stats[currentStatsIdx];
    
    /* sender and channel is  "key" */
    if (h->sender != s->node_id || h->channel != s->channel) {
        findIdx = -1;
        lastIdx = -1;
        for (i=0; i<NODES_IN_TEST; i++) {
            if (h->sender == stats[i].node_id && h->channel == stats[i].channel) {
                findIdx = i;
            }
            if (stats[i].node_id != 0) {
                lastIdx = i;
            }
        }

        if (lastIdx == NODES_IN_TEST - 1) {
            // stats memory full
            currentStatsIdx = -1;
            return;
        }
        else if (findIdx > -1) {
            // <sender,channel> already in stats memory
            currentStatsIdx = findIdx;
	    s = &stats[currentStatsIdx];
	} 
	else if (lastIdx == -1) {
            // new <sender,channel>, stats memory empty
            currentStatsIdx = 0;
            s = &stats[currentStatsIdx];
            s->node_id = h->sender;
            s->platform_id = h->platform_id;
            s->channel = h->channel;
        } else if (lastIdx < NODES_IN_TEST - 1) {
            // new <sender,channel>
            currentStatsIdx = lastIdx + 1;
            s = &stats[currentStatsIdx];
            s->node_id = h->sender;
            s->platform_id = h->platform_id;
            s->channel = h->channel;
        }
    }

    s->total++;
    
    /* error analysis */
    
    uint8_t length = packetbuf_totlen();
    
    if (length != TEST_PACKET_SIZE) {
#if DEBUG
        printf("rcvd length=%d\n", length);
#endif
        
#if TRACK_ERRORS
        if (length <= 2)
            s->zeroLength++;
        else if (length < expectedLength)
            s->tooShort++;
        else
            s->tooLong++;
#endif
        return;
    }
    
    if (crc8(h, sizeof(*h)) != 0) {
#if DEBUG
        puts("header crc bad!");
#endif
#if TRACK_ERRORS
        s->badHeader++;
#endif
        return;
    }
    
    uint16_t numNibbleErrors = patternCheck(
                                            (uint16_t *)packetbuf_hdrptr(),
                                            TEST_PACKET_SIZE,
                                            h->packetNumber,
                                            HEADER_SIZE,
#if TRACK_ERRORS
                                            s->symbolErrors,
                                            s->confusionMatrix,
                                            s->correctCounts
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
        s->badContents++;
        s->numBadNibbles += numNibbleErrors;
        if (numNibbleErrors > MAX_NIBBLES) {
            printf("numNibbleErrors > MAX_NIBBLES\n");
        }
        s->badNibbles[numNibbleErrors - 1]++;
#endif
        return;
    }
    else if (!packetbuf_attr(PACKETBUF_ATTR_CRC_OK)) {
#if DEBUG
        puts("Corruption not detected, but CRC bad\n");
#endif
        // debugHexdump(packetbuf_hdrptr(), TEST_PACKET_SIZE);
#if TRACK_ERRORS
        s->badPHYCrcGoodContents++;
#endif
        return;
    }
    
    if (h->packetNumber > PACKETS_IN_TEST) {
        // whoops. all integrity checks succeeded, but the packet is obviously in error.
        printf("rx a packet with invalid number (%d)\n", h->packetNumber);
#if TRACK_ERRORS
        s->badHeader++;
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
    
    
    /* correct data */
    
    s->fine++;
    rssi = (uint8_t) ((int) packetbuf_attr(PACKETBUF_ATTR_RSSI) + 128);
    s->rssiSum += rssi;
    s->lqiSum += packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
    if (rssi > s->rssiMax) s->rssiMax = rssi;
    if (rssi == 0 || rssi < s->rssiMin) s->rssiMin = rssi;
    
    
#if DEBUG || 1
    packetsReceived[h->packetNumber - 1] = 1;
#endif
}

RIME_SNIFFER(printSniffer, inputPacket, NULL);

// -------------------------------------------------------------

static void print_help(void)
{
    printf("pdr-test: version=%s", VERSION);
    printf("tx [[11-26] [max|0|-7|-15|min]]  -- send ch/pwr\n");
    printf("rx [11-26]   -- receive on chan\n");
    printf("ch [11-26]   -- chan read/set\n");
    printf("stat         -- report/clr\n");
    printf("te           -- board temp\n");
    printf("txp [max|0|-7|-15|min] -- tx pwr\n");
    printf("help         -- this menu\n");
    printf("upgr         -- reboot via bootloader\n");
}

static int cmd_txp(uint8_t verbose)
{
    char *p = strtok(NULL, delim);
    
    if(p) {

      if(!strcmp(p, "max")) {
	radio_set_txpower(RADIO_POWER_MAX);
      }
      else if(!strcmp(p, "0")) {
	radio_set_txpower(RADIO_POWER_ZERO_DB);
      }
      else if(!strcmp(p, "-7")) {
	radio_set_txpower(RADIO_POWER_MINUS7_DB);
      }
      else if(!strcmp(p, "-15")) {
	radio_set_txpower(RADIO_POWER_MINUS15_DB);
      }
      else if(!strcmp(p, "min")) {
	radio_set_txpower(RADIO_POWER_MIN);
      }
      else {
	printf("Invalid power\n");;
	return 0;
      }
    }
    if(verbose)
      printf("platform_txp=%d\n", radio_get_txpower());
    return 1;
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
    int i;
    //printf("in: '%s'\n", line);
    p = strtok((char *)&line[0], (const char *) delim);
    
    if (!p) return;
    
    if (!strcmp(p, "tx") || !strcmp(line, "TX")) {
        if( !cmd_chan(0)) return;

        if( !cmd_txp(0))
	  return;

        etimer_set(&periodic, READY_PRINT_INTERVAL);
        currentState = STATE_TX;
        sendPacketNumber = 0;
        rtimer_set(&rt, RTIMER_NOW() + RTIMER_ARCH_SECOND/10, 1, rtimerCallback, NULL);
    }
    else if (!strcmp(p, "rx") || !strcmp(line, "RX")) {
        // XXX always required for the GW server
        if( !cmd_chan(0)) return;
        
        etimer_set(&periodic, READY_PRINT_INTERVAL);
    }
    else if (!strcmp(p, "ch") || !strcmp(line, "chan")) {
        cmd_chan(1);
    }
    else if (!strcmp(p, "help") || !strcmp(line, "h")) {
        print_help();
    }
    else if (!strcmp(p, "stat") || !strcmp(line, "stats")) {
        for(i=0; i < NODES_IN_TEST; i++) {
            printStats(&stats[i]);
        }
        clearStats();
    }
    else if (!strcmp(line, "te") || !strcmp(line, "temp")) {
        printf("temp=%i\n", temp_sensor.value(0));
    }
    else if (!strcmp(p, "txp") || !strcmp(line, "txpower")) {
        cmd_txp(1);
    }
#ifdef CONTIKI_TARGET_AVR_RSS2
    else if (!strcmp(p, "upgr") || !strcmp(line, "upgrade")) {
        printf("OK\n");
	printf("%c", eof);
        cli();
        wdt_enable(WDTO_15MS);
        while(1);
    }
#endif
    else printf("Illegal command '%s'\n", line);
}

//-------------------------------------------------------------

static void print_local_info(void)
{
    printf("pdr-test: version=%s", VERSION);
    printf(" Max nodes_in_test=%d\n", NODES_IN_TEST);
    printf(" Local node_id=%u\n", node_id);
    printf(" platform_id=%u\n", platform_id);
    printf(" temp=%i\n", temp_sensor.value(0));
    printf(" channel=%d\n",  radio_get_channel());
    printf(" platform tx pwr=%d\n",  radio_get_txpower());
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
    clearStats();
    print_local_info();

    currentState = STATE_RX;
    currentStatsIdx = 0;
    radio_set_txpower(TEST_TXPOWER);
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