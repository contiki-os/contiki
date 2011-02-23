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
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
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

/* Test rtimers, also useful for pings and time stamps in simulator */
#define TESTRTIMER 0
#if TESTRTIMER
#define PINGS 0
#define STAMPS 30
uint8_t rtimerflag=1;
uint16_t rtime;
struct rtimer rt;
void rtimercycle(void) {rtimerflag=1;}
#endif /* TESTRTIMER */

#if UIP_CONF_IPV6_RPL
/*---------------------------------------------------------------------------*/
/*---------------------------------  RPL   ----------------------------------*/
/*---------------------------------------------------------------------------*/
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
    /* Do not bounce packets back over SLIP if the packet was received
       over SLIP */
//    PRINTF("slip-bridge: Destination off-link but no route\n");
 // } else {
    PRINTF("SUT: %u\n", uip_len);
    mac_LowpanToEthernet();
//  }
}
const struct uip_fallback_interface rpl_interface = {
  init, output
};

#if RPL_BORDER_ROUTER
#include "net/rpl/rpl.h"

// avr-objdump --section .bss -x ravenusbstick.elf
uint16_t dag_id[] PROGMEM = {0x1111, 0x1100, 0, 0, 0, 0, 0, 0x0011};

PROCESS(border_router_process, "RPL Border Router");
PROCESS_THREAD(border_router_process, ev, data)
{

  PROCESS_BEGIN();

  PROCESS_PAUSE();

{ rpl_dag_t *dag;
  char buf[sizeof(dag_id)];
  memcpy_P(buf,dag_id,sizeof(dag_id));
  dag = rpl_set_root((uip_ip6addr_t *)buf);

/* Assign bbbb::200 to the uip stack, and bbbb::1 to the host network interface, e.g. $ip -6 address add bbbb::1/64 dev usb0 */
/* Note the jackdaw uip stack will get packets intended for usb if they have the same address! */
/* $ifconfig usb0 -arp on Ubuntu to skip the neighbor solicitations. Add explicit neighbors on other OSs */
  if(dag != NULL) {
    PRINTF("created a new RPL dag\n");

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
 //   rpl_set_prefix(rpl_get_dag(RPL_ANY_INSTANCE), &ipaddr, 64);
 //   rpl_repair_dag(rpl_get_dag(RPL_ANY_INSTANCE));

  }

  PROCESS_END();
}
#endif /* RPL_BORDER_ROUTER */

#endif /* UIP_CONF_IPV6_RPL */

/*-------------------------------------------------------------------------*/
/*----------------------Configuration of the .elf file---------------------*/
typedef struct {unsigned char B2;unsigned char B1;unsigned char B0;} __signature_t;
#define SIGNATURE __signature_t __signature __attribute__((section (".signature")))
SIGNATURE = {
/* Older AVR-GCCs may not define the SIGNATURE_n bytes so use explicit values */
  .B2 = 0x82,//SIGNATURE_2, //AT90USB128x
  .B1 = 0x97,//SIGNATURE_1, //128KB flash
  .B0 = 0x1E,//SIGNATURE_0, //Atmel
};
FUSES ={.low = 0xde, .high = 0x99, .extended = 0xff,};

/* Put default MAC address in EEPROM */
#if !JACKDAW_CONF_USE_SETTINGS
uint8_t mac_address[8] EEMEM = {0x02, 0x12, 0x13, 0xff, 0xfe, 0x14, 0x15, 0x16};
#endif

static uint8_t get_channel_from_eeprom() {
#if JACKDAW_CONF_USE_SETTINGS
	uint8_t chan = settings_get_uint8(SETTINGS_KEY_CHANNEL, 0);
	if(!chan)
		chan = RF_CHANNEL;
	return chan;
#else
	uint8_t eeprom_channel;
	uint8_t eeprom_check;

	eeprom_channel = eeprom_read_byte((uint8_t *)9);
	eeprom_check = eeprom_read_byte((uint8_t *)10);

	if(eeprom_channel==~eeprom_check)
		return eeprom_channel;

#ifdef CHANNEL_802_15_4
	return(CHANNEL_802_15_4);
#else
	return 26;
#endif		

#endif
	
}

static bool
get_eui64_from_eeprom(uint8_t macptr[8]) {
#if JACKDAW_CONF_USE_SETTINGS
	size_t size = 8;

	if(settings_get(SETTINGS_KEY_EUI64, 0, (unsigned char*)macptr, &size)==SETTINGS_STATUS_OK)
		return true;
		
	// Fallback to reading the traditional mac address
	eeprom_read_block ((void *)macptr,  0, 8);
#else
	eeprom_read_block ((void *)macptr,  &mac_address, 8);
#endif
	return macptr[0]!=0xFF;
}

#if JACKDAW_CONF_RANDOM_MAC
static bool
set_eui64_to_eeprom(const uint8_t macptr[8]) {
#if JACKDAW_CONF_USE_SETTINGS
	return settings_set(SETTINGS_KEY_EUI64, macptr, 8)==SETTINGS_STATUS_OK;
#else
	eeprom_write_block((void *)macptr,  &mac_address, 8);
	return true;
#endif
}

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

static uint16_t
get_panid_from_eeprom(void) {
#if JACKDAW_CONF_USE_SETTINGS
	uint16_t x = settings_get_uint16(SETTINGS_KEY_PAN_ID, 0);
	if(!x)
		x = IEEE802154_PANID;
	return x;
#else
	// TODO: Writeme!
	return IEEE802154_PANID;
#endif
}

static uint16_t
get_panaddr_from_eeprom(void) {
#if JACKDAW_CONF_USE_SETTINGS
	return settings_get_uint16(SETTINGS_KEY_PAN_ADDR, 0);
#else
	// TODO: Writeme!
	return 0;
#endif
}


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
    } while (p<RAMEND-100);
}
#endif

  /* Initialize hardware */
  // Checks for "finger", jumps to DFU if present.
  init_lowlevel();
  
  /* Clock */
  clock_init();

