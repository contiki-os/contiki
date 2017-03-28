/*
 * Copyright (c) 2006, Technical University of Munich
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
 * This file is part of the Contiki operating system.
 *
 * Reworked for avr-rss2 platform.  Robert Olsson <robert@radio-sensors.com>
 */

#define PRINTF(FORMAT, args ...) printf_P(PSTR(FORMAT),##args)

#define ANNOUNCE_BOOT 1    /* adds about 600 bytes to program size */
#if ANNOUNCE_BOOT
#define PRINTA(FORMAT, args ...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTA(...)
#endif

#define DEBUG 0
#if DEBUG
#define PRINTD(FORMAT, args ...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTD(...)
#endif

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <dev/watchdog.h>

#include "loader/symbols-def.h"
#include "loader/symtab.h"

#include "params.h"
#include "rss2.h"
#include "leds.h"
#include "i2c.h"
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/ipv6/sicslowpan.h"

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#include "dev/rs232.h"
#include "dev/serial-line.h"
#include "dev/slip.h"

#if AVR_WEBSERVER
#include "httpd-fs.h"
#include "httpd-cgi.h"
#endif

#ifdef COFFEE_FILES
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#endif

#if UIP_CONF_ROUTER && 0
#include "net/routing/rimeroute.h"
#include "net/rime/rime-udp.h"
#endif

#include "net/rime/rime.h"

/* Track interrupt flow through mac, rdc and radio driver */
/* #define DEBUGFLOWSIZE 32 */
#if DEBUGFLOWSIZE
uint8_t debugflowsize, debugflow[DEBUGFLOWSIZE];
#define DEBUGFLOW(c) if(debugflowsize < (DEBUGFLOWSIZE - 1)) debugflow[debugflowsize++] = c
#else
#define DEBUGFLOW(c)
#endif

/* Get periodic prints from idle loop, from clock seconds or rtimer interrupts */
/* Use of rtimer will conflict with other rtimer interrupts such as contikimac radio cycling */
/* STAMPS will print ENERGEST outputs if that is enabled. */
#define PERIODICPRINTS 1
#if PERIODICPRINTS
/* #define PINGS 64 */
#define ROUTES 600
#define STAMPS 60
#define STACKMONITOR 1024
uint32_t clocktime;
#define TESTRTIMER 0
#if TESTRTIMER
uint8_t rtimerflag = 1;
struct rtimer rt;
void
rtimercycle(void)
{
  rtimerflag = 1;
}
#endif
#endif

uint16_t node_id; /* Can be set by cooja */

uint16_t ledtimer_red, ledtimer_yellow;
uint16_t i2c_probed; /* i2c devices we have probed */


/*-------------------------------------------------------------------------*/
/*----------------------Configuration of the .elf file---------------------*/
#if 1
/* The proper way to set the signature is */
#include <avr/signature.h>
#else
/* Older avr-gcc's may not define the needed SIGNATURE bytes. Do it manually if you get an error */
typedef struct {const unsigned char B2;
                const unsigned char B1;
                const unsigned char B0;
} __signature_t;
#define SIGNATURE __signature_t __signature __attribute__((section(".signature")))
SIGNATURE = {
  .B2 = 0x01, /* SIGNATURE_2, //ATMEGA128rfa1 */
  .B1 = 0xA7, /* SIGNATURE_1, //128KB flash */
  .B0 = 0x1E, /* SIGNATURE_0, //Atmel */
};
#endif

#if 1
/* JTAG, SPI enabled, Internal RC osc, Boot flash size 4K, 6CK+65msec delay, brownout disabled */
FUSES = { .low = 0xe2, .high = 0x99, .extended = 0xff, };
#else
/* JTAG+SPI, Boot 4096 words @ $F000, Internal  oscillator, startup 6 CK +0 ms, Brownout 1.8 volts */
FUSES = { .low = 0xC2, .high = 0x99, .extended = 0xfe, };
#endif

