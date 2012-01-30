/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org) and Contiki.
 *
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

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "contiki.h"

#include "dev/leds.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/xmem.h"
#include "dev/button-sensor.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "lib/include/mc1322x.h"

#if WITH_UIP6
#include "net/sicslowpan.h"
#include "net/uip-ds6.h"
#include "net/mac/sicslowmac.h"
#endif /* WITH_UIP6 */

#include "net/rime.h"

#include "sys/autostart.h"
#include "sys/profile.h"

/* from libmc1322x */
#include "mc1322x.h"
#include "default_lowlevel.h"
#include "contiki-maca.h"
#include "contiki-uart.h"

/* Get periodic prints from idle loop, from clock seconds or rtimer interrupts */
/* Use of rtimer will conflict with other rtimer interrupts such as contikimac radio cycling */
#define PERIODICPRINTS 0
#if PERIODICPRINTS
//#define PINGS 64
#define ROUTES 300
#define STAMPS 60
#define STACKMONITOR 600
//#define HEAPMONITOR 60
uint16_t clocktime;
#define TESTRTIMER 0
#if TESTRTIMER
uint8_t rtimerflag=1;
struct rtimer rt;
void rtimercycle(void) {rtimerflag=1;}
#endif
#endif

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

#ifndef RIMEADDR_NVM
#define RIMEADDR_NVM 0x1E000
#endif

#ifndef RIMEADDR_NBYTES
#define RIMEADDR_NBYTES 8
#endif

#if UIP_CONF_ROUTER

#ifndef UIP_ROUTER_MODULE
#ifdef UIP_CONF_ROUTER_MODULE
#define UIP_ROUTER_MODULE UIP_CONF_ROUTER_MODULE
#else /* UIP_CONF_ROUTER_MODULE */
#define UIP_ROUTER_MODULE rimeroute
#endif /* UIP_CONF_ROUTER_MODULE */
#endif /* UIP_ROUTER_MODULE */

extern const struct uip_router UIP_ROUTER_MODULE;

#endif /* UIP_CONF_ROUTER */

#if DCOSYNCH_CONF_ENABLED
static struct timer mgt_timer;
#endif

#ifndef WITH_UIP
#define WITH_UIP 0
#endif

#if WITH_UIP
#include "net/uip.h"
#include "net/uip-fw.h"
#include "net/uip-fw-drv.h"
#include "net/uip-over-mesh.h"
static struct uip_fw_netif slipif =
  {UIP_FW_NETIF(192,168,1,2, 255,255,255,255, slip_send)};
static struct uip_fw_netif meshif =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, uip_over_mesh_send)};

#endif /* WITH_UIP */

#define UIP_OVER_MESH_CHANNEL 8
#if WITH_UIP
static uint8_t is_gateway;
#endif /* WITH_UIP */

/*---------------------------------------------------------------------------*/
void uip_log(char *msg) { printf("%c",msg); }
/*---------------------------------------------------------------------------*/
#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif
/*---------------------------------------------------------------------------*/
#if WITH_UIP
static void
set_gateway(void)
{
  if(!is_gateway) {
    leds_on(LEDS_RED);
    printf("%d.%d: making myself the IP network gateway.\n\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    printf("IPv4 address of the gateway: %d.%d.%d.%d\n\n",
	   uip_ipaddr_to_quad(&uip_hostaddr));
    uip_over_mesh_set_gateway(&rimeaddr_node_addr);
    uip_over_mesh_make_announced_gateway();
    is_gateway = 1;
  }
}
#endif /* WITH_UIP */
/*---------------------------------------------------------------------------*/
static void
print_processes(struct process * const processes[])
{
  /*  const struct process * const * p = processes;*/
  printf("Starting");
  while(*processes != NULL) {
    printf(" '%s'", (*processes)->name);
    processes++;
  }
  printf("\n");
}
/*--------------------------------------------------------------------------*/

SENSORS(&button_sensor);

