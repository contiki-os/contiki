/** @file hal/micro/cortexm3/memmap.h
 * @brief STM32W108 series memory map definitions used by the full hal
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#ifndef __MEMMAP_H__
#define __MEMMAP_H__

// Include the chip specific definitions
#ifndef LOADER
  #if defined (CORTEXM3_STM32W108)
    #include "hal/micro/cortexm3/stm32w108/memmap.h"
  #elif defined (CORTEXM3_STM32F103)
    #include "hal/micro/cortexm3/stm32f103ret/memmap.h"
  #else
    #error no appropriate micro defined
  #endif
#endif

//=============================================================================
// A union that describes the entries of the vector table.  The union is needed
// since the first entry is the stack pointer and the remainder are function
// pointers.
//
// Normally the vectorTable below would require entries such as:
//   { .topOfStack = x },
//   { .ptrToHandler = y },
// But since ptrToHandler is defined first in the union, this is the default
// type which means we don't need to use the full, explicit entry.  This makes
// the vector table easier to read because it's simply a list of the handler
// functions.  topOfStack, though, is the second definition in the union so
// the full entry must be used in the vectorTable.
//=============================================================================
typedef union
{
  void (*ptrToHandler)(void);
  void *topOfStack;
} HalVectorTableType;


// ****************************************************************************
// If any of these address table definitions ever need to change, it is highly
// desirable to only add new entries, and only add them on to the end of an
// existing address table... this will provide the best compatibility with
// any existing code which may utilize the tables, and which may not be able to 
// be updated to understand a new format (example: bootloader which reads the 
// application address table)

// Generic Address table definition which describes leading fields which
// are common to all address table types
typedef struct {
  void *topOfStack;
  void (*resetVector)(void);
  void (*nmiHandler)(void);
  void (*hardFaultHandler)(void);
  int16u type;
  int16u version;
  const HalVectorTableType *vectorTable;
  // Followed by more fields depending on the specific address table type
} HalBaseAddressTableType;

// Hal only references the FAT
#include "memmap-fat.h"

#endif //__MEMMMAP_H__

