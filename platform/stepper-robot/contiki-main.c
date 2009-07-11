#include <AT91SAM7S64.h>
#include <interrupt-utils.h>
#include <string.h>
#include <debug-uart.h>
#include <ctype.h>
#include <stdio.h>
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

#include "net/mac/nullmac.h"
#include "net/rime.h"

#include "contiki-main.h"

#ifndef RF_CHANNEL
#define RF_CHANNEL 15
#endif


#if WITH_UIP
#include "net/uip.h"
#include "net/uip-fw.h"
#include "net/uip-fw-drv.h"
#include "net/uip-over-mesh.h"


static struct uip_fw_netif meshif =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, uip_over_mesh_send)};

#define UIP_OVER_MESH_CHANNEL 9

uint8_t is_gateway = 0;
struct trickle_conn gateway_trickle;

#endif /* WITH_UIP */

extern char __heap_end__;
extern char __heap_start__;


rimeaddr_t node_addr __attribute__((weak)) = {{0,2}};

/*--------------------------------------------------------------------------*/
#if WITH_UIP

/* Receive address of gateway */

static void
trickle_recv(struct trickle_conn *c)
{
  if (!is_gateway) {
    struct gateway_msg *msg;
    msg = rimebuf_dataptr();
    printf("%d.%d: gateway message: %d.%d\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	   msg->gateway.u8[0], msg->gateway.u8[1]);
    
    uip_over_mesh_set_gateway(&msg->gateway);
  }
}

const static struct trickle_callbacks trickle_call = {trickle_recv};

/*---------------------------------------------------------------------------*/
#endif

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
  leds_init();
  clock_init();
  process_init();
  process_start(&etimer_process, NULL);

  ctimer_init();

  enableIRQ();

  printf("Beginning CC2420 setup\n");
  cc2420_init();
  cc2420_set_pan_addr(0x2024, 0, NULL);
  cc2420_set_channel(RF_CHANNEL);
  rime_init(nullmac_init(&cc2420_driver));
  printf("CC2420 setup done\n");

  rimeaddr_set_node_addr(&node_addr);
  printf("Rime started with address %d.%d\n", node_addr.u8[0], node_addr.u8[1]);
  
  #if WITH_UIP
  {
    uip_ipaddr_t hostaddr, netmask;
    
    uip_init();

    uip_ipaddr(&hostaddr, 172,16,
               rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
    uip_ipaddr(&netmask, 255,255,0,0);
    uip_ipaddr_copy(&meshif.ipaddr, &hostaddr);
    printf("Host addr\n");
    uip_sethostaddr(&hostaddr);
    uip_setnetmask(&netmask);
    uip_over_mesh_set_net(&hostaddr, &netmask);
    /*    uip_fw_register(&slipif);*/
    /*uip_over_mesh_set_gateway_netif(&slipif);*/
    uip_fw_register(&meshif);
    uip_fw_default(&meshif);
    printf("Mesh init\n");
    uip_over_mesh_init(UIP_OVER_MESH_CHANNEL);
    printf("uIP started with IP address %d.%d.%d.%d\n",
           uip_ipaddr_to_quad(&hostaddr));
  }
#endif /* WITH_UIP */


 
#if WITH_UIP
  process_start(&tcpip_process, NULL);
  process_start(&uip_fw_process, NULL); /* Start IP output */
  
  trickle_open(&gateway_trickle, CLOCK_SECOND * 4, GATEWAY_TRICKLE_CHANNEL,
               &trickle_call);
#endif /* WITH_UIP */

  printf("Heap size: %ld bytes\n", &__heap_end__ - (char*)sbrk(0));
  printf("Started\n");

  /* procinit_init(); */

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
