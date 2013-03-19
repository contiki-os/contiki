/** 
 * @file token-phy.h
 * @brief Definitions for phy tokens.
 *
 * The file token-phy.h should not be included directly. 
 * It is accessed by the other token files. 
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef DEFINE_INDEXED_TOKEN
  #define DEFINE_INDEXED_TOKEN(name, type, arraysize, ...)  \
    TOKEN_DEF(name, CREATOR_##name, 0, 1, type, (arraysize),  __VA_ARGS__)
#endif


#if defined PHY_STM32W108XX 

#ifdef DEFINETYPES
typedef struct {
  uint8_t vcoAtLna; // the VCO tune value at the time LNA value was calculated.
  uint8_t modDac;   // msb : cal needed , bit 0-5 : value
  uint8_t filter;   // msb : cal needed , bit 0-4 : value
  uint8_t lna;      // msb : cal needed , bit 0-5 : value
} tokTypeStackCalData;
#endif 


#ifdef DEFINETOKENS

#define CREATOR_STACK_CAL_DATA         0xD243 // msb+'R'+'C' (Radio Calibration)
#define STACK_CAL_DATA_ARRAY_SIZE       16


// This breaks the simulated eeprom unit test, so we ifdef it out
// of existence.
#ifndef ST_TEST
DEFINE_INDEXED_TOKEN(STACK_CAL_DATA,
                     tokTypeStackCalData, STACK_CAL_DATA_ARRAY_SIZE,
                     { 
                       0xff, // vcoAtLna default: invalid VCO value.
                       0x80, // modDac default: cal needed.
                       0x80, // filter default: cal needed.
                       0x80  // lna default: cal needed.
                     })
#endif //ST_TEST

#endif //DEFINETOKENS

#endif // defined PHY_STM32W108XX 

