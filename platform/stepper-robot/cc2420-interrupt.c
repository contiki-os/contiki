#include <AT91SAM7S64.h>
#include <contiki-conf.h>
#include <sys/process.h>
#include <net/uip.h>
#include <dev/cc2420.h>
#include <interrupt-utils.h>

static void NACKEDFUNC 
cc2420_fifop_interrupt (void) /* System Interrupt Handler */ 
{
  ISR_STORE();
  ISR_ENABLE_NEST();
  cc2420_interrupt();
  ISR_DISABLE_NEST(); 
  *AT91C_AIC_EOICR = 0;                   /* End of Interrupt */
  ISR_RESTORE();
}

void
cc2420_interrupt_fifop_int_init(void)
{
  *AT91C_PIOA_ASR = AT91C_PA30_IRQ1;
  *AT91C_PIOA_PDR = AT91C_PA30_IRQ1;
  AT91C_AIC_SMR[AT91C_ID_IRQ1] = AT91C_AIC_SRCTYPE_POSITIVE_EDGE | 4;
  AT91C_AIC_SVR[AT91C_ID_IRQ1] = (unsigned long)cc2420_fifop_interrupt;
  /* *AT91C_AIC_IECR = (1 << AT91C_ID_IRQ1); */
}

#ifndef __MAKING_DEPS__

inline int splhigh(void)
{
  int save;
#ifndef __THUMBEL__
  asm("mrs %0, CPSR\n\torr r1,%0,#0x80\n\tmsr CPSR_c, r1" : "=r" (save)::"r1");
#else
#error Must be compiled in ARM mode
#endif
  return save;
}

inline void splx(int saved)
{
#ifndef __THUMBELL__
  asm("msr CPSR_c, %0" ::"r" (saved));
#else
#error Must be compiled in ARM mode
#endif
}

#endif /* __MAKING_DEPS__ */
