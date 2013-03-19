/** @file hal/micro/cortexm3/flash.c
 * @brief Implements the generic flash manipulation routines.
 * 
 * The file 'flash-sw-spec.txt' should provide *all* the information needed
 * to understand and work with the FLITF and flash.
 * 
 * 
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include "error.h"
#include "hal/micro/cortexm3/bootloader/fib-bootloader.h"
#include "hal/micro/cortexm3/mpu.h"
#include "memmap.h"
#include "flash.h"

#ifdef FLASH_PROGRAMMING_WITH_EMPTY_FIB
#define ST_EMU_TEST
#endif
// A translation table used to convert FibStatus codes to corresponding
//  StStatus values
static const StStatus fibToStStatus[] = {
  ST_SUCCESS,                    // FIB_SUCCESS             0
  ST_BAD_ARGUMENT,               // FIB_ERR_UNALIGNED       1
  ST_BAD_ARGUMENT,               // FIB_ERR_INVALID_ADDRESS 2
  ST_BAD_ARGUMENT,               // FIB_ERR_INVALID_TYPE    3
  ST_ERR_FLASH_PROG_FAIL,        // FIB_ERR_WRITE_PROTECTED 4
  ST_ERR_FLASH_PROG_FAIL,        // FIB_ERR_WRITE_FAILED    5
  ST_ERR_FLASH_WRITE_INHIBITED,  // FIB_ERR_ERASE_REQUIRED  6
  ST_ERR_FLASH_VERIFY_FAILED     // FIB_ERR_VERIFY_FAILED   7
};
  
//The purpose of flashEraseIsActive and halFlashEraseIsActive() is so that
//interrupts can query the flash library to find out of Flash Erase is
//active when their ISR gets invoked.  This is useful because Flash Erase
//causes the chip to go ATOMIC for 21ms and this delay will disrupt interrupt
//latency.  By having a sinple API that an ISR can query for this state,
//the ISR can appriopriately adjust for a 21ms latency time.
boolean flashEraseIsActive = FALSE;
boolean halFlashEraseIsActive(void)
{
  return flashEraseIsActive;
}


// Emulators do not have FIB bootloaders, so need to include a copy of
// these core flash routines.  

#if defined(ST_EMU_TEST)

static void enableFlitf(void)
{
  //First, unlock the FLITF by writing the two key values to the Flash
  //Protection Unlock register
  FPEC_KEY = FPEC_KEY1;
  FPEC_KEY = FPEC_KEY2;
  
  //Second, unlock the CIB by writing the two key values to the CIB
  //Protection Unlock register
  OPT_KEY = FPEC_KEY1;
  OPT_KEY = FPEC_KEY2;
  
  //Turn on the FPEC clock for flash manipulation operations
  FPEC_CLKREQ = FPEC_CLKREQ_FIELD;
  
  //make sure the FPEC clock is running before we proceed
  while( (FPEC_CLKSTAT&FPEC_CLKACK) != FPEC_CLKACK) {}
  
  //just in case, wait until the flash is no longer busy
  while( (FLASH_STATUS&FLASH_STATUS_FLA_BSY) == FLASH_STATUS_FLA_BSY ) {}
}

static void disableFlitf(void)
{
  //make sure the FPEC is completely idle before turning off the clock
  while( (FPEC_CLKSTAT&FPEC_CLKBSY) == FPEC_CLKBSY) {}
  
  //Turn off the FPEC clock now that we're done
  FPEC_CLKREQ = FPEC_CLKREQ_RESET;
  
  //Set LOCK and clear OPTWREN to lock both the FLITF and the CIB.
  //NOTE: The PROG bit must also be cleared otherwise Flash can still
  //      be programmed even with the LOCK bit set.  BugzID: 6267
  FLASH_CTRL = FLASH_CTRL_LOCK; //lock the flash from further accesses
}

static FibStatus fibFlashWrite(uint32_t address, uint8_t *data, uint32_t length, uint32_t dummy)
{
  uint32_t i;
  uint16_t *ptr;
  FibStatus status = FIB_SUCCESS;
  // Address and length must be half-word aligned.
  if ((address & 1) || (length & 1)) {
    return FIB_ERR_UNALIGNED;
  }
  // Start and end address must be in MFB or CIB.
  if (!((address >= MFB_BOTTOM && address + length <= MFB_TOP + 1)
        || (address >= CIB_BOTTOM && address + length <= CIB_TOP + 1))) {
    return FIB_ERR_INVALID_ADDRESS;
  }
  enableFlitf();
  ptr = (uint16_t *)address;
  for (i = 0; i < length; i += 2) {
    uint16_t currentData = *ptr;
    uint16_t newData = HIGH_LOW_TO_INT(data[i + 1], data[i]);
    // Only program the data if it makes sense to do so.
    if (currentData == newData) {
      // If the new data matches the flash, don't bother doing anything.
    } else if (currentData == 0xFFFF || newData == 0x0000) {
      // If the flash is 0xFFFF we're allowed to write anything.
      // If the new data is 0x0000 it doesn't matter what the flash is.
      // OPTWREN must stay set to keep CIB unlocked.
      if ((CIB_OB_BOTTOM <= (uint32_t)ptr) && ((uint32_t)ptr <= CIB_OB_TOP)) {
        FLASH_CTRL = (FLASH_CTRL_OPTWREN | FLASH_CTRL_OPTPROG);
      } else {
        FLASH_CTRL = (FLASH_CTRL_OPTWREN | FLASH_CTRL_PROG);
      }
      // Assigning data to the address performs the actual write.
      (*ptr) = newData;
      // Wait for the busy bit to clear, indicating operation is done.
      while ((FLASH_STATUS & FLASH_STATUS_FLA_BSY) != 0) {}
      // Reset the operation complete flag.
      FLASH_STATUS = FLASH_STATUS_EOP;
      // Check if any error bits have been tripped, and if so, exit.
      // The bit PAGE_PROG_ERR is not relevant in this programming mode.
      if (FLASH_STATUS & (FLASH_STATUS_WRP_ERR | FLASH_STATUS_PROG_ERR)) {
        if (FLASH_STATUS & FLASH_STATUS_WRP_ERR) {
          status = FIB_ERR_WRITE_PROTECTED;
        } else {
          status = FIB_ERR_WRITE_FAILED;
        }
        FLASH_STATUS = FLASH_STATUS_WRP_ERR;
        FLASH_STATUS = FLASH_STATUS_PROG_ERR;
        break;
      }
    } else {
      status = FIB_ERR_ERASE_REQUIRED;
      break;
    }
    ptr++;
  }
  disableFlitf();
  return status;
}

static FibStatus fibFlashWriteVerify(uint32_t address, uint8_t *data, uint32_t length)
{
  uint32_t i;
  uint8_t *ptr = (uint8_t *)address;
  for (i = 0; i < length; i++) {
    if (*ptr != data[i]) {
      return FIB_ERR_VERIFY_FAILED;
    }
    ptr++;
  }
  return FIB_SUCCESS;
}

static FibStatus fibFlashErase(FibEraseType eraseType, uint32_t address)
{
  uint32_t eraseOp;
  uint32_t *ptr;
  uint32_t length;
  FibStatus status = FIB_SUCCESS;
  if (BYTE_0(eraseType) == MFB_MASS_ERASE) {
    eraseOp = FLASH_CTRL_MASSERASE;
    ptr = (uint32_t *)MFB_BOTTOM;
    length = MFB_SIZE_W;
  } else if (BYTE_0(eraseType) == MFB_PAGE_ERASE) {
    if (address < MFB_BOTTOM || address > MFB_TOP) {
      return FIB_ERR_INVALID_ADDRESS;
    }
    eraseOp = FLASH_CTRL_PAGEERASE;
    ptr = (uint32_t *)(address & MFB_PAGE_MASK_B);
    length = MFB_PAGE_SIZE_W;
  } else if (BYTE_0(eraseType) == CIB_ERASE) {
    eraseOp = FLASH_CTRL_OPTWREN | FLASH_CTRL_OPTERASE;
    ptr = (uint32_t *)CIB_BOTTOM;
    length = CIB_SIZE_W;
  } else {
    return FIB_ERR_INVALID_TYPE;
  }
  if ((eraseType & DO_ERASE) != 0) {
    enableFlitf();
    FLASH_CTRL = eraseOp;
    if (BYTE_0(eraseType) == MFB_PAGE_ERASE) {
      FLASH_ADDR = (address & MFB_PAGE_MASK_B);
    }
    eraseOp |= FLASH_CTRL_FLA_START;
    // Perform the actual erase.
    FLASH_CTRL = eraseOp;
    // Wait for the busy bit to clear, indicating operation is done.
    while ((FLASH_STATUS & FLASH_STATUS_FLA_BSY) != 0) {}
    // Reset the operation complete flag.
    FLASH_STATUS = FLASH_STATUS_EOP;
    // Check for errors; the only relevant one for erasing is write protection.
    if (FLASH_STATUS & FLASH_STATUS_WRP_ERR) {
      FLASH_STATUS = FLASH_STATUS_WRP_ERR;
      status = FIB_ERR_WRITE_PROTECTED;
    }
    disableFlitf();
  }
  if (status == FIB_SUCCESS
      && (eraseType & DO_VERIFY) != 0) {
    uint32_t i;
    for (i = 0; i < length; i++) {
      if (*ptr != 0xFFFFFFFF) {
        return FIB_ERR_VERIFY_FAILED;
      }
      ptr++;
    }
  }
  return status;
}
#endif // ST_EMU_TEST

static boolean verifyFib(void)
{
  // Ensure that a programmed FIB of a proper version is present
  return ( (halFixedAddressTable.baseTable.type == FIXED_ADDRESS_TABLE_TYPE) &&
           ( ( (halFixedAddressTable.baseTable.version & FAT_MAJOR_VERSION_MASK) 
               == 0x0000 ) &&
             (halFixedAddressTable.baseTable.version >= 0x0002) 
           )
         );
}

//The parameter 'eraseType' chooses which erasure will be performed while
//the 'address' parameter chooses the page to be erased during MFB page erase.
StStatus halInternalFlashErase(uint8_t eraseType, uint32_t address)
{
  FibStatus status;
  
  ATOMIC(
    BYPASS_MPU(
      flashEraseIsActive = TRUE;
      #if defined(ST_EMU_TEST)
        // Always try to use the FIB bootloader if its present
        if(verifyFib()) {
          status = halFixedAddressTable.fibFlashErase(
                                             (((uint32_t)eraseType) | DO_ERASE), 
                                             address);
        } else {
          status = fibFlashErase((((uint32_t)eraseType) | DO_ERASE), address);
        }
      #else



        assert(verifyFib());
        status = halFixedAddressTable.fibFlashErase(
                                           (((uint32_t)eraseType) | DO_ERASE), 
                                           address);
      #endif
    )
  )
  //If there are any interrupts pending that could have been delayed for 21ms,
  //they will be serviced here since we exit the ATOMIC block.  These ISRs
  //can query the flash library and find out that erasing is active.  After
  //this point, we're no longer ATOMIC/disrupting latency so our erase
  //active flag should be cleared.
  flashEraseIsActive = FALSE;
  
  if(status!=FIB_SUCCESS) {
    return fibToStStatus[status];
  }

  #if defined(ST_EMU_TEST)
    // Always try to use the FIB bootloader if its present
    if(verifyFib()) {
      status = halFixedAddressTable.fibFlashErase(
                                          (((uint32_t)eraseType) | DO_VERIFY), 
                                          address);
    } else {
      status = fibFlashErase((((uint32_t)eraseType) | DO_VERIFY), address);
    }
  #else
    status = halFixedAddressTable.fibFlashErase(
                                        (((uint32_t)eraseType) | DO_VERIFY), 
                                        address);
  #endif
  return fibToStStatus[status];
}


//The parameter 'address' defines the starting address of where the
//programming will occur - this parameter MUST be half-word aligned since all
//programming operations are HW.  The parameter 'data' is a pointer to a buffer
//containin the 16bit half-words to be written.  Length is the number of 16bit
//half-words contained in 'data' to be written to flash.
//NOTE: This function can NOT write the option bytes and will throw an error
//if that is attempted.
StStatus halInternalFlashWrite(uint32_t address, uint16_t * data, uint32_t length)
{
  FibStatus status;
    
  length = length * 2;  // fib routines specify length in bytes
  
  ATOMIC(
    BYPASS_MPU( 
      #if defined(ST_EMU_TEST)
        // Always try to use the FIB bootloader if its present
        if(verifyFib()) {
          status = halFixedAddressTable.fibFlashWrite(address, 
                                                      (uint8_t *)data, 
                                                      length,
                                                      0);
        } else {
          status = fibFlashWrite(address, (uint8_t *)data, length, 0);
        }
      #else



        // Ensure that a programmed FIB of a proper version is present
        assert(verifyFib());
        status = halFixedAddressTable.fibFlashWrite(address, 
                                                    (uint8_t *)data, 
                                                    length,
                                                    0);
      #endif
    )
  )
  
  if(status!=FIB_SUCCESS) {
    return fibToStStatus[status];
  }
  
  #if defined(ST_EMU_TEST)
    // Always try to use the FIB bootloader if its present
    if(verifyFib()) {
      status = halFixedAddressTable.fibFlashWrite(address,
                                                  (uint8_t *)data,
                                                  0,
                                                  length);
    } else {
      status = fibFlashWriteVerify(address, (uint8_t *)data, length);
    }
  #else
    status = halFixedAddressTable.fibFlashWrite(address,
                                                (uint8_t *)data,
                                                0,
                                                length);
  #endif
  
  return fibToStStatus[status];
}


//The parameter 'byte' is the option byte number to be programmed.  This
//parameter can have a value of 0 through 7.  'data' is the 8bit value to be
//programmed into the option byte since the hardware will calculate the
//compliment and program the full 16bit option byte.
StStatus halInternalCibOptionByteWrite(uint8_t byte, uint8_t data)
{
  uint16_t dataAndInverse = HIGH_LOW_TO_INT(~data, data);
  // There are only 8 option bytes, don't try to program more than that.
  if (byte > 7) {
    return ST_ERR_FLASH_PROG_FAIL;
  }
  return halInternalFlashWrite(CIB_OB_BOTTOM + (byte << 1), &dataAndInverse, 1);
}


