/** @file hal/micro/cortexm3/nvm.c
 * @brief Cortex-M3 Non-Volatile Memory data storage system.
 *
 * This file implements the NVM data storage system.  Refer to nvm.h for
 * full documentation of how the NVM data storage system works, is configured,
 * and is accessed.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include "error.h"

#ifdef NVM_RAM_EMULATION

static uint16_t calibrationData[32+2]={
   0xFFFF, 0xFFFF,
   0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
   0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
   0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
   0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

uint8_t halCommonReadFromNvm(void *data, uint32_t offset, uint16_t length)
{
 halCommonMemCopy(data, ((uint8_t *) calibrationData) + offset, length); 
  return ST_SUCCESS;
}
uint8_t halCommonWriteToNvm(const void *data, uint32_t offset, uint16_t length)
{
  halCommonMemCopy(((uint8_t *) calibrationData) + offset, data, length);
  return ST_SUCCESS;
}

#else

//flash.h gives access to halInternalFlashErase and halInternalFlashWrite.
#include "hal/micro/cortexm3/flash.h"
//nvm.h includes memmap.h.  These two headers define the key parameters:
//  MFB_PAGE_SIZE_B
//  MFB_TOP
//  NVM_LEFT_PAGE
//  NVM_RIGHT_PAGE
//  NVM_DATA_SIZE_B
//  NVM_FLASH_PAGE_COUNT
//  NVM_MGMT_SIZE_B
#include "hal/micro/cortexm3/nvm.h"

//Define two variables that hold the actual NVM data storage.  LEFT and RIGHT
//are not required to be continuous memory blocks so they can be define
//separately.  The linker is responsible for placing these storage containers
//on flash page boundaries.
NO_STRIPPING __no_init VAR_AT_SEGMENT (const uint8_t nvmStorageLeft[NVM_DATA_SIZE_B], __NVM__);
NO_STRIPPING __no_init VAR_AT_SEGMENT (const uint8_t nvmStorageRight[NVM_DATA_SIZE_B], __NVM__);

static uint8_t determineState(void)
{
  uint32_t leftMgmt = *(uint32_t *)NVM_LEFT_PAGE;
  uint32_t rightMgmt = *(uint32_t *)NVM_RIGHT_PAGE;
  uint8_t state=0;
  
  if((leftMgmt==0xFFFF0000) && (rightMgmt==0xFFFFFFFF)) {
    //State 1 and state 4 use identical mgmt words.  The function
    //determineState() is only called at the start of a NVM read
    //or write.  During a read, state 1 and 4 both read from the
    //LEFT so there is no reason to make a distinction.  During
    //a write, the system will see the current page as LEFT and
    //therefore be transitioning from LEFT to RIGHT so state 4 is
    //correct.  State 1 is only required to transition from 0 to 2.
    state = 4;
  } else if((leftMgmt==0xFFFF0000) && (rightMgmt==0xFF00FFFF)) {
    state = 2;
  } else if((leftMgmt==0xFFFF0000) && (rightMgmt==0xFF000000)) {
    state = 3;
  } else if((leftMgmt==0xFFFF0000) && (rightMgmt==0xFFFFFFFF)) {
    state = 4;
  } else if((leftMgmt==0xFFFF0000) && (rightMgmt==0xFFFFFF00)) {
    state = 5;
  } else if((leftMgmt==0xFF000000) && (rightMgmt==0xFFFFFF00)) {
    state = 6;
  } else if((leftMgmt==0xFF000000) && (rightMgmt==0xFFFF0000)) {
    state = 7;
  } else if((leftMgmt==0xFFFFFFFF) && (rightMgmt==0xFFFF0000)) {
    state = 8;
  } else if((leftMgmt==0xFFFFFF00) && (rightMgmt==0xFFFF0000)) {
    state = 9;
  } else if((leftMgmt==0xFFFFFF00) && (rightMgmt==0xFF000000)) {
    state = 10;
  } else {
    //State 0 is used to indicate erased or invalid.
    state = 0;
  }
  
  return state;
}


uint8_t halCommonReadFromNvm(void *data, uint32_t offset, uint16_t length)
{
  uint16_t i;
  uint16_t *flash;
  //Remember: all flash writes are 16bits.
  uint16_t *ram = (uint16_t*)data;
  
  //The NVM data storage system cannot function if the LEFT and RIGHT
  //storage are not aligned to physical flash pages.
  assert((NVM_LEFT_PAGE%MFB_PAGE_SIZE_B)==0);
  assert((NVM_RIGHT_PAGE%MFB_PAGE_SIZE_B)==0);
  //The offset of the NVM data must be 16bit aligned.
  assert((offset&0x1)==0);
  //The length of the NVM data must be 16bit aligned.
  assert((length&0x1)==0);
  
  assert(offset+length<NVM_DATA_SIZE_B);
  
  //Obtain the data from NVM storage.
  switch(determineState()) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 9:
    case 10:
      flash = (uint16_t *)(NVM_LEFT_PAGE+offset);
      for(i=0;i<(length/2);i++) {
        ram[i] = flash[i];
      }
    break;
    case 5:
    case 6:
    case 7:
    case 8:
      flash = (uint16_t *)(NVM_RIGHT_PAGE+offset);
      for(i=0;i<(length/2);i++) {
        ram[i] = flash[i];
      }
    break;
    case 0:
    default:
      //Reading from NVM while the mgmt bytes are in an invalid state
      //should not return any bytes actually found in flash.  Instead,
      //return nothing but 0xFF.  This is legitimate because the next
      //call to the write function will also find invalid mgmt bytes
      //and trigger an erasure of NVM, after which the NVM really will
      //contain just 0xFF for data (plus the new data supplied during
      //the write call).
      for(i=0;i<(length/2);i++) {
        ram[i] = 0xFFFF;
      }
    //Inform the calling code. using ST_ERR_FATAL, that there were
    //invalid mgmt bytes and 0xFF was forcefully returned.
    return ST_ERR_FATAL;
  }
  
  return ST_SUCCESS;
}

uint16_t *halCommonGetAddressFromNvm(uint32_t offset)
{
  uint16_t *flash;
  
  //The NVM data storage system cannot function if the LEFT and RIGHT
  //storage are not aligned to physical flash pages.
  assert((NVM_LEFT_PAGE%MFB_PAGE_SIZE_B)==0);
  assert((NVM_RIGHT_PAGE%MFB_PAGE_SIZE_B)==0);
  //The offset of the NVM data must be 16bit aligned.
  assert((offset&0x1)==0);
  
  //Obtain the data from NVM storage.
  switch(determineState()) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 9:
    case 10:
      flash = (uint16_t *)(NVM_LEFT_PAGE+offset);
    break;
    case 5:
    case 6:
    case 7:
    case 8:
      flash = (uint16_t *)(NVM_RIGHT_PAGE+offset);
    break;
    case 0:
    default:
      // Flash is in an invalid state 
      // Fix it with a dummy write and then return the flash page left
      {
	uint16_t dummy = 0xFFFF;
	halCommonWriteToNvm(&dummy, 0, 2);
	flash = (uint16_t *)(NVM_LEFT_PAGE+offset);
      }
  }
  
  return flash;
}


static uint8_t erasePage(uint32_t page)
{
  StStatus status;
  uint32_t i, k;
  uint32_t address;
  uint8_t *flash;
  
  //Erasing a LEFT or RIGHT page requires erasing all of the flash pages.
  //Since the mgmt bytes are stored at the bottom of a page, the flash pages
  //are erased from the top down ensuring that that mgmt words are the last
  //data to be erased.  This way, if a reset occurs while erasing, the mgmt
  //words are still valid the next time determineState() is called.
  for(i=NVM_FLASH_PAGE_COUNT;i>0;i--) {
    address = (page+((i-1)*MFB_PAGE_SIZE_B));
    flash = (uint8_t *)address;
    //Scan the page to determine if it is fully erased already.
    //If the flash is not erased, erase it.  The purpose of scanning
    //first is to save a little time if erasing is not required.
    for(k=0;k<MFB_PAGE_SIZE_B;k++,flash++) {
      if(*flash != 0xFF) {
        status = halInternalFlashErase(MFB_PAGE_ERASE, address);
        if(status != ST_SUCCESS) {
          return status;
        }
        //Don't bother looking at the rest of this flash page and just
        //move to the next.
        k=MFB_PAGE_SIZE_B;
      }
    }
  }
  return ST_SUCCESS;
}


//This macro is responsible for erasing an NVM page (LEFT or RIGHT).
#define ERASE_PAGE(page)          \
  do {                            \
    status = erasePage(page);     \
    if(status != ST_SUCCESS) { \
      return status;              \
    }                             \
  } while(0)


//This macro is responsible for writing the new data into the destination
//page and copying existing data from the source page to the
//destination page.
#define WRITE_DATA(destPage, srcPage, offset, length)                     \
  do {                                                                    \
    /*Copy all data below the new data from the srcPage to the destPage*/ \
    status = halInternalFlashWrite(destPage+NVM_MGMT_SIZE_B,              \
                                   (uint16_t *)(srcPage+NVM_MGMT_SIZE_B),   \
                                   (offset-NVM_MGMT_SIZE_B)/2);           \
    if(status != ST_SUCCESS) { return status; }                        \
    /*Write the new data*/                                                \
    status = halInternalFlashWrite(destPage+offset,                       \
                                   ram,                                   \
                                   (length)/2);                           \
    if(status != ST_SUCCESS) { return status; }                        \
    /*Copy all data above the new data from the srcPage to the destPage*/ \
    status = halInternalFlashWrite(destPage+offset+length,                \
                                   (uint16_t *)(srcPage+offset+length),     \
                                   (NVM_DATA_SIZE_B-                      \
                                    length-offset-                        \
                                    NVM_MGMT_SIZE_B)/2);                  \
    if(status != ST_SUCCESS) { return status; }                        \
  } while(0)

