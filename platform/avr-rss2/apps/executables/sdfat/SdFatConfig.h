/* Arduino SdFat Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * \file
 * \brief configuration definitions
 */
#ifndef SdFatConfig_h
#define SdFatConfig_h
#include <stdint.h>
#ifdef __AVR__
#include <avr/io.h>
#endif  // __AVR__
//------------------------------------------------------------------------------
/**
 * Set USE_LONG_FILE_NAMES nonzero to use long file names (LFN).
 * Long File Name are limited to a maximum length of 255 characters.
 *
 * This implementation allows 7-bit characters in the range
 * 0X20 to 0X7E except the following characters are not allowed:
 *
 *  < (less than)
 *  > (greater than)
 *  : (colon)
 *  " (double quote)
 *  / (forward slash)
 *  \ (backslash)
 *  | (vertical bar or pipe)
 *  ? (question mark)
 *  * (asterisk)
 *
 */
#define USE_LONG_FILE_NAMES 1
//------------------------------------------------------------------------------
/**
 * Set ARDUINO_FILE_USES_STREAM nonzero to use Stream as the base class
 * for the Arduino File class.  If ARDUINO_FILE_USES_STREAM is zero, Print
 * will be used as the base class for the Arduino File class.
 *
 * You can save some flash if you do not use Stream input functions such as
 * find(), findUntil(), readBytesUntil(), readString(), readStringUntil(),
 * parseInt(), and parseFloat().
 */
#define ARDUINO_FILE_USES_STREAM 1
//------------------------------------------------------------------------------
/**
 * The symbol SD_SPI_CONFIGURATION defines SPI access to the SD card.
 *
 * IF SD_SPI_CONFIGUTATION is define to be zero, only the SdFat class
 * is define and SdFat uses a fast custom SPI implementation if avaiable.
 * If SD_HAS_CUSTOM_SPI is zero, the standard SPI library is used.
 *
 * If SD_SPI_CONFIGURATION is define to be one, only the SdFat class is
 * define and SdFat uses the standard Arduino SPI.h library.
 *
 * If SD_SPI_CONFIGURATION is define to be two, only the SdFat class is
 * define and SdFat uses software SPI on the pins defined below.
 *
 * If SD_SPI_CONFIGURATION is define to be three, the three classes, SdFat,
 * SdFatLibSpi, and SdFatSoftSpi are defined.  SdFat uses the fast
 * custom SPI implementation. SdFatLibSpi uses the standard Arduino SPI
 * library.  SdFatSoftSpi is a template class that uses Software SPI. The
 * template parameters define the software SPI pins.  See the ThreeCard
 * example for simultaneous use of all three classes.
 */
#define SD_SPI_CONFIGURATION 3
//------------------------------------------------------------------------------
/**
 * If SD_SPI_CONFIGURATION is defined to be two, these definitions
 * will define the pins used for software SPI.
 *
 * The default definition allows Uno shields to be used on other boards.
 */
/** Software SPI Master Out Slave In pin */
uint8_t const SOFT_SPI_MOSI_PIN = 11;
/** Software SPI Master In Slave Out pin */
uint8_t const SOFT_SPI_MISO_PIN = 12;
/** Software SPI Clock pin */
uint8_t const SOFT_SPI_SCK_PIN = 13;
//------------------------------------------------------------------------------
/** 
 * Set MAINTAIN_FREE_CLUSTER_COUNT nonzero to keep the count of free clusters
 * updated.  This will increase the speed of the freeClusterCount() call
 * after the first call.  Extra flash will be required.
 */
#define MAINTAIN_FREE_CLUSTER_COUNT 1
//------------------------------------------------------------------------------
/**
 * To enable SD card CRC checking set USE_SD_CRC nonzero.
 *
 * Set USE_SD_CRC to 1 to use a smaller slower CRC-CCITT function.
 *
 * Set USE_SD_CRC to 2 to used a larger faster table driven CRC-CCITT function.
 */
#define USE_SD_CRC 0
//------------------------------------------------------------------------------
/**
 * Set ENABLE_SPI_TRANSACTIONS nonzero to enable the SPI transaction feature
 * of the standard Arduino SPI library.  You must include SPI.h in your
 * programs when ENABLE_SPI_TRANSACTIONS is nonzero.
 */
