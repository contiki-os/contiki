/*
 * Copyright (c) 2015 NXP B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Thomas Haydon
 * Integrated into Contiki by Beshr Al Nahas
 *
 */

#include <jendefs.h>
#include <AppHardwareApi.h>
#include <MicroInt.h>
#include "exceptions.h"

#ifndef EXCEPTION_STALLS_SYSTEM
#define EXCEPTION_STALLS_SYSTEM 0
#endif /* EXCEPTION_STALLS_SYSTEM */

#ifndef PRINT_STACK_ON_REBOOT
#define PRINT_STACK_ON_REBOOT 1
#endif /* PRINT_STACK_ON_REBOOT */

/** Define to dump the stack on exception */
#ifndef EXC_DUMP_STACK
#define EXC_DUMP_STACK
#endif /* EXC_DUMP_STACK */

/** Define to dump registers on exception */
#ifndef EXC_DUMP_REGS
/* #define EXC_DUMP_REGS */
#endif /* EXC_DUMP_REGS */

/* Select whether exception vectors should be in RAM or Flash based on chip family */
#if (defined JENNIC_CHIP_FAMILY_JN514x)
#define EXCEPTION_VECTORS_LOCATION_RAM
#elif (defined JENNIC_CHIP_FAMILY_JN516x)
#define EXCEPTION_VECTORS_LOCATION_FLASH
#else
#error Unsupported chip family selected
#endif /* JENNIC_CHIP_FAMILY */

#if (defined EXCEPTION_VECTORS_LOCATION_RAM)
/* RAM exception vectors are set up at run time */
/* Addresses of exception vectors in RAM */
#define BUS_ERROR                   *((volatile uint32 *)(0x4000000))
#define TICK_TIMER                  *((volatile uint32 *)(0x4000004))
#define UNALIGNED_ACCESS            *((volatile uint32 *)(0x4000008))
#define ILLEGAL_INSTRUCTION         *((volatile uint32 *)(0x400000c))
#define EXTERNAL_INTERRUPT          *((volatile uint32 *)(0x4000010))
#define SYSCALL                     *((volatile uint32 *)(0x4000014))
#define TRAP                        *((volatile uint32 *)(0x4000018))
#define GENERIC                     *((volatile uint32 *)(0x400001c))
#define STACK_OVERFLOW              *((volatile uint32 *)(0x4000020))
#elif (defined EXCEPTION_VECTORS_LOCATION_FLASH)
/* Flash exception vectors are set up at compile time */
#else
#error Unknown exception vector location
#endif /* EXCEPTION_VECTORS_LOCATION */

/* Locations in stack trace of important information */
#define STACK_REG                   1
#define PROGRAM_COUNTER             18
#define EFFECTIVE_ADDR              19

/* Number of registers */
#define REG_COUNT                   16

/* Chip dependant RAM size */
#if defined(JENNIC_CHIP_JN5148) || defined(JENNIC_CHIP_JN5148J01)
#define EXCEPTION_RAM_TOP 0x04020000
#else
#define EXCEPTION_RAM_TOP 0x04008000
#endif

static void exception_handler(uint32 *pu32Stack, eExceptionType eType);
static void *heap_alloc_overflow_protect(void *pvPointer, uint32 u32Size, bool_t bClear);
/*---------------------------------------------------------------------------*/
#if PRINT_STACK_ON_REBOOT
static void hexprint(uint8 v);
static void hexprint32(uint32 v);
static void printstring(const char *s);
#endif /* PRINT_STACK_ON_REBOOT */

/* For debugging */
static const char *debug_filename = "nothing";
static int debug_line = -1;

void
debug_file_line(const char *file, int line)
{
  debug_filename = file;
  debug_line = line;
}
extern uint32 heap_location;
extern void *(*prHeap_AllocFunc)(void *, uint32, bool_t);
PRIVATE void *(*prHeap_AllocOrig)(void *, uint32, bool_t);

