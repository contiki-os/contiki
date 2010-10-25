/** @file hal/micro/cortexm3/mfg-token.c
 * @brief Cortex-M3 Manufacturing-Token system
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#include PLATFORM_HEADER
#include "error.h"
#include "hal/micro/cortexm3/flash.h"
#include "mfg-token.h"




#define DEFINETOKENS
#define TOKEN_MFG(name,creator,iscnt,isidx,type,arraysize,...) \
  const int16u TOKEN_##name = TOKEN_##name##_ADDRESS;
  #include "hal/micro/cortexm3/token-manufacturing.h"
#undef TOKEN_DEF
#undef TOKEN_MFG
#undef DEFINETOKENS








static const int8u nullEui[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };


void halInternalGetMfgTokenData(void *data, int16u ID, int8u index, int8u len)
{
  int8u *ram = (int8u*)data;
  
  //0x7F is a non-indexed token.  Remap to 0 for the address calculation
  index = (index==0x7F) ? 0 : index;
  
  if(ID == MFG_EUI_64_LOCATION) {
    //There are two EUI64's stored in the Info Blocks, St and Custom.
    //0x0A00 is the address used by the generic EUI64 token, and it is
    //token.c's responbility to pick the returned EUI64 from either St
    //or Custom.  Return the Custom EUI64 if it is not all FF's, otherwise
    //return the St EUI64.
    tokTypeMfgEui64 eui64;
    halCommonGetMfgToken(&eui64, TOKEN_MFG_CUSTOM_EUI_64);
    if(MEMCOMPARE(eui64,nullEui, 8 /*EUI64_SIZE*/) == 0) {
      halCommonGetMfgToken(&eui64, TOKEN_MFG_ST_EUI_64);
    }
    MEMCOPY(ram, eui64, 8 /*EUI64_SIZE*/);
  } else {
    //read from the Information Blocks.  The token ID is only the
    //bottom 16bits of the token's actual address.  Since the info blocks
    //exist in the range DATA_BIG_INFO_BASE-DATA_BIG_INFO_END, we need
    //to OR the ID with DATA_BIG_INFO_BASE to get the real address.
    int32u realAddress = (DATA_BIG_INFO_BASE|ID) + (len*index);
    int8u *flash = (int8u *)realAddress;














    MEMCOPY(ram, flash, len);
  }
}


void halInternalSetMfgTokenData(int16u token, void *data, int8u len)
{
  StStatus flashStatus;
  int32u realAddress = (DATA_BIG_INFO_BASE|token);
  int8u * flash = (int8u *)realAddress;
  int32u i;
  
  //The flash library (and hardware) requires the address and length to both
  //be multiples of 16bits.  Since this API is only valid for writing to
  //the CIB, verify that the token+len falls within the CIB.
  assert((token&1) != 1);
  assert((len&1) != 1);
  assert((realAddress>=CIB_BOTTOM) && ((realAddress+len-1)<=CIB_TOP));
  
  //CIB manufacturing tokens can only be written by on-chip code if the token
  //is currently unprogrammed.  Verify the entire token is unwritten.  The
  //flash library performs a similar check, but verifying here ensures that
  //the entire token is unprogrammed and will prevent partial writes.
  for(i=0;i<len;i++) {
    assert(flash[i] == 0xFF);
  }
  
  //Remember, the flash library operates in 16bit quantities, but the
  //token system operates in 8bit quantities.  Hence the divide by 2.
  flashStatus = halInternalFlashWrite(realAddress, data, (len/2));
  assert(flashStatus == ST_SUCCESS);
}

