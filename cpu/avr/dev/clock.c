
#include "sys/clock.h"
#include "dev/clock-avr.h"
#include "sys/etimer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile clock_time_t count;
static volatile uint8_t scount;
volatile unsigned long seconds;
long sleepseconds;

/* Set RADIOSTATS to monitor radio on time (must also be set in the radio driver) */
#if RF230BB && WEBSERVER
#define RADIOSTATS 1
#endif
#if RADIOSTATS
static volatile uint8_t rcount;
volatile unsigned long radioontime;
extern uint8_t RF230_receive_on;
#endif

/*
  CLOCK_SECOND is the number of ticks per second.
  It is defined through CONF_CLOCK_SECOND in the contiki-conf.h for each platform.
  The usual AVR default is ~125 ticks per second, counting a prescaler the CPU clock
  using the 8 bit timer0.
  
  As clock_time_t is an unsigned 16 bit data type, intervals up to 524 seconds
  can be measured with 8 millisecond precision. 
  For longer intervals a 32 bit global is incremented every second. 
 
  clock-avr.h contains the specific setup code for each mcu.

*/
/*---------------------------------------------------------------------------*/
/* This routine can be called to add seconds to the clock after a sleep
 * of an integral number of seconds.
 */
void clock_adjust_seconds(uint8_t howmany) {
   seconds += howmany;
   sleepseconds +=howmany;
#if RADIOSTATS
  if (RF230_receive_on) radioontime += howmany;
#endif
}
/*---------------------------------------------------------------------------*/
/* These routines increment the second counters.
 * Calling these avoids the interrupt overhead of pushing many registers on the stack.
 */
static void increment_seconds(void) __attribute__ ((noinline));
static void increment_seconds(void)
{
  seconds++;
}
#if RADIOSTATS
extern volatile uint8_t rf230_calibrate;
static void increment_radioontime(void)  __attribute__ ((noinline));
static void increment_radioontime(void)
{
static uint8_t calibrate_interval;
  radioontime++;
  if (++calibrate_interval==0) {
    rf230_calibrate=1;
  }
}
#endif
/*---------------------------------------------------------------------------*/
//SIGNAL(SIG_OUTPUT_COMPARE0)
ISR(AVR_OUTPUT_COMPARE_INT)
{
  count++;
  if(++scount == CLOCK_SECOND) {
    scount = 0;
    increment_seconds();
//  seconds++;
  }
#if RADIOSTATS
  if (RF230_receive_on) {
    if (++rcount == CLOCK_SECOND) {
      rcount=0;
      increment_radioontime();
 //   radioontime++;
    }
  }
#endif
  if(etimer_pending()) {
    etimer_request_poll();
  }
}

/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  cli ();
  OCRSetup();
//scount = count = 0;
  sei ();
}

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  clock_time_t tmp;
  do {
    tmp = count;
  } while(tmp != count);
  return tmp;
}
/*---------------------------------------------------------------------------*/
/**
 * Delay the CPU for a multiple of TODO
 */
void
clock_delay(unsigned int i)
{
  for (; i > 0; i--) {		/* Needs fixing XXX */
    unsigned j;
    for (j = 50; j > 0; j--)
      asm volatile("nop");
  }
}

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 1 / 125 sec = 0.008 ms.
 *
 */
void
clock_wait(int i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
    // TODO
}

unsigned long
clock_seconds(void)
{
  unsigned long tmp;
  do {
    tmp = seconds;
  } while(tmp != seconds);
  return tmp;
}
