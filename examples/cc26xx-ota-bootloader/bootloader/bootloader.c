#include <stdint.h>
#include "ota.h"

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

int
main(void)
{
  OTAMetadata_t example_metadata = {
    0, 0, 0x27, 0x1, 0x2, 0x2000
  };
  if ( FlashProgram((uint8_t*)&example_metadata, 0x1000, OTA_METADATA_LENGTH) == FAPI_STATUS_SUCCESS ) {
    return 0;
  } else {
    return -1;
  }

  generate_fake_metadata();

  jump_to_image( CURRENT_FIRMWARE );
  return 0;
}
