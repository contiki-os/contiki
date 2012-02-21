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

/**
 * \file
 *         Contiki 2.4 kernel for Jackdaw USB stick
 *
 * \author
 *         Simon Barner <barner@in.tum.de>
 *         David Kopf <dak664@embarqmail.com>
 */

#define DEBUG 0
#if DEBUG
#define PRINTD(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTD(...)
#endif

#include <avr/pgmspace.h>
#include <avr/fuse.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "lib/mmem.h"
#include "loader/symbols-def.h"
#include "loader/symtab.h"

#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "contiki-raven.h"

/* Set ANNOUNCE to send boot messages to USB or RS232 serial port */
#define ANNOUNCE 1

/* But only if a serial port exists */
#if USB_CONF_SERIAL||USB_CONF_RS232
#define PRINTA(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTA(...)
#endif

#include "usb_task.h"
#if USB_CONF_SERIAL
#include "cdc_task.h"
#endif
#if USB_CONF_RS232
#include "dev/rs232.h"
#endif

#include "rndis/rndis_task.h"
#if USB_CONF_STORAGE
#include "storage/storage_task.h"
#endif

#include "dev/watchdog.h"
#include "dev/usb/usb_drv.h"

#if JACKDAW_CONF_USE_SETTINGS
#include "settings.h"
#endif

#if RF230BB           //radio driver using contiki core mac
#include "radio/rf230bb/rf230bb.h"
#include "net/mac/frame802154.h"
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
rimeaddr_t macLongAddr;
#define	tmp_addr	macLongAddr
#else                 //legacy radio driver using Atmel/Cisco 802.15.4'ish MAC
#include <stdbool.h>
#include "mac.h"
#include "sicslowmac.h"
#include "sicslowpan.h"
#include "ieee-15-4-manager.h"
#endif /* RF230BB */

/* Test rtimers, also useful for pings, time stamps, routes, stack monitor */
#define TESTRTIMER 0
#if TESTRTIMER
#define PINGS 0
#define STAMPS 60
#define ROUTES 120
#define STACKMONITOR  600
uint8_t rtimerflag=1;
uint16_t rtime;
struct rtimer rt;
void rtimercycle(void) {rtimerflag=1;}
#endif /* TESTRTIMER */

#if UIP_CONF_IPV6_RPL
/*---------------------------------------------------------------------------*/
/*---------------------------------  RPL   ----------------------------------*/
/*---------------------------------------------------------------------------*/
/* TODO: Put rpl code into another file, once it stabilizes                  */
/* Set up fallback interface links to direct stack tcpip output to ethernet  */
static void
init(void)
{
}
void mac_LowpanToEthernet(void);
static void
output(void)
{
//  if(uip_ipaddr_cmp(&last_sender, &UIP_IP_BUF->srcipaddr)) {
    /* Do not bounce packets back over USB if the packet was received from USB */
//    PRINTA("JACKDAW router: Destination off-link but no route\n");
 // } else {
    PRINTD("SUT: %u\n", uip_len);
    mac_LowpanToEthernet();  //bounceback trap is done in lowpanToEthernet
//  }
}
const struct uip_fallback_interface rpl_interface = {
  init, output
};

#if RPL_BORDER_ROUTER
#include "net/rpl/rpl.h"

// avr-objdump --section .bss -x ravenusbstick.elf
const uint16_t dag_id[] PROGMEM = {0x1111, 0x1100, 0, 0, 0, 0, 0, 0x0011};

PROCESS(border_router_process, "RPL Border Router");
PROCESS_THREAD(border_router_process, ev, data)
{

  PROCESS_BEGIN();

  PROCESS_PAUSE();

{ rpl_dag_t *dag;
  char buf[sizeof(dag_id)];
  memcpy_P(buf,dag_id,sizeof(dag_id));
  dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)buf);