#define ENABLE_SPI_TRANSACTIONS 0
//------------------------------------------------------------------------------
/**
 * Set FAT12_SUPPORT nonzero to enable use if FAT12 volumes.
 * FAT12 has not been well tested and requires additional flash.
 */
#define FAT12_SUPPORT 0
//------------------------------------------------------------------------------
/**
 * Set DESTRUCTOR_CLOSES_FILE nonzero to close a file in its destructor.
 *
 * Causes use of lots of heap in ARM.
 */
#define DESTRUCTOR_CLOSES_FILE 0
//------------------------------------------------------------------------------
/**
 * Call flush for endl if ENDL_CALLS_FLUSH is nonzero
 *
 * The standard for iostreams is to call flush.  This is very costly for
 * SdFat.  Each call to flush causes 2048 bytes of I/O to the SD.
 *
 * SdFat has a single 512 byte buffer for SD I/O so it must write the current
 * data block to the SD, read the directory block from the SD, update the
 * directory entry, write the directory block to the SD and read the data
 * block back into the buffer.
 *
 * The SD flash memory controller is not designed for this many rewrites
 * so performance may be reduced by more than a factor of 100.
 *
 * If ENDL_CALLS_FLUSH is zero, you must call flush and/or close to force
 * all data to be written to the SD.
 */
#define ENDL_CALLS_FLUSH 0
//------------------------------------------------------------------------------
/**
 * SPI SCK divisor for SD initialization commands.
 * or greater
 */
#ifdef __AVR__
const uint8_t SPI_SCK_INIT_DIVISOR = 64;
#else
const uint8_t SPI_SCK_INIT_DIVISOR = 128;
#endif
//------------------------------------------------------------------------------
/**
 * Set USE_SEPARATE_FAT_CACHE nonzero to use a second 512 byte cache
 * for FAT table entries.  This improves performance for large writes
 * that are not a multiple of 512 bytes.
 */
#ifdef __arm__
#define USE_SEPARATE_FAT_CACHE 1
#else  // __arm__
#define USE_SEPARATE_FAT_CACHE 0
#endif  // __arm__
//------------------------------------------------------------------------------
/**
 * Set USE_MULTI_BLOCK_IO nonzero to use multi-block SD read/write.
 *
 * Don't use mult-block read/write on small AVR boards.
 */
#if defined(RAMEND) && RAMEND < 3000
#define USE_MULTI_BLOCK_IO 0
#else  // RAMEND
#define USE_MULTI_BLOCK_IO 1
#endif  // RAMEND
//------------------------------------------------------------------------------
/**
 * Determine the default SPI configuration.
 */
#if defined(__AVR__)\
  || defined(__SAM3X8E__) || defined(__SAM3X8H__)\
  || (defined(__arm__) && defined(CORE_TEENSY))\
  || defined(__STM32F1__)\
  || defined(PLATFORM_ID)\
  || defined(DOXYGEN)
// Use custom fast implementation.
#define SD_HAS_CUSTOM_SPI 1
#else  // SD_HAS_CUSTOM_SPI
// Use standard SPI library.
#define SD_HAS_CUSTOM_SPI 0
#endif  // SD_HAS_CUSTOM_SPI
//-----------------------------------------------------------------------------
/**
 *  Number of hardware interfaces.
 */
#if defined(PLATFORM_ID)
#if Wiring_SPI1 && Wiring_SPI2
#define SPI_INTERFACE_COUNT 3
#elif Wiring_SPI1
#define SPI_INTERFACE_COUNT 2
#endif  // Wiring_SPI1 && Wiring_SPI2
#endif  // defined(PLATFORM_ID)
// default is one
#ifndef SPI_INTERFACE_COUNT
#define SPI_INTERFACE_COUNT 1
#endif  // SPI_INTERFACE_COUNT
//------------------------------------------------------------------------------
/**
 * Check if API to select HW SPI interface is needed.
 */
#if SPI_INTERFACE_COUNT > 1 && SD_HAS_CUSTOM_SPI\
  && SD_SPI_CONFIGURATION != 1 && SD_SPI_CONFIGURATION != 2
#define IMPLEMENT_SPI_INTERFACE_SELECTION 1
#else  // SPI_INTERFACE_COUNT > 1
#define IMPLEMENT_SPI_INTERFACE_SELECTION 0
#endif  // SPI_INTERFACE_COUNT > 1
#endif  // SdFatConfig_h
