#include "ota.h"

uint8_t ota_images[3] = OTA_ADDRESSES;

/**
 *    OTA Flash-specific Functions
 */
/*******************************************************************************
 * @fn      FlashRead
 *
 * @brief   Read data from the CC26xx Internal Flash.
 *
 * @param   pui8DataBuffer  - Pointer to uint8_t array to store flash data in.
 *
 * @param   ui32Address     - Flash address to begin reading from.
 *
 * @param   ui32Count       - Number of bytes to read from flash, starting at
 *                            ui32Address.
 *
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
/*******************************************************************************
 * @fn      print_metadata
 *
 * @brief   Print formatted OTA image metadata to UART.
 *
 */
void
print_metadata( OTAMetadata_t *metadata ) {
  PRINTF("Firmware Size: %#x\n", metadata->size);
  PRINTF("Firmware Version: %u\n", metadata->version);
  PRINTF("Firmware UID: %#x\n", metadata->uid);
}
/*
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
}*/

/*******************************************************************************
 * @fn      update_firmware
 *
 * @brief   Overwrite firmware located in internal flash with the firmware
 *          stored in an external flash OTA slot.
 *
 * @param   ota_slot    - The OTA slot index of the firmware image to be copied.
 *                          0 = "Golden Image" backup, aka factory restore
 *                          1, 2, 3 = OTA Download slots
 *
 * @return  0 or error code
 */
int
update_firmware( uint8_t ota_slot ) {
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t ota_image_address;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we want to copy over an OTA download
    ota_image_address = ota_images[ ota_slot ];
  } else {
    //  If ota_slot = 0, it means we want to copy over the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

  //  (2) Get metadata about the new version
  OTAMetadata_t new_firmware;
  FlashRead( (uint8_t *)&new_firmware, ota_image_address, OTA_METADATA_LENGTH );

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

/*******************************************************************************
 * @fn      store_firmware_page
 *
 * @brief   Store 4096 bytes of firmware data in external flash at the specified
 *          address.
 *
 * @param   ext_address   - External flash address to store page_data.
 *
 * @param   page_data     - Pointer to the data buffer to be written to ext_address.
 *                          Note: page_data can be larger than 4096 bytes, but
 *                          only the first 4096 bytes will be stored.
 *
 * @return  0 or error code
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


/*******************************************************************************
 * @fn      jump_to_image
 *
 * @brief   Begin executing another firmware binary located in internal flash.
 *
 * @param   destination_address - Internal flash address of the vector table for
 *                                the firmware binary that is to be booted into.
 *                                Since this OTA lib prepends metadata to each
 *                                binary, the true VTOR start address will be
 *                                OTA_METADATA_SPACE bytes past this address.
 *
 */
void
jump_to_image(uint32_t destination_address)
{
  if ( destination_address ) {
    //  Only add the metadata length offset if destination_address is NOT 0!
    //  (Jumping to 0x0 is used to reboot the device)
    destination_address += OTA_METADATA_SPACE;
  }
  destination_address += OTA_RESET_VECTOR;
  __asm("LDR R0, [%[dest]]"::[dest]"r"(destination_address)); //  Load the destination address
  __asm("ORR R0, #1");                                        //  Make sure the Thumb State bit is set.
  __asm("BX R0");                                             //  Branch execution
}