/* Assign separate addresses to the jackdaw uip stack and the host network interface, but with the same prefix */
/* E.g. bbbb::200 to the jackdaw and bbbb::1 to the host network interface with $ip -6 address add bbbb::1/64 dev usb0 */
/* Otherwise the host will trap packets intended for the jackdaw, just as the jackdaw will trap RF packets intended for the host */
/* $ifconfig usb0 -arp on Ubuntu to skip the neighbor solicitations. Add explicit neighbors on other OSs */
  if(dag != NULL) {
    PRINTD("created a new RPL dag\n");

#if UIP_CONF_ROUTER_RECEIVE_RA
//Contiki stack will shut down until assigned an address from the interface RA
//Currently this requires changes in the core rpl-icmp6.c to pass the link-local RA broadcast

#else
    uip_ip6addr_t ipaddr;
    uip_ip6addr(&ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x200);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
    rpl_set_prefix(dag, &ipaddr, 64);
#endif
  }
}
  /* The border router runs with a 100% duty cycle in order to ensure high
     packet reception rates. */
 // NETSTACK_MAC.off(1);

  while(1) {
    PROCESS_YIELD();
    /* Local and global dag repair can be done from the jackdaw menu */

  }

  PROCESS_END();
}
#endif /* RPL_BORDER_ROUTER */

#endif /* UIP_CONF_IPV6_RPL */

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
  .B2 = 0x82,//SIGNATURE_2, //AT90USB128x
  .B1 = 0x97,//SIGNATURE_1, //128KB flash
  .B0 = 0x1E,//SIGNATURE_0, //Atmel
};
#endif

FUSES ={.low = 0xde, .high = 0x99, .extended = 0xff,};

/* Save the default settings into program flash memory */
const uint8_t default_mac_address[8] PROGMEM = {0x02, 0x12, 0x13, 0xff, 0xfe, 0x14, 0x15, 0x16};
#ifdef CHANNEL_802_15_4
const uint8_t default_channel PROGMEM = CHANNEL_802_15_4;
#else
const uint8_t default_channel PROGMEM = 26;
#endif
#ifdef IEEE802154_PANID
const uint16_t default_panid PROGMEM = IEEE802154_PANID;
#else
const uint16_t default_panid PROGMEM = 0xABCD;
#endif
#ifdef IEEE802154_PANADDR
const uint16_t default_panaddr PROGMEM = IEEE802154_PANID;
#else
const uint16_t default_panaddr PROGMEM = 0;
#endif
#ifdef RF230_MAX_TX_POWER
const uint8_t default_txpower PROGMEM = RF230_MAX_TX_POWER;
#else
const uint8_t default_txpower PROGMEM = 0;
#endif

#if JACKDAW_CONF_RANDOM_MAC
#include "rng.h"
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
#endif /* JACKDAW_CONF_RANDOM_MAC */

#if !JACKDAW_CONF_USE_SETTINGS
/****************************No settings manager*****************************/
/* If not using the settings manager, put the default values into EEMEM
 * These can be manually changed and kept over program reflash.
 * The channel and bit complement are used to check EEMEM integrity,
 * If corrupt all values will be rewritten with the default flash values.
 * To make this work, get the channel before anything else.
 */