//This macro is responsible for writing 16bits of management data to
//the proper management address.
#define WRITE_MGMT_16BITS(address, data)                  \
  do{                                                     \
    uint16_t value = data;                                  \
    status = halInternalFlashWrite((address), &value, 1); \
    if(status != ST_SUCCESS) {                         \
      return status;                                      \
    }                                                     \
  } while(0)


uint8_t halCommonWriteToNvm(const void *data, uint32_t offset, uint16_t length)
{
  StStatus status;
  uint8_t state, exitState;
  uint32_t srcPage;
  uint32_t destPage;
  //Remember: NVM data storage works on 16bit quantities.
  uint16_t *ram = (uint16_t*)data;
  
  //The NVM data storage system cannot function if the LEFT and RIGHT
  //storage are not aligned to physical flash pages.
  assert((NVM_LEFT_PAGE%MFB_PAGE_SIZE_B)==0);
  assert((NVM_RIGHT_PAGE%MFB_PAGE_SIZE_B)==0);
  //The offset of the NVM data must be 16bit aligned.
  assert((offset&0x1)==0);
  //The length of the NVM data must be 16bit aligned.
  assert((length&0x1)==0);
  //It is illegal to write to an offset outside of NVM storage.
  assert(offset+length<NVM_DATA_SIZE_B);
  
  
  state = determineState();
  
  switch(state) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 9:
    case 10:
      srcPage = NVM_LEFT_PAGE;
      destPage = NVM_RIGHT_PAGE;
      exitState = 7;
    break;
    case 5:
    case 6:
    case 7:
    case 8:
      srcPage = NVM_RIGHT_PAGE;
      destPage = NVM_LEFT_PAGE;
      exitState = 3;
    break;
    case 0:
    default:
      //Invalid state.  Default to writing to the LEFT page.  Defaulting to
      //using RIGHT as the source page is valid since the RIGHT page
      //will also be erased and therefore produce 0xFF for data values.
      state = 0;
      srcPage = NVM_RIGHT_PAGE;
      destPage = NVM_LEFT_PAGE;
      exitState = 3;
    break;
  }
  
  //Advance the state machine.  Starting on state 3 requires state 7 to
  //exit and starting on state 7 requires state 3 to exit.  Starting on
  //any other state requires either 3 or 7 to exit.
  //NOTE:  Refer to nvm.h for a description of the states and how the
  //       state transitions correspond to erasing, writing data, and
  //       writing mgmt values.
  while(TRUE) {
    switch(state) {
      case 0:
        //State 0 is the only state where the source page needs to be erased.
        ERASE_PAGE(srcPage);
        ERASE_PAGE(destPage);
        WRITE_DATA(destPage, srcPage, offset, length);
        WRITE_MGMT_16BITS(NVM_LEFT_PAGE+0, 0x0000);
        state=1;
      break;
      case 1:
        WRITE_MGMT_16BITS(NVM_RIGHT_PAGE+2, 0xFF00);
        state=2;
      break;
      case 2:
        WRITE_MGMT_16BITS(NVM_RIGHT_PAGE+0, 0x0000);
        state=3;
      break;
      case 3:
        if(exitState==3) {
          return ST_SUCCESS;
        }
        ERASE_PAGE(destPage);
        state=4;
      break;
      case 4:
        WRITE_DATA(destPage, srcPage, offset, length);
        WRITE_MGMT_16BITS(NVM_RIGHT_PAGE+0, 0xFF00);
        state=5;
      break;
      case 5:
        WRITE_MGMT_16BITS(NVM_LEFT_PAGE+2, 0xFF00);
        state=6;
      break;
      case 6:
        WRITE_MGMT_16BITS(NVM_RIGHT_PAGE+0, 0x0000);
        state=7;
      break;
      case 7:
        if(exitState==7) {
          return ST_SUCCESS;
        }
        ERASE_PAGE(destPage);
        state=8;
      break;
      case 8:
        WRITE_DATA(destPage, srcPage, offset, length);
        WRITE_MGMT_16BITS(NVM_LEFT_PAGE+0, 0xFF00);
        state=9;
      break;
      case 9:
        WRITE_MGMT_16BITS(NVM_RIGHT_PAGE+2, 0xFF00);
        state=10;
      break;
      case 10:
        WRITE_MGMT_16BITS(NVM_LEFT_PAGE+0, 0x0000);
        state=3;
      break;
    }
  }
}

#endif // NVM_RAM_EMULATION
