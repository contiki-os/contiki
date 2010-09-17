#ifndef __AVR_BOOTLOADER_H__
#define __AVR_BOOTLOADER_H__

#include <inttypes.h>
#include <avr/io.h>
#include <stdbool.h>

#define 	ATTR_INIT_SECTION(SectionIndex)   __attribute__ ((naked, section (".init" #SectionIndex )))
#define 	ATTR_NO_INIT   __attribute__ ((section (".noinit")))

#ifndef BOOTLOADER_SEC_SIZE_BYTES
#define BOOTLOADER_SEC_SIZE_BYTES				(0x1000)
#endif

#ifndef BOOTLOADER_START_ADDRESS
#define BOOTLOADER_START_ADDRESS	(FLASHEND-BOOTLOADER_SEC_SIZE_BYTES+1)
#endif

#define MAGIC_BOOT_KEY            0xDC42ACCA

extern void Jump_To_Bootloader(void);
extern bool bootloader_is_present(void);

#endif