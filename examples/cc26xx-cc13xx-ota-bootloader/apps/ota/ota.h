/** @file   ota.h
 *  @brief  OTA Image R/W and Verification
 *  @author Mark Solters <msolters@gmail.com>
 */

#ifndef OTA_H
#define OTA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 *    OTA Image Memory Map
 */
//  Internal Flash
#define CURRENT_FIRMWARE    0x2          //  Address where the current system firmware is stored in internal flash
//  External Flash
#define GOLDEN_IMAGE        0x19           //  Address where the factory-given firmware is stored in external flash (for backup)
#define OTA_ADDRESSES       { 0x32, 0x4B, 0x64 }
extern uint8_t ota_images[3]; //  Addresses where OTA updates are stored in external flash

#define OTA_RESET_VECTOR    0x4     //  RESET ISR Vector (see )
#define FLASH_PAGE_SIZE     0x1000

typedef struct OTAMetadata {
  uint16_t crc;             //
  uint16_t crc_shadow;      //
  uint32_t size;            //  Size of firmware image
  uint32_t uuid;             //  Integer representing unique firmware ID
  uint16_t version;         //  Integer representing firmware version
} OTAMetadata_t;
/**
 *  OTA_METADATA_SPACE:
 *    We allow some space for the OTA metadata, which is placed immediately
 *    before firmware image content.  However, the VTOR table in firmware must
 *    be 256-byte aligned.  So, the minimum space we can set for metadata is
 *    0x100, not the mere 0x10 that it requires.
 *
 *  OTA_METADATA_LENGTH:
 *    This is just the size of the OTAMetadata_t struct, which is 4-byte
 *    aligned.  We use 14 bytes currently, so this struct will be 16 bytes.
 */
#define OTA_METADATA_SPACE 0x100
#define OTA_METADATA_LENGTH sizeof(OTAMetadata_t)

void FlashRead(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count);

uint16_t crc16(uint16_t crc, uint8_t val);

void print_metadata( OTAMetadata_t *metadata );

int get_current_metadata( OTAMetadata_t *ota_slot_metadata  );

int get_ota_slot_metadata( uint8_t ota_slot, OTAMetadata_t *ota_slot_metadata );

int overwrite_ota_slot_metadata( uint8_t ota_slot, OTAMetadata_t *ota_slot_metadata );

int backup_golden_image();

int verify_current_firmware( OTAMetadata_t *current_firmware_metadata );

int verify_ota_slot( uint8_t ota_slot );

bool validate_ota_metadata( OTAMetadata_t *metadata );

int find_matching_ota_slot( uint16_t version );

int find_empty_ota_slot();

int find_oldest_ota_image();

int find_newest_ota_image();

int erase_extflash_page( uint32_t ext_address );

int update_firmware( uint8_t ota_slot );

int erase_ota_image( uint8_t ota_slot );

int store_firmware_data( uint32_t ext_address, uint8_t *data, size_t data_length );

void jump_to_image(uint32_t destination_address);

#endif
