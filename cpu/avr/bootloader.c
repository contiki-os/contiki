#include "bootloader.h"
#include "dev/watchdog.h"
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "dev/usb/usb_drv.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

/* Not all AVR toolchains alias MCUSR to the older MSUSCR name */
#if !defined (MCUSR) && defined (MCUCSR)
#warning *** MCUSR not defined, using MCUCSR instead ***
#define MCUSR MCUCSR
#endif

#ifndef EEPROM_MAGIC_BYTE_ADDR
#define EEPROM_MAGIC_BYTE_ADDR   (uint8_t*)(E2END-3)
#endif

volatile uint32_t Boot_Key ATTR_NO_INIT;

extern void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);

bool
bootloader_is_present(void)
{
#if defined(BOOTLOADER_START_ADDRESS)
  return pgm_read_word_far(BOOTLOADER_START_ADDRESS) != 0xFFFF;
#else
  return false;
#endif
}

void
Jump_To_Bootloader(void)
{
  /* Disable all interrupts */
  cli();

#ifdef UDCON
  /* If USB is used, detach from the bus */
  Usb_detach();

  uint8_t i;

  /* Wait two seconds for the USB detachment to register on the host */
  for(i = 0; i < 200; i++) {
    _delay_ms(10);
    watchdog_periodic();
  }
#endif

  /* Set the bootloader key to the magic value and force a reset */
  Boot_Key = MAGIC_BOOT_KEY;

  eeprom_write_byte(EEPROM_MAGIC_BYTE_ADDR, 0xFF);

  /* Enable interrupts */
  sei();

  watchdog_reboot();
}

void
Bootloader_Jump_Check(void)
{
  /* If the reset source was the bootloader and the key is correct,
   * clear it and jump to the bootloader
   */
  if(MCUSR & (1 << WDRF)) {
    MCUSR = 0;
    if(Boot_Key == MAGIC_BOOT_KEY) {
      Boot_Key = 0;
      wdt_disable();

      /* Disable all interrupts */
      cli();

      eeprom_write_byte(EEPROM_MAGIC_BYTE_ADDR, 0xFF);

      /* Enable interrupts */
      sei();

      ((void (*)(void))(BOOTLOADER_START_ADDRESS)) ();
    } else {
      /* The watchdog fired. Probably means we
       * crashed. Wait two seconds before continuing.
	   */

      Boot_Key++;
      uint8_t i;

      for(i = 0; i < 200; i++) {
        _delay_ms(10);
        watchdog_periodic();
      }
    }
  } else {
    Boot_Key = MAGIC_BOOT_KEY - 4;
  }
}
