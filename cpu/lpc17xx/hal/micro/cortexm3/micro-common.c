/** @file micro-common.c
 *  @brief STM32W108 micro specific HAL functions common to 
 *  full and minimal hal
 *
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */



#include PLATFORM_HEADER
#include BOARD_HEADER
#include "error.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

void halInternalEnableWatchDog(void)
{
  //Just to be on the safe side, restart the watchdog before enabling it
  WDOG_RESET = 1;
  WDOG_KEY = 0xEABE;
  WDOG_CFG = WDOG_ENABLE;
}

void halInternalResetWatchDog(void)
{
  //Writing any value will restart the watchdog
  WDOG_RESET = 1;
}

void halInternalDisableWatchDog(int8u magicKey)
{
  if (magicKey == MICRO_DISABLE_WATCH_DOG_KEY) {
    WDOG_KEY = 0xDEAD;
    WDOG_CFG = WDOG_DISABLE;
  }
}

boolean halInternalWatchDogEnabled(void)
{
  if(WDOG_CFG&WDOG_ENABLE) {
    return TRUE;
  } else {
    return FALSE;
  }
}

void halGpioConfig(int32u io, int32u config)
{
  static volatile int32u *const configRegs[] = 
    { (volatile int32u *)GPIO_PACFGL_ADDR,
      (volatile int32u *)GPIO_PACFGH_ADDR,
      (volatile int32u *)GPIO_PBCFGL_ADDR,
      (volatile int32u *)GPIO_PBCFGH_ADDR,
      (volatile int32u *)GPIO_PCCFGL_ADDR,
      (volatile int32u *)GPIO_PCCFGH_ADDR };
  int32u portcfg;
  portcfg = *configRegs[io/4];                // get current config                   
  portcfg = portcfg & ~((0xF)<<((io&3)*4));   // mask out config of this pin
  *configRegs[io/4] = portcfg | (config <<((io&3)*4));
}

void halGpioSet(int32u gpio, boolean value)
{
  if(gpio/8 < 3) {
    if (value) {
      *((volatile int32u *)(GPIO_PxSET_BASE+(GPIO_Px_OFFSET*(gpio/8)))) = BIT(gpio&7);
    } else {
      *((volatile int32u *)(GPIO_PxCLR_BASE+(GPIO_Px_OFFSET*(gpio/8)))) = BIT(gpio&7);
    }
  }
}

int16u halInternalStartSystemTimer(void)
{
  //Since the SleepTMR is the only timer maintained during deep sleep, it is
  //used as the System Timer (RTC).  We maintain a 32 bit hardware timer
  //configured for a tick value time of 1024 ticks/second (0.9765625 ms/tick)
  //using either the 10 kHz internal SlowRC clock divided and calibrated to
  //1024 Hz or the external 32.768 kHz crystal divided to 1024 Hz.
  //With a tick time of ~1ms, this 32bit timer will wrap after ~48.5 days.
  
  //disable top-level interrupt while configuring
  INT_CFGCLR = INT_SLEEPTMR;
  
  #ifdef ENABLE_OSC32K
    #ifdef DIGITAL_OSC32_EXT
      //Disable both OSC32K and SLOWRC if using external digital clock input
      SLEEPTMR_CLKEN = 0;
    #else//!DIGITAL_OSC32_EXT
      //Enable the 32kHz XTAL (and disable SlowRC since it is not needed)
      SLEEPTMR_CLKEN = SLEEPTMR_CLK32KEN;
    #endif
    //Sleep timer configuration is the same for crystal and external clock
    SLEEPTMR_CFG = (SLEEPTMR_ENABLE            | //enable TMR
                   (0 << SLEEPTMR_DBGPAUSE_BIT)| //TMR paused when halted
                   (5 << SLEEPTMR_CLKDIV_BIT)  | //divide down to 1024Hz
                   (1 << SLEEPTMR_CLKSEL_BIT)) ; //select XTAL
  #else //!ENABLE_OSC32K
    //Enable the SlowRC (and disable 32kHz XTAL since it is not needed)
    SLEEPTMR_CLKEN = SLEEPTMR_CLK10KEN;
    SLEEPTMR_CFG = (SLEEPTMR_ENABLE            | //enable TMR
                   (0 << SLEEPTMR_DBGPAUSE_BIT)| //TMR paused when halted
                   (0 << SLEEPTMR_CLKDIV_BIT)  | //already 1024Hz
                   (0 << SLEEPTMR_CLKSEL_BIT)) ; //select SlowRC
    #ifndef DISABLE_RC_CALIBRATION
      halInternalCalibrateSlowRc(); //calibrate SlowRC to 1024Hz
    #endif//DISABLE_RC_CALIBRATION
  #endif//ENABLE_OSC32K
  
  //clear out any stale interrupts
  INT_SLEEPTMRFLAG = (INT_SLEEPTMRWRAP | INT_SLEEPTMRCMPA | INT_SLEEPTMRCMPB);
  //turn off second level interrupts.  they will be enabled elsewhere as needed
  INT_SLEEPTMRCFG = INT_SLEEPTMRCFG_RESET;
  //enable top-level interrupt
  INT_CFGSET = INT_SLEEPTMR;
  
  return 0;
}


