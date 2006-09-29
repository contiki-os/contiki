/*
 * This file is ripped from x86/mtarch.c of the Contiki Multi-threading library.
 * Fredrik Osterlind <fros@sics.se>
 */

#include <stdio.h>
#include "sys/cooja_mt.h"

struct frame {
  unsigned long flags;
  unsigned long ebp;
  unsigned long edi;
  unsigned long esi;
  unsigned long edx;
  unsigned long ecx;
  unsigned long ebx;
  unsigned long eax;
  unsigned long retaddr;
  unsigned long retaddr2;
  unsigned long data;
};

/*--------------------------------------------------------------------------*/
void
cooja_mtarch_init(void)
{
  
}
/*--------------------------------------------------------------------------*/
void
cooja_mtarch_start(struct cooja_mtarch_thread *t,
	     void (*function)(void *), void *data)
{
  
  struct frame *f = (struct frame *)&t->stack[COOJA_MTARCH_STACKSIZE - sizeof(struct frame)/4];
  int i;
  
  for(i = 0; i < COOJA_MTARCH_STACKSIZE; ++i) {
    t->stack[i] = i;
  }
  
  memset(f, 0, sizeof(struct frame));
  f->retaddr = (unsigned long)function;
  f->data    = (unsigned long)data;
  t->sp      = (unsigned long)&f->flags;
  f->ebp     = (unsigned long)&f->eax;
}
/*--------------------------------------------------------------------------*/
static unsigned long cooja_spsave, cooja_sptmp;
static struct cooja_mtarch_thread *cooja_running_thread;

static void
cooja_sw(void)
{
  
  asm("pushl %eax");
  asm("pushl %ebx");
  asm("pushl %ecx");
  asm("pushl %edx");
  asm("pushl %esi");
  asm("pushl %edi");
  asm("pushl %ebp");
 // asm("pushl %ebp"); /* XXX: should push FPU flags here. */
  asm("movl %esp, cooja_spsave");

  cooja_sptmp = cooja_running_thread->sp;
  cooja_running_thread->sp = cooja_spsave;

  asm("movl cooja_sptmp, %esp");
 // asm("popl %ebp"); /* XXX: should pop FPU flags here. */
  asm("popl %ebp");  
  asm("popl %edi");
  asm("popl %esi");
  asm("popl %edx");
  asm("popl %ecx");
  asm("popl %ebx");
  asm("popl %eax");
}

/*--------------------------------------------------------------------------*/
void
cooja_mtarch_exec(struct cooja_mtarch_thread *t)
{
  cooja_running_thread = t;
  cooja_sw();
  cooja_running_thread = NULL;
}
/*--------------------------------------------------------------------------*/
void
cooja_mtarch_remove(void)
{

}
/*--------------------------------------------------------------------------*/
void
cooja_mtarch_yield(void)
{
  cooja_sw();
}
/*--------------------------------------------------------------------------*/
void
cooja_mtarch_pstop(void)
{
  
}
/*--------------------------------------------------------------------------*/
void
cooja_mtarch_pstart(void)
{
  
}
/*--------------------------------------------------------------------------*/
int
cooja_mtarch_stack_usage(struct cooja_mt_thread *t)
{
  int i;
  for(i = 0; i < COOJA_MTARCH_STACKSIZE; ++i) {
    if(t->thread.stack[i] != i) {
      return COOJA_MTARCH_STACKSIZE - i;
    }
  }
  return -1;
}
/*--------------------------------------------------------------------------*/
