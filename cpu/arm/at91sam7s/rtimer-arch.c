#include "rtimer-arch.h"
#include <AT91SAM7S64.h>
#include "rtimer-arch-interrupt.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static rtimer_clock_t offset;

void
rtimer_arch_init(void)
{
  offset = 0;
  RTIMER_ARCH_TIMER_BASE->TC_CMR =
    (AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP | AT91C_TC_CLKS_TIMER_DIV5_CLOCK);
  RTIMER_ARCH_TIMER_BASE->TC_RA = 0xffff;
  RTIMER_ARCH_TIMER_BASE->TC_IER = AT91C_TC_CPAS;
  *AT91C_PMC_PCER = (1 << RTIMER_ARCH_TIMER_ID);
  AT91C_AIC_SMR[RTIMER_ARCH_TIMER_ID] =
    AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 6;
  AT91C_AIC_SVR[RTIMER_ARCH_TIMER_ID] =  (unsigned long)rtimer_interrupt;
  *AT91C_AIC_IECR = (1 << RTIMER_ARCH_TIMER_ID);
  RTIMER_ARCH_TIMER_BASE->TC_CCR = AT91C_TC_SWTRG | AT91C_TC_CLKEN;
  PRINTF("rtimer_arch_init: Done\n");
}

void
rtimer_arch_schedule(rtimer_clock_t t)
{
  RTIMER_ARCH_TIMER_BASE->TC_RA = t + offset;
  PRINTF("rtimer_arch_schedule: %d\n",t);
}

void
rtimer_arch_set(rtimer_clock_t t)
{
  offset = t -  RTIMER_ARCH_TIMER_BASE->TC_CV;
}

rtimer_clock_t
rtimer_arch_now(void)
{
  return RTIMER_ARCH_TIMER_BASE->TC_CV + offset;
}
