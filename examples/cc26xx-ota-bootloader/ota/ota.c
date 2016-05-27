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

/**
 *    Copies a stored firmware image from external flash into internal flash.
 *    OTA image to copy is specified by OTA slot index (0, 1, or 2).
 */
int
update_firmware( uint8_t ota_slot ) {
  uint32_t ota_image_address = ota_images[ ota_slot ] << 12;

  //  (1) Get metadata about the new version
  //OTAMetadata_t new_firmware;
  //FlashRead( (uint8_t *)&new_firmware, ota_image_address, OTA_METADATA_LENGTH );

  //  (2) Validate the new firmware (CRC)
  //  return -1 if not valid!


  //  (3) Erase internal user pages from 0x02000 to 0x27000
  //      Overwrite them with the corresponding image pages from
  //      external flash.
  uint8_t sector_num;
  uint8_t page_data[ FLASH_PAGE_SIZE ];
  //  Each firmware image is 25 pages big at most
  for (sector_num=0; sector_num<25; sector_num++) {
    //  Erase internal flash page
    FlashSectorErase( (sector_num+CURRENT_FIRMWARE) << 12 );

    //  Read one page from the new external flash image
    int eeprom_access = ext_flash_open();

    if(!eeprom_access) {
      PRINTF("[external-flash]:\tError - Could not access EEPROM.\n");
      ext_flash_close();
    }

    eeprom_access = ext_flash_read( (ota_image_address + (sector_num << 12)), FLASH_PAGE_SIZE, (uint8_t *)&page_data);

    ext_flash_close();

    if(!eeprom_access) {
      PRINTF("[external-flash]:\tError - Could not read EEPROM.\n");
      ext_flash_close();
    }

    FlashProgram( page_data, ((sector_num+CURRENT_FIRMWARE)<<12), FLASH_PAGE_SIZE );
  }

  //  (4) Reboot


  return 0;
}

/**
 *    Store one page of firmware data in external flash.
 *      ext_address is the flash address to store the data
 *      page_data is a pointer to the data buffer to be written.
 *    NOTE: Only 1 page (the first 4096 bytes) of the buffer will be written.
 */
int
store_firmware_page( uint32_t ext_address, uint8_t *page_data ) {

  //  (1) Erase external flash page first!
  int eeprom_access;
  eeprom_access = ext_flash_open();

  if(!eeprom_access) {
    PRINTF("[external-flash]:\tError - could not access EEPROM.\n");
    ext_flash_close();
  }

  eeprom_access = ext_flash_erase( ext_address, FLASH_PAGE_SIZE );

  if(!eeprom_access) {
    PRINTF("[external-flash]:\tError - Could not erase EEPROM.\n");
    ext_flash_close();
  }

  //  (2) Copy page_data into external flash chip.
  eeprom_access = ext_flash_write( ext_address, FLASH_PAGE_SIZE, page_data );

  if(!eeprom_access) {
    PRINTF("[external-flash]:\tError - Could not write to EEPROM.\n");
    ext_flash_close();
  }

  ext_flash_close();
  return 0;
}

/**
 *    Begin executing a new firmware image located at destination_address.
 */
void
jump_to_image(uint32_t destination_address)
{
  destination_address += /*OTA_METADATA_LENGTH +*/ OTA_RESET_VECTOR;
  __asm("LDR R0, [%[dest]]"::[dest]"r"(destination_address)); //  Load the destination address
  __asm("ORR R0, #1");                                        //  Make sure the Thumb State bit is set.
  __asm("BX R0");                                             //  Branch execution
}
