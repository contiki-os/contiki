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

#define DEBUG 0
#if DEBUG
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#define PRINTSHORT(FORMAT,args...) printf_P(PSTR(FORMAT),##args)

#else
#define PRINTF(...)
#define PRINTSHORT(...)
#endif

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

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

#if UIP_CONF_ROUTER
#include "net/routing/rimeroute.h"
#include "net/rime/rime-udp.h"
#endif

#include "net/rime.h"

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

/* Put default MAC address in EEPROM */
#if WEBSERVER
extern uint8_t mac_address[8];     //These are defined in httpd-fsdata.c via makefsdata.h 
extern uint8_t server_name[16];
extern uint8_t domain_name[30];
#else
uint8_t mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
#endif


static uint8_t get_channel_from_eeprom() {
	uint8_t eeprom_channel;
	uint8_t eeprom_check;

	eeprom_channel = eeprom_read_byte((uint8_t *)9);
	eeprom_check = eeprom_read_byte((uint8_t *)10);

	if(eeprom_channel==~eeprom_check)
		return eeprom_channel;
	return 26;
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


/*-------------------------Low level initialization------------------------*/
/*------Done in a subroutine to keep main routine stack usage small--------*/
void initialize(void)
{
  //calibrate_rc_osc_32k(); //CO: Had to comment this out
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
#if ANNOUNCE_BOOT
  printf_P(PSTR("\n*******Booting %s*******\n"),CONTIKI_VERSION_STRING);
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
 
  memcpy(&uip_lladdr.addr, &addr.u8, 8);	
  rf230_set_pan_addr(
	get_panid_from_eeprom(),
	get_panaddr_from_eeprom(),
	(uint8_t *)&addr.u8
  );
  rf230_set_channel(get_channel_from_eeprom());

  rimeaddr_set_node_addr(&addr); 

  PRINTF("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n",addr.u8[0],addr.u8[1],addr.u8[2],addr.u8[3],addr.u8[4],addr.u8[5],addr.u8[6],addr.u8[7]);

  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

#if ANNOUNCE_BOOT
  printf_P(PSTR("%s %s, channel %u"),NETSTACK_MAC.name, NETSTACK_RDC.name,rf230_get_channel());
  if (NETSTACK_RDC.channel_check_interval) {//function pointer is zero for sicslowmac
    unsigned short tmp;
    tmp=CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval == 0 ? 1:\
                                   NETSTACK_RDC.channel_check_interval());
    if (tmp<65535) printf_P(PSTR(", check rate %u Hz"),tmp);
  }
  printf_P(PSTR("\n"));
#endif

#if UIP_CONF_ROUTER
#if ANNOUNCE_BOOT
  printf_P(PSTR("Routing Enabled\n"));
#endif
//  rime_init(rime_udp_init(NULL));
// uip_router_register(&rimeroute);
#endif

  process_start(&tcpip_process, NULL);

  #else
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
    printf_P(PSTR("No index.html file found, creating upload.html!\n"));
    printf_P(PSTR("Formatting FLASH file system for coffee..."));
    cfs_coffee_format();
    printf_P(PSTR("Done!\n"));
    fa = cfs_open( "/index.html", CFS_WRITE);
    int r = cfs_write(fa, &"It works!", 9);
    if (r<0) printf_P(PSTR("Can''t create /index.html!\n"));
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
       printf_P(PSTR("IPv6 Address: %s\n"),buf);
	}
  }
   eeprom_read_block (buf,server_name, sizeof(server_name));
   buf[sizeof(server_name)]=0;
   printf_P(PSTR("%s"),buf);
   eeprom_read_block (buf,domain_name, sizeof(domain_name));
   buf[sizeof(domain_name)]=0;
   size=httpd_fs_get_size();
#ifndef COFFEE_FILES
   printf_P(PSTR(".%s online with fixed %u byte web content\n"),buf,size);
#elif COFFEE_FILES==1
   printf_P(PSTR(".%s online with static %u byte EEPROM file system\n"),buf,size);
#elif COFFEE_FILES==2
   printf_P(PSTR(".%s online with dynamic %u KB EEPROM file system\n"),buf,size>>10);
#elif COFFEE_FILES==3
   printf_P(PSTR(".%s online with static %u byte program memory file system\n"),buf,size);
#elif COFFEE_FILES==4
   printf_P(PSTR(".%s online with dynamic %u KB program memory file system\n"),buf,size>>10);
#endif /* COFFEE_FILES */

#else
   printf_P(PSTR("Online\n"));
#endif /* WEBSERVER */

#endif /* ANNOUNCE_BOOT */
}

/*---------------------------------------------------------------------------*/
void log_message(char *m1, char *m2)
{
  printf_P(PSTR("%s%s\n"), m1, m2);
}
/* Test rtimers, also useful for pings and time stamps in simulator */
#define TESTRTIMER 0
#if TESTRTIMER
#define PINGS 60
#define STAMPS 30
uint8_t rtimerflag=1;
uint16_t rtime;
struct rtimer rt;
void rtimercycle(void) {rtimerflag=1;}
#endif /* TESTRTIMER */

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
//Various entry points for debugging in AVR simulator
//    NETSTACK_RADIO.send(packetbuf_hdrptr(), 42);
//    process_poll(&rf230_process);
//    packetbuf_clear();
//    len = rf230_read(packetbuf_dataptr(), PACKETBUF_SIZE);
//    packetbuf_set_datalen(42);
//    NETSTACK_RDC.input();
    watchdog_periodic();
#if TESTRTIMER
    if (rtimerflag) {  //8 seconds is maximum interval, my raven 6% slow
      rtimer_set(&rt, RTIMER_NOW()+ RTIMER_ARCH_SECOND*1UL, 1,(void *) rtimercycle, NULL);
      rtimerflag=0;
#if STAMPS
      if ((rtime%STAMPS)==0) {
        printf("%us ",rtime);
      }
#endif
      rtime+=1;
#if PINGS
      if ((rtime%PINGS)==0) {
        PRINTF("**Ping\n");
        raven_ping6();
      }
#endif
    }
#endif

//Use with RF230BB DEBUGFLOW to show path through driver
#if RF230BB&&0
extern uint8_t debugflowsize,debugflow[];
  if (debugflowsize) {
    debugflow[debugflowsize]=0;
    printf("%s",debugflow);
    debugflowsize=0;
   }
#endif

#if RF230BB&&0
    if (rf230processflag) {
      printf("rf230p%d",rf230processflag);
      rf230processflag=0;
    }
#endif

#if RF230BB&&0
    if (rf230_interrupt_flag) {
 //   if (rf230_interrupt_flag!=11) {
        PRINTSHORT("**RI%u",rf230_interrupt_flag);
 //   }
      rf230_interrupt_flag=0;
    }
#endif
  }
  return 0;
}