/* Symbol defined by the linker script */
/* marks the end of the stack */
extern void *stack_low_water_mark;

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/*---------------------------------------------------------------------------*/
#if PRINT_STACK_ON_REBOOT
#include "dev/uart0.h"
#define printchar(X) uart0_write_direct(X)
/*---------------------------------------------------------------------------*/
static void
hexprint(uint8 v)
{
  const char hexconv[] = "0123456789abcdef";
  printchar(hexconv[v >> 4]);
  printchar(hexconv[v & 0x0f]);
}
/*---------------------------------------------------------------------------*/
static void
hexprint32(uint32 v)
{
  hexprint(((uint32)v) >> (uint32)24);
  hexprint(((uint32)v) >> (uint32)16);
  hexprint(((uint32)v) >> (uint32)8);
  hexprint((v) & 0xff);
}
/*---------------------------------------------------------------------------*/
static void
printstring(const char *s)
{
  while(*s) {
    printchar(*s++);
  }
}
#endif /* PRINT_STACK_ON_REBOOT */

/****************************************************************************
 *
 * NAME: vEXC_Register
 *
 * DESCRIPTION:
 * Set up exceptions. When in RAM, overwrite the default vectors with ours.
 * We also patch the heap allocation function so that we can keep tabs on
 * the amount of free heap.
 *
 * PARAMETERS: None
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void
vEXC_Register(void)
{
#ifdef EXCEPTION_VECTORS_LOCATION_RAM
  /* Overwrite exception vectors, pointing them all at the generic handler */
  BUS_ERROR = (uint32)exception_handler;
  UNALIGNED_ACCESS = (uint32)exception_handler;
  ILLEGAL_INSTRUCTION = (uint32)exception_handler;
  SYSCALL = (uint32)exception_handler;
  TRAP = (uint32)exception_handler;
  GENERIC = (uint32)exception_handler;
  STACK_OVERFLOW = (uint32)exception_handler;
#endif /* EXCEPTION_VECTORS_LOCATION */

  prHeap_AllocOrig = prHeap_AllocFunc;
  prHeap_AllocFunc = heap_alloc_overflow_protect;
}
#ifdef EXCEPTION_VECTORS_LOCATION_FLASH
/* If exception vectors are in flash, define the handler functions here to be linked in */
/* These function names are defined in the 6x linker script for the various exceptions */
/* Point them all at the generic handler */
PUBLIC void
vException_BusError(uint32 *pu32Stack, eExceptionType eType)
{
  exception_handler(pu32Stack, eType);
}
PUBLIC void
vException_UnalignedAccess(uint32 *pu32Stack, eExceptionType eType)
{
  exception_handler(pu32Stack, eType);
}
PUBLIC void
vException_IllegalInstruction(uint32 *pu32Stack, eExceptionType eType)
{
  exception_handler(pu32Stack, eType);
}
PUBLIC void
vException_SysCall(uint32 *pu32Stack, eExceptionType eType)
{
  exception_handler(pu32Stack, eType);
}
PUBLIC void
vException_Trap(uint32 *pu32Stack, eExceptionType eType)
{
  exception_handler(pu32Stack, eType);
}
PUBLIC void
vException_StackOverflow(uint32 *pu32Stack, eExceptionType eType)
{
  exception_handler(pu32Stack, eType);
}
#endif /* EXCEPTION_VECTORS_LOCATION_FLASH */

