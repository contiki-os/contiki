#include <AT91SAM7S64.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <sys/etimer.h>
#include <dev/leds.h>
#include <debug-uart.h>
#include <interrupt-utils.h>

volatile const char * volatile input_line = NULL;
volatile unsigned int input_line_len = 0;

static void
recv_input(const char *str, unsigned int len)
{
  /* Assume that the line is handled before any new characters is written
     to the buffer */
  input_line = str;
  input_line_len = len;
}
PROCESS(blink_process, "LED blink process");

PROCESS_THREAD(blink_process, ev , data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  etimer_set(&timer, CLOCK_SECOND/2);
   while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			     ev== PROCESS_EVENT_TIMER);
    if (ev == PROCESS_EVENT_EXIT) break;
    leds_invert(LEDS_RED);
#if 0
    {
      DISABLE_FIFOP_INT();
      printf("FSMSTATE:  %04x",cc2420_getreg(CC2420_FSMSTATE));
      ENABLE_FIFOP_INT();
      if (SFD_IS_1) printf(" SFD");
      if (FIFO_IS_1) printf(" FIFO");
      if (FIFOP_IS_1) printf(" FIFOP");
      putchar('\n');
    }
#endif
    etimer_reset(&timer);
  }
  printf("Ended process\n");
  PROCESS_END();
}
PROCINIT(&etimer_process, &blink_process);
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
  dbg_set_input_handler(recv_input);
  leds_arch_init();
  clock_init();
  
  process_init();
  printf("Started\n");
  
  procinit_init();
  enableIRQ(); 
  printf("Processes running\n");
  while(1) {
    do {
      /* Reset watchdog. */
     } while(process_run() > 0);
    /* Idle! */
    /* Stop processor clock */
    *AT91C_PMC_SCDR |= AT91C_PMC_PCK;
  }
  return 0;
}
