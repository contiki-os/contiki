#include <stdint.h>

/**
 *    OTA Image Memory Map
 */
#define OTA_IMAGE_1        0x2000  //  The "Golden Image" initial OTA firmware provided on the chip
#define OTA_IMAGE_2        0x8000  //  Our first OTA version upgrade

/**
 *    OTA defines
 */
#define OTA_IMAGE_HEADER_LENGTH   0x10    //  Length of OTA image metadata (CRC, version, data, etc.)
#define OTA_IMAGE_RESET_VECTOR    0x4     //  RESET ISR Vector (see )

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
