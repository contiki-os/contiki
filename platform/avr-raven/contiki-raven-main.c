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
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)

#define ANNOUNCE_BOOT 1    //adds about 600 bytes to program size
#if ANNOUNCE_BOOT
#define PRINTA(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTA(...)
#endif

#define DEBUG 0
#if DEBUG
#define PRINTD(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTD(...)
#endif

/* Track interrupt flow through mac, rdc and radio driver */
#if DEBUGFLOWSIZE
uint8_t debugflowsize,debugflow[DEBUGFLOWSIZE];
#define DEBUGFLOW(c) if (debugflowsize<(DEBUGFLOWSIZE-1)) debugflow[debugflowsize++]=c
#else
#define DEBUGFLOW(c)
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

#if AVR_WEBSERVER
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

/* Get periodic prints from idle loop, from clock seconds or rtimer interrupts */
/* Use of rtimer will conflict with other rtimer interrupts such as contikimac radio cycling */
/* STAMPS will print ENERGEST outputs if that is enabled. */
#define PERIODICPRINTS 1
#if PERIODICPRINTS
//#define PINGS 64
#define ROUTES 600
#define STAMPS 60
#define STACKMONITOR 600
uint32_t clocktime;
#define TESTRTIMER 0
#if TESTRTIMER
uint8_t rtimerflag=1;
struct rtimer rt;
void rtimercycle(void) {rtimerflag=1;}
#endif
#endif

#if WITH_NODE_ID
uint16_t node_id;
#endif

/*-------------------------------------------------------------------------*/
/*----------------------Configuration of the .elf file---------------------*/
#if 1
/* The proper way to set the signature is */
#include <avr/signature.h>
#else
/* Older avr-gcc's may not define the needed SIGNATURE bytes. Do it manually if you get an error */
typedef struct {const unsigned char B2;const unsigned char B1;const unsigned char B0;} __signature_t;
#define SIGNATURE __signature_t __signature __attribute__((section (".signature")))
SIGNATURE = {
  .B2 = 0x05,//SIGNATURE_2, //ATMEGA1284p
  .B1 = 0x97,//SIGNATURE_1, //128KB flash
  .B0 = 0x1E,//SIGNATURE_0, //Atmel
};
#endif

/* JTAG, SPI enabled, Internal RC osc, Boot flash size 4K, 6CK+65msec delay, brownout disabled */
FUSES ={.low = 0xe2, .high = 0x99, .extended = 0xff,};

/* Put the default settings into program flash memory */
/* Webserver builds can set some defaults in httpd-fsdata.c via makefsdata.h */
#if AVR_WEBSERVER
extern uint8_t default_mac_address[8];
extern uint8_t default_server_name[16];
extern uint8_t default_domain_name[30];
#else
#ifdef MAC_ADDRESS
uint8_t default_mac_address[8] PROGMEM = MAC_ADDRESS;
#else
uint8_t default_mac_address[8] PROGMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
#endif
#ifdef SERVER_NAME
uint8_t default_server_name[16] PROGMEM = SERVER_NAME;
#else
uint8_t default_server_name[16] PROGMEM = "Raven_webserver";
#endif
#ifdef DOMAIN_NAME
uint8_t default_domain_name[30] PROGMEM = DOMAIN_NAME
#else
uint8_t default_domain_name[30] PROGMEM = "localhost";
#endif
#endif /* AVR_WEBSERVER */

#ifdef NODE_ID
uint16_t default_nodeid PROGMEM = NODEID;
#else
uint16_t default_nodeid PROGMEM = 0;
#endif
#ifdef CHANNEL_802_15_4
uint8_t default_channel PROGMEM = CHANNEL_802_15_4;
#else
uint8_t default_channel PROGMEM = 26;
#endif
#ifdef IEEE802154_PANID
uint16_t default_panid PROGMEM = IEEE802154_PANID;
#else
uint16_t default_panid PROGMEM = 0xABCD;
#endif
#ifdef IEEE802154_PANADDR
uint16_t default_panaddr PROGMEM = IEEE802154_PANID;
#else
uint16_t default_panaddr PROGMEM = 0;
#endif
#ifdef RF230_MAX_TX_POWER
uint8_t default_txpower PROGMEM = RF230_MAX_TX_POWER;
#else
uint8_t default_txpower PROGMEM = 0;
#endif

