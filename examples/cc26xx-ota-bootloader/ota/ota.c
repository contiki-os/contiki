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
FlashRead(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
{
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
print_metadata( OTAMetadata_t *metadata )
{
  PRINTF("Firmware Size: %#x\n", metadata->size);
  PRINTF("Firmware Version: %u\n", metadata->version);
  PRINTF("Firmware UUID: %#x\n", metadata->uuid);
}


/*******************************************************************************
 * @fn      get_ota_slot_metadata
 *
 * @brief   Get the metadata belonging to an OTA slot in external flash.
 *
 * @return  OTAMetadata_t object read from the corresponding ota_slot
 */
OTAMetadata_t
get_ota_slot_metadata( uint8_t ota_slot )
{
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t ota_image_address;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we want to copy over an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];
  } else {
    //  If ota_slot = 0, it means we want to copy over the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

  OTAMetadata_t ota_slot_metadata;

  int eeprom_access = ext_flash_open();

  if(!eeprom_access) {
    PRINTF("[external-flash]:\tError - Could not access EEPROM.\n");
    ext_flash_close();
  }

  eeprom_access = ext_flash_read( ota_image_address, OTA_METADATA_LENGTH, (uint8_t *)&ota_slot_metadata);

  if(!eeprom_access) {
    PRINTF("[external-flash]:\tError - Could not read EEPROM.\n");
    ext_flash_close();
  }

  ext_flash_close();

  return ota_slot_metadata;
}


/*******************************************************************************
 * @fn      validate_ota_slot
 *
 * @brief   Returns true only if the metadata provided indicates the OTA slot
 *          is populated and valid.
 *
 * @return  True if the OTA slot is populated and valid.  Otherwise, false.
 */
bool
validate_ota_slot( OTAMetadata_t *metadata )
{
  // First, we check to see if every byte in the metadata is 0xFF.
  // If this is the case, this metadata is "erased" and therefore we assume
  // the OTA slot to be empty.
  uint8_t *metadata_ptr = (uint8_t *)metadata;
  int b = OTA_METADATA_LENGTH;
  while (b--) {
    //printf("%u: %u, ", b, *(uint8_t *)metadata);
    if ( *metadata_ptr++ == 0xff ) {
      //  Do nothing!
    } else {
      //  We encountered a non-erased byte.  Let's assume this is valid.
      return true;
    }
  }

  //  If we get this far, all metadata bytes were cleared (0xff)
  return false;
}


/*******************************************************************************
 * @fn      find_empty_ota_slot
 *
 * @brief   Find the first empty OTA download slot.  Failing this, find the slot
 *          with the most out-of-date firmware version.
 *
 * @return  The OTA slot index of the empty/oldest OTA slot.  This will never be
 *          0 because the Golden Image should never be erased.
 */
int
find_empty_ota_slot()
{
  //  Iterate through each of the 3 OTA download slots.
  for ( int slot=1; slot<4 ; slot++ ) {

    //  (1) Get the metadata of the current OTA download slot.
    OTAMetadata_t ota_slot_metadata = get_ota_slot_metadata( slot );

    //  (2) Is this slot empty? If yes, return corresponding OTA index.
    if ( validate_ota_slot( &ota_slot_metadata ) == false ) {
      return slot;
    }
  }

  //  If execution goes this far, no empty slot was found.  Now, we look for
  //  the oldest OTA slot instead.
  return find_oldest_ota_image();
}

/*******************************************************************************
 * @fn      find_oldest_ota_image
 *
 * @brief   Find the OTA slot containing the most out-of-date firmware version.
 *          OTA slots are in external flash.
 *
 * @return  The OTA slot index of the oldest firmware version.  The oldest
 *          possible OTA slot is 0, which is the Golden Image.
 */
int
find_oldest_ota_image()
{
  //  By construction, the oldest firmware should be the Golden Image, v0
  int oldest_ota_slot = 0;
  uint16_t oldest_firmware_version = 0;

  //  Iterate through each of the 3 OTA download slots.
  for ( int slot=1; slot<4 ; slot++ ) {
    //  (1) Get the metadata of the current OTA download slot.
    OTAMetadata_t ota_slot_metadata = get_ota_slot_metadata( slot );

    //  (3) Is this slot populated? If not, skip.
    //  TODO

    //  (4) Is this the oldest non-Golden Image image we've found thus far?
    if ( oldest_firmware_version ) {
      if ( ota_slot_metadata.version < oldest_firmware_version ) {
        oldest_ota_slot = (slot+1);
        oldest_firmware_version = ota_slot_metadata.version;
      }
    } else {
      oldest_ota_slot = slot;
      oldest_firmware_version = ota_slot_metadata.version;
    }
  }

  PRINTF("Oldest OTA slot: #%u; Firmware v%u\n", oldest_ota_slot, oldest_firmware_version);

  return oldest_ota_slot;
}

/*******************************************************************************
 * @fn      find_newest_ota_image
 *
 * @brief   Find the OTA slot containing the most recent firmware version.
 *          OTA slots are in external flash.
 *
 * @return  The OTA slot index of the newest firmware version.
 */
int
find_newest_ota_image()
{
  //  At first, we only assume knowledge of the Golden Image, v0
  int newest_ota_slot = 0;
  uint16_t newest_firmware_version = 0;

  //  Iterate through each of the 3 OTA download slots.
  for ( int slot=1; slot<4 ; slot++ ) {
    //  (1) Get the metadata of the current OTA download slot.
    OTAMetadata_t ota_slot_metadata = get_ota_slot_metadata( slot );

    //  (2) Is this slot populated? If not, skip.
    if ( validate_ota_slot( &ota_slot_metadata) == false ) {
      continue;
    }

    //  (3) Is this the newest non-Golden Image image we've found thus far?
    if ( ota_slot_metadata.version > newest_firmware_version ) {
      newest_ota_slot = slot;
      newest_firmware_version = ota_slot_metadata.version;
    }
  }

  PRINTF("Newest OTA slot: #%u; Firmware v%u\n", newest_ota_slot, newest_firmware_version);
  return newest_ota_slot;
}


/*******************************************************************************
 * @fn      erase_ota_image
 *
 * @brief   Clear an OTA slot in external flash.
 *
 * @param   ota_slot    - The OTA slot index of the firmware image to be copied.
 *                          Only 1-3 are valid arguments, as the Golden Image
 *                          should never be erased.
 *
 * @return  0 or error code
 */
int
erase_ota_image( uint8_t ota_slot )
{
  //  (1) Get page address of the ota_slot in ext-flash
  uint8_t ota_image_base_address = ota_images[ (ota_slot-1) ];

  int eeprom_access;
  eeprom_access = ext_flash_open();
  if(!eeprom_access) {
    PRINTF("[external-flash]:\tError - could not access EEPROM.\n");
    ext_flash_close();
  }

  //  (2) Erase each page in the OTA download slot!
  for (int page=0; page<25; page++) {
    uint32_t ext_address = ( ota_image_base_address + page ) << 12;

    eeprom_access = ext_flash_erase( ext_address, FLASH_PAGE_SIZE );

    if(!eeprom_access) {
      PRINTF("[external-flash]:\tError - Could not erase EEPROM.\n");
      ext_flash_close();
    }
  }

  ext_flash_close();

  return 0;

}


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
update_firmware( uint8_t ota_slot )
{
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t ota_image_address;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we want to copy over an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];
  } else {
    //  If ota_slot = 0, it means we want to copy over the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

  //  (2) Get metadata about the new version
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

    if(!eeprom_access) {
      PRINTF("[external-flash]:\tError - Could not read EEPROM.\n");
      ext_flash_close();
    }

    ext_flash_close();

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
store_firmware_page( uint32_t ext_address, uint8_t *page_data )
{
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
