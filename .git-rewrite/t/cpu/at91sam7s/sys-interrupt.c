#include <sys-interrupt.h>
#include <interrupt-utils.h>
#include <AT91SAM7S64.h>

#define ATTR

#ifndef NULL
#define NULL 0
#endif


static SystemInterruptHandler *handlers = NULL;

static void
system_int_safe (void) __attribute__((noinline));

static void
system_int_safe (void) 
{
  SystemInterruptHandler *h;
  h = handlers;
  while (h) {
    if (h->handler()) break;
    h = h->next;
  }
}
  
static void NACKEDFUNC ATTR
system_int (void) /* System Interrupt Handler */ 
{
  ISR_ENTRY();
  system_int_safe();
  *AT91C_AIC_EOICR = 0;                   /* End of Interrupt */
  ISR_EXIT();
}

static unsigned int enabled = 0; /* Number of times the system
				    interrupt has been enabled */

#define DIS_INT *AT91C_AIC_IDCR = (1 << AT91C_ID_SYS)
#define EN_INT if (enabled > 0) *AT91C_AIC_IECR = (1 << AT91C_ID_SYS)

void
sys_interrupt_enable()
{
  if (enabled++ == 0) {
    /* Level trigged at priority 5 */
    AT91C_AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 5;
    /* Interrupt vector */
    AT91C_AIC_SVR[AT91C_ID_SYS] = (unsigned long) system_int;
    /* Enable */
    EN_INT;
  }
}


void
sys_interrupt_disable()
{
  if (--enabled == 0) {
    DIS_INT;
  }
}

void
sys_interrupt_append_handler(SystemInterruptHandler *handler)
{
  SystemInterruptHandler **h = &handlers;
  while(*h) {
    h = &(*h)->next;
  }
  DIS_INT;
  *h = handler;
  handler->next = NULL;
  EN_INT;
}

void
sys_interrupt_prepend_handler(SystemInterruptHandler *handler)
{
  DIS_INT;
  handler->next = handlers;
  handlers = handler;
  EN_INT;
}

void
sys_interrupt_remove_handler(SystemInterruptHandler *handler)
{
  SystemInterruptHandler **h = &handlers;
  while(*h) {
    if (*h == handler) {
      DIS_INT;
      *h = handler->next;
      EN_INT;
      break;
    }
    h = &(*h)->next;
  }
}
