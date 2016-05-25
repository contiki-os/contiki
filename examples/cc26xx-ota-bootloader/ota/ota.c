#include "ota.h"

uint8_t ota_images[3] = OTA_ADDRESSES;

/**
 *    OTA Flash-specific Functions
 */
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

/**
 *    OTA Metadata Functions
 */
/**
 *  Print information read from an OTA metadata object.
 */
void
print_metadata( OTAMetadata_t *metadata ) {
  PRINTF("Firmware Version: %u\n", metadata->version);
  PRINTF("Firmware UID: %lu\n", metadata->uid);
  PRINTF("Firmware Location: %u\n", metadata->offset);
  PRINTF("Firmware Size: %u\n", metadata->size);
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
      PRINTF("[external-flash]:\tError - could not access EEPROM.\n");
      ext_flash_close();
    }

    eeprom_access = ext_flash_erase( (ota_images[i] << 12), sizeof(OTAMetadata_t));

    if(!eeprom_access) {
      PRINTF("[external-flash]:\tError - Could not erase EEPROM.\n");
      ext_flash_close();
    }

    eeprom_access = ext_flash_write( (ota_images[i] << 12), sizeof(OTAMetadata_t),
                         (uint8_t *)&ota_metadata);

    if(!eeprom_access) {
      PRINTF("[external-flash]:\tError - Could not write to EEPROM.\n");
      ext_flash_close();
    }

    ext_flash_close();
  }
}
