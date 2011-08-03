#include "bootloader.h"
#include "dev/watchdog.h"
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "dev/usb/usb_drv.h"

//Not all AVR toolchains alias MCUSR to the older MSUSCR name
//#if defined (__AVR_ATmega8__) || defined (__AVR_ATmega8515__) || defined (__AVR_ATmega16__)
#if !defined (MCUSR) && defined (MCUCSR)
#warning *** MCUSR not defined, using MCUCSR instead ***
#define MCUSR MCUCSR
#endif

volatile uint32_t Boot_Key ATTR_NO_INIT;

bool
bootloader_is_present(void) {
#if defined(RAMPZ) 
	return pgm_read_word_far(BOOTLOADER_START_ADDRESS)!=0xFFFF;
#else
/* Probably can just return false when < 64K flash */
//	return pgm_read_word_near(BOOTLOADER_START_ADDRESS)!=0xFFFF;
	return false;
#endif
}
void
Jump_To_Bootloader(void)
{
	uint8_t i;
	
#ifdef UDCON
	// If USB is used, detach from the bus
	Usb_detach();
#endif

	// Disable all interrupts
	cli();

	// Set the bootloader key to the magic value and force a reset
	Boot_Key = MAGIC_BOOT_KEY;

	// Wait two seconds for the USB detachment to register on the host
	for (i = 0; i < 128; i++)
		_delay_ms(16);

	// Set the bootloader key to the magic value and force a reset
	Boot_Key = MAGIC_BOOT_KEY;
	
	watchdog_reboot();
}

extern void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);

void
Bootloader_Jump_Check(void)
{
	// If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
	if(MCUSR & (1<<WDRF)) {
		MCUSR = 0;
		if(Boot_Key == MAGIC_BOOT_KEY) {
			Boot_Key = 0;
			wdt_disable();
			
			((void (*)(void))BOOTLOADER_START_ADDRESS)();
		} else {
			Boot_Key++;
		}
	} else {
		Boot_Key = MAGIC_BOOT_KEY-4;
	}
}
