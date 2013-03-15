/** @file micro.c
 * @brief STM32W108 micro specific minimal HAL functions
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include BOARD_HEADER
#include "error.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"
#include "micro/system-timer.h"
#include "micro/adc.h"
#include "micro/cortexm3/memmap.h"
#include "micro/cortexm3/iap_bootloader.h"

#include <stdlib.h>
#include <string.h>

extern void halBoardInit(void);

void halInit(void)
{
  //Disable the REG_EN external regulator enable signal.  Out of reset this
  //signal overrides PA7.  By disabling it early, PA7 is reclaimed as a GPIO.
  //If an external regulator is required, the following line of code should
  //be deleted.
  GPIO_DBGCFG &= ~GPIO_EXTREGEN;
  halInternalSetRegTrim(FALSE);
  halBoardInit();
  halPowerUp();
  halInternalCalibrateFastRc();
  
  #ifndef DISABLE_WATCHDOG
    halInternalEnableWatchDog();
  #endif

  halInternalStartSystemTimer();
}


void halReboot(void)
{
  INTERRUPTS_OFF();
  

  //FCLK must be 6MHz to allow the SYSRESETREQ signal to cleanly
  //propagate and reset the chip.  Switch SYSCLK first since we need
  //the cycles used by switching FCLK to guarantee the SYSCLK is
  //stable and ready for SYSRESETREQ.
  OSC24M_CTRL = OSC24M_CTRL_RESET; //Guarantee SYSCLK is sourced from OSCHF
  CPU_CLKSEL = CPU_CLKSEL_RESET;   //Guarantee FCLK is sourced from PCLK
  
  SCS_AIRCR = (0x05FA0000 | SCS_AIRCR_SYSRESETREQ); // trigger the reset
  //NOTE: SYSRESETREQ is not the same as nRESET.  It will not do the debug
  //pieces: DWT, ITM, FPB, vector catch, etc
}

void halPowerDown(void)
{
  halBoardPowerDown();
}

void halPowerUp(void)
{
  halInternalInitAdc();
  halCommonCalibratePads();  
  halInternalSwitchToXtal();
  halBoardPowerUp();
}

static uint16_t seed0 = 0xbeef;
static uint16_t seed1 = 0xface;

void halCommonSeedRandom(uint32_t seed)
{
  seed0 = (uint16_t) seed;
  if (seed0 == 0)
    seed0 = 0xbeef;
  seed1 = (uint16_t) (seed >> 16);
  if (seed1 == 0)
    seed1 = 0xface;
}

static uint16_t shift(uint16_t *val, uint16_t taps)
{
  uint16_t newVal = *val;

  if (newVal & 0x8000)
    newVal ^= taps;
  *val = newVal << 1;
  return newVal;
}

uint16_t halCommonGetRandom(void)
{
  return (shift(&seed0, 0x0062)
          ^ shift(&seed1, 0x100B));
}

void halCommonMemCopy(void *dest, const void *source, uint8_t bytes)
{
  memcpy(dest, source, bytes);
}

int8_t halCommonMemCompare(const void *source0, const void *source1, uint8_t bytes)
{
  return memcmp(source0, source1, bytes);
}

void halCommonMemSet(void *dest, uint8_t val, uint16_t bytes)
{
  memset(dest, val, bytes);
}

#pragma pack(1)
typedef struct appSwitchStruct {
  uint32_t signature;
  uint8_t mode;
  uint8_t channel;
  union {
    uint16_t panID;
    uint16_t offset;
  } param;
} appSwitchStructType;
#pragma pack()
static appSwitchStructType *appSwitch = (appSwitchStructType *) RAM_BOTTOM;

StStatus halBootloaderStart(uint8_t mode, uint8_t channel, uint16_t panID)
{
    if (mode ==  IAP_BOOTLOADER_MODE_UART) {
      uint8_t cut = *(volatile uint8_t *) 0x08040798;
      if (!( (halFixedAddressTable.baseTable.type == FIXED_ADDRESS_TABLE_TYPE) &&
           ( ( (halFixedAddressTable.baseTable.version & FAT_MAJOR_VERSION_MASK) 
               == 0x0000 ) &&
             (halFixedAddressTable.baseTable.version == 0x0003) //checking presence of valid version 
	   ) && (cut >= 2) && (cut <= 3)))     
	/* Cut not supported */
	return ST_ERR_FATAL;
    } else {
      /* Check that OTA bootloader is at the base of the flash */
      if (*((uint32_t *) (MFB_BOTTOM + 28)) == IAP_BOOTLOADER_APP_SWITCH_SIGNATURE) {
         appSwitch->channel = ((channel >= 11) && (channel <= 26)) ? channel :IAP_BOOTLOADER_DEFAULT_CHANNEL;
         appSwitch->param.panID = panID;
      } else {
          return ST_ERR_FATAL;
      }
    }
    appSwitch->signature = IAP_BOOTLOADER_APP_SWITCH_SIGNATURE;
    appSwitch->mode = mode;
    halReboot();
  
  return (mode <= IAP_BOOTLOADER_MODE_OTA) ? ST_ERR_FATAL: ST_BAD_ARGUMENT;
}
