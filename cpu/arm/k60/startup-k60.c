#include <stdint.h>
#include <stddef.h>
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

/* Initialize all data used by the C runtime. */
static void __attribute__((unused))
init_data(void)
{
  copy_initialized();

  clear_bss();

  copy_ramcode();
}

/* newlib's initialization function */
extern void __libc_init_array(void);

/* our local copy of newlib init */
void call_init_array(void);

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

  call_init_array(); /* or __libc_init_array() as provided by newlib or other libc */

  main();
  /* main should never return, but just in case... */
  while(1);
}

/* Initialize static C++ objects etc. */

/* The implementation of call_constructors is based on newlib's
 * __libc_init_array() copyright CodeSourcery */

/* The below copyright notice applies only to the function call_constructors,
 * copied from newlib 2.0 */
/*
 * Copyright (C) 2004 CodeSourcery, LLC
 *
 * Permission to use, copy, modify, and distribute this file
 * for any purpose is hereby granted without fee, provided that
 * the above copyright notice and this notice appears in all
 * copies.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

/* By default, initialize all C runtime data after preinit */
void _init(void) __attribute__((weak,alias("init_data")));

void
call_init_array(void)
{
  size_t count;
  size_t i;

  count = __preinit_array_end - __preinit_array_start;
  for (i = 0; i < count; i++)
    __preinit_array_start[i]();

  _init();

  count = __init_array_end - __init_array_start;
  for (i = 0; i < count; i++)
    __init_array_start[i]();
}
