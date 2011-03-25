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
 * @(#)$$
 */
#define ANNOUNCE_BOOT 1    //adds about 600 bytes to program size
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)

#define DEBUG 0
#if DEBUG
#define PRINTFD(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTFD(...)
#endif

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <dev/watchdog.h>

#include "loader/symbols-def.h"
#include "loader/symtab.h"

#if RF230BB        //radio driver using contiki core mac
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#include "net/mac/framer-802154.h"
#include "net/sicslowpan.h"

#else                 //radio driver using Atmel/Cisco 802.15.4'ish MAC
#include <stdbool.h>
#include "mac.h"
#include "sicslowmac.h"
#include "sicslowpan.h"
#include "ieee-15-4-manager.h"
#endif /*RF230BB*/

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#include "dev/rs232.h"
#include "dev/serial-line.h"
#include "dev/slip.h"

#ifdef RAVEN_LCD_INTERFACE
#include "raven-lcd.h"
#endif

#if WEBSERVER
#include "httpd-fs.h"
#include "httpd-cgi.h"
#endif

#ifdef COFFEE_FILES
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#endif

#if UIP_CONF_ROUTER&&0
#include "net/routing/rimeroute.h"
#include "net/rime/rime-udp.h"
#endif

#include "net/rime.h"

/* Test rtimers, also for pings, stack monitor, neighbor/route printout and time stamps */
#define TESTRTIMER 0
#if TESTRTIMER
//#define PINGS 64
#define ROUTES 64
#define STAMPS 30
#define STACKMONITOR 128

uint8_t rtimerflag=1;
uint16_t rtime;
struct rtimer rt;
void rtimercycle(void) {rtimerflag=1;}
static void ipaddr_add(const uip_ipaddr_t *addr);

#endif /* TESTRTIMER */

/*-------------------------------------------------------------------------*/
/*----------------------Configuration of the .elf file---------------------*/
typedef struct {unsigned char B2;unsigned char B1;unsigned char B0;} __signature_t;
#define SIGNATURE __signature_t __signature __attribute__((section (".signature")))
SIGNATURE = {
/* Older AVR-GCCs may not define the SIGNATURE_n bytes so use explicit 1284p values */
  .B2 = 0x05,//SIGNATURE_2,
  .B1 = 0x97,//SIGNATURE_1,
  .B0 = 0x1E,//SIGNATURE_0,
};
FUSES ={.low = 0xe2, .high = 0x99, .extended = 0xff,};

/*----------------------Configuration of EEPROM---------------------------*/
/* Use existing EEPROM if it passes the integrity test, else reinitialize with build values */

/* Put default MAC address in EEPROM */
#if WEBSERVER
extern uint8_t mac_address[8];     //These are defined in httpd-fsdata.c via makefsdata.h
extern uint8_t server_name[16];
extern uint8_t domain_name[30];
#else
uint8_t mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
#endif


#ifdef CHANNEL_802_15_4
uint8_t rf_channel[2] EEMEM = {CHANNEL_802_15_4, ~CHANNEL_802_15_4};
#else
uint8_t rf_channel[2] EEMEM = {22, ~22};
#endif
	volatile uint8_t eeprom_channel;
static uint8_t get_channel_from_eeprom() {
//	volatile uint8_t eeprom_channel;
	uint8_t eeprom_check;
	eeprom_channel = eeprom_read_byte(&rf_channel[0]);
	eeprom_check = eeprom_read_byte(&rf_channel[1]);

	if(eeprom_channel==~eeprom_check)
		return eeprom_channel;

#ifdef CHANNEL_802_15_4
	return(CHANNEL_802_15_4);
#else
	return 26;
#endif
}

static bool get_mac_from_eeprom(uint8_t* macptr) {
	eeprom_read_block ((void *)macptr,  &mac_address, 8);
	return true;
}

static uint16_t get_panid_from_eeprom(void) {
	// TODO: Writeme!
	return IEEE802154_PANID;
}