void
init_lowlevel(void)
{
	/* button init */
	/* set up kbi */
	enable_irq_kbi(4);
	kbi_edge(4);
	enable_ext_wu(4);
//	kbi_pol_neg(7);
//	kbi_pol_pos(7);
//	gpio_sel0_pullup(29);
//	gpio_pu0_disable(29);

	trim_xtal();
	
	/* uart init */
	uart_init(BRINC, BRMOD, SAMP);
	
	default_vreg_init();

	maca_init();

	set_channel(RF_CHANNEL - 11); /* channel 11 */
	set_power(0x12); /* 0x12 is the highest, not documented */

	enable_irq(CRM);

#if USE_32KHZ_XTAL
	enable_32khz_xtal();
#else
	cal_ring_osc();
#endif

#if USE_32KHZ_XTAL
	*CRM_RTC_TIMEOUT = 32768 * 10; 
#else 
	*CRM_RTC_TIMEOUT = cal_rtc_secs * 10;
#endif

#if (USE_WDT == 1)
	/* set the watchdog timer timeout to 1 sec */
	cop_timeout_ms(WDT_TIMEOUT);
	/* enable the watchdog timer */
	CRM->COP_CNTLbits.COP_EN = 1;
#endif

	/* XXX debug */
	/* trigger periodic rtc int */
//	clear_rtc_wu_evt();
//	enable_rtc_wu();
//	enable_rtc_wu_irq();
}

#if RIMEADDR_SIZE == 1
const rimeaddr_t addr_ff = { { 0xff } };
#else /*RIMEADDR_SIZE == 2*/
#if RIMEADDR_SIZE == 2
const rimeaddr_t addr_ff = { { 0xff, 0xff } };
#else /*RIMEADDR_SIZE == 2*/
#if RIMEADDR_SIZE == 8
const rimeaddr_t addr_ff = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
#endif /*RIMEADDR_SIZE == 8*/
#endif /*RIMEADDR_SIZE == 2*/
#endif /*RIMEADDR_SIZE == 1*/

void iab_to_eui64(rimeaddr_t *eui64, uint32_t oui, uint16_t iab, uint32_t ext) {
	/* OUI for IABs */
	eui64->u8[0] =  0x00;
	eui64->u8[1] =  0x50;
	eui64->u8[2] =  0xc2;

	/* IAB */
	eui64->u8[3] = (iab >> 4)  & 0xff;
	eui64->u8[4] = (iab << 4) &  0xf0;

	/* EXT */

	eui64->u8[4] |= (ext >> 24) & 0xf;
	eui64->u8[5] = (ext >> 16) & 0xff;
	eui64->u8[6] = (ext >> 8)  & 0xff;
	eui64->u8[7] =  ext        & 0xff;
}

void oui_to_eui64(rimeaddr_t *eui64, uint32_t oui, uint64_t ext) {
	/* OUI */
	eui64->u8[0] = (oui >> 16) & 0xff;
	eui64->u8[1] = (oui >> 8)  & 0xff;
	eui64->u8[2] =  oui        & 0xff;

	/* EXT */
	eui64->u8[3] = (ext >> 32) & 0xff;
	eui64->u8[4] = (ext >> 24) & 0xff;
	eui64->u8[5] = (ext >> 16) & 0xff;
	eui64->u8[6] = (ext >> 8)  & 0xff;
	eui64->u8[7] =  ext        & 0xff;
}

unsigned short node_id = 0;