uint8_t eemem_mac_address[8] EEMEM = {0x02, 0x12, 0x13, 0xff, 0xfe, 0x14, 0x15, 0x16};
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
static uint8_t get_channel_from_eeprom() {
	uint8_t x[2];
	*(uint16_t *)x = eeprom_read_word ((uint16_t *)&eemem_channel);
    if((uint8_t)x[0]!=(uint8_t)~x[1]) {//~x[1] can promote comparison to 16 bit
/* Verification fails, rewrite everything */
    uint8_t mac[8];
#if JACKDAW_CONF_RANDOM_MAC
    PRINTA("Generating random MAC address.\n");
    generate_new_eui64(&mac);
#else
    {uint8_t i; for (i=0;i<8;i++) mac[i] = pgm_read_byte_near(default_mac_address+i);}
#endif
	eeprom_write_block(&mac,  &eemem_mac_address, 8);
  	eeprom_write_word(&eemem_panid  , pgm_read_word_near(&default_panid));
   	eeprom_write_word(&eemem_panaddr, pgm_read_word_near(&default_panaddr));
    eeprom_write_byte(&eemem_txpower, pgm_read_byte_near(&default_txpower));
    x[0] = pgm_read_byte_near(&default_channel);
    x[1]= ~x[0];
    eeprom_write_word((uint16_t *)&eemem_channel, *(uint16_t *)x);    
  }
  return x[0];
}
static bool get_eui64_from_eeprom(uint8_t macptr[8]) {
	eeprom_read_block ((void *)macptr, &eemem_mac_address, 8);
	return macptr[0]!=0xFF;
}
static uint16_t get_panid_from_eeprom(void) {
	return eeprom_read_word(&eemem_panid);
}
static uint16_t get_panaddr_from_eeprom(void) {
	return eeprom_read_word (&eemem_panaddr);
}
static uint8_t get_txpower_from_eeprom(void)
{
	return eeprom_read_byte(&eemem_txpower);
}

#else /* !JACKDAW_CONF_USE_SETTINGS */
/******************************Settings manager******************************/
static uint8_t get_channel_from_eeprom() {
	uint8_t x = settings_get_uint8(SETTINGS_KEY_CHANNEL, 0);
	if(!x) x = pgm_read_byte_near(&default_channel);
	return x;
}
static bool get_eui64_from_eeprom(uint8_t macptr[8]) {
	size_t size = 8;
	if(settings_get(SETTINGS_KEY_EUI64, 0, (unsigned char*)macptr, &size)==SETTINGS_STATUS_OK) {
      PRINTD("<=Get EEPROM MAC address.\n");
      return true;		
    }
#if JACKDAW_CONF_RANDOM_MAC
    PRINTA("--Generating random MAC address.\n");
    generate_new_eui64(macptr);
#else
    {uint8_t i;for (i=0;i<8;i++) macptr[i] = pgm_read_byte_near(default_mac_address+i);}
#endif
    settings_add(SETTINGS_KEY_EUI64,(unsigned char*)macptr,8);
    PRINTA("->Set EEPROM MAC address.\n");
	return true;
}
static uint16_t get_panid_from_eeprom(void) {
    uint16_t x;
    if (settings_check(SETTINGS_KEY_PAN_ID,0)) {
        x = settings_get_uint16(SETTINGS_KEY_PAN_ID,0);
        PRINTD("<-Get EEPROM PAN ID of %04x.\n",x);
    } else {
	    x=pgm_read_word_near(&default_panid);
        if (settings_add_uint16(SETTINGS_KEY_PAN_ID,x)==SETTINGS_STATUS_OK) {
          PRINTA("->Set EEPROM PAN ID to %04x.\n",x);
        }
    }
	return x;
}
static uint16_t get_panaddr_from_eeprom(void) {
    uint16_t x;
    if (settings_check(SETTINGS_KEY_PAN_ADDR,0)) {
        x = settings_get_uint16(SETTINGS_KEY_PAN_ADDR,0);
        PRINTD("<-Get EEPROM PAN address of %04x.\n",x);
    } else {
	    x=pgm_read_word_near(&default_panaddr);
        if (settings_add_uint16(SETTINGS_KEY_PAN_ADDR,x)==SETTINGS_STATUS_OK) {
          PRINTA("->Set EEPROM PAN address to %04x.\n",x);
        }
    }        
	return x;
}
static uint8_t get_txpower_from_eeprom(void) {
    uint8_t x;
    if (settings_check(SETTINGS_KEY_TXPOWER,0)) {
        x = settings_get_uint8(SETTINGS_KEY_TXPOWER,0);
        PRINTD("<-Get EEPROM tx power of %d. (0=max)\n",x);
    } else {
	    x=pgm_read_byte_near(&default_txpower);
        if (settings_add_uint8(SETTINGS_KEY_TXPOWER,x)==SETTINGS_STATUS_OK) {
          PRINTA("->Set EEPROM tx power of %d. (0=max)\n",x);
        }
    }
	return x;
}
#endif /* !JACKDAW_CONF_USE_SETTINGS */

