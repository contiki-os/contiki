#ifndef SYS_INTERRUPT_H_QIHZ66NP8K__
#define SYS_INTERRUPT_H_QIHZ66NP8K__


/* Returns true if it handled an activbe interrupt */
typedef int (*SystemInterruptFunc)();

typedef struct _SystemInterruptHandler SystemInterruptHandler;
struct _SystemInterruptHandler
{
  SystemInterruptHandler *next;
  SystemInterruptFunc handler;
};


void
sys_interrupt_enable();

void
sys_interrupt_disable();

void
sys_interrupt_append_handler(SystemInterruptHandler *handler);

void
sys_interrupt_prepend_handler(SystemInterruptHandler *handler);

void
sys_interrupt_remove_handler(SystemInterruptHandler *handler);

#endif /* SYS_INTERRUPT_H_QIHZ66NP8K__ */