static uint16_t get_panaddr_from_eeprom(void) {
	// TODO: Writeme!
	return 0;
}

void calibrate_rc_osc_32k();

/*-------------------------Low level initialization------------------------*/
/*------Done in a subroutine to keep main routine stack usage small--------*/
void initialize(void)
{
  watchdog_init();
  watchdog_start();

#ifdef RAVEN_LCD_INTERFACE
  /* First rs232 port for Raven 3290 port */
  rs232_init(RS232_PORT_0, USART_BAUD_38400,USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);
  /* Set input handler for 3290 port */
  rs232_set_input(0,raven_lcd_serial_input);
#endif

  /* Second rs232 port for debugging */
  rs232_init(RS232_PORT_1, USART_BAUD_57600,USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);
  /* Redirect stdout to second port */
  rs232_redirect_stdout(RS232_PORT_1);
  clock_init();

#if STACKMONITOR
  /* Simple stack pointer highwater monitor. Checks for magic numbers in the main
   * loop. In conjuction with TESTRTIMER, never-used stack will be printed
   * every STACKMONITOR seconds.
   */
{
extern uint16_t __bss_end;
uint16_t p=(uint16_t)&__bss_end;
    do {
      *(uint16_t *)p = 0x4242;
      p+=10;
    } while (p<SP-10); //don't overwrite our own stack
}
#endif
  
#define CONF_CALIBRATE_OSCCAL 0
#if CONF_CALIBRATE_OSCCAL
{
extern uint8_t osccal_calibrated;
uint8_t i;
  PRINTF("\nBefore calibration OSCCAL=%x\n",OSCCAL);
  for (i=0;i<10;i++) { 
    calibrate_rc_osc_32k();  
    PRINTF("Calibrated=%x\n",osccal_calibrated);
//#include <util/delay_basic.h>
//#define delay_us( us )   ( _delay_loop_2(1+(us*F_CPU)/4000000UL) ) 
//   delay_us(50000);
 }
   clock_init();
}
#endif 

#if ANNOUNCE_BOOT
  PRINTF("\n*******Booting %s*******\n",CONTIKI_VERSION_STRING);
#endif

/* rtimers needed for radio cycling */
  rtimer_init();

 /* Initialize process subsystem */
  process_init();
 /* etimers must be started before ctimer_init */
  process_start(&etimer_process, NULL);

#if RF230BB

  ctimer_init();
  /* Start radio and radio receive process */
  NETSTACK_RADIO.init();

  /* Set addresses BEFORE starting tcpip process */

  rimeaddr_t addr;
  memset(&addr, 0, sizeof(rimeaddr_t));
  get_mac_from_eeprom(addr.u8);
 
#if UIP_CONF_IPV6 
  memcpy(&uip_lladdr.addr, &addr.u8, 8);
#endif  
  rf230_set_pan_addr(
	get_panid_from_eeprom(),
	get_panaddr_from_eeprom(),
	(uint8_t *)&addr.u8
  );
  rf230_set_channel(get_channel_from_eeprom());

  rimeaddr_set_node_addr(&addr); 

  PRINTFD("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n",addr.u8[0],addr.u8[1],addr.u8[2],addr.u8[3],addr.u8[4],addr.u8[5],addr.u8[6],addr.u8[7]);

  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

#if ANNOUNCE_BOOT
  PRINTF("%s %s, channel %u",NETSTACK_MAC.name, NETSTACK_RDC.name,rf230_get_channel());
  if (NETSTACK_RDC.channel_check_interval) {//function pointer is zero for sicslowmac
    unsigned short tmp;
    tmp=CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval == 0 ? 1:\
                                   NETSTACK_RDC.channel_check_interval());
    if (tmp<65535) printf_P(PSTR(", check rate %u Hz"),tmp);
  }
  PRINTF("\n");

#if UIP_CONF_IPV6_RPL
  PRINTF("RPL Enabled\n");
