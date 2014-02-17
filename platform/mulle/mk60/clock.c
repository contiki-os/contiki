/**
 * \file
 *         Clock module library port for MK60DZ10.
 * \author
 *         Tony Persson <tony.persson@rubico.com>
 */

#include "contiki.h"
#include "contiki-conf.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include "MK60N512VMD100.h"

static volatile clock_time_t current_tick;
static volatile unsigned long current_seconds = 0;
static volatile unsigned long second_countdown = CLOCK_SECOND;

extern volatile uint8_t do_sleep;
extern volatile uint32_t ds;
/*
 * Get the system time.
 */
clock_time_t clock_time(void)
{
  return current_tick;
}

/*
 * Get the system time in seconds.
 */
unsigned long clock_seconds(void)
{
  return current_seconds;
}

/*
 * Get the system time in seconds.
 */
void clock_set_seconds(unsigned long sec)
{
  current_seconds = sec;
}

/*
 * Delay the CPU.
 */
void clock_delay(unsigned int delay)
{

}

/*
 * Delay the CPU for a number of clock ticks.
 */
void clock_wait(clock_time_t delay) 
{

}

/*
 * Initialize the clock module.
 * 
 * Generates interrupt at 1kHz.
 */
void clock_init(void)
{
  /* Setup Low Power Timer (LPT) */

  SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK;    /* Enable LPT clock gate */
  LPTMR0_CNR = 0;
  LPTMR0_CMR = 16-1;                       /* Underflow every x+1 milliseconds */
  LPTMR0_PSR = 0x05;                      /* PBYP, LPO 1 KHz selected */
  LPTMR0_CSR = 0x40;                      /* TIE */
  LPTMR0_CSR = 0x41;                      /* TIE | TEN */

  /* Enable LPT interrupt */
  NVICISER2 |= 0x00200000;
}

/*
 * LPTMR ISR
 */
void __attribute__((interrupt( irq ))) _isr_low_power_timer(void)
    {

  LPTMR0_CSR |= 0x80;

  /* Contiki event polling */
  current_tick++;

  if (--second_countdown == 0)
  {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
  if(etimer_pending())
  {
    etimer_request_poll();
  }
    }