#if USB_CONF_RS232
  /* Use rs232 port for serial out (tx, rx, gnd are the three pads behind jackdaw leds */
  rs232_init(RS232_PORT_0, USART_BAUD_57600,USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);
  /* Redirect stdout to second port */
  rs232_redirect_stdout(RS232_PORT_0);
#if ANNOUNCE
  printf_P(PSTR("\n\n\n********BOOTING CONTIKI*********\n"));
#endif
#endif
	
	Leds_init();
	
  /* rtimer init needed for low power protocols */
  rtimer_init();

  /* Process subsystem. */
  process_init();

  /* etimer process must be started before ctimer init */
  process_start(&etimer_process, NULL);
  
#if RF230BB
  ctimer_init();
  /* Start radio and radio receive process */
  /* Note this starts RF230 process, so must be done after process_init */
  NETSTACK_RADIO.init();

  /* Set addresses BEFORE starting tcpip process */

  memset(&tmp_addr, 0, sizeof(rimeaddr_t));
  if(!get_eui64_from_eeprom(tmp_addr.u8)) {
#if JACKDAW_CONF_RANDOM_MAC
    // It doesn't look like we have a valid EUI-64 address
	// so let's try to make a new one from scratch.
    Leds_off();
    Led2_on();
    generate_new_eui64(tmp_addr.u8);
	if(!set_eui64_to_eeprom(tmp_addr.u8)) {
		watchdog_periodic();
		int i;
		for(i=0;i<20;i++) {
			Led1_toggle();
			_delay_ms(100);
		}
		Led1_off();
	}
	Led2_off();
#else
	tmp_addr.u8[0]=0x02;
	tmp_addr.u8[1]=0x12;
	tmp_addr.u8[2]=0x13;
	tmp_addr.u8[3]=0xff;
	tmp_addr.u8[4]=0xfe;
	tmp_addr.u8[5]=0x14;
	tmp_addr.u8[6]=0x15;
	tmp_addr.u8[7]=0x16;
#endif /* JACKDAW_CONF_RANDOM_MAC */
  }
  

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
  
#if JACKDAW_CONF_USE_SETTINGS
/* Allow radio code to overrite power for testing miniature Raven mesh */
#ifndef RF230_MAX_TX_POWER
   rf230_set_txpower(settings_get_uint8(SETTINGS_KEY_TXPOWER,0));
#endif
#endif

  rimeaddr_set_node_addr(&tmp_addr); 

  /* Initialize stack protocols */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  rf230_set_channel(get_channel_from_eeprom());