/*-------------------------------------------------------------------------*/
/*-----------------------------Low level initialization--------------------*/
static void initialize(void) {

  watchdog_init();
  watchdog_start();

#if CONFIG_STACK_MONITOR
  /* Simple stack pointer highwater monitor. The 'm' command in cdc_task.c
   * looks for the first overwritten magic number.
   */
{
extern uint16_t __bss_end;
uint16_t p=(uint16_t)&__bss_end;
    do {
      *(uint16_t *)p = 0x4242;
      p+=100;
    } while (p<SP-100); //don't overwrite our own stack
}
#endif

  /* Initialize hardware */
  // Checks for "finger", jumps to DFU if present.
  init_lowlevel();
  
  /* Clock */
  clock_init();

  /* Leds are referred to by number to prevent any possible confusion :) */
  /* Led0 Blue Led1 Red Led2 Green Led3 Yellow */
  Leds_init();
  Led1_on();

/* Get a random (or probably different) seed for the 802.15.4 packet sequence number.
 * Some layers will ignore duplicates found in a history (e.g. Contikimac)
 * causing the initial packets to be ignored after a short-cycle restart.
 */
  ADMUX =0x1E;              //Select AREF as reference, measure 1.1 volt bandgap reference.
  ADCSRA=1<<ADEN;           //Enable ADC, not free running, interrupt disabled, fastest clock
  ADCSRA|=1<<ADSC;          //Start conversion
  while (ADCSRA&(1<<ADSC)); //Wait till done
  PRINTD("ADC=%d\n",ADC);
  random_init(ADC);
  ADCSRA=0;                 //Disable ADC
  
#if USB_CONF_RS232
  /* Use rs232 port for serial out (tx, rx, gnd are the three pads behind jackdaw leds */
  rs232_init(RS232_PORT_0, USART_BAUD_57600,USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);
  /* Redirect stdout to second port */
  rs232_redirect_stdout(RS232_PORT_0);
#if ANNOUNCE
  PRINTA("\n\n*******Booting %s*******\n",CONTIKI_VERSION_STRING);
#endif
#endif
	
  /* rtimer init needed for low power protocols */
  rtimer_init();

  /* Process subsystem. */
  process_init();

  /* etimer process must be started before USB or ctimer init */
  process_start(&etimer_process, NULL);

  Led2_on();
  /* Now we can start USB enumeration */
  process_start(&usb_process, NULL);

  /* Start CDC enumeration, bearing in mind that it may fail */
  /* Hopefully we'll get a stdout for startup messages, if we don't already */
#if USB_CONF_SERIAL
  process_start(&cdc_process, NULL);
{unsigned short i;
  for (i=0;i<65535;i++) {
    process_run();
    watchdog_periodic();
    if (stdout) break;
  }
#if !USB_CONF_RS232
  PRINTA("\n\n*******Booting %s*******\n",CONTIKI_VERSION_STRING);
#endif
}
#endif
  if (!stdout) Led3_on();
  
#if RF230BB
#if JACKDAW_CONF_USE_SETTINGS
  PRINTA("Settings manager will be used.\n");
#else
{uint8_t x[2];
	*(uint16_t *)x = eeprom_read_word((uint16_t *)&eemem_channel);
	if((uint8_t)x[0]!=(uint8_t)~x[1]) {
        PRINTA("Invalid EEPROM settings detected. Rewriting with default values.\n");
        get_channel_from_eeprom();
    }
}
#endif

  ctimer_init();
  /* Start radio and radio receive process */
  /* Note this starts RF230 process, so must be done after process_init */
  NETSTACK_RADIO.init();

  /* Set addresses BEFORE starting tcpip process */

  memset(&tmp_addr, 0, sizeof(rimeaddr_t));

  if(get_eui64_from_eeprom(tmp_addr.u8));
   
  //Fix MAC address
  init_net();

#if UIP_CONF_IPV6
  memcpy(&uip_lladdr.addr, &tmp_addr.u8, 8);
#endif

  rf230_set_pan_addr(
	get_panid_from_eeprom(),
	get_panaddr_from_eeprom(),
	(uint8_t *)&tmp_addr.u8
  );
  
  rf230_set_channel(get_channel_from_eeprom());
  rf230_set_txpower(get_txpower_from_eeprom());

  rimeaddr_set_node_addr(&tmp_addr); 

  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

#if ANNOUNCE
  PRINTA("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n\r",tmp_addr.u8[0],tmp_addr.u8[1],tmp_addr.u8[2],tmp_addr.u8[3],tmp_addr.u8[4],tmp_addr.u8[5],tmp_addr.u8[6],tmp_addr.u8[7]);
  PRINTA("%s %s, channel %u",NETSTACK_MAC.name, NETSTACK_RDC.name,rf230_get_channel());
  if (NETSTACK_RDC.channel_check_interval) {
    unsigned short tmp;
    tmp=CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval == 0 ? 1:\
                        NETSTACK_RDC.channel_check_interval());
    if (tmp<65535) PRINTA(", check rate %u Hz",tmp);
  }
  PRINTA("\n");
