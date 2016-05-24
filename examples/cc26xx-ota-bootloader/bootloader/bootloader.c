#include <stdint.h>

/**
 *    OTA Image Memory Map
 */
#define CURRENT_FIRMWARE    0x2000          //  Address where the current system firmware is stored in internal flash
//  External Flash Addresses (To get true flash address << 12 these numbers)
#define GOLDEN_IMAGE        0x19;           //  Address where the factory-given firmware is stored in external flash (for backup)
uint8_t ota_images = [ 0x32, 0x4B, 0x64 ];  //  Addresses where OTA updates are stored in external flash

/**
 *    OTA defines
 */
#define OTA_IMAGE_RESET_VECTOR    0x4     //  RESET ISR Vector (see )

typedef struct OTAMetadata {
  uint16_t crc;             //
  uint16_t crc_shadow;      //
  uint16_t size;            //  Size of firmware image
  uint16_t version;         //  Integer representing firmware version
  uint32_t uid;             //  Integer representing unique firmware ID
  uint16_t offset;          //  At what flash sector does this image reside?
} OTAMetadata_t;

#define OTA_IMAGE_HEADER_LENGTH 14  //  Length of OTA image metadata in bytes (CRC, version, data, etc.)

/**
 *  Load address of reset function from the fixed location
 *  of the image's reset vector and jump.
 */
void
jump_to_image(uint32_t destination_address)
{
  destination_address += OTA_IMAGE_HEADER_LENGTH + OTA_IMAGE_RESET_VECTOR;
  __asm("LDR R0, [%[dest]]"::[dest]"r"(destination_address)); //  Load the destination address
  __asm("LDR R1, [R0]");                                      //  Get the branch address
  __asm("ORR R1, #1");                                        //  Make sure the Thumb State bit is set.
  __asm("BX R1");                                             //  Branch execution
}

int
main(void)
{
  jump_to_image( OTA_IMAGE_1 );
  return 0;
}