/* Get a pseudo random number using the ADC */
static uint8_t
rng_get_uint8(void) {
uint8_t i,j;
  ADCSRA=1<<ADEN;             //Enable ADC, not free running, interrupt disabled, fastest clock
  for (i=0;i<4;i++) {
    ADMUX = 0;                //toggle reference to increase noise
    ADMUX =0x1E;              //Select AREF as reference, measure 1.1 volt bandgap reference.
    ADCSRA|=1<<ADSC;          //Start conversion
    while (ADCSRA&(1<<ADSC)); //Wait till done
	j = (j<<2) + ADC;
  }
  ADCSRA=0;                   //Disable ADC
  PRINTD("rng issues %d\n",j);
  return j;
}

#if CONTIKI_CONF_RANDOM_MAC
static void
generate_new_eui64(uint8_t eui64[8]) {
	eui64[0] = 0x02;
	eui64[1] = rng_get_uint8();
	eui64[2] = rng_get_uint8();
	eui64[3] = 0xFF;
	eui64[4] = 0xFE;
	eui64[5] = rng_get_uint8();
	eui64[6] = rng_get_uint8();
	eui64[7] = rng_get_uint8();
}
#endif

#if !CONTIKI_CONF_SETTINGS_MANAGER
/****************************No settings manager*****************************/
/* If not using the settings manager, put the default values into EEMEM
 * These can be manually changed and kept over program reflash.
 * The channel and bit complement are used to check EEMEM integrity,
 * If corrupt all values will be rewritten with the default flash values.
 * To make this work, get the channel before anything else.
 */
#if AVR_WEBSERVER
extern uint8_t eemem_mac_address[8];     //These are defined in httpd-fsdata.c via makefsdata.h
extern uint8_t eemem_server_name[16];
extern uint8_t eemem_domain_name[30];
#else
#ifdef MAC_ADDRESS
uint8_t eemem_mac_address[8] EEMEM = MAC_ADDRESS;
#else
uint8_t eemem_mac_address[8] EEMEM = {0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, 0x44, 0x55};
#endif
#ifdef SERVER_NAME
uint8_t eemem_server_name[16] EEMEM = SERVER_NAME;
#else
uint8_t eemem_server_name[16] EEMEM = "Raven_webserver";
#endif
#ifdef DOMAIN_NAME
uint8_t eemem_domain_name[30] EEMEM = DOMAIN_NAME
#else
uint8_t eemem_domain_name[30] EEMEM = "localhost";
#endif
#endif /*AVR_WEBSERVER */

#ifdef NODE_ID
uint16_t eemem_nodeid EEMEM = NODEID;
#else
uint16_t eemem_nodeid EEMEM = 0;
#endif
#ifdef CHANNEL_802_15_4
uint8_t eemem_channel[2] EEMEM = {CHANNEL_802_15_4, ~CHANNEL_802_15_4};
#else
uint8_t eemem_channel[2] EMEM = {26, ~26};
#endif
#ifdef IEEE802154_PANID
uint16_t eemem_panid EEMEM = IEEE802154_PANID;
#else
uint16_t eemem_panid EEMEM = 0xABCD;
#endif
#ifdef IEEE802154_PANADDR
uint16_t eemem_panaddr EEMEM = IEEE802154_PANADDR;
#else
uint16_t eemem_panaddr EEMEM = 0;
#endif
#ifdef RF230_MAX_TX_POWER
uint8_t eemem_txpower EEMEM = RF230_MAX_TX_POWER;
#else
uint8_t eemem_txpower EEMEM = 0;
#endif

