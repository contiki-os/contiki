/*
 * Copyright (c) 2016, 
 * Authors:
 *   Atis Elsts       <atis.elsts@bristol.ac.uk>
 *   Christian Rohner <christian.rohner@it.uu.se>
 *   Robert Olsson    <roolss@kth.se> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* Main program for Packet Delivery Ratio, (PDR) testing. 

   Data file format:
   * timestamp unix time format
   * tx node model (platform)
   * tx node id
   * tx node temperature (multiplied by 100)
   * rx node model (platform)
   * rx node id
   * rx node temperature
   * channel
   * Tx setting/power
   * number of packets with good checksum (from 100 total in a test)
   * number of all packets received
   * rssi level, average for 100 packets
   * lqi level

Example:
1466035452.0 > avr-rss2 49771 1168 | avr-rss2 49551 1075 | 11 -7 | 100 100 -10 255

*/

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
bool txpower_sweep;
uint8_t sendPacketNumber;
uint8_t channel;
uint8_t platform_id;
int16_t txtemp;


struct rtimer rt;
static struct etimer periodic;

struct stats_info stats[STAT_SIZE];
int8_t currentStatsIdx;

// needed to link fastrandom.h
uint32_t fastrandomKey;
int8_t numTestsInSenderRole;

const char *delim = " \t\r,";

#define END_OF_FILE 26
uint8_t eof = END_OF_FILE;

static int set_txpower(uint8_t p);

/* avr-rss2 plaftform has unresolved issues with TX-power 
   for low power settings. Remapping the TX mappinng as below 
   results in a decreasing TX power settings. */

#if CONTIKI_TARGET_AVR_RSS2
uint8_t tx_corr_tab[16] ={0,1,2,3,4,15,5,14,6,13,7,12,8,11,9,10};

void radio_set_txpower_avr_rss2(uint8_t txp) 
{
  if(txp > 15)
    return rf230_set_txpower(0); /* Max */
  return rf230_set_txpower(tx_corr_tab[txp]);
}
#endif 

// -------------------------------------------------------------
//
// To simplify the code, all internal RSSI calculations are done using non-negative numbers.
// This function maps the resulting value back to the CC2x20-specific valye range.
// return value in [dBm]
//
static inline int16_t platform_rssi_dBm(uint16_t rssi, uint8_t platform)
{
    int16_t dBm = 0;
    
    if (platform == 3) {
        // ATmega128RFA1 Datasheet, page 70
        if (rssi == 0) dBm = -90;
        else if (rssi >= 28) dBm = -10;
        else dBm = -90 + 3 * (rssi-1);
    } else {
        // for cc2420 and cc2520 based platfroms, the radio RSSI value is signed 8 bit integer
        dBm = rssi - 128;
    }
    return dBm;
}

// -------------------------------------------------------------

extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#if TRACK_ERRORS
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
#endif

void clearStats(void)
{
    memset(&stats, 0, sizeof(stats));
    currentStatsIdx = 0;
}

uint8_t txpower;

static uint8_t get_txpower(void)
{
  return txpower;
}

static char *get_txpower_string(uint8_t p)
{
  if((p <= TX_POWER_MIN) && (p >= TX_POWER_MAX)) 
    return tx_power_list[p];
  return "N/A";
}

void debugHexdump(char *p, int len)
{
  int i;
  for(i=0; i<len; ++i) 
    {
      printf("%02x ", p[i] & 0xFF);
      if(! (i % 16)) 
	printf("\n");
    }
  printf("\n");
}