uint8_t
rng_get_uint8(void)
{
#if 1
  /* Upper two RSSI reg bits (RND_VALUE) are random in rf231 */
  uint8_t j;
  j = (PHY_RSSI & 0xc0) + ((PHY_RSSI >> 2) & 0x30) + ((PHY_RSSI >> 4) & 0x0c) + ((PHY_RSSI >> 6) & 0x03);
#else
/* Get a pseudo random number using the ADC */
  uint8_t i, j;
  ADCSRA = 1 << ADEN;             /* Enable ADC, not free running, interrupt disabled, fastest clock */
  for(i = 0; i < 4; i++) {
    ADMUX = 0;                /* toggle reference to increase noise */
    ADMUX = 0x1E;              /* Select AREF as reference, measure 1.1 volt bandgap reference. */
    ADCSRA |= 1 << ADSC;          /* Start conversion */
    while(ADCSRA & (1 << ADSC)) ;  /* Wait till done */
    j = (j << 2) + ADC;
  }
  ADCSRA = 0;                   /* Disable ADC */
#endif
  PRINTD("rng issues %d\n", j);
  return j;
}
/*-------------------------Low level initialization------------------------*/
/*------Done in a subroutine to keep main routine stack usage small--------*/
void
initialize(void)
{
  watchdog_init();
  watchdog_start();
  leds_init();
  serial_line_init();

  rs232_init(RS232_PORT_0, USART_BAUD_38400, USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);
  rs232_redirect_stdout(RS232_PORT_0);

#if 0
  /* Do it my way... */
  //UBRR0L =  8; UBRR0H = 0; UCSR0A = (0 << U2X0);  // 115.2k  err=-3.5%  
  //UBRR0L = 16; UBRR0H = 0; UCSR0A = (1 << U2X0);  // 115.2k     2.1%  
  //UBRR0L =  3; UBRR0H = 0; UCSR0A = (1 << U2X0);  // 500k         0%
#endif

  rs232_set_input(RS232_PORT_0, serial_line_input_byte);

  clock_init();

  if(MCUSR & (1 << PORF)) {
    PRINTD("Power-on reset.\n");
  }
  if(MCUSR & (1 << EXTRF)) {
    PRINTD("External reset!\n");
  }
  if(MCUSR & (1 << BORF)) {
    PRINTD("Brownout reset!\n");
  }
  if(MCUSR & (1 << WDRF)) {
    PRINTD("Watchdog reset!\n");
  }
  if(MCUSR & (1 << JTRF)) {
    PRINTD("JTAG reset!\n");
  }

  i2c_init(100000); /* 100 bit/s */

#if STACKMONITOR
  /* Simple stack pointer highwater monitor. Checks for magic numbers in the main
   * loop. In conjuction with PERIODICPRINTS, never-used stack will be printed
   * every STACKMONITOR seconds.
   */
  {
    extern uint16_t __bss_end;
    uint16_t p = (uint16_t)&__bss_end;
    do {
      *(uint16_t *)p = 0x4242;
      p += 10;
    } while(p < SP - 10); /* don't overwrite our own stack */
  }
#endif

#define CONF_CALIBRATE_OSCCAL 0
#if CONF_CALIBRATE_OSCCAL
  void calibrate_rc_osc_32k();
  {
    extern uint8_t osccal_calibrated;
    uint8_t i;
    PRINTD("\nBefore calibration OSCCAL=%x\n", OSCCAL);
    for(i = 0; i < 10; i++) {
      calibrate_rc_osc_32k();
      PRINTD("Calibrated=%x\n", osccal_calibrated);
/* #include <util/delay_basic.h> */
/* #define delay_us( us )   ( _delay_loop_2(1+(us*F_CPU)/4000000UL) ) */
/*   delay_us(50000); */
    }
    clock_init();
  }
#endif

  PRINTA("\n*******Booting %s*******\n", CONTIKI_VERSION_STRING);

/* rtimers needed for radio cycling */
  rtimer_init();

/* we can initialize the energest arrays here */
  energest_init();
/* after the timer intitialisation we start the cpu measurement */
  ENERGEST_ON(ENERGEST_TYPE_CPU);


  /* Initialize process subsystem */
  process_init();

  /* etimers must be started before ctimer_init */
  process_start(&etimer_process, NULL);
  ctimer_init();

  /* Start radio and radio receive process */
  NETSTACK_RADIO.init();

/* Get a random seed for the 802.15.4 packet sequence number.
 * Some layers will ignore duplicates found in a history (e.g. Contikimac)
 * causing the initial packets to be ignored after a short-cycle restart.
 */
  random_init(rng_get_uint8());

  /* Set addresses BEFORE starting tcpip process */

  linkaddr_t addr;
  char eui64[8];

  printf("I2C: ");
  i2c_probed = i2c_probe();
  printf("\n");
  
  if( i2c_probed & I2C_AT24MAC ) {
    i2c_at24mac_read((char *)&eui64, 1);
    linkaddr_set_node_addr((linkaddr_t *) &eui64);
    node_id = (eui64[1] << 8) + eui64[7];
  }
  else {
    printf("Random EUI64 address generated\n");
    eui64[0] = 0xfc; /* Atmels OUI */
    eui64[1] = 0xc2; 
    eui64[2] = 0x3d;
    eui64[3] = 0;
    eui64[4] = 0;
    eui64[5] = 0;
    eui64[6] = node_id >> 8;
    eui64[7] = node_id & 0xff;
    linkaddr_set_node_addr((linkaddr_t *)&eui64);
  }

  /* memcpy(&uip_lladdr.addr, &addr.u8, sizeof(linkaddr_t)); */

#if NETSTACK_CONF_WITH_IPV6
  memcpy(&addr.u8, &eui64,  sizeof(linkaddr_t));
  memcpy(&uip_lladdr.addr, &addr.u8, sizeof(linkaddr_t));
#endif

  rf230_set_pan_addr(params_get_panid(), params_get_panaddr(), (uint8_t *)&addr.u8);
  rf230_set_channel(params_get_channel());
  rf230_set_txpower(params_get_txpower());

#if NETSTACK_CONF_WITH_IPV6
  PRINTA("EUI-64 MAC: %x-%x-%x-%x-%x-%x-%x-%x\n", addr.u8[0], addr.u8[1], addr.u8[2], addr.u8[3], addr.u8[4], addr.u8[5], addr.u8[6], addr.u8[7]);
#else
  PRINTA("MAC address ");
  uint8_t i;
  addr.u8[0] = eui64[1] ; 
  addr.u8[1] = eui64[7];

  for(i = sizeof(linkaddr_t); i > 0; i--) {
    PRINTA("%x:", addr.u8[i - 1]);
  }
  PRINTA("\n");
#endif

  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

#if ANNOUNCE_BOOT
  PRINTA("MAC=%s, RDC=%s, NETWORK=%s, channel=%-u, check-rate-Hz=%-u, tx-power=%-u\n", NETSTACK_MAC.name,
         NETSTACK_RDC.name, NETSTACK_NETWORK.name, rf230_get_channel(),
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1 : NETSTACK_RDC.channel_check_interval()),
         rf230_get_txpower());
