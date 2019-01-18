#include <avr/io.h>

#include "contiki-conf.h"

void
cpu_init(void)
{
  asm volatile ("clr r1");	/* No longer needed. */
}

extern int __bss_end;

#define STACK_EXTRA 32
static char *cur_break = (char *)(&__bss_end + 1);

/*
 * Allocate memory from the heap. Check that we don't collide with the
 * stack right now (some other routine might later). A watchdog might
 * be used to check if cur_break and the stack pointer meet during
 * runtime.
 */
void *
sbrk(int incr)
{
  char *stack_pointer;

  stack_pointer = (char *)SP;
  stack_pointer -= STACK_EXTRA;
  if(incr > (stack_pointer - cur_break))
    return (void *)-1;          /* ENOMEM */

  void *old_break = cur_break;
  cur_break += incr;
  /*
   * If the stack was never here then [old_break .. cur_break] should
   * be filled with zeros.
  */
  return old_break;
}