#endif
#if UIP_CONF_ROUTER
  PRINTF("Routing Enabled\n");
#endif

#endif /* ANNOUNCE_BOOT */

// rime_init(rime_udp_init(NULL));
// uip_router_register(&rimeroute);

  process_start(&tcpip_process, NULL);

#else
/* Original RF230 combined mac/radio driver */
/* mac process must be started before tcpip process! */
  process_start(&mac_process, NULL);
  process_start(&tcpip_process, NULL);
#endif /*RF230BB*/

#ifdef RAVEN_LCD_INTERFACE
  process_start(&raven_lcd_process, NULL);
#endif

  /* Autostart other processes */
  autostart_start(autostart_processes);

  //Give ourselves a prefix
  // init_net();

  /*---If using coffee file system create initial web content if necessary---*/
#if COFFEE_FILES
  int fa = cfs_open( "/index.html", CFS_READ);
  if (fa<0) {     //Make some default web content
    PRINTF("No index.html file found, creating upload.html!\n");
    PRINTF("Formatting FLASH file system for coffee...");
    cfs_coffee_format();
    PRINTF("Done!\n");
    fa = cfs_open( "/index.html", CFS_WRITE);
    int r = cfs_write(fa, &"It works!", 9);
    if (r<0) PRINTF("Can''t create /index.html!\n");
    cfs_close(fa);
//  fa = cfs_open("upload.html"), CFW_WRITE);
// <html><body><form action="upload.html" enctype="multipart/form-data" method="post"><input name="userfile" type="file" size="50" /><input value="Upload" type="submit" /></form></body></html>
  }
#endif /* COFFEE_FILES */

/* Add addresses for testing */
#if 0
{  
  uip_ip6addr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
//  uip_ds6_prefix_add(&ipaddr,64,0);
}
#endif

/*--------------------------Announce the configuration---------------------*/
#if ANNOUNCE_BOOT

#if WEBSERVER
  uint8_t i;
  char buf[80];
  unsigned int size;

  for (i=0;i<UIP_DS6_ADDR_NB;i++) {
	if (uip_ds6_if.addr_list[i].isused) {	  
	   httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr,buf);
       PRINTF("IPv6 Address: %s\n",buf);
	}
  }
   eeprom_read_block (buf,server_name, sizeof(server_name));
   buf[sizeof(server_name)]=0;
   PRINTF("%s",buf);
   eeprom_read_block (buf,domain_name, sizeof(domain_name));
   buf[sizeof(domain_name)]=0;
   size=httpd_fs_get_size();
#ifndef COFFEE_FILES
   PRINTF(".%s online with fixed %u byte web content\n",buf,size);
#elif COFFEE_FILES==1
   PRINTF(".%s online with static %u byte EEPROM file system\n",buf,size);
#elif COFFEE_FILES==2
   PRINTF(".%s online with dynamic %u KB EEPROM file system\n",buf,size>>10);
#elif COFFEE_FILES==3
   PRINTF(".%s online with static %u byte program memory file system\n",buf,size);
#elif COFFEE_FILES==4
   PRINTF(".%s online with dynamic %u KB program memory file system\n",buf,size>>10);
#endif /* COFFEE_FILES */

#else
   PRINTF("Online\n");
#endif /* WEBSERVER */

#endif /* ANNOUNCE_BOOT */
}

#if RF230BB
extern char rf230_interrupt_flag, rf230processflag;
#endif

