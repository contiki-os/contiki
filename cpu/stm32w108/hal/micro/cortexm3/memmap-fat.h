/** @file hal/micro/cortexm3/memmap-fat.h
 * @brief STM32W108 series memory map fixed address table definition
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#ifndef __MEMMAP_FAT_H__
#define __MEMMAP_FAT_H__

#ifndef __MEMMAP_TABLES_H__
  // if we weren't included via memmap-tables.h, define a dummy type for the BAT
  typedef void HalBootloaderAddressTableType;
#endif

#include "hal/micro/cortexm3/bootloader/fib-bootloader.h"

// ****************************************************************************
// If any of these address table definitions ever need to change, it is highly
// desirable to only add new entries, and only add them on to the end of an
// existing address table... this will provide the best compatibility with
// any existing code which may utilize the tables, and which may not be able to 
// be updated to understand a new format (example: bootloader which reads the 
// application address table)


// Description of the Fixed Address Table (FAT)
typedef struct {
  HalBaseAddressTableType baseTable;
  void *CustomerInformationBlock;
  HalBootloaderAddressTableType *bootloaderAddressTable;
  void *startOfUnusedRam;
  // ** pointers to shared functions **
  FibStatus (* fibFlashWrite)(int32u address, int8u *data,
                              int32u writeLength, int32u verifyLength);
  FibStatus (* fibFlashErase)(FibEraseType eraseType, int32u address);
} HalFixedAddressTableType;

extern const HalFixedAddressTableType halFixedAddressTable; 

#define FIXED_ADDRESS_TABLE_TYPE        (0x0FA7)

// The current versions of the address tables.
// Note that the major version should be updated only when a non-backwards
// compatible change is introduced (like removing or rearranging fields)
// adding new fields is usually backwards compatible, and their presence can
// be indicated by incrementing only the minor version
#define FAT_VERSION                     (0x0003)
#define FAT_MAJOR_VERSION               (0x0000)
#define FAT_MAJOR_VERSION_MASK          (0xFF00)


#endif //__MEMMAP_FAT_H__