static uint8_t
get_channel_from_eeprom() {
  uint8_t x[2];
  *(uint16_t *)x = eeprom_read_word ((uint16_t *)&eemem_channel);
/* Don't return an invalid channel number */
  if( (x[0]<11) || (x[0] > 26)) x[1]=x[0];
/* Do exclusive or test on the two values read */
  if((uint8_t)x[0]!=(uint8_t)~x[1]) {//~x[1] can promote comparison to 16 bit
/* Verification fails, rewrite everything */
    uint8_t i,buffer[32];
    PRINTD("EEPROM is corrupt, rewriting with defaults.\n");
#if CONTIKI_CONF_RANDOM_MAC
    PRINTA("Generating random MAC address.\n");
    generate_new_eui64(&buffer);
#else
    for (i=0;i<sizeof(default_mac_address);i++) buffer[i] = pgm_read_byte_near(default_mac_address+i);
#endif
    cli();
    eeprom_write_block(&buffer,  &eemem_mac_address, sizeof(eemem_mac_address));
    for (i=0;i<sizeof(default_server_name);i++) buffer[i] = pgm_read_byte_near(default_server_name+i);
    eeprom_write_block(&buffer,  &eemem_server_name, sizeof(eemem_server_name));
    for (i=0;i<sizeof(default_domain_name);i++) buffer[i] = pgm_read_byte_near(default_domain_name+i);
    eeprom_write_block(&buffer,  &eemem_domain_name, sizeof(eemem_domain_name));
    eeprom_write_word(&eemem_panid  , pgm_read_word_near(&default_panid));
    eeprom_write_word(&eemem_panaddr, pgm_read_word_near(&default_panaddr));
    eeprom_write_byte(&eemem_txpower, pgm_read_byte_near(&default_txpower));
    eeprom_write_word(&eemem_nodeid, pgm_read_word_near(&default_nodeid));
    x[0] = pgm_read_byte_near(&default_channel);
    x[1]= ~x[0];
    eeprom_write_word((uint16_t *)&eemem_channel, *(uint16_t *)x);
	sei();
  }
/* Always returns a valid channel */
  return x[0];
}
static bool
get_eui64_from_eeprom(uint8_t macptr[sizeof(rimeaddr_t)]) {
  cli();
  eeprom_read_block ((void *)macptr, &eemem_mac_address, sizeof(rimeaddr_t));
  sei();
  return macptr[0]!=0xFF;
}
static uint16_t
get_panid_from_eeprom(void) {
  return eeprom_read_word(&eemem_panid);
}
static uint16_t
get_panaddr_from_eeprom(void) {
  return eeprom_read_word (&eemem_panaddr);
}
static uint8_t
get_txpower_from_eeprom(void)
{
  return eeprom_read_byte(&eemem_txpower);
}

#else /* !CONTIKI_CONF_SETTINGS_MANAGER */
/******************************Settings manager******************************/
#include "settings.h"
/* Disable the settings add routines to reduce eeprom wear during testing */
#if 0
#define settings_add(...) 0
#define settings_add_uint8(...) 0
#define settings_add_uint16(...) 0
#endif

#if AVR_WEBSERVER
extern uint8_t eemem_mac_address[8];     //These are defined in httpd-fsdata.c via makefsdata.h
extern uint8_t eemem_server_name[16];
extern uint8_t eemem_domain_name[30];
#endif

