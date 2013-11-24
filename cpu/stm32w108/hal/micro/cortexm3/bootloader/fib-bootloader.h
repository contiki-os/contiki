/** @file hal/micro/cortexm3/bootloader/fib-bootloader.h
 * @brief Definition and description of FIB bootloader shared functions.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef FIB_BOOTLOADER_H_
#define FIB_BOOTLOADER_H_

//------------------------------------------------------------------------------
// Reset signatures.

#define FIB_RESET_GO_ZERO          0xF00F0100
// Reset signatures 0xF00F0100 to 0xF00F010F can be chosen using the go command.
#define FIB_RESET_GO_JUMP          0xF00F0110
#define FIB_RESET_BAUD_RATE        0xF00F0111
#define FIB_RESET_READ_UNPROTECT   0xF00F0112
#define FIB_RESET_GPIO_BOOTMODE    0xF00F0113
#define FIB_RESET_PART_DATA        0xF00F0114
#define FIB_RESET_NMI_HARD_FAULT   0xF00F0115

//------------------------------------------------------------------------------
// Status values.

typedef uint32_t FibStatus;

#define FIB_SUCCESS             0
#define FIB_ERR_UNALIGNED       1
#define FIB_ERR_INVALID_ADDRESS 2
#define FIB_ERR_INVALID_TYPE    3
#define FIB_ERR_WRITE_PROTECTED 4
#define FIB_ERR_WRITE_FAILED    5
#define FIB_ERR_ERASE_REQUIRED  6
#define FIB_ERR_VERIFY_FAILED   7

//------------------------------------------------------------------------------
// Erase types.

typedef uint32_t FibEraseType;

#define MFB_MASS_ERASE 0x01
#define MFB_PAGE_ERASE 0x02
#define CIB_ERASE      0x03

#define DO_ERASE  0x0100
#define DO_VERIFY 0x0200

//------------------------------------------------------------------------------
// Shared flash functions.

FibStatus fibFlashWrite(uint32_t address, uint8_t *data,
                        uint32_t writeLength, uint32_t verifyLength);

FibStatus fibFlashErase(FibEraseType eraseType, uint32_t address);

#endif //FIB_BOOTLOADER_H_
