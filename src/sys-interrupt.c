#include "sys-interrupt.h"
#include "interrupt-utils.h"

#include "embedded_types.h"

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
  ISR_EXIT();
}

static unsigned int enabled = 0; /* Number of times the system
				    interrupt has been enabled */

#define INTCNTL 0x80020000
#define DIS_INT *((volatile uint32_t *)INTCNTL) = 3 << 19;
#define EN_INT if (enabled > 0) *((volatile uint32_t *)INTCNTL) = 0;

void
sys_interrupt_enable()
{
  if (enabled++ == 0) {
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