void
set_rimeaddr(rimeaddr_t *addr) 
{
	nvmType_t type=0;
	nvmErr_t err;	
	volatile uint8_t buf[RIMEADDR_NBYTES];
	rimeaddr_t eui64;
	int i;
		
	err = nvm_detect(gNvmInternalInterface_c, &type);

	err = nvm_read(gNvmInternalInterface_c, type, (uint8_t *)buf, RIMEADDR_NVM, RIMEADDR_NBYTES);

	rimeaddr_copy(addr,&rimeaddr_null);

	for(i=0; i<RIMEADDR_CONF_SIZE; i++) {		
		addr->u8[i] = buf[i];
	}

	if (memcmp(addr, &addr_ff, RIMEADDR_CONF_SIZE)==0) {

		//set addr to EUI64
#ifdef IAB		
   #ifdef EXT_ID
		PRINTF("address in flash blank, setting to defined IAB and extension.\n\r");
	  	iab_to_eui64(&eui64, OUI, IAB, EXT_ID);
   #else  /* ifdef EXT_ID */
		PRINTF("address in flash blank, setting to defined IAB with a random extension.\n\r");
		iab_to_eui64(&eui64, OUI, IAB, *MACA_RANDOM);
   #endif /* ifdef EXT_ID */

#else  /* ifdef IAB */

   #ifdef EXT_ID
		PRINTF("address in flash blank, setting to defined OUI and extension.\n\r");
		oui_to_eui64(&eui64, OUI, EXT_ID);
   #else  /*ifdef EXT_ID */
		PRINTF("address in flash blank, setting to defined OUI with a random extension.\n\r");
		oui_to_eui64(&eui64, OUI, ((*MACA_RANDOM << 32) | *MACA_RANDOM));
   #endif /*endif EXTID */

#endif /* ifdef IAB */

		rimeaddr_copy(addr, &eui64);		
#ifdef FLASH_BLANK_ADDR
		PRINTF("flashing blank address\n\r");
		err = nvm_write(gNvmInternalInterface_c, type, &(eui64.u8), RIMEADDR_NVM, RIMEADDR_NBYTES);		
#endif /* ifdef FLASH_BLANK_ADDR */
	} else {
		PRINTF("loading rime address from flash.\n\r");
	}

	node_id = (addr->u8[6] << 8 | addr->u8[7]);
	rimeaddr_set_node_addr(addr);
}

int
main(void)
{
	volatile uint32_t i;
	rimeaddr_t addr;

	/* Initialize hardware and */
	/* go into user mode */
	init_lowlevel();

#if STACKMONITOR
  /* Simple stack pointer highwater monitor. Checks for magic numbers in the main
   * loop. In conjuction with PERIODICPRINTS, never-used stack will be printed
   * every STACKMONITOR seconds.
   */
{
extern uint32_t __und_stack_top__, __sys_stack_top__;
uint32_t p=(uint32_t)&__und_stack_top__;
    do {
      *(uint32_t *)p = 0x42424242;
      p+=16;
    } while (p<(uint32_t)&__sys_stack_top__-100); //don't overwrite our own stack
}
#endif
#if HEAPMONITOR
  /* Simple heap pointer highwater monitor. Checks for magic numbers in the main
   * loop. In conjuction with PERIODICPRINTS, never-used heap will be printed
   * every HEAPMONITOR seconds.
   * This routine assumes a linear FIFO heap as used by the printf _sbrk call.
   */
{
extern uint32_t __heap_start__, __heap_end__;
uint32_t p=(uint32_t)&__heap_end__-4;
  do {
     *(uint32_t *)p = 0x42424242;
	 p-=4;
  } while (p>=(uint32_t)&__heap_start__);
}
#endif

	/* Clock */
	clock_init();	

	/* LED driver */
	leds_init();

	/* control TX_ON with the radio */
	GPIO->FUNC_SEL.GPIO_44 = 2;
	GPIO->PAD_DIR.GPIO_44 = 1;

	/* Process subsystem */
	process_init();
	process_start(&etimer_process, NULL);
	process_start(&contiki_maca_process, NULL);

	ctimer_init();

	set_rimeaddr(&addr);

	printf("Rime started with address ");
	for(i = 0; i < sizeof(addr.u8) - 1; i++) {
		printf("%02X:", addr.u8[i]);
	}
	printf("%02X\n", addr.u8[i]);


#if WITH_UIP6
  memcpy(&uip_lladdr.addr, &addr.u8, sizeof(uip_lladdr.addr));
  /* Setup nullmac-like MAC for 802.15.4 */
/*   sicslowpan_init(sicslowmac_init(&cc2420_driver)); */
/*   printf(" %s channel %u\n", sicslowmac_driver.name, RF_CHANNEL); */

  /* Setup X-MAC for 802.15.4 */
  queuebuf_init();
  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  printf("%s %s, channel check rate %lu Hz, radio channel %u\n",
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
                         NETSTACK_RDC.channel_check_interval()),
         RF_CHANNEL);

  process_start(&tcpip_process, NULL);

  printf("Tentative link-local IPv6 address ");
  {
    int i, a;
    for(a = 0; a < UIP_DS6_ADDR_NB; a++) {
      if (uip_ds6_if.addr_list[a].isused) {
	for(i = 0; i < 7; ++i) {
	  printf("%02x%02x:",
		 uip_ds6_if.addr_list[a].ipaddr.u8[i * 2],
		 uip_ds6_if.addr_list[a].ipaddr.u8[i * 2 + 1]);
	}
	printf("%02x%02x\n",
	       uip_ds6_if.addr_list[a].ipaddr.u8[14],
	       uip_ds6_if.addr_list[a].ipaddr.u8[15]);
      }
    }
  }
  
  if(1) {
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

  
#else /* WITH_UIP6 */

  NETSTACK_RDC.init();
  NETSTACK_MAC.init();
  NETSTACK_NETWORK.init();

  printf("%s %s, channel check rate %lu Hz, radio channel %u\n",
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0? 1:
                         NETSTACK_RDC.channel_check_interval()),
         RF_CHANNEL);
