#include <stdint.h>
#include "K60.h"
#include "core-clocks.h"

extern int main(void); /* the reset handler will invoke main() after hardware startup. */

/* The reset handler does not need any function prologue/epilogue since it is
 * only called after a reset */
void reset_handler(void) __attribute__((naked));

/* Start of .data section in RAM */
extern uint32_t _data_start[];
/* End of .data section in RAM */
extern uint32_t _data_end[];
/* Start of .data section in flash */
extern uint32_t _data_load[];

/*
 * Copy all initialized variables in .data from flash to RAM.
 * .data must be 4-byte aligned!
 */
static void
copy_initialized(void)
{
  uint32_t * ram = _data_start;
  uint32_t * rom = _data_load;
  while(ram < _data_end) {
    *(ram++) = *(rom++);
  }
}

/* Start of .bss section in RAM */
extern uint32_t __bss_start[];
/* End of .bss section in RAM */
extern uint32_t __bss_end[];

/*
 * Clear out .bss section.
 * .bss must be 4-byte aligned!
 */
static void
clear_bss(void)
{
  uint32_t *p = __bss_start;
  while(p < __bss_end) {
    *p = 0x0ul;
    ++p;
  }
}

/* Start of .ramcode section in RAM */
extern uint32_t _ramcode_start[];
/* End of .ramcode section in RAM */
extern uint32_t _ramcode_end[];
/* Start of .ramcode section in flash */
extern uint32_t _ramcode_load[];
/*
 * Copy the ramcode section to RAM.
 */
static void
copy_ramcode(void)
{
  uint32_t * ram = _ramcode_start;
  uint32_t * rom = _ramcode_load;
  while(ram < _ramcode_end) {
    *(ram++) = *(rom++);
  }
}

/* Stack pointer will be set to _stack_start by the hardware at reset/power on */
void
reset_handler(void)
{
  /* Disable watchdog to allow single stepping through the startup code. */
  /** \todo Only disable watchdog on debug builds. */
  /*
   * The following unlock sequence must be completed within 256 bus cycles or
   * the watchdog will reset the system. The watchdog is enabled by default at
   * power on.
   *
   * The sequence is:
   * 1. Write 0xC520 to the unlock register
   * 2. Write 0xD928 to the unlock register
   *
   * Watchdog is now unlocked to allow us to change its settings
   *
   * 3. Clear the WDOGEN bit of the WDOG_STCTRLH register to completely disable
   *    the watchdog.
   *
   * It is now possible to single step through the code without the watchdog
   * resetting the system.
   */
  WDOG_UNLOCK = 0xC520;
  WDOG_UNLOCK = 0xD928;
  WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;

  /*
   * The line below this comment is the earliest possible location for a
   * breakpoint when debugging the startup code.
   */

  core_clocks_init_early();

  copy_initialized();

  clear_bss();

  copy_ramcode();

  main();
  /* main should never return, but just in case... */
  while(1);
}

