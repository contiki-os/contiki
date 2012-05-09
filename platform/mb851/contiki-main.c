/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 * $Id: contiki-main.c,v 1.2 2010/10/27 14:05:24 salvopitru Exp $
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*			Contiki main file.
* \author
*			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
*			Chi-Anh La <la@imag.fr>
*/
/*---------------------------------------------------------------------------*/


#include PLATFORM_HEADER
#include "hal/error.h"
#include "hal/hal.h"
#include BOARD_HEADER
#include "micro/adc.h"

#include <stdio.h>


#include "contiki.h"

#include "dev/watchdog.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/temperature-sensor.h"
#include "dev/acc-sensor.h"
#include "dev/uart1.h"
#include "dev/serial-line.h"

#include "dev/stm32w-radio.h"
#include "net/netstack.h"
#include "net/rime/rimeaddr.h"
#include "net/rime.h"
#include "net/rime/rime-udp.h"
#include "net/uip.h"

#if WITH_UIP6
#include "net/uip-ds6.h"
#endif /* WITH_UIP6 */

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr.u8[0], lladdr.u8[1], lladdr.u8[2], lladdr.u8[3],lladdr.u8[4], lladdr.u8[5], lladdr.u8[6], lladdr.u8[7])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


#if UIP_CONF_IPV6
PROCINIT(&tcpip_process, &sensors_process);
#else
PROCINIT(&sensors_process);
#warning "No TCP/IP process!"
#endif

SENSORS(&button_sensor, &temperature_sensor, &acc_sensor);

/* The default CCA threshold is set to -77, which is the same as the
   default setting on the TI CC2420. */
#define DEFAULT_RADIO_CCA_THRESHOLD -77

/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  int i;
  union {
    uint8_t u8[8];
  } eui64;
  
  int8u *stm32w_eui64 = ST_RadioGetEui64();
  {
    int8u c;
    /* Copy the EUI-64 to lladdr converting from Little-Endian to
       Big-Endian. */
    for(c = 0; c < 8; c++) {
      eui64.u8[c] = stm32w_eui64[7 - c];
    }
  }

#if UIP_CONF_IPV6
  memcpy(&uip_lladdr.addr, &eui64, sizeof(uip_lladdr.addr));
#endif

#if UIP_CONF_IPV6
  rimeaddr_set_node_addr((rimeaddr_t *)&eui64);
#else
  rimeaddr_set_node_addr((rimeaddr_t *)&eui64.u8[8 - RIMEADDR_SIZE]);
#endif

  printf("Rime started with address ");
  for(i = 0; i < sizeof(rimeaddr_t) - 1; i++) {
    printf("%d.", rimeaddr_node_addr.u8[i]);
  }
  printf("%d\n", rimeaddr_node_addr.u8[i]);
}
/*---------------------------------------------------------------------------*/
int
main(void)
{
  
  /*
   * Initalize hardware.
   */
  halInit();
  clock_init();
  
  uart1_init(115200);
  
  /* Led initialization */
  leds_init();
    
  INTERRUPTS_ON(); 

  PRINTF("\r\nStarting ");
  PRINTF(CONTIKI_VERSION_STRING);
  PRINTF(" on MB851\r\n"); 

  /*
   * Initialize Contiki and our processes.
   */
  
  process_init();
  
#if WITH_SERIAL_LINE_INPUT
  uart1_set_input(serial_line_input_byte);
  serial_line_init();
#endif
  /* rtimer and ctimer should be initialized before radio duty cycling
     layers */
  rtimer_init();
  /* etimer_process should be initialized before ctimer */
  process_start(&etimer_process, NULL);  
  ctimer_init();

  netstack_init();

  set_rime_addr();

  printf("%s %s, channel check rate %lu Hz\n",
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
                                  NETSTACK_RDC.channel_check_interval()));
  printf("802.15.4 PAN ID 0x%x, EUI-%d:",
      IEEE802154_CONF_PANID, UIP_CONF_LL_802154?64:16);
  uip_debug_lladdr_print(&rimeaddr_node_addr);
  printf(", radio channel %u\n", RF_CHANNEL);

  procinit_init();

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  /* Set the Clear Channel Assessment (CCA) threshold of the
     radio. The CCA threshold is used both for sending packets and for
     waking up ContikiMAC nodes. If the CCA threshold is too high,
     ContikiMAC will not wake up from neighbor transmissions. If the
     CCA threshold is too low, transmissions will be too restrictive
     and no packets will be sent. DEFAULT_RADIO_CCA_THRESHOLD is
     defined in this file. */
  ST_RadioSetEdCcaThreshold(DEFAULT_RADIO_CCA_THRESHOLD);
  
  autostart_start(autostart_processes);
#if UIP_CONF_IPV6
  printf("Tentative link-local IPv6 address ");
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 7; ++i) {
      printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
             lladdr->ipaddr.u8[i * 2 + 1]);
    }
    printf("%02x%02x\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
  }


  if(!UIP_CONF_IPV6_RPL) {
    uip_ipaddr_t ipaddr;
    int i;
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
    printf("Tentative global IPv6 address ");
    for(i = 0; i < 7; ++i) {
      printf("%02x%02x:",
             ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
    }
    printf("%02x%02x\n",
           ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
  }
#endif /* UIP_CONF_IPV6 */
  
  watchdog_start();
  
  while(1) {
    
    int r;    
    
    do {
      /* Reset watchdog. */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);
    
    
    
    ENERGEST_OFF(ENERGEST_TYPE_CPU);
    /* watchdog_stop(); */
    ENERGEST_ON(ENERGEST_TYPE_LPM);
    /* Go to idle mode. */
    halSleepWithOptions(SLEEPMODE_IDLE,0);
    /* We are awake. */
    /* watchdog_start(); */
    ENERGEST_OFF(ENERGEST_TYPE_LPM);
    ENERGEST_ON(ENERGEST_TYPE_CPU);  
    
  }
  
}



/*int8u errcode __attribute__(( section(".noinit") ));

void halBaseBandIsr(){
  
  errcode = 1;
  leds_on(LEDS_RED);
}

void BusFault_Handler(){
  
  errcode = 2; 
  leds_on(LEDS_RED);
}

void halDebugIsr(){
  
  errcode = 3;
  leds_on(LEDS_RED);  
}

void DebugMon_Handler(){
  
  errcode = 4;
  //leds_on(LEDS_RED);  
}

void HardFault_Handler(){
  
  errcode = 5; 
  //leds_on(LEDS_RED);
  //halReboot();
}

void MemManage_Handler(){
  
  errcode = 6; 
  //leds_on(LEDS_RED);
  //halReboot();
}

void UsageFault_Handler(){
  
  errcode = 7; 
  //leds_on(LEDS_RED);
  //halReboot();
}

void Default_Handler() 
{ 
  //errcode = 8; 
  leds_on(LEDS_RED);
  halReboot();
}*/