void printStats(struct stats_info *s)
{
    int16_t temp = 0;
    int16_t average_rssi;
    int16_t rssi;
    uint8_t lqi;
    
    if (s->channel == 0) return;
    
    if (s->fine == 0) {
        rssi = 0;
        lqi = 0;
    } else {
        average_rssi = s->rssiSum / s->fine + (s->rssiSum % s->fine != 0);
        rssi = platform_rssi_dBm(average_rssi, s->platform_id);
        lqi = 255 - s->lqiSumDiff / s->fine;
    }
    
    temp = temp_sensor.value(0);
    
    printf("%s %5u %i | %s %5u %i | %u %s | ",
           platform_list[platform_id],
           node_id,
           temp,
           platform_list[s->platform_id],
           s->node_id,
           s->txtemp,
           s->channel,
           get_txpower_string(s->txpower));
    
    printf("%u %u %d %u\n",
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
            sendPacketNumber++;
            h->sender = node_id;
            h->channel = radio_get_channel();
            h->txpower = get_txpower();
            h->txtemp = txtemp;
            h->platform_id = platform_id;
            h->packetNumber = sendPacketNumber;
            h->crc = crc8(h, sizeof(*h) - 1);
            
            patternFill((uint16_t *)sendBuffer, TEST_PACKET_SIZE, h->packetNumber, HEADER_SIZE);
            
            NETSTACK_RADIO.send(sendBuffer, TEST_PACKET_SIZE /*, 0*/);
            
            if (h->packetNumber >= PACKETS_IN_TEST) {
                printf("%s: pkts=%d channel=%d, txpower=%s\n", COMMAND_TX_FINISHED, h->packetNumber,
                       radio_get_channel(), get_txpower_string(txpower));

                if (!txpower_sweep || !get_txpower()) {
                    currentState = STATE_RX;
                    txpower_sweep = false;
                }
                else {
                    set_txpower(get_txpower()-1);
                    sendPacketNumber = 0;
                    currentState = STATE_TX;
                    next += 64*PACKET_SEND_INTERVAL;
                }
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
    
    if (currentStatsIdx < 0 || currentStatsIdx >= STAT_SIZE) return;
    // TODO: better handling when stats memory full.
    // at the moment: ignoring new packets.
    
    /* sanity check */
    if (h->channel != channel) return;
    if (h->platform_id == 0 || h->platform_id > PLATFORM_ID_MAX) return;
    
    s = &stats[currentStatsIdx];
    
    /* sender and channel is  "key" */
    if ((crc8(h, sizeof(*h)) == 0) && (h->sender != s->node_id || h->channel != s->channel || h->txpower != s->txpower)) {
        findIdx = -1;
        lastIdx = -1;
        for (i=0; i<STAT_SIZE; i++) {
            if (h->sender == stats[i].node_id && h->channel == stats[i].channel && h->txpower == s->txpower) {
                findIdx = i;
            }
            if (stats[i].node_id != 0) {
                lastIdx = i;
            }
        }

        if (lastIdx == STAT_SIZE - 1) {
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
            s->txpower = h->txpower;
            s->txtemp = h->txtemp;
        } else if (lastIdx < STAT_SIZE - 1) {
            // new <sender,channel>
            currentStatsIdx = lastIdx + 1;
            s = &stats[currentStatsIdx];
            s->node_id = h->sender;
            s->platform_id = h->platform_id;
            s->channel = h->channel;
            s->txpower = h->txpower;
            s->txtemp = h->txtemp;
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
    rssi = (uint8_t) (int) packetbuf_attr(PACKETBUF_ATTR_RSSI);
    s->rssiSum += rssi;
    s->lqiSumDiff += 255 - packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);   // sum up difference to 255 (save memory)
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
    printf("tx [[11-26] [max|0|-7|-15|min|sweep]]  -- send ch/pwr\n");
    printf("rx [11-26]   -- receive on chan\n");
    printf("ch [11-26]   -- chan read/set\n");
    printf("stat         -- report received stats\n");
    printf("info         -- node info\n");
    printf("te           -- node temp\n");
    printf("txp [max|0|-7|-15|min] -- tx pwr\n");
    printf("help         -- this menu\n");
    printf("upgr         -- reboot via bootloader\n");
}

static int set_txpower(uint8_t p) 
{
    if(p == TX_POWER_MAX) {
        txpower = TX_POWER_MAX;
        radio_set_txpower(RADIO_POWER_MAX);
    }
    else if(p == TX_POWER_0DB) {
        txpower = TX_POWER_0DB;
        radio_set_txpower(RADIO_POWER_ZERO_DB);
    }
    else if(p == TX_POWER_MINUS7_DB) {
        txpower = TX_POWER_MINUS7_DB;
        radio_set_txpower(RADIO_POWER_MINUS7_DB);
    }
    else if(p == TX_POWER_MINUS15_DB) {
        txpower = TX_POWER_MINUS15_DB;
        radio_set_txpower(RADIO_POWER_MINUS15_DB);
    }
    else if(p == TX_POWER_MIN) {
        txpower = TX_POWER_MIN;
        radio_set_txpower(RADIO_POWER_MIN);
    }
    else {
        printf("Invalid power\n");;
        return 0;
    }
    return txpower;
}

static int cmd_txp(uint8_t verbose)
{
    char *p = strtok(NULL, delim);
    txpower_sweep = false;
    
    if(p) {
        if(!strcmp(p, "max")) {
            set_txpower(TX_POWER_MAX);
        }
        else if(!strcmp(p, "0")) {
            set_txpower(TX_POWER_0DB);
        }
        else if(!strcmp(p, "-7")) {
            set_txpower(TX_POWER_MINUS7_DB);
        }
        else if(!strcmp(p, "-15")) {
            set_txpower(TX_POWER_MINUS15_DB);
        }
        else if(!strcmp(p, "min")) {
            set_txpower(TX_POWER_MIN);
        }
        else if(!strcmp(p, "sweep")) {
            // TODO: increase the stats memory (use with care)
            set_txpower(TX_POWER_MIN);
            txpower_sweep = true;
        }
        else {
            printf("Invalid power\n");;
            return 0;
        }
    }
    if(verbose)
        printf("txpower=%s\n", get_txpower_string(txpower));
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

static void print_info(void)
{
    printf("pdr-test: version=%s", VERSION);
    printf(" Max STAT_SIZE=%d\n", STAT_SIZE);
    printf(" platform=%s\n", platform_list[platform_id]);
    printf(" node_id=%u\n", node_id);
    printf(" temp=%i\n", temp_sensor.value(0));
    printf(" channel=%d\n",  radio_get_channel());
    printf(" tx pwr=%s\n",  get_txpower_string(txpower));
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

        if( !cmd_txp(0)) return;

        txtemp = temp_sensor.value(0);
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
        for(i=0; i < STAT_SIZE; i++) {
            printStats(&stats[i]);
        }
        clearStats();
        printf("%s\n", COMMAND_STAT_FINISHED);
    }
    else if (!strcmp(line, "te") || !strcmp(line, "temp")) {
        printf("temp=%i\n", temp_sensor.value(0));
    }
    else if (!strcmp(p, "txp") || !strcmp(line, "txpower")) {
        cmd_txp(1);
    }
    else if (!strcmp(p, "i") || !strcmp(line, "info")) {
      print_info();
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

AUTOSTART_PROCESSES(&controlProcess);
PROCESS_THREAD(controlProcess, ev, data)
{
    PROCESS_BEGIN();
    
    SENSORS_ACTIVATE(temp_sensor);
    SENSORS_ACTIVATE(button_sensor);
    
#ifdef CONTIKI_TARGET_AVR_RSS2
    NETSTACK_RADIO.off();
    rf230_set_rpc(0x0); /* Disable reduced power (RPC) features */
    NETSTACK_RADIO.on();
#endif
    
    platform_id = PLATFORM_ID;
    channel = DEFAULT_CHANNEL;
    radio_set_channel(channel);
    clearStats();
    print_info();

    currentState = STATE_RX;
    currentStatsIdx = 0;
    txpower = TX_POWER_0DB;
    radio_set_txpower(RADIO_POWER_ZERO_DB);
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
