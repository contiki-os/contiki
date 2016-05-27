#ifndef OTA_H
#define OTA_H

#if DEBUG
  #include <stdio.h>
  #define PRINTF(...) printf(__VA_ARGS__)
#else
  #define PRINTF(...)
#endif

#include "ext-flash.h"
#include "driverlib/flash.h"

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
  uint16_t size;            //  Size of firmware image
  uint16_t version;         //  Integer representing firmware version
  uint32_t uid;             //  Integer representing unique firmware ID
  uint16_t offset;          //  At what flash sector does this image reside?
} OTAMetadata_t;
#define OTA_METADATA_LENGTH sizeof(OTAMetadata_t)  //  Length of OTA image metadata in bytes (CRC, version, data, etc.)

extern void
FlashRead(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count);

extern void
print_metadata( OTAMetadata_t *metadata );

extern void
generate_fake_metadata();

extern int
update_firmware( uint8_t ota_slot );

extern int
store_firmware_page( uint32_t ext_address, uint8_t *page_data );

extern void
jump_to_image(uint32_t destination_address);

#endif
