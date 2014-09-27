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
  /* Setup 32768 Hz clock source */
  /** \todo clean up RTC initialization procedure on Mulle */
  /* NB: this has probably already been performed during boot up and core clock
   * setup on Mulle. */
  /* Enable RTC clock gate */
  SIM->SCGC6 |= SIM_SCGC6_RTC_MASK;
  /* Enable RTC oscillator */
  BITBAND_REG(RTC->CR, RTC_CR_OSCE_SHIFT) = 1;
  /* Select RTC oscillator as ERCLK32K */
  SIM->SOPT1 = (SIM->SOPT1 & ~(SIM_SOPT1_OSC32KSEL_MASK)) | SIM_SOPT1_OSC32KSEL(0b10);

  /* Setup Low Power Timer (LPT) */

  /* Enable LPT clock gate */
  SIM->SCGC5 |= SIM_SCGC5_LPTIMER_MASK;

  /* Disable timer to change settings. */
  /* Logic is reset when the timer is disabled, TCF flag is also cleared on disable. */
  LPTMR0->CSR = 0x00;
  /* Underflow every x+1 clocks */
  LPTMR0->CMR = (LPTMR_CMR_COMPARE((32768 / CLOCK_SECOND) - 1));
  /* Prescaler bypass, LPTMR is clocked directly by ERCLK32K. */
  LPTMR0->PSR = (LPTMR_PSR_PBYP_MASK | LPTMR_PSR_PCS(0b10));
  /* Enable timer, enable interrupts. */
  LPTMR0->CSR |= (LPTMR_CSR_TEN_MASK | LPTMR_CSR_TIE_MASK);

  /* Enable LPT interrupt */
  NVIC_EnableIRQ(LPTimer_IRQn);
}

/*
 * LPTMR ISR
 */
void
_isr_lpt(void)
{
  /* Clear timer compare flag by writing a 1 to it */
  BITBAND_REG(LPTMR0->CSR, LPTMR_CSR_TCF_SHIFT) = 1;
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