#endif

#if UIP_CONF_IPV6_RPL
#if RPL_BORDER_ROUTER
  process_start(&tcpip_process, NULL);
  process_start(&border_router_process, NULL);
  PRINTD ("RPL Border Router Started\n");
#else
  process_start(&tcpip_process, NULL);
  PRINTD ("RPL Started\n");
#endif
#if RPL_HTTPD_SERVER
  extern struct process httpd_process;
  process_start(&httpd_process, NULL);
  PRINTD ("Webserver Started\n");
#endif
#endif /* UIP_CONF_IPV6_RPL */

#else  /* RF230BB */
/* The order of starting these is important! */
  process_start(&mac_process, NULL);
  process_start(&tcpip_process, NULL);
#endif /* RF230BB */

  /* Start ethernet network and storage process */
  process_start(&usb_eth_process, NULL);
#if USB_CONF_STORAGE
  process_start(&storage_process, NULL);
#endif

  /* Autostart other processes */
  /* There are none in the default build so autostart_processes will be unresolved in the link. */
  /* The AUTOSTART_PROCESSES macro which defines it can only be used in the .co module. */
  /* See /examples/ravenusbstick/ravenusb.c for an autostart template. */
#if 0
  autostart_start(autostart_processes);
#endif

#if ANNOUNCE
#if USB_CONF_RS232
  PRINTA("Online.\n");
#else
  PRINTA("Online. Type ? for Jackdaw menu.\n");
#endif
#endif

Leds_off();
}

