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

#include <stepper-steps.h>
#include <stepper.h>
#include <stepper-move.h>

#include "net/mac/nullmac.h"
#include "net/rime.h"


#ifndef RF_CHANNEL
#define RF_CHANNEL 15
#endif

#ifndef WITH_UIP
#define WITH_UIP 1
#endif

#if WITH_UIP
#include "net/uip.h"
#include "net/uip-fw.h"
#include "net/uip-fw-drv.h"
#include "net/uip-over-mesh.h"

static struct uip_fw_netif meshif =
  {UIP_FW_NETIF(172,16,0,0, 255,255,0,0, uip_over_mesh_send)};

#define UIP_OVER_MESH_CHANNEL 9

#endif /* WITH_UIP */

static rimeaddr_t node_addr = {{0,2}};

extern char __heap_end__;
extern char __heap_start__;


static const uint32_t stepper0_steps_acc[] = MICRO_STEP(0,3);
static const uint32_t stepper0_steps_run[] = MICRO_STEP(0,2);
static const uint32_t stepper0_steps_hold[] = MICRO_STEP(0,1);

static const uint32_t stepper1_steps_acc[] = MICRO_STEP(1,3);
static const uint32_t stepper1_steps_run[] = MICRO_STEP(1,2);
static const uint32_t stepper1_steps_hold[] = MICRO_STEP(1,1);

static StepperAccSeq seq_heap[40];

static void
init_seq_heap()
{
  unsigned int i;
  for(i = 0; i < sizeof(seq_heap)/sizeof(seq_heap[0]); i++) {
    seq_heap[i].next = NULL;
    stepper_free_seq(&seq_heap[i]);
  }
}

static void
robot_stepper_init()
{
  init_seq_heap();
  stepper_init(AT91C_BASE_TC0, AT91C_ID_TC0);
  *AT91C_PIOA_OER = STEPPER_INHIBIT;
  *AT91C_PIOA_MDER = STEPPER_INHIBIT; /*  | STEPPER0_IOMASK; */
  *AT91C_PIOA_CODR = STEPPER_INHIBIT;
  stepper_init_io(1, STEPPER_IOMASK(0), stepper0_steps_acc,
		  stepper0_steps_run, stepper0_steps_hold,
		  (sizeof(stepper0_steps_run) / sizeof(stepper0_steps_run[0])));
  stepper_init_io(0, STEPPER_IOMASK(1), stepper1_steps_acc,
		  stepper1_steps_run, stepper1_steps_hold,
		  (sizeof(stepper1_steps_run) / sizeof(stepper1_steps_run[0])));}


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
  leds_arch_init();
  clock_init();
  process_init();
  process_start(&etimer_process, NULL);
  ctimer_init();

  robot_stepper_init();

  enableIRQ();

  cc2420_init();
  cc2420_set_pan_addr(0x2024, 0, &uip_hostaddr.u16[1]);
  cc2420_set_channel(RF_CHANNEL);
  rime_init(nullmac_init(&cc2420_driver));
  printf("CC2420 setup done\n");

  rimeaddr_set_node_addr(&node_addr);
  
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
#endif /* WITH_UIP */
  
  printf("Heap size: %ld bytes\n", &__heap_end__ - (char*)sbrk(0));
  printf("Started\n");

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
