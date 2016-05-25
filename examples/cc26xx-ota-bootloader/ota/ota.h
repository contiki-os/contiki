#ifndef OTA_H
#define OTA_H

#include "ext-flash.h"
#include "driverlib/flash.h"

/**
 *    OTA Image Memory Map
 */
#define CURRENT_FIRMWARE    0x2000          //  Address where the current system firmware is stored in internal flash
//  External Flash Addresses (To get true flash address << 12 these numbers)
#define GOLDEN_IMAGE        0x19           //  Address where the factory-given firmware is stored in external flash (for backup)
#define OTA_ADDRESSES       { 0x32, 0x4B, 0x64 }
extern uint8_t ota_images[3]; //  Addresses where OTA updates are stored in external flash

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

#endif