/*-------------------------------------------------------------------------*/
/*---------------------------------Main Routine----------------------------*/
int
main(void)
{
  /* GCC depends on register r1 set to 0 (?) */
  asm volatile ("clr r1");
  
  /* Initialize in a subroutine to maximize stack space */
  initialize();

#if DEBUG
{struct process *p;
 for(p = PROCESS_LIST();p != NULL; p = ((struct process *)p->next)) {
  PRINTA("Process=%p Thread=%p  Name=\"%s\" \n",p,p->thread,PROCESS_NAME_STRING(p));
 }
}
#endif

  while(1) {
    process_run();

    watchdog_periodic();

/* Print rssi of all received packets, useful for range testing */
#ifdef RF230_MIN_RX_POWER
    uint8_t lastprint;
    if (rf230_last_rssi != lastprint) {        //can be set in halbb.c interrupt routine
        PRINTA("%u ",rf230_last_rssi);
        lastprint=rf230_last_rssi;
    }
#endif

#if 0
/* Clock.c can trigger a periodic PLL calibration in the RF230BB driver.
 * This can show when that happens.
 */
    extern uint8_t rf230_calibrated;
    if (rf230_calibrated) {
      PRINTA("\nRF230 calibrated!\n");
      rf230_calibrated=0;
    }
#endif

#if TESTRTIMER
/* Timeout can be increased up to 8 seconds maximum.
 * A one second cycle is convenient for triggering the various debug printouts.
 * The triggers are staggered to avoid printing everything at once.
 * My Jackdaw is 4% slow.
 */
    if (rtimerflag) {
      rtimer_set(&rt, RTIMER_NOW()+ RTIMER_ARCH_SECOND*1UL, 1,(void *) rtimercycle, NULL);
      rtimerflag=0;

#if STAMPS
if ((rtime%STAMPS)==0) {
  PRINTA("%us ",rtime);
  if (rtime%STAMPS*10) PRINTA("\n");
}
#endif
      rtime+=1;

#if PINGS && UIP_CONF_IPV6_RPL
extern void raven_ping6(void);
if ((rtime%PINGS)==1) {
  PRINTA("**Ping\n");
  raven_ping6();
}
#endif

#if ROUTES && UIP_CONF_IPV6_RPL
if ((rtime%ROUTES)==2) {
      
extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_route_t uip_ds6_routing_table[];
extern uip_ds6_netif_t uip_ds6_if;

  uint8_t i,j;
  PRINTA("\nAddresses [%u max]\n",UIP_DS6_ADDR_NB);
  for (i=0;i<UIP_DS6_ADDR_NB;i++) {
    if (uip_ds6_if.addr_list[i].isused) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTA("\n");
    }
  }
  PRINTA("\nNeighbors [%u max]\n",UIP_DS6_NBR_NB);
  for(i = 0,j=1; i < UIP_DS6_NBR_NB; i++) {
    if(uip_ds6_nbr_cache[i].isused) {
      uip_debug_ipaddr_print(&uip_ds6_nbr_cache[i].ipaddr);
      PRINTA("\n");
      j=0;
    }
  }
  if (j) PRINTA("  <none>");
  PRINTA("\nRoutes [%u max]\n",UIP_DS6_ROUTE_NB);
  for(i = 0,j=1; i < UIP_DS6_ROUTE_NB; i++) {
    if(uip_ds6_routing_table[i].isused) {
      uip_debug_ipaddr_print(&uip_ds6_routing_table[i].ipaddr);
      PRINTA("/%u (via ", uip_ds6_routing_table[i].length);
      uip_debug_ipaddr_print(&uip_ds6_routing_table[i].nexthop);
 //     if(uip_ds6_routing_table[i].state.lifetime < 600) {
        PRINTA(") %lus\n", uip_ds6_routing_table[i].state.lifetime);
 //     } else {
 //       PRINTA(")\n");
 //     }
      j=0;
    }
  }
  if (j) PRINTA("  <none>");
  PRINTA("\n---------\n");
}
#endif

#if STACKMONITOR && CONFIG_STACK_MONITOR
if ((rtime%STACKMONITOR)==3) {
  extern uint16_t __bss_end;
  uint16_t p=(uint16_t)&__bss_end;
  do {
    if (*(uint16_t *)p != 0x4242) {
      PRINTA("Never-used stack > %d bytes\n",p-(uint16_t)&__bss_end);
      break;
    }
    p+=100;
  } while (p<RAMEND-10);
}
#endif

    }
#endif /* TESTRTIMER */

//Use with RF230BB DEBUGFLOW to show path through driver
#if RF230BB&&0
extern uint8_t debugflowsize,debugflow[];  //in rf230bb.c
  if (debugflowsize) {
    debugflow[debugflowsize]=0;
    PRINTA("%s",debugflow);
    debugflowsize=0;
   }
#endif

  }
  return 0;
}