#endif /* WITH_UIP6 */

  *MACA_MACPANID = 0xcdab; /* this is the hardcoded contiki pan, register is PACKET order */
  *MACA_MAC16ADDR = 0xffff; /* short addressing isn't used, set this to 0xffff for now */

  *MACA_MAC64HI =
	  addr.u8[0] << 24 |
	  addr.u8[1] << 16 |
	  addr.u8[2] << 8 |
	  addr.u8[3];
  *MACA_MAC64LO =
	  addr.u8[4] << 24 |
	  addr.u8[5] << 16 |
	  addr.u8[6] << 8 |
	  addr.u8[7];
  PRINTF("setting panid 0x%04x\n\r", *MACA_MACPANID);
  PRINTF("setting short mac 0x%04x\n\r", *MACA_MAC16ADDR);
  PRINTF("setting long mac 0x%08x_%08x\n\r", *MACA_MAC64HI, *MACA_MAC64LO);

#if MACA_AUTOACK
  set_prm_mode(AUTOACK);
#endif

#if PROFILE_CONF_ON
  profile_init();
#endif /* PROFILE_CONF_ON */

#if TIMESYNCH_CONF_ENABLED
  timesynch_init();
  timesynch_set_authority_level(rimeaddr_node_addr.u8[0]);
#endif /* TIMESYNCH_CONF_ENABLED */

