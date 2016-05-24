#include <stdint.h>
#include "driverlib/flash.h"

/**
 *    OTA Image Memory Map
 */
#define CURRENT_FIRMWARE    0x2000          //  Address where the current system firmware is stored in internal flash
//  External Flash Addresses (To get true flash address << 12 these numbers)
#define GOLDEN_IMAGE        0x19;           //  Address where the factory-given firmware is stored in external flash (for backup)
uint8_t ota_images[3] = { 0x32, 0x4B, 0x64 };  //  Addresses where OTA updates are stored in external flash

/**
 *    OTA defines
 */
#define OTA_RESET_VECTOR    0x4     //  RESET ISR Vector (see )

typedef struct OTAMetadata {
  uint16_t crc;             //
  uint16_t crc_shadow;      //
  uint16_t size;            //  Size of firmware image
  uint16_t version;         //  Integer representing firmware version
  uint32_t uid;             //  Integer representing unique firmware ID
  uint16_t offset;          //  At what flash sector does this image reside?
} OTAMetadata_t;

#define OTA_METADATA_LENGTH 14  //  Length of OTA image metadata in bytes (CRC, version, data, etc.)

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
 *    Begin executing a new firmware image located at destination_address.
 */
void
jump_to_image(uint32_t destination_address)
{
  destination_address += OTA_METADATA_LENGTH + OTA_RESET_VECTOR;
  __asm("LDR R0, [%[dest]]"::[dest]"r"(destination_address)); //  Load the destination address
  __asm("LDR R1, [R0]");                                      //  Get the branch address
  __asm("ORR R1, #1");                                        //  Make sure the Thumb State bit is set.
  __asm("BX R1");                                             //  Branch execution
}

int
main(void)
{
  OTAMetadata_t example_metadata;
  example_metadata.version = 0;
  example_metadata.size = 27;
  example_metadata.offset = 0x2000;
  FlashProgram((uint8_t*)&example_metadata, 0x1000, OTA_METADATA_LENGTH);

  //jump_to_image( OTA_IMAGE_1 );
  return 0;
}
