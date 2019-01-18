#include "rtimer-arch-interrupt.h"
#include "rtimer-arch.h"
#include <interrupt-utils.h>
#include <AT91SAM7S64.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* Here we have a proper stack frame and can use local variables */
static void rtimer_int_safe() __attribute((noinline));
static void
rtimer_int_safe()
{
  unsigned int status;
  status = RTIMER_ARCH_TIMER_BASE->TC_SR;
  if (status & AT91C_TC_CPAS) {
    rtimer_run_next();
  }
  *AT91C_AIC_EOICR = 0;
}

void NACKEDFUNC
rtimer_interrupt (void) {
  ISR_STORE();
  ISR_ENABLE_NEST();
  rtimer_int_safe();
  ISR_DISABLE_NEST();
  ISR_RESTORE();
}