/****************************************************************************
 *
 * NAME: exception_handler
 *
 * DESCRIPTION:
 * Generic exception handler which is called whether the vectors are in RAM or flash
 *
 * PARAMETERS: None
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
static void
exception_handler(uint32 *pu32Stack, eExceptionType eType)
{
#if (defined EXC_DUMP_STACK) || (defined EXC_DUMP_REGS)
  int i;
#endif
  uint32 u32EPCR, u32EEAR, u32Stack;
  char *pcString;

  MICRO_DISABLE_INTERRUPTS();

  switch(eType) {
  case E_EXC_BUS_ERROR:
    pcString = "BUS";
    break;

  case E_EXC_UNALIGNED_ACCESS:
    pcString = "ALIGN";
    break;

  case E_EXC_ILLEGAL_INSTRUCTION:
    pcString = "ILLEGAL";
    break;

  case E_EXC_SYSCALL:
    pcString = "SYSCALL";
    break;

  case E_EXC_TRAP:
    pcString = "TRAP";
    break;

  case E_EXC_GENERIC:
    pcString = "GENERIC";
    break;

  case E_EXC_STACK_OVERFLOW:
    pcString = "STACK";
    break;

  default:
    pcString = "UNKNOWN";
    break;
  }

  if(bAHI_WatchdogResetEvent()) {
    pcString = "WATCHDOG";
  }
  vAHI_WatchdogStop();

  /* Pull the EPCR and EEAR values from where they've been saved by the ROM exception handler */
  u32EPCR = pu32Stack[PROGRAM_COUNTER];
  u32EEAR = pu32Stack[EFFECTIVE_ADDR];
  u32Stack = pu32Stack[STACK_REG];

  /* Log the exception */
  printstring("\n\n\n");
  printstring(pcString);
  printstring(" EXCEPTION @ $");
  hexprint32(u32EPCR);
  printstring("  EA: ");
  hexprint32(u32EEAR);
  printstring("  SK: ");
  hexprint32(u32Stack);
  printstring("  HP: ");
  hexprint32(((uint32 *)&heap_location)[0]);
  printstring("\n");
  printstring(" File: ");
  printstring(debug_filename);
  printstring(" Line: ");
  hexprint32(debug_line);
  printstring("\n");

#ifdef EXC_DUMP_REGS
  printstring("\nREGS: ");
  /* Pull and print the registers from saved locations */
  for(i = 0; i < REG_COUNT; i += 4) {
    printstring("R");
    hexprint(i);
    printstring("-");
    hexprint(i + 3);
    printstring(": ");
    hexprint(pu32Stack[i]);
    printstring("  ");
    hexprint32(pu32Stack[i + 1]);
    printstring("  ");
    hexprint32(pu32Stack[i + 2]);
    printstring("  ");
    hexprint32(pu32Stack[i + 3]);
    printstring("\n");
  }
#endif

#ifdef EXC_DUMP_STACK
  /* Print the stack */
  printstring("\nRAM top: ");
  hexprint32(EXCEPTION_RAM_TOP);
  printstring("\nSTACK: \n");
  pu32Stack = (uint32 *)(u32Stack & 0xFFFFFFF0);
  for(i = 0; (pu32Stack + i) < (uint32 *)(EXCEPTION_RAM_TOP); i += 4) {
    printstring("@");
    hexprint32((uint32)(pu32Stack + i));
    printstring(": ");
    hexprint32(pu32Stack[i]);
    printstring("  ");
    hexprint32(pu32Stack[i + 1]);
    printstring("  ");
    hexprint32(pu32Stack[i + 2]);
    printstring("  ");
    hexprint32(pu32Stack[i + 3]);
    printstring("\n");
  }
#endif

  if(eType == E_EXC_SYSCALL) {
    return;
  }

#if EXCEPTION_STALLS_SYSTEM
  while(1) {
  }
#else /* EXCEPTION_STALLS_SYSTEM */
  /* Software reset */
  vAHI_WatchdogException(0);
  vAHI_SwReset();
#endif /* EXCEPTION_STALLS_SYSTEM */
}
/****************************************************************************
 *
 * NAME: heap_alloc_overflow_protect
 *
 * DESCRIPTION:
 * New heap allocation function that sets the stack overflow location to the new
 * top address of the heap.
 *
 * PARAMETERS:  Name             RW  Usage
 *              pvPointer    W   Location of allocated heap memory
 *              u32Size          R   Number of bytes to allocate
 *              bClear           R   Flag to set new memory to 0
 *
 * RETURNS:
 * Pointer to new memory
 *
 ****************************************************************************/
static void *
heap_alloc_overflow_protect(void *pvPointer, uint32 u32Size, bool_t bClear)
{
  void *pvAlloc;
  /* Call original heap allocation function */
  pvAlloc = prHeap_AllocOrig(pvPointer, u32Size, bClear);
  /*
   * Initialise the stack overflow exception to trigger if the end of the
   * stack is reached. See the linker command file to adjust the allocated
   * stack size.
   */
  /* Set stack overflow address */
  vAHI_SetStackOverflow(TRUE, ((uint32 *)&heap_location)[0]);
  return pvAlloc;
}
