#include <stdio.h>
#include <stdint.h>

FILE *firmware_bin;
#define HAL_WORD_SIZE 4

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

int
main(int argc, char *argv[]) {

  //  (1) Open the firmware .bin file
  if ( !argv[1] ) {
    printf("Please provide a .bin file to compute the CRC on as the first argument.\n");
    return -1;
  }
  firmware_bin = fopen( argv[1], "rb" );

  //  (2) Run the CRC16 calculation over the file.  Print result.
  uint16_t crc_result = crcCalc();
  printf( "%#x\n", crc_result );

  //  (3) Close the .bin file.
  fclose( firmware_bin );

  return 0;
}