#if UIP_CONF_IPV6_RPL
  PRINTA("RPL Enabled\n");
#endif
#if UIP_CONF_ROUTER
  PRINTA("Routing Enabled\n");
#endif

#endif /* ANNOUNCE_BOOT */

#if NETSTACK_CONF_WITH_IPV6 || NETSTACK_CONF_WITH_IPV4
  process_start(&tcpip_process, NULL);
#endif

  /* Autostart other processes */
  autostart_start(autostart_processes);

  /*---If using coffee file system create initial web content if necessary---*/
#if COFFEE_FILES
  int fa = cfs_open("/index.html", CFS_READ);
  if(fa < 0) {     /* Make some default web content */
    PRINTA("No index.html file found, creating upload.html!\n");
    PRINTA("Formatting FLASH file system for coffee...");
    cfs_coffee_format();
    PRINTA("Done!\n");
    fa = cfs_open("/index.html", CFS_WRITE);
    int r = cfs_write(fa, &"It works!", 9);
    if(r < 0) {
      PRINTA("Can''t create /index.html!\n");
    }
    cfs_close(fa);
/*  fa = cfs_open("upload.html"), CFW_WRITE); */
/* <html><body><form action="upload.html" enctype="multipart/form-data" method="post"><input name="userfile" type="file" size="50" /><input value="Upload" type="submit" /></form></body></html> */
  }
