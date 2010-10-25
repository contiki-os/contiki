/** @file micro.c
 * @brief STM32W108 micro specific minimal HAL functions
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include "error.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"
#include "micro/system-timer.h"
#include "micro/adc.h"

#include <stdlib.h>
#include <string.h>


void halInit(void)
{
  //Disable the REG_EN external regulator enable signal.  Out of reset this
  //signal overrides PA7.  By disabling it early, PA7 is reclaimed as a GPIO.
  //If an external regulator is required, the following line of code should
  //be deleted.
  GPIO_DBGCFG &= ~GPIO_EXTREGEN;
  halInternalSetRegTrim(FALSE);
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
}

void halPowerUp(void)
{
  halInternalInitAdc();
  halCommonCalibratePads();  
  halInternalSwitchToXtal();
}

static int16u seed0 = 0xbeef;
static int16u seed1 = 0xface;

void halCommonSeedRandom(int32u seed)
{
  seed0 = (int16u) seed;
  if (seed0 == 0)
    seed0 = 0xbeef;
  seed1 = (int16u) (seed >> 16);
  if (seed1 == 0)
    seed1 = 0xface;
}

static int16u shift(int16u *val, int16u taps)
{
  int16u newVal = *val;

  if (newVal & 0x8000)
    newVal ^= taps;
  *val = newVal << 1;
  return newVal;
}

int16u halCommonGetRandom(void)
{
  return (shift(&seed0, 0x0062)
          ^ shift(&seed1, 0x100B));
}

void halCommonMemCopy(void *dest, const void *source, int8u bytes)
{
  memcpy(dest, source, bytes);
}

int8s halCommonMemCompare(const void *source0, const void *source1, int8u bytes)
{
  return memcmp(source0, source1, bytes);
}

void halCommonMemSet(void *dest, int8u val, int16u bytes)
{
  memset(dest, val, bytes);
}
