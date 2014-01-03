/*
 * efm32lg256 linker script
 * Defines Memory sizes and include main efm32 linker
*/

MEMORY
{
  rom (rx) :  ORIGIN = 0x00000000, LENGTH = 256K
  ram (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

INCLUDE efm32.ld