#endif /* COFFEE_FILES */

/* Add addresses for testing */
#if 0
  {
    uip_ip6addr_t ipaddr;
    uip_ip6addr(&ipaddr, 0xfd00, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
/*  uip_ds6_prefix_add(&ipaddr,64,0); */
  }
#endif
/*--------------------------Announce the configuration---------------------*/
#if ANNOUNCE_BOOT
#if AVR_WEBSERVER
  { uint8_t i;
    char buf[80];
    unsigned int size;

    for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
      if(uip_ds6_if.addr_list[i].isused) {
        httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr, buf);
        PRINTA("IPv6 Address: %s\n", buf);
      }
    }
    cli();
    eeprom_read_block(buf, eemem_server_name, sizeof(eemem_server_name));
    sei();
    buf[sizeof(eemem_server_name)] = 0;
    PRINTA("%s", buf);
    cli();
    eeprom_read_block(buf, eemem_domain_name, sizeof(eemem_domain_name));
    sei();
    buf[sizeof(eemem_domain_name)] = 0;
    size = httpd_fs_get_size();
#ifndef COFFEE_FILES
    PRINTA(".%s online with fixed %u byte web content\n", buf, size);
#elif COFFEE_FILES == 1
    PRINTA(".%s online with static %u byte EEPROM file system\n", buf, size);
#elif COFFEE_FILES == 2
    PRINTA(".%s online with dynamic %u KB EEPROM file system\n", buf, size >> 10);
#elif COFFEE_FILES == 3
    PRINTA(".%s online with static %u byte program memory file system\n", buf, size);
#elif COFFEE_FILES == 4
    PRINTA(".%s online with dynamic %u KB program memory file system\n", buf, size >> 10);
#endif /* COFFEE_FILES */
  }
#else
  PRINTA("Online\n");
#endif
#endif /* ANNOUNCE_BOOT */

  ledtimer_red = 1000;
  leds_on(LEDS_RED);
}