/*-------------------------------------------------------------------------*/
/*------------------------- Main Scheduler loop----------------------------*/
/*-------------------------------------------------------------------------*/
int
main(void)
{
  initialize();

  while(1) {
    process_run();
    watchdog_periodic();

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
    if (rf230_calibrated) {
      PRINTF("\nRF230 calibrated!\n");
      rf230_calibrated=0;
    }
#endif

#if TESTRTIMER
/* Timeout can be increased up to 8 seconds maximum.
 * A one second cycle is convenient for triggering the various debug printouts.
 * The triggers are staggered to avoid printing everything at once.
 * My raven is 6% slow.
 */
    if (rtimerflag) {
      rtimer_set(&rt, RTIMER_NOW()+ RTIMER_ARCH_SECOND*1UL, 1,(void *) rtimercycle, NULL);
      rtimerflag=0;

#if STAMPS
if ((rtime%STAMPS)==0) {
  PRINTF("%us ",rtime);
}
#endif
      rtime+=1;

#if PINGS
if ((rtime%PINGS)==1) {
  PRINTF("**Ping\n");
  raven_ping6();
}
#endif

#if ROUTES
if ((rtime%ROUTES)==2) {
      
 //#if UIP_CONF_IPV6_RPL
//#include "rpl.h"
extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_route_t uip_ds6_routing_table[];
extern uip_ds6_netif_t uip_ds6_if;

  uint8_t i,j;
  PRINTF("\nAddresses [%u max]\n",UIP_DS6_ADDR_NB);
  for (i=0;i<UIP_DS6_ADDR_NB;i++) {
    if (uip_ds6_if.addr_list[i].isused) {	  
      ipaddr_add(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }
  PRINTF("\nNeighbors [%u max]\n",UIP_DS6_NBR_NB);
  for(i = 0,j=1; i < UIP_DS6_NBR_NB; i++) {
    if(uip_ds6_nbr_cache[i].isused) {
      ipaddr_add(&uip_ds6_nbr_cache[i].ipaddr);
      PRINTF("\n");
      j=0;
    }
  }
  if (j) PRINTF("  <none>");
  PRINTF("\nRoutes [%u max]\n",UIP_DS6_ROUTE_NB);
  for(i = 0,j=1; i < UIP_DS6_ROUTE_NB; i++) {
    if(uip_ds6_routing_table[i].isused) {
      ipaddr_add(&uip_ds6_routing_table[i].ipaddr);
      PRINTF("/%u (via ", uip_ds6_routing_table[i].length);
      ipaddr_add(&uip_ds6_routing_table[i].nexthop);
 //     if(uip_ds6_routing_table[i].state.lifetime < 600) {
        PRINTF(") %lus\n", uip_ds6_routing_table[i].state.lifetime);
 //     } else {
 //       PRINTF(")\n");
 //     }
      j=0;
    }
  }
  if (j) PRINTF("  <none>");
  PRINTF("\n---------\n");
}
#endif

#if STACKMONITOR
if ((rtime%STACKMONITOR)==3) {
  extern uint16_t __bss_end;
  uint16_t p=(uint16_t)&__bss_end;
  do {
    if (*(uint16_t *)p != 0x4242) {
      PRINTF("Never-used stack > %d bytes\n",p-(uint16_t)&__bss_end);
      break;
    }
    p+=10;
  } while (p<RAMEND-10);
}
#endif

    }
#endif /* TESTRTIMER */

//Use with RF230BB DEBUGFLOW to show path through driver
#if RF230BB&&0
extern uint8_t debugflowsize,debugflow[];
  if (debugflowsize) {
    debugflow[debugflowsize]=0;
    PRINTF("%s",debugflow);
    debugflowsize=0;
   }
#endif

#if RF230BB&&0
    if (rf230processflag) {
      PRINTF("rf230p%d",rf230processflag);
      rf230processflag=0;
    }
#endif

#if RF230BB&&0
    if (rf230_interrupt_flag) {
 //   if (rf230_interrupt_flag!=11) {
        PRINTF("**RI%u",rf230_interrupt_flag);
 //   }
      rf230_interrupt_flag=0;
    }
#endif
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

void log_message(char *m1, char *m2)
{
  PRINTF("%s%s\n", m1, m2);
}

#if ROUTES
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int8_t i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) PRINTF("::");
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        PRINTF(":");
      }
      PRINTF("%x",a);
    }
  }
}
#endif
