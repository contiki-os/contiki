#include <AT91SAM7S64.h>
#include <interrupt-utils.h>
#include <string.h>
#include <debug-uart.h>
#include <ctype.h>
#include <stdio.h>
#include <net/uip.h>
#include "net/uip-fw-drv.h"
#include "net/uaodv.h"
#include <dev/cc2420.h>
#include <dev/cc2420_const.h>
#include <dev/spi.h>
#include <dev/leds.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <sys/autostart.h>
#include <sys/etimer.h>
#include <net/psock.h>
#include <unistd.h>


#ifndef RF_CHANNEL
#define RF_CHANNEL 15
#endif

extern char __heap_end__;
extern char __heap_start__;

struct uip_fw_netif cc2420if =
  {UIP_FW_NETIF(172,16,0,129, 255,255,0,0, cc2420_send_ip)};

#if 0
/* Wathcdog is  already disabled in startup code */
static void
wdt_setup()
{

}
#endif

static void
wdt_reset()
{
  *AT91C_WDTC_WDCR = (0xa5<<24) | AT91C_WDTC_WDRSTT;
}

#if 0
static uip_ipaddr_t gw_addr = {{172,16,0,1}};
#endif

PROCINIT(&etimer_process, &tcpip_process, &cc2420_process,
	 &uip_fw_process /*,  &uaodv_process */
	 );


int
main()
{
  disableIRQ();
  disableFIQ();
  *AT91C_AIC_IDCR = 0xffffffff;
  *AT91C_PMC_PCDR = 0xffffffff;
  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA);
  
  dbg_setup_uart();
  printf("Initialising\n");
  leds_arch_init();
  clock_init();
  uip_sethostaddr(&cc2420if.ipaddr);
  uip_setnetmask(&cc2420if.netmask);
  
  /*uip_setdraddr(&gw_addr);*/
  cc2420_init();
  cc2420_set_chan_pan_addr(RF_CHANNEL,  HTONS(0x2024), uip_hostaddr.u16[1], NULL);
  process_init();
  uip_init();
  uip_fw_default(&cc2420if); 
  tcpip_set_forwarding(1);
  printf("Heap size: %ld bytes\n", &__heap_end__ - (char*)sbrk(0));
  printf("Started\n");

  procinit_init();
  enableIRQ(); 
  cc2420_on();
  autostart_start(autostart_processes);
  printf("Processes running\n");
  while(1) {
    do {
      /* Reset watchdog. */
      wdt_reset();
    } while(process_run() > 0);
    /* Idle! */
    /* Stop processor clock */
    *AT91C_PMC_SCDR |= AT91C_PMC_PCK;
  }
  return 0;
}