#if WITH_UIP
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL);	/* Start IP output */
  process_start(&slip_process, NULL);

  slip_set_input_callback(set_gateway);

  {
    uip_ipaddr_t hostaddr, netmask;

    uip_init();

    uip_ipaddr(&hostaddr, 172,16,
	       rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
    uip_ipaddr(&netmask, 255,255,0,0);
    uip_ipaddr_copy(&meshif.ipaddr, &hostaddr);

    uip_sethostaddr(&hostaddr);
    uip_setnetmask(&netmask);
    uip_over_mesh_set_net(&hostaddr, &netmask);
    /*    uip_fw_register(&slipif);*/
    uip_over_mesh_set_gateway_netif(&slipif);
    uip_fw_default(&meshif);
    uip_over_mesh_init(UIP_OVER_MESH_CHANNEL);
    printf("uIP started with IP address %d.%d.%d.%d\n",
	   uip_ipaddr_to_quad(&hostaddr));
  }
#endif /* WITH_UIP */

  process_start(&sensors_process, NULL);
  
  print_processes(autostart_processes);
  autostart_start(autostart_processes);
 
  /* Main scheduler loop */
  while(1) {
	  check_maca();

#if (USE_WDT == 1)
	  cop_service();
#endif

	  if(uart1_input_handler != NULL) {
		  if(uart1_can_get()) {
			  uart1_input_handler(uart1_getc());
		  }
	  }
	         
	  process_run();

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
  printf("\r%u(%u)s ",clocktime,radioontime);
#else
  printf("%us\n",clocktime);
#endif

}
#endif
#if TESTRTIMER
      clocktime+=1;
#endif

#if PINGS && UIP_CONF_IPV6
extern void raven_ping6(void); 
if ((clocktime%PINGS)==1) {
  printf("**Ping\n");
  raven_ping6();
}
#endif

#if ROUTES && UIP_CONF_IPV6
if ((clocktime%ROUTES)==2) {
      
extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_route_t uip_ds6_routing_table[];
extern uip_ds6_netif_t uip_ds6_if;

  uint8_t i,j;
  printf("\nAddresses [%u max]\n",UIP_DS6_ADDR_NB);
  for (i=0;i<UIP_DS6_ADDR_NB;i++) {
    if (uip_ds6_if.addr_list[i].isused) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }
  printf("\nNeighbors [%u max]\n",UIP_DS6_NBR_NB);
  for(i = 0,j=1; i < UIP_DS6_NBR_NB; i++) {
    if(uip_ds6_nbr_cache[i].isused) {
      uip_debug_ipaddr_print(&uip_ds6_nbr_cache[i].ipaddr);
      printf("\n");
      j=0;
    }
  }
  if (j) printf("  <none>");
  printf("\nRoutes [%u max]\n",UIP_DS6_ROUTE_NB);
  for(i = 0,j=1; i < UIP_DS6_ROUTE_NB; i++) {
    if(uip_ds6_routing_table[i].isused) {
      uip_debug_ipaddr_print(&uip_ds6_routing_table[i].ipaddr);
      printf("/%u (via ", uip_ds6_routing_table[i].length);
      uip_debug_ipaddr_print(&uip_ds6_routing_table[i].nexthop);
 //     if(uip_ds6_routing_table[i].state.lifetime < 600) {
        printf(") %lus\n", uip_ds6_routing_table[i].state.lifetime);
 //     } else {
 //       printf(")\n");
 //     }
      j=0;
    }
  }
  if (j) printf("  <none>");
  printf("\n---------\n");
}
#endif

#if STACKMONITOR
if ((clocktime%STACKMONITOR)==3) {
extern uint32_t __und_stack_top__, __sys_stack_top__;
uint32_t p=(uint32_t)&__und_stack_top__;
  do {
    if (*(uint32_t *)p != 0x42424242) {
      printf("Never-Used stack > %d bytes\n",p-(uint32_t)&__und_stack_top__);
      break;
    }
    p+=16;
  } while (p<(uint32_t)&__sys_stack_top__-100);
}
#endif
#if HEAPMONITOR
if ((clocktime%HEAPMONITOR)==4) {
extern uint32_t __heap_start__, __heap_end__;
uint32_t p=(uint32_t)&__heap_end__-4;
  do {
    if (*(uint32_t *)p != 0x42424242) {
      break;
    }
    p-=4;
  } while (p>=(uint32_t)&__heap_start__);
  printf("Never-used heap >= %d bytes\n",(uint32_t)&__heap_end__-p-4);
#if 0
#include <stdlib.h>
char *ptr=malloc(1);  //allocates 16 bytes from the heap
printf("********Got pointer %x\n",ptr);
#endif
}
#endif

    }
#endif /* PERIODICPRINTS */
  }
  
  return 0;
}

/*---------------------------------------------------------------------------*/
#if LOG_CONF_ENABLED
void
log_message(char *m1, char *m2)
{
  printf("%s%s\n", m1, m2);
}
#endif /* LOG_CONF_ENABLED */
