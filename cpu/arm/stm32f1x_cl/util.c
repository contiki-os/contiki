#include <stdint.h>

uint32_t retval;

uint32_t
get_msp(void)
{
  asm("ldr r1, =retval");
  asm("mrs r0, msp");
  asm("str r0, [r1]");
  return retval;
}