#if ANNOUNCE && USB_CONF_RS232
  printf_P(PSTR("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n\r"),tmp_addr.u8[0],tmp_addr.u8[1],tmp_addr.u8[2],tmp_addr.u8[3],tmp_addr.u8[4],tmp_addr.u8[5],tmp_addr.u8[6],tmp_addr.u8[7]);
  printf_P(PSTR("%s %s, channel %u"),NETSTACK_MAC.name, NETSTACK_RDC.name,rf230_get_channel());
  if (NETSTACK_RDC.channel_check_interval) {
    unsigned short tmp;
    tmp=CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval == 0 ? 1:\
                        NETSTACK_RDC.channel_check_interval());
    if (tmp<65535) printf_P(PSTR(", check rate %u Hz"),tmp);
  }
  printf_P(PSTR("\n"));
#endif

#if UIP_CONF_IPV6_RPL
#if RPL_BORDER_ROUTER
  process_start(&tcpip_process, NULL);
  process_start(&border_router_process, NULL);
  PRINTF ("RPL Border Router Started\n");
#else
  process_start(&tcpip_process, NULL);
  PRINTF ("RPL Started\n");
#endif
#if RPL_HTTPD_SERVER
  extern struct process httpd_process;
  process_start(&httpd_process, NULL);
  PRINTF ("Webserver Started\n");
#endif
#endif /* UIP_CONF_IPV6_RPL */

#else  /* RF230BB */
/* The order of starting these is important! */
  process_start(&mac_process, NULL);
  process_start(&tcpip_process, NULL);
#endif /* RF230BB */

  /* Setup USB */
  process_start(&usb_process, NULL);
#if USB_CONF_SERIAL
  process_start(&cdc_process, NULL);
#endif
  process_start(&usb_eth_process, NULL);
#if USB_CONF_STORAGE
  process_start(&storage_process, NULL);
#endif
  
#if ANNOUNCE
#if USB_CONF_SERIAL&&!USB_CONF_RS232
{unsigned short i;
   printf_P(PSTR("\n\n\n********BOOTING CONTIKI*********\n\r"));
  /* Allow USB CDC to keep up with printfs */
  for (i=0;i<8000;i++) process_run();
#if RF230BB
  printf_P(PSTR("MAC address %x:%x:%x:%x:%x:%x:%x:%x\n\r"),tmp_addr.u8[0],tmp_addr.u8[1],tmp_addr.u8[2],tmp_addr.u8[3],tmp_addr.u8[4],tmp_addr.u8[5],tmp_addr.u8[6],tmp_addr.u8[7]);
  for (i=0;i<8000;i++) process_run();
  printf_P(PSTR("%s %s, channel %u"),NETSTACK_MAC.name, NETSTACK_RDC.name,rf230_get_channel());
  if (NETSTACK_RDC.channel_check_interval) {
    i=CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval == 0 ? 1:\
                      NETSTACK_RDC.channel_check_interval());
    if (i<65535) printf_P(PSTR(", check rate %u Hz"),i);
   }
   printf_P(PSTR("\n\r"));
   for (i=0;i<8000;i++) process_run();
#endif /* RF230BB */
  printf_P(PSTR("System online.\n\r"));
}
#elif USB_CONF_RS232
  printf_P(PSTR("System online.\n"));
#endif
#endif /* ANNOUNCE */
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
  printf_P(PSTR("Process=%p Thread=%p  Name=\"%s\" \n"),p,p->thread,p->name);
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
        printf_P(PSTR("%u "),rf230_last_rssi);
        lastprint=rf230_last_rssi;
    }
#endif

#if TESTRTIMER
    if (rtimerflag) {  //8 seconds is maximum interval, my jackdaw 4% slow
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
        pingsomebody();
      }
#endif
    }
#endif /* TESTRTIMER */

/* Use with rf230bb.c DEBUGFLOW to show the sequence of driver calls from the uip stack */
#if RF230BB&&0
extern uint8_t debugflowsize,debugflow[];
  if (debugflowsize) {
    debugflow[debugflowsize]=0;
    printf("%s",debugflow);
    debugflowsize=0;
   }
#endif

/* Use for low level interrupt debugging */
#if RF230BB&&0
extern uint8_t rf230interruptflag;   //in halbb.c
extern uint8_t rf230processflag;     //in rf230bb.c
  if (rf230processflag) {
    printf("**RF230 process flag %u\n\r",rf230processflag);
    rf230processflag=0;
  }
  if (rf230interruptflag) {
//  if (rf230interruptflag!=11) {
      printf("**RF230 Interrupt %u\n\r",rf230interruptflag);
 // }
    rf230interruptflag=0;
  }
#endif
  }

  return 0;
}

