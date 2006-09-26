
#include <stdio.h>
#include "sys/mt.h"

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
mtarch_init(void)
{
  
}
/*--------------------------------------------------------------------------*/
void
mtarch_start(struct mtarch_thread *t,
	     void (*function)(void *), void *data)
{
  
  struct frame *f = (struct frame *)&t->stack[MTARCH_STACKSIZE - sizeof(struct frame)/4];
  int i;
  
  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    t->stack[i] = i;
  }
  
  memset(f, 0, sizeof(struct frame));
  f->retaddr = (unsigned long)function;
  f->data    = (unsigned long)data;
  t->sp      = (unsigned long)&f->flags;
  f->ebp     = (unsigned long)&f->eax;
}
/*--------------------------------------------------------------------------*/
static unsigned long spsave, sptmp;
static struct mtarch_thread *running_thread;

static void
sw(void)
{
  
  asm("pushl %eax");
  asm("pushl %ebx");
  asm("pushl %ecx");
  asm("pushl %edx");
  asm("pushl %esi");
  asm("pushl %edi");
  asm("pushl %ebp");
  asm("pushl %ebp"); /* XXX: should push FPU flags here. */
  asm("movl %esp, spsave");

  sptmp = running_thread->sp;
  running_thread->sp = spsave;

  asm("movl sptmp, %esp");
  asm("popl %ebp"); /* XXX: should pop FPU flags here. */
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
mtarch_exec(struct mtarch_thread *t)
{
  running_thread = t;
  sw();
  running_thread = NULL;
}
/*--------------------------------------------------------------------------*/
void
mtarch_remove(void)
{

}
/*--------------------------------------------------------------------------*/
void
mtarch_yield(void)
{
  sw();
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstop(void)
{
  
}
/*--------------------------------------------------------------------------*/
void
mtarch_pstart(void)
{
  
}
/*--------------------------------------------------------------------------*/
int
mtarch_stack_usage(struct mt_thread *t)
{
  int i;
  for(i = 0; i < MTARCH_STACKSIZE; ++i) {
    if(t->thread.stack[i] != i) {
      return MTARCH_STACKSIZE - i;
    }
  }
}
/*--------------------------------------------------------------------------*/
