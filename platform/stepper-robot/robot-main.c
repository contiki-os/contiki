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

#include <stepper-steps.h>
#include <stepper.h>
#include <stepper-move.h>


#ifndef RF_CHANNEL
#define RF_CHANNEL 15
#endif

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

struct uip_fw_netif cc2420if =
  {UIP_FW_NETIF(172,16,0,2, 255,255,0,0, cc2420_send_ip)};


#if 0
PROCESS(udprecv_process, "UDP recv process");

PROCESS_THREAD(udprecv_process, ev, data)
{
  static struct uip_udp_conn *c;

  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("udprecv_process starting\n");

  {
    uip_ipaddr_t any;
    uip_ipaddr(&any, 0,0,0,0);
    c = udp_new(&any, HTONS(0), NULL);
    uip_udp_bind(c, HTONS(4321));
  }
  
  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event && uip_newdata()) {
      u8_t *src = ((struct uip_udpip_hdr *)uip_buf)->srcipaddr.u8;
      printf("%d.%d.%d.%d: %s\n",
	     src[0], src[1], src[2], src[3], (char *)uip_appdata);
    }
  }

 exit:
  /* Contiki does automatic garbage collection of uIP state and we
   * need not worry about that. */
  printf("udprecv_process exiting\n");
  PROCESS_END();
}

PROCESS(wd_test_process, "Watchdog test process");


PROCESS_THREAD(wd_test_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  
  printf("tcp_test_process starting\n");

  etimer_set(&timer, 25*CLOCK_SECOND);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			     ev== PROCESS_EVENT_TIMER);
    if (ev == PROCESS_EVENT_EXIT) break;
    printf("Blocking execution\n");
    while(1);
  }
  
  PROCESS_END();
}
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

  robot_stepper_init();
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
