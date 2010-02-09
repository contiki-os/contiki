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
#define DEBUG 0
#if DEBUG
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
int pingtimer1=0,pingtimer2=0;
#if RF230BB
extern int rf230_interrupt_flag;
#endif
#else
#define PRINTF(...)
#endif

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

//#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"

#ifdef RF230BB        //radio driver using contiki core mac
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#include "net/sicslowpan.h"
#include "net/uip-netif.h"
#include "net/mac/sicslowmac.h"
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
//#include "node-id.h"

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

/*-----------------------Initial contiki processes--------------------------*/
#ifdef RAVEN_LCD_INTERFACE
#ifdef RF230BB
PROCINIT(&etimer_process, &tcpip_process, &raven_lcd_process);
#else
PROCINIT(&etimer_process, &mac_process, &tcpip_process, &raven_lcd_process);
#endif /*RF230BB*/
#else
#ifdef RF230BB
PROCINIT(&etimer_process, &tcpip_process);
#elif WEBSERVER    //TODO:get hello-world to compile with ipv6
PROCINIT(&etimer_process, &mac_process, &tcpip_process);
#else
PROCINIT(&etimer_process);
#endif /*RF230BB*/
#endif /*RAVEN_LCD_INTERFACE*/

/*-------------------------Low level initialization------------------------*/
/*------Done in a subroutine to keep main routine stack usage small--------*/
void initialize(void)
{
  //calibrate_rc_osc_32k(); //CO: Had to comment this out

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
  printf_P(PSTR("\n*******Booting %s*******\n"),CONTIKI_VERSION_STRING);

 /* Initialize process subsystem */
  process_init();

#ifdef RF230BB
{
  /* Start radio and radio receive process */
  rf230_init();
  sicslowpan_init(sicslowmac_init(&rf230_driver));
//  ctimer_init();
//  sicslowpan_init(lpp_init(&rf230_driver));
//  rime_init(sicslowmac_driver.init(&rf230_driver));
//  rime_init(lpp_init(&rf230_driver));

  /* Set addresses BEFORE starting tcpip process */

  rimeaddr_t addr;
  memset(&addr, 0, sizeof(rimeaddr_t));
  AVR_ENTER_CRITICAL_REGION();
  eeprom_read_block ((void *)&addr.u8,  &mac_address, 8);
  AVR_LEAVE_CRITICAL_REGION();
 
  memcpy(&uip_lladdr.addr, &addr.u8, 8);	
  rf230_set_pan_addr(IEEE802154_PANID, 0, (uint8_t *)&addr.u8);

  rf230_set_channel(24);
  rimeaddr_set_node_addr(&addr); 
  PRINTF("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n",addr.u8[0],addr.u8[1],addr.u8[2],addr.u8[3],addr.u8[4],addr.u8[5],addr.u8[6],addr.u8[7]);

 // uip_ip6addr(&ipprefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
 // uip_netif_addr_add(&ipprefix, UIP_DEFAULT_PREFIX_LEN, 0, AUTOCONF);
 // uip_nd6_prefix_add(&ipprefix, UIP_DEFAULT_PREFIX_LEN, 0);
 // PRINTF("Prefix %x::/%u\n",ipprefix.u16[0],UIP_DEFAULT_PREFIX_LEN);

#if UIP_CONF_ROUTER
  rime_init(rime_udp_init(NULL));
  uip_router_register(&rimeroute);
#endif

  PRINTF("Driver: %s, Channel: %u\n", sicslowmac_driver.name, rf230_get_channel()); 
}
#endif /*RF230BB*/

  /* Register initial processes */
  procinit_init(); 

  /* Autostart processes */
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
#endif

/*--------------------------Announce the configuration---------------------*/
#define ANNOUNCE_BOOT 1    //adds about 400 bytes to program size
#if ANNOUNCE_BOOT

#if WEBSERVER

  uint8_t i;
  char buf[80];
  unsigned int size;
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
#endif

/* Add prefixes for testing */
#if 0
{  
  uip_ip6addr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_netif_addr_autoconf_set(&ipaddr, &uip_lladdr);
  uip_netif_addr_add(&ipaddr, 16, 0, TENTATIVE);
}
#endif
#if 0
{
  uip_ip6addr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_netif_addr_add(&ipaddr, UIP_DEFAULT_PREFIX_LEN, 0, AUTOCONF);
  uip_nd6_prefix_add(&ipaddr, UIP_DEFAULT_PREFIX_LEN, 0);
}
#endif

  for(i = 0; i < UIP_CONF_NETIF_MAX_ADDRESSES; i ++) {
   if(uip_netif_physical_if.addresses[i].state != NOT_USED) {
      httpd_cgi_sprint_ip6(*(uip_ipaddr_t*)&uip_netif_physical_if.addresses[i],buf);
      printf_P(PSTR("IPv6 Address: %s\n"),buf);
   }
  }
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

/*-------------------------------------------------------------------------*/
/*------------------------- Main Scheduler loop----------------------------*/
/*-------------------------------------------------------------------------*/
int
main(void)
{
  initialize();
  while(1) {
    process_run();

#if DEBUG
    if (rf230_interrupt_flag) {
      if (rf230_interrupt_flag!=11) {
        PRINTF("*****Radio interrupt %u\n",rf230_interrupt_flag);
        rf230_interrupt_flag=0;
      }
    }
#endif
#if PINGS
    if (pingtimer1++==10000) {
      pingtimer1=0;
      if (pingtimer2++==1000) { 
        PRINTF("-------Ping\n");
        pingtimer2=0;
        raven_ping6();
      }
    }
#endif

  }
  return 0;
}
