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
#include "K60.h"

#define DEBUG 0
#if DEBUG
#include "stdio.h"
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static volatile clock_time_t current_tick;
static volatile unsigned long current_seconds = 0;
static volatile unsigned long second_countdown = CLOCK_SECOND;

/*
 * Get the system time.
 */
clock_time_t
clock_time(void)
{
  return current_tick;
}
/*
 * Get the system time in seconds.
 */
unsigned long
clock_seconds(void)
{
  return current_seconds;
}
/*
 * Get the system time in seconds.
 */
void
clock_set_seconds(unsigned long sec)
{
  current_seconds = sec;
}
/*
 * Delay the CPU.
 */
void
clock_delay(unsigned int delay)
{
}
/*
 * Delay the CPU for a number of clock ticks.
 */
void
clock_wait(clock_time_t delay)
{
}
/*
 * Initialize the clock module.
 *
 * Generates interrupt from external 32kHz crystal.
 */
/* TODO(Henrik) move to platform, init may differ between platforms. */
void
clock_init(void)
{
  /* Setup Low Power Timer (LPT) */

  /* Setup 32768 Hz clock source */
  /** \todo clean up RTC initialization procedure on Mulle */
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;    /* Enable RTC clock gate */
  RTC_CR |= 0x00000100;         /* Enable RTC oscillator */
  SIM_SOPT1 |= 0x00080000;      /* Select RTC oscillator as ERCLK32K */

  SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK;    /* Enable LPT clock gate */
  LPTMR0_CNR = 0;
  LPTMR0_CMR = (32768 / CLOCK_SECOND) - 1;  /* Underflow every x+1 clocks */
  LPTMR0_PSR = 0x06;            /* PBYP, ERCLK32K */
  LPTMR0_CSR = 0x40;                      /* TIE */
  LPTMR0_CSR = 0x41;                      /* TIE | TEN */

  /* Enable LPT interrupt */
  NVICISER2 |= 0x00200000;
}
/*
 * LPTMR ISR
 */
void __attribute__((interrupt))
_isr_lpt(void)
{
  /* Clear timer compare flag by writing a 1 to it */
  LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;
  PRINTF("LPT: Interrupt\n");

  /* Contiki event polling */
  current_tick++;

  if(--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
  if(etimer_pending()) {
    etimer_request_poll();
  }
}
