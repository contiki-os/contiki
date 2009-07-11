#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include <debug-uart.h>

#include <AT91SAM7S64.h>
#include <sys-interrupt.h>

#define PIV  ((MCK/CLOCK_SECOND/16)-1)

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;


static int pit_handler_func()
{
  if (!(*AT91C_PITC_PISR & AT91C_PITC_PITS)) return 0;  /* Check PIT
							   Interrupt */
  current_clock++;
  if(etimer_pending() && etimer_next_expiration_time() <= current_clock) {
    etimer_request_poll();
    /* dbg_printf("%d,%d\n", clock_time(),etimer_next_expiration_time  	()); */

  }
  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
  (void)*AT91C_PITC_PIVR;
  return 1;
}

static SystemInterruptHandler pit_handler = {NULL, pit_handler_func};

void
clock_init()
{
  sys_interrupt_append_handler(&pit_handler);
  *AT91C_PITC_PIMR = (AT91C_PITC_PITIEN |    /* PIT Interrupt Enable */ 
		      AT91C_PITC_PITEN  |    /* PIT Enable */
		      PIV);
  sys_interrupt_enable();
}

clock_time_t
clock_time(void)
{
  return current_clock;
}

/* The inner loop takes 4 cycles. The outer 5+SPIN_COUNT*4. */

#define SPIN_TIME 2 /* us */
#define SPIN_COUNT (((MCK*SPIN_TIME/1000000)-5)/4)

#ifndef __MAKING_DEPS__

void
clock_delay(unsigned int t)
{
#ifdef __THUMBEL__ 
  asm volatile("1: mov r1,%2\n2:\tsub r1,#1\n\tbne 2b\n\tsub %0,#1\n\tbne 1b\n":"=l"(t):"0"(t),"l"(SPIN_COUNT));
#else
#error Must be compiled in thumb mode
#endif
}

unsigned long
clock_seconds(void)
{
  return current_seconds;
}
#endif /* __MAKING_DEPS__ */
