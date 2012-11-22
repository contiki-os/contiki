#include "lpc17xx_systick.h"
#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
//static unsigned int second_countdown = CLOCK_SECOND;
static unsigned int second_countdown = 100;

void SysTick_handler(void) __attribute__ ((interrupt));

void
SysTick_handler(void)
{
  //Clear System Tick counter flag
  SYSTICK_ClearCounterFlag();

  current_clock++;
  if(etimer_pending() && etimer_next_expiration_time() <= current_clock) {
    etimer_request_poll();
    /* printf("%d,%d\n", clock_time(),etimer_next_expiration_time  	()); */

  }

  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
}


void
clock_init()
{
  //Initialize System Tick with 10ms time interval
  SYSTICK_InternalInit(10);
  //Enable System Tick interrupt
  SYSTICK_IntCmd(ENABLE);
  //Enable System Tick Counter
  SYSTICK_Cmd(ENABLE);
}

clock_time_t
clock_time(void)
{
  return current_clock;
}

unsigned long
clock_seconds(void)
{
  return current_seconds;
}

//Do nothing
void clock_delay(unsigned int delay){
  return;
}