#if ROUTES && NETSTACK_CONF_WITH_IPV6
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int8_t i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        PRINTF("::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        PRINTF(":");
      }
      PRINTF("%x", a);
    }
  }
}
#endif
/*-------------------------------------------------------------------------*/
/*------------------------- Main Scheduler loop----------------------------*/
/*-------------------------------------------------------------------------*/
int
main(void)
{
#if NETSTACK_CONF_WITH_IPV6
  uip_ds6_nbr_t *nbr;
#endif /* NETSTACK_CONF_WITH_IPV6 */
 

  initialize();

  while(1) {
    process_run();
    watchdog_periodic();

    /* Turn off LED's */
    if(ledtimer_red) {
      if(--ledtimer_red == 0) {
        leds_off(LEDS_RED);
      }
    }
    if(ledtimer_yellow) {
      if(--ledtimer_yellow == 0) {
        leds_off(LEDS_YELLOW);
      }
    }
    leds_off(LEDS_RED);
    leds_off(LEDS_YELLOW);

#if 0
/* Various entry points for debugging in the AVR Studio simulator.
 * Set as next statement and step into the routine.
 */
    NETSTACK_RADIO.send(packetbuf_hdrptr(), 42);
    process_poll(&rf230_process);
    packetbuf_clear();
    len = rf230_read(packetbuf_dataptr(), PACKETBUF_SIZE);
    packetbuf_set_datalen(42);
    NETSTACK_RDC.input();
#endif

#if 0
/* Clock.c can trigger a periodic PLL calibration in the RF230BB driver.
 * This can show when that happens.
 */
    extern uint8_t rf230_calibrated;
    if(rf230_calibrated) {
      PRINTD("\nRF230 calibrated!\n");
      rf230_calibrated = 0;
    }
#endif

/* Set DEBUGFLOWSIZE in contiki-conf.h to track path through MAC, RDC, and RADIO */
#if DEBUGFLOWSIZE
    if(debugflowsize) {
      debugflow[debugflowsize] = 0;
      PRINTF("%s", debugflow);
      debugflowsize = 0;
    }
#endif

#if PERIODICPRINTS
#if TESTRTIMER
/* Timeout can be increased up to 8 seconds maximum.
 * A one second cycle is convenient for triggering the various debug printouts.
 * The triggers are staggered to avoid printing everything at once.
 */
    if(rtimerflag) {
      rtimer_set(&rt, RTIMER_NOW() + RTIMER_ARCH_SECOND * 1UL, 1, (void *)rtimercycle, NULL);
      rtimerflag = 0;
#else
    if(clocktime != clock_seconds()) {
      clocktime = clock_seconds();
#endif

#if STAMPS
      if((clocktime % STAMPS) == 0) {
#if ENERGEST_CONF_ON
#include "lib/print-stats.h"
        print_stats();
#elif RADIOSTATS
        extern volatile unsigned long radioontime;
        PRINTF("%u(%u)s\n", clocktime, radioontime);
#else
        PRINTF("%us\n", clocktime);
#endif
      }
#endif
#if TESTRTIMER
      clocktime += 1;
#endif

#if PINGS && NETSTACK_CONF_WITH_IPV6
      extern void raven_ping6(void);
      if((clocktime % PINGS) == 1) {
        PRINTF("**Ping\n");
        raven_ping6();
      }
#endif

#if ROUTES && NETSTACK_CONF_WITH_IPV6
      if((clocktime % ROUTES) == 2) {

        extern uip_ds6_netif_t uip_ds6_if;

        uint8_t i, j;
        PRINTF("\nAddresses [%u max]\n", UIP_DS6_ADDR_NB);
        for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
          if(uip_ds6_if.addr_list[i].isused) {
            ipaddr_add(&uip_ds6_if.addr_list[i].ipaddr);
            PRINTF("\n");
          }
        }
        PRINTF("\nNeighbors [%u max]\n", NBR_TABLE_MAX_NEIGHBORS);
        j = 0;
        for(nbr = nbr_table_head(ds6_neighbors);
            nbr != NULL;
            nbr = nbr_table_next(ds6_neighbors, nbr)) {
          ipaddr_add(&nbr->ipaddr);
          PRINTF("\n");
          j++;
        }
        if(!j) {
          PRINTF("  <none>");
        }
        PRINTF("\nRoutes [%u max]\n", UIP_DS6_ROUTE_NB);
        {
          uip_ds6_route_t *r;
          j = 0;
          for(r = uip_ds6_route_head();
              r != NULL;
              r = uip_ds6_route_next(r)) {
            ipaddr_add(&r->ipaddr);
            PRINTF("/%u (via ", r->length);
            ipaddr_add(uip_ds6_route_nexthop(r));
            PRINTF(") %lus\n", r->state.lifetime);
            j++;
          }
        }
        if(!j) {
          PRINTF("  <none>");
        }
        PRINTF("\n---------\n");
      }
#endif

#if STACKMONITOR
      if((clocktime % STACKMONITOR) == 3) {
        extern uint16_t __bss_end;
        uint16_t p = (uint16_t)&__bss_end;
        do {
          if(*(uint16_t *)p != 0x4242) {
            PRINTF("Never-used stack > %d bytes\n", p - (uint16_t)&__bss_end);
            break;
          }
          p += 10;
        } while(p < RAMEND - 10);
      }
#endif
    }
#endif /* PERIODICPRINTS */

#if RF230BB && 0
    extern uint8_t rf230processflag;
    if(rf230processflag) {
      PRINTF("rf230p%d", rf230processflag);
      rf230processflag = 0;
    }
#endif

#if RF230BB && 0
    extern uint8_t rf230_interrupt_flag;
    if(rf230_interrupt_flag) {
      /*   if (rf230_interrupt_flag!=11) { */
      PRINTF("**RI%u", rf230_interrupt_flag);
      /*   } */
      rf230_interrupt_flag = 0;
    }
#endif
  }
  return 0;
}
/*---------------------------------------------------------------------------*/

void
log_message(char *m1, char *m2)
{
  PRINTF("%s%s\n", m1, m2);
}
