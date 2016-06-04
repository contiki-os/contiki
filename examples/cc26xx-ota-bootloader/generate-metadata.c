#include <stdio.h>
#include <stdint.h>

FILE *firmware_bin;
FILE *firmware_out;
#define HAL_WORD_SIZE 4

const int i = 1;
#define is_bigendian() ( (*(char*)&i) == 0 )

uint32_t firmware_size = 0;
typedef struct OTAMetadata {
  uint16_t crc;             //
  uint16_t crc_shadow;      //
  uint32_t size;            //  Size of firmware image
  uint32_t uuid;             //  Integer representing unique firmware ID
  uint16_t version;         //  Integer representing firmware version
} OTAMetadata_t;


/*********************************************************************
 * @fn          crc16
 *
 * @brief       Run the CRC16 Polynomial calculation over the byte parameter.
 *
 * @param       crc - Running CRC calculated so far.
 * @param       val - Value on which to run the CRC16.
 *
 * @return      crc - Updated for the run.
 */
static uint16_t
crc16(uint16_t crc, uint8_t val)
{
  const uint16_t poly = 0x1021;
  uint8_t cnt;

  for (cnt = 0; cnt < 8; cnt++, val <<= 1)
  {
    uint8_t msb = (crc & 0x8000) ? 1 : 0;

    crc <<= 1;

    if (val & 0x80)
    {
      crc |= 0x0001;
    }

    if (msb)
    {
      crc ^= poly;
    }
  }

  return crc;
}

static uint16_t
crcCalcWord(uint8_t *_word, uint16_t imageCRC)
{
  int idx;
  for (idx = 0; idx < HAL_WORD_SIZE; idx++)
  {
    //printf("%#x ", _word[idx]);
    imageCRC = crc16(imageCRC, _word[idx]);
  }
  firmware_size += 4;
  return imageCRC;
}

/*********************************************************************
 * @fn      crcCalc
 *
 * @brief   Run the CRC16 Polynomial calculation over the provided firmware .bin
 *
 * @param   None
 *
 * @return  The CRC16 calculated.
 */
static uint16_t
crcCalc(void)
{
  uint16_t imageCRC = 0;

  uint8_t idx;
  uint8_t _word[ HAL_WORD_SIZE ]; //  a 4-byte buffer
  size_t nret;

  while ( 1 == (nret = fread(_word, HAL_WORD_SIZE, 1, firmware_bin)) ) {
    imageCRC = crcCalcWord( _word, imageCRC );
  }

  if (nret) {
    imageCRC = crcCalcWord( _word, imageCRC );
  }

  //printf("\n");

  // IAR note explains that poly must be run with value zero for each byte of
  // the crc.
  imageCRC = crc16(imageCRC, 0);
  imageCRC = crc16(imageCRC, 0);

  // Return the CRC calculated over the image.
  return imageCRC;
}

inline uint16_t
swap_endian_16(uint16_t val)
{
	uint16_t swapped_val = (val<<8) | (val>>8);
  return swapped_val;
}

inline uint32_t
swap_endian_32(uint32_t val)
{
	uint32_t swapped_val = (val<<24) | ((val<<8) & 0x00ff0000) |
		  ((val>>8) & 0x0000ff00) | (val>>24);
  return swapped_val;
}

int
main(int argc, char *argv[]) {

  if ( !argv[1] ) {
    printf("Please provide a .bin file to compute the CRC on as the first argument.\n");
    return -1;
  }

  if ( !argv[2] ) {
    printf("Please provide a 16-bit hex firmware version integer as the second argument.\n");
    return -1;
  }

  if ( !argv[3] ) {
    printf("Please provide a 32-bit hex UUID integer as the third argument.\n");
    return -1;
  }

  //  (1) Open the firmware .bin file
  firmware_bin = fopen( argv[1], "rb" );

  //  (2) Run the CRC16 calculation over the file.  Print result.
  uint16_t crc_result = crcCalc();
  printf( "%#x\n", crc_result );

  //  (3) Close the .bin file.
  fclose( firmware_bin );

  //  (4) Generate OTA image metadata
  OTAMetadata_t metadata;
  metadata.crc = crc_result;
  metadata.crc_shadow = crc_result;
  metadata.size = firmware_size;
  sscanf( argv[2], "%xu", &(metadata.version) );
  sscanf( argv[3], "%xu", &(metadata.uuid) );

  //swap_endian_32( &metadata.uuid );

  uint8_t output_buffer[ sizeof(OTAMetadata_t) ];
  memcpy( output_buffer, (uint8_t *)&metadata, sizeof(OTAMetadata_t) );

  //  (4) Open the output firmware .bin file
  firmware_out = fopen( "firmware-metadata.bin", "wb" );

  //  (5) Write the metadata
  fwrite(output_buffer, sizeof(output_buffer), 1, firmware_out);

  //  (6) 0xff spacing until firmware binary starts
  uint8_t blank_buffer[240];
  for (int b=0; b<240; b++) {
    blank_buffer[ b ] = 0xff;
  }
  fwrite( blank_buffer, 240, 1, firmware_out);

  fclose( firmware_out );

  return 0;
}
