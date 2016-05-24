/**
 *    Example Contiki 3.0 Firmware
 *    Blinks the LEDs @ 1Hz.

 *    If we see the LED blinking, we know
 *    the bootloader loaded our code!
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "ti-lib.h"
#include "gpio.h"
#include "sys/ctimer.h"
#include "sys/timer.h"

#include "ext-flash.h"
#include "driverlib/flash.h"

#define BLINKER_PIN	BOARD_LED_1

PROCESS(blinker_test_loop, "GPIO Blinker Lifecycle");
AUTOSTART_PROCESSES(&blinker_test_loop);

struct ctimer blink_timer;
bool blink_state = false;

void
blink_looper()
{
  GPIOPinWrite( BLINKER_PIN, blink_state );
  blink_state = !blink_state;
  ctimer_reset( &blink_timer );
}



/**
 *        Bootloader Testing!
 */


/**
 *    OTA Image Memory Map
 */
#define CURRENT_FIRMWARE    0x2000          //  Address where the current system firmware is stored in internal flash
//  External Flash Addresses (To get true flash address << 12 these numbers)
#define GOLDEN_IMAGE        0x19;           //  Address where the factory-given firmware is stored in external flash (for backup)
uint8_t ota_images[] = { 0x32, 0x4B, 0x64 };  //  Addresses where OTA updates are stored in external flash

typedef struct OTAMetadata {
  uint16_t crc;             //
  uint16_t crc_shadow;      //
  uint16_t size;            //  Size of firmware image
  uint16_t version;         //  Integer representing firmware version
  uint32_t uid;             //  Integer representing unique firmware ID
  uint16_t offset;          //  At what flash sector does this image reside?
} OTAMetadata_t;
#define OTA_METADATA_LENGTH sizeof(OTAMetadata_t)  //  Length of OTA image metadata in bytes (CRC, version, data, etc.)

OTAMetadata_t current_firmware;

/**
 *    A helper function to read from the CC26xx Internal Flash.
 *      pui8DataBuffer: Pointer to uint8_t array to store flash data in.
 *      ui32Address: Flash address to begin reading from.
 *      ui32Count: Number of bytes to read from flash, starting at ui32Address.
 */
void
FlashRead(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count) {
  uint8_t *pui8ReadAddress = (uint8_t *)ui32Address;
  while (ui32Count--) {
    *pui8DataBuffer++ = *pui8ReadAddress++;
  }
}


void
print_metadata( OTAMetadata_t *metadata ) {
  printf("Firmware Version: %u\n", metadata->version);
  printf("Firmware UID: %lu\n", metadata->uid);
  printf("Firmware Location: %u\n", metadata->offset);
  printf("Firmware Size: %u\n", metadata->size);
}

void
generate_fake_metadata() {
  //  Write some temp. fake metadata to the OTA slots
  for (int i=0; i<sizeof(ota_images); i++) {
    //  (1) Make fake metadata
    OTAMetadata_t ota_metadata = {
      0, 0, 0x27, i, (0x1230 + i), 0x2000
    };

    //  (2) Write data to external flash
    int eeprom_access;
    eeprom_access = ext_flash_open();

    if(!eeprom_access) {
      printf("[external-flash]:\tError - could not access EEPROM.\n");
      ext_flash_close();
    }

    eeprom_access = ext_flash_erase( (ota_images[i] << 12), sizeof(OTAMetadata_t));

    if(!eeprom_access) {
      printf("[external-flash]:\tError - Could not erase EEPROM.\n");
      ext_flash_close();
    }

    eeprom_access = ext_flash_write( (ota_images[i] << 12), sizeof(OTAMetadata_t),
                         (uint8_t *)&ota_metadata);

    if(!eeprom_access) {
      printf("[external-flash]:\tError - Could not write to EEPROM.\n");
      ext_flash_close();
    }

    ext_flash_close();
  }
}

PROCESS_THREAD(blinker_test_loop, ev, data)
{
  PROCESS_BEGIN();

  //	(1)	UART Output
  printf("OTA Image Example: Starting\n");

  //	(2)	Start blinking green LED
	GPIODirModeSet( BLINKER_PIN, GPIO_DIR_MODE_OUT);
  ctimer_set( &blink_timer, (CLOCK_SECOND/2), blink_looper, NULL);





  //  (1) Get metadata about the current firmware version
  FlashRead( (uint8_t *)&current_firmware, 0x1000, OTA_METADATA_LENGTH );
  printf("\nCurrent Firmware\n");
  print_metadata( &current_firmware );

  ext_flash_init();

  //generate_fake_metadata();

  //  (2) Get metadata about OTA firmwares stored in the external flash.
  for (int img=0; img<sizeof(ota_images); img++) {
    OTAMetadata_t ota_firmare;

    /* Make sure the external flash is in the lower power mode */
    int eeprom_access = ext_flash_open();

    if(!eeprom_access) {
      printf("[external-flash]:\tError - Could not access EEPROM.\n");
      ext_flash_close();
      return false;
    }

    eeprom_access = ext_flash_read((ota_images[img] << 12), OTA_METADATA_LENGTH, (uint8_t *)&ota_firmare);

    ext_flash_close();

    if(!eeprom_access) {
      printf("[external-flash]:\tError - Could not read EEPROM.\n");
      ext_flash_close();
    }

    printf("\nOTA Firmware Slot %u\n", img);
    print_metadata( &ota_firmare );
  }

  PROCESS_END();
}