static uint8_t
get_channel_from_eeprom() {
  uint8_t x;
  size_t  size = 1;
  if (settings_get(SETTINGS_KEY_CHANNEL, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    if ((x<11) || (x>26)) {
      PRINTF("Unusual RF channel %u in EEPROM\n",x);
	}
    PRINTD("<=Get RF channel %u.\n",x);
  } else {
    x = pgm_read_byte_near(&default_channel);
    if (settings_add_uint8(SETTINGS_KEY_CHANNEL,x ) == SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM RF channel to %d.\n",x);
    }
  }
  return x;
}
static bool
get_eui64_from_eeprom(uint8_t macptr[8]) {
  size_t size = sizeof(rimeaddr_t); 
  if(settings_get(SETTINGS_KEY_EUI64, 0, (unsigned char*)macptr, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<=Get MAC address.\n");
    return true;		
  }
#if CONTIKI_CONF_RANDOM_MAC
  PRINTD("--Generating random MAC address.\n");
  generate_new_eui64(macptr);
#else
  {uint8_t i;for (i=0;i<8;i++) macptr[i] = pgm_read_byte_near(default_mac_address+i);}
#endif
  if (settings_add(SETTINGS_KEY_EUI64,(unsigned char*)macptr,8)) {
    PRINTD("->Set EEPROM MAC address.\n");
  }
  return true;
}
static uint16_t
get_panid_from_eeprom(void) {
  uint16_t x;
  size_t  size = 2;
  if (settings_get(SETTINGS_KEY_PAN_ID, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get PAN ID of %04x.\n",x);
  } else {
    x=pgm_read_word_near(&default_panid);
    if (settings_add_uint16(SETTINGS_KEY_PAN_ID,x)==SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM PAN ID to %04x.\n",x);
    }
  }
  return x;
}
static uint16_t
get_panaddr_from_eeprom(void) {
  uint16_t x;
  size_t  size = 2;
  if (settings_get(SETTINGS_KEY_PAN_ADDR, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get PAN address of %04x.\n",x);
  } else {
    x=pgm_read_word_near(&default_panaddr);
    if (settings_add_uint16(SETTINGS_KEY_PAN_ADDR,x)==SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM PAN address to %04x.\n",x);
    }
  }        
  return x;
}
static uint8_t
get_txpower_from_eeprom(void) {
  uint8_t x;
  size_t  size = 1;
  if (settings_get(SETTINGS_KEY_TXPOWER, 0,(unsigned char*)&x, &size) == SETTINGS_STATUS_OK) {
    PRINTD("<-Get tx power of %d. (0=max)\n",x);
  } else {
    x=pgm_read_byte_near(&default_txpower);
    if (settings_add_uint8(SETTINGS_KEY_TXPOWER,x)==SETTINGS_STATUS_OK) {
      PRINTD("->Set EEPROM tx power of %d. (0=max)\n",x);
    }
  }
  return x;
}
#endif /* CONTIKI_CONF_SETTINGS_MANAGER */

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
   * loop. In conjuction with PERIODICPRINTS, never-used stack will be printed
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

/* Get a random (or probably different) seed for the 802.15.4 packet sequence number.
 * Some layers will ignore duplicates found in a history (e.g. Contikimac)
 * causing the initial packets to be ignored after a short-cycle restart.
 */
 random_init(rng_get_uint8());

#define CONF_CALIBRATE_OSCCAL 0
#if CONF_CALIBRATE_OSCCAL
void calibrate_rc_osc_32k();
{
extern uint8_t osccal_calibrated;
uint8_t i;
  PRINTD("\nBefore calibration OSCCAL=%x\n",OSCCAL);
  for (i=0;i<10;i++) { 
    calibrate_rc_osc_32k();  
    PRINTD("Calibrated=%x\n",osccal_calibrated);
//#include <util/delay_basic.h>
//#define delay_us( us )   ( _delay_loop_2(1+(us*F_CPU)/4000000UL) ) 
//   delay_us(50000);
 }
   clock_init();
}
#endif 

  PRINTA("\n*******Booting %s*******\n",CONTIKI_VERSION_STRING);

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
//  memset(&addr, 0, sizeof(rimeaddr_t));
  get_eui64_from_eeprom(addr.u8);
 
#if UIP_CONF_IPV6 
  memcpy(&uip_lladdr.addr, &addr.u8, sizeof(rimeaddr_t));
#elif WITH_NODE_ID
  node_id=get_panaddr_from_eeprom();
  addr.u8[1]=node_id&0xff;
  addr.u8[0]=(node_id&0xff00)>>8;
  PRINTA("Node ID from eeprom: %X\n",node_id);
#endif  
  rimeaddr_set_node_addr(&addr); 

  rf230_set_pan_addr(
	get_panid_from_eeprom(),
	get_panaddr_from_eeprom(),
	(uint8_t *)&addr.u8
  );
  rf230_set_channel(get_channel_from_eeprom());
  rf230_set_txpower(get_txpower_from_eeprom());

#if UIP_CONF_IPV6
  PRINTA("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n\r",addr.u8[0],addr.u8[1],addr.u8[2],addr.u8[3],addr.u8[4],addr.u8[5],addr.u8[6],addr.u8[7]);
#else
  PRINTA("MAC address ");
  uint8_t i;
  for (i=sizeof(rimeaddr_t); i>0; i--){
    PRINTA("%x:",addr.u8[i-1]);
  }
  PRINTA("\n");
#endif

  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

#if ANNOUNCE_BOOT
  PRINTA("%s %s, channel %u power %u",NETSTACK_MAC.name, NETSTACK_RDC.name,rf230_get_channel()),rf230_get_txpower();
  if (NETSTACK_RDC.channel_check_interval) {//function pointer is zero for sicslowmac
    unsigned short tmp;
    tmp=CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval == 0 ? 1:\
                                   NETSTACK_RDC.channel_check_interval());
    if (tmp<65535) PRINTA(", check rate %u Hz",tmp);
  }
  PRINTA("\n");

#if UIP_CONF_IPV6_RPL
  PRINTA("RPL Enabled\n");
#endif
#if UIP_CONF_ROUTER
  PRINTA("Routing Enabled\n");
#endif

#endif /* ANNOUNCE_BOOT */

// rime_init(rime_udp_init(NULL));
// uip_router_register(&rimeroute);

  process_start(&tcpip_process, NULL);

#else /* !RF230BB */
/* Original RF230 combined mac/radio driver */
/* mac process must be started before tcpip process! */
  process_start(&mac_process, NULL);
  process_start(&tcpip_process, NULL);
#endif /* RF230BB */

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
    PRINTA("No index.html file found, creating upload.html!\n");
    PRINTA("Formatting FLASH file system for coffee...");
    cfs_coffee_format();
    PRINTA("Done!\n");
    fa = cfs_open( "/index.html", CFS_WRITE);
    int r = cfs_write(fa, &"It works!", 9);
    if (r<0) PRINTA("Can''t create /index.html!\n");
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
{
#if AVR_WEBSERVER
  uint8_t i;
  char buf[80];
  unsigned int size;

  for (i=0;i<UIP_DS6_ADDR_NB;i++) {
	if (uip_ds6_if.addr_list[i].isused) {	  
	   httpd_cgi_sprint_ip6(uip_ds6_if.addr_list[i].ipaddr,buf);
       PRINTA("IPv6 Address: %s\n",buf);
	}
  }
   cli();
   eeprom_read_block (buf,eemem_server_name, sizeof(eemem_server_name));
   sei();
   buf[sizeof(eemem_server_name)]=0;
   PRINTA("%s",buf);
   cli();
   eeprom_read_block (buf,eemem_domain_name, sizeof(eemem_domain_name));
   sei();
   buf[sizeof(eemem_domain_name)]=0;
   size=httpd_fs_get_size();
#ifndef COFFEE_FILES
   PRINTA(".%s online with fixed %u byte web content\n",buf,size);
#elif COFFEE_FILES==1
   PRINTA(".%s online with static %u byte EEPROM file system\n",buf,size);
#elif COFFEE_FILES==2
   PRINTA(".%s online with dynamic %u KB EEPROM file system\n",buf,size>>10);
#elif COFFEE_FILES==3
   PRINTA(".%s online with static %u byte program memory file system\n",buf,size);
#elif COFFEE_FILES==4
   PRINTA(".%s online with dynamic %u KB program memory file system\n",buf,size>>10);
#endif /* COFFEE_FILES */

#else
   PRINTA("Online\n");
#endif /* AVR_WEBSERVER */

#endif /* ANNOUNCE_BOOT */
}
}

#if ROUTES && UIP_CONF_IPV6
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
      PRINTD("\nRF230 calibrated!\n");
      rf230_calibrated=0;
    }
#endif

#if DEBUGFLOWSIZE
  if (debugflowsize) {
    debugflow[debugflowsize]=0;
    PRINTF("%s",debugflow);
    debugflowsize=0;
   }
#endif

    watchdog_periodic();

#if PERIODICPRINTS
#if TESTRTIMER
/* Timeout can be increased up to 8 seconds maximum.
 * A one second cycle is convenient for triggering the various debug printouts.
 * The triggers are staggered to avoid printing everything at once.
 */
    if (rtimerflag) {
      rtimer_set(&rt, RTIMER_NOW()+ RTIMER_ARCH_SECOND*1UL, 1,(void *) rtimercycle, NULL);
      rtimerflag=0;
#else
  if (clocktime!=clock_seconds()) {
     clocktime=clock_seconds();
#endif

#if STAMPS
if ((clocktime%STAMPS)==0) {
#if ENERGEST_CONF_ON
#include "lib/print-stats.h"
	print_stats();
#elif RADIOSTATS
extern volatile unsigned long radioontime;
  PRINTF("%u(%u)s\n",clocktime,radioontime);
#else
  PRINTF("%us\n",clocktime);
#endif

}
#endif
#if TESTRTIMER
      clocktime+=1;
#endif

#if PINGS && UIP_CONF_IPV6
extern void raven_ping6(void); 
if ((clocktime%PINGS)==1) {
  PRINTF("**Ping\n");
  raven_ping6();
}
#endif

#if ROUTES && UIP_CONF_IPV6
if ((clocktime%ROUTES)==2) {
      
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
if ((clocktime%STACKMONITOR)==3) {
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
#endif /* PERIODICPRINTS */

#if RF230BB&&0
extern uint8_t rf230processflag;
    if (rf230processflag) {
      PRINTF("rf230p%d",rf230processflag);
      rf230processflag=0;
    }
#endif

#if RF230BB&&0
extern uint8_t rf230_interrupt_flag;
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
