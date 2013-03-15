/*
 * File: clocks.c
 * Description: STM32W108 internal, clock specific HAL functions
 * This file is provided for completeness and it should not be modified
 * by customers as it comtains code very tightly linked to undocumented
 * device features
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include "error.h"

#include "hal/hal.h"
#include "hal/micro/cortexm3/mpu.h"
#include "hal/micro/cortexm3/mfg-token.h"


//Provide a simple means for enabling calibration debug output
#define CALDBG(x)
//#define CALDBG(x) x

//The slowest frequency for the 10kHz RC source is 8kHz (125us).  The PERIOD
//register updates every 16 cycles, so to be safe 17 cycles = 2125us.  But,
//we need twice this maximum time because the period measurement runs
//asynchronously, and the value of CLKRC_TUNE is changed immediately before
//the delay.
#define SLOWRC_PERIOD_SETTLE_TIME 4250
//The CLK_PERIOD register measures the number of 12MHz clock cycles that
//occur in 16 cycles of the SlowRC clock.  This is meant to smooth out the the
//noise inherently present in the analog RC source.  While these 16 cycles
//smooths out most noise, there is still some jitter in the bottom bits of
//CLK_PERIOD.  To further smooth out the noise, we take several readings of
//CLK_PERIOD and average them out.  Testing has shown that the bottom 3 and 4
//bits of CLK_PERIOD contain most of the jitter.  Averaging 8 samples will
//smooth out 3 bits of jitter and provide a realiable and stable reading useful
//in the calculations, while taking much less time than 16 or 32 samples.
#define SLOWRC_PERIOD_SAMPLES 8
//The register CLK1K_CAL is a fractional divider that divides the 10kHz analog
//source with the goal of generating a 1024Hz, clk1k output.
//  10000Hz / CLK1K_CAL = 1024Hz.
//Since the CLK_PERIOD register measures the number of 12MHz cycles in 16
//cycles of the RC:
//     16 * 12000000
//     ------------- = ~10kHz
//      CLK_PERIOD
//and
//  ~10kHz / 1024 = X
//where X is the fractional number that belongs in CLK1K_CAL.  Since the
//integer portion of CLK1K_CAL is bits 15:11 and the fractional is 10:0,
//multiplying X by 2048 (bit shift left by 11) generates the proper CLK1K_CAL
//register value.
//
//Putting this all together:
//     16 * 12000000 * 2048     384000000
//     --------------------  = ------------  =  CLK1K_CAL
//      CLK_PERIOD * 1024       CLK_PERIOD
//
#define CLK1K_NUMERATOR 384000000
void halInternalCalibrateSlowRc( void )
{
  uint8_t i;
  uint32_t average=0;
  int16_t delta;
  uint32_t period;
  
  CALDBG(
    stSerialPrintf(ST_ASSERT_SERIAL_PORT, "halInternalCalibrateSlowRc:\r\n");
  )
  
  ////---- STEP 1: coarsely tune SlowRC in analog section to ~10kHz ----////
    //To operate properly across the full temperature and voltage range,
    //the RC source in the analog section needs to be first coarsely tuned
    //to 10kHz.  The CLKRC_TUNE register, which is 2's compliment, provides 16
    //steps at ~400Hz per step yielding approximate frequences of 8kHz at 7
    //and 15kHz at -8.
    //Start with our reset values for TUNE and CAL
    CLK_PERIODMODE = 0; //measure SlowRC
    CLKRC_TUNE = CLKRC_TUNE_RESET;
    CLK1K_CAL = CLK1K_CAL_RESET;
    //wait for the PERIOD register to properly update
    halCommonDelayMicroseconds(SLOWRC_PERIOD_SETTLE_TIME);
    //Measure the current CLK_PERIOD to obtain a baseline
    CALDBG(
      stSerialPrintf(ST_ASSERT_SERIAL_PORT,
      "period: %u, ", CLK_PERIOD);
      stSerialPrintf(ST_ASSERT_SERIAL_PORT, "%u Hz\r\n", 
                       ((uint16_t)(((uint32_t)192000000)/((uint32_t)CLK_PERIOD))));
    )
    //For 10kHz, the ideal CLK_PERIOD is 19200.  Calculate the PERIOD delta.
    //It's possible for a chip's 10kHz source RC to be too far out of range
    //for the CLKRC_TUNE to bring it back to 10kHz.  Therefore, we have to
    //ensure that our delta correction does not exceed the tune range so
    //tune has to be capped to the end of the vailable range so it does not
    //wrap.  Even if we cannot achieve 10kHz, the 1kHz calibration can still
    //properly correct to 1kHz.
    //Each CLKRC_TUNE step yields a CLK_PERIOD delta of *approximately* 800.
    //Calculate how many steps we are off.  While dividing by 800 may seem
    //like an ugly calculation, the precision of the result is worth the small
    //bit of code and time needed to do a divide.
    period = CLK_PERIOD;
    //Round to the nearest integer
    delta = (19200+400) - period;
    delta /= 800;
    //CLKRC_TUNE is a 4 bit signed number.  cap the delta to 7/-8
    if(delta > 7) {
      delta = 7;
    }
    if(delta < -8) {
      delta = -8;
    }
    CALDBG(
      stSerialPrintf(ST_ASSERT_SERIAL_PORT, "TUNE steps delta: %d\r\n",
                        delta);
    )
    CLKRC_TUNE = delta;
    //wait for PERIOD to update before taking another sample
    halCommonDelayMicroseconds(SLOWRC_PERIOD_SETTLE_TIME);
    CALDBG(
      stSerialPrintf(ST_ASSERT_SERIAL_PORT,
      "period: %u, ", CLK_PERIOD);
      stSerialPrintf(ST_ASSERT_SERIAL_PORT, "%u Hz\r\n", 
                       ((uint16_t)(((uint32_t)192000000)/((uint32_t)CLK_PERIOD))));
    )
    //The analog section should now be producing an output of ~10kHz
    
  ////---- STEP 2: fine tune the SlowRC to 1024Hz ----////
    //Our goal is to generate a 1024Hz source.  The register CLK1K_CAL is a
    //fractional divider that divides the 10kHz analog source and generates
    //the clk1k output.  At reset, the default value is 0x5000 which yields a
    //division of 10.000.  By averaging several samples of CLK_PERIOD, we
    //can then calculate the proper divisor need for CLK1K_CAL to make 1024Hz.
    for(i=0;i<SLOWRC_PERIOD_SAMPLES;i++) {
      halCommonDelayMicroseconds(SLOWRC_PERIOD_SETTLE_TIME);
      average += CLK_PERIOD;
    }
    //calculate the average, with proper rounding
    average = (average+(SLOWRC_PERIOD_SAMPLES/2))/SLOWRC_PERIOD_SAMPLES;
    CALDBG(
      stSerialPrintf(ST_ASSERT_SERIAL_PORT, "average: %u, %u Hz\r\n",
        ((uint16_t)average), ((uint16_t)(((uint32_t)192000000)/((uint32_t)average))));
    )
    
    //using an average period sample, calculate the clk1k divisor
    CLK1K_CAL = (uint16_t)(CLK1K_NUMERATOR/average);
    CALDBG(
      stSerialPrintf(ST_ASSERT_SERIAL_PORT,"CLK1K_CAL=%2X\r\n",CLK1K_CAL);
    )
    //The SlowRC timer is now producing a 1024Hz tick (+/-2Hz).
    
  CALDBG(
    stSerialPrintf(ST_ASSERT_SERIAL_PORT, "DONE\r\n");
  )
}


//The slowest frequency for the FastRC source is 4MHz (250ns).  The PERIOD
//register updates every 256 cycles, so to be safe 257 cycles = 64us.  But,
//we need twice this maximum time because the period measurement runs
//asynchronously, and the value of OSCHF_TUNE is changed immediately before
//the delay.
#define FASTRC_PERIOD_SETTLE_TIME 128
//The CLK_PERIOD register measures the number of 12MHz cycles in 256
//cycles of OSCHF:
//     256 * 12000000
//     ------------- = ~12MHz
//      CLK_PERIOD
void halInternalCalibrateFastRc(void)
{
  int32_t newTune = -16;
  
  CALDBG(
    stSerialPrintf(ST_ASSERT_SERIAL_PORT, "halInternalCalibrateFastRc:\r\n");
  )
  
  ////---- coarsely tune FastRC in analog section to ~12MHz ----////
    //The RC source in the analog section needs to be coarsely tuned
    //to 12MHz.  The OSCHF_TUNE register, which is 2's compliment, provides 32
    //steps at ~0.5MHz per step yielding approximate frequences of 4MHz at 15
    //and 20MHz at -16.
    CLK_PERIODMODE = 1; //measure FastRC
    CALDBG(
      //start at the fastest possible frequency
      OSCHF_TUNE = newTune;
      //wait for the PERIOD register to properly update
      halCommonDelayMicroseconds(FASTRC_PERIOD_SETTLE_TIME);
      //Measure the current CLK_PERIOD to obtain a baseline
      stSerialPrintf(ST_ASSERT_SERIAL_PORT,
      "period: %u, ", CLK_PERIOD);
      stSerialPrintf(ST_ASSERT_SERIAL_PORT, "%u kHz\r\n", 
                       ((uint16_t)((((uint32_t)3072000000)/((uint32_t)CLK_PERIOD))/1000)));
    )
    //For 12MHz, the ideal CLK_PERIOD is 256.  Tune the frequency down until
    //the period is <= 256, which says the frequency is as close to 12MHz as
    //possible (without going over 12MHz)
    //Start at the fastest possible frequency (-16) and increase to the slowest
    //possible (15).  When CLK_PERIOD is <=256 or we run out of tune values,
    //we're done.
    for(;newTune<16;newTune++) {
      //decrease frequency by one step (by increasing tune value)
      OSCHF_TUNE = newTune;
      //wait for the PERIOD register to properly update
      halCommonDelayMicroseconds(FASTRC_PERIOD_SETTLE_TIME);
      //kickout if we're tuned
      if(CLK_PERIOD>=256) {
        break;
      }
    }
    CALDBG(
      //Measure the current CLK_PERIOD to show the final result
      stSerialPrintf(ST_ASSERT_SERIAL_PORT,
      "period: %u, ", CLK_PERIOD);
      stSerialPrintf(ST_ASSERT_SERIAL_PORT, "%u kHz\r\n", 
                       ((uint16_t)((((uint32_t)3072000000)/((uint32_t)CLK_PERIOD))/1000)));
    )
    
    //The analog section should now be producing an output of 11.5MHz - 12.0MHz
}










































#define OSC24M_BIASTRIM_OFFSET  (0x2)
#define OSC24M_BIASTRIM_MIN     (0+OSC24M_BIASTRIM_OFFSET)
#define OSC24M_BIASTRIM_MAX     OSC24M_BIASTRIM_OSC24M_BIAS_TRIM_MASK
#define OSC24M_BIASTRIM_MSB     (1 << (OSC24M_BIASTRIM_OSC24M_BIAS_TRIM_BITS-1))
#define OSC24M_BIASTRIM_UNINIT  (0xFFFF)
tokTypeMfgOsc24mBiasTrim biasTrim=OSC24M_BIASTRIM_UNINIT;


































//This function is intended to be called periodically, from the stack and
//application, to check the XTAL bias trim is within appropriate levels
//and adjust if not.  This function is *not* designed to be used before
//halInternalSwitchToXtal has been called.
void halCommonCheckXtalBiasTrim(void)
{
  //HI is set indicating the trim value is too high.  Decrement the trim.
  if((OSC24M_COMP & OSC24M_HI) == OSC24M_HI) {
    biasTrim--;
  }
  
  //LO is cleared indicating the trim value is too low.  Inrement the trim.
  if((OSC24M_COMP & OSC24M_LO) != OSC24M_LO) {
    biasTrim++;
    //Add an offset to the bias trim as a factor of safety.
    if(biasTrim < (OSC24M_BIASTRIM_MAX - OSC24M_BIASTRIM_OFFSET)) {
      biasTrim +=  OSC24M_BIASTRIM_OFFSET;
    } else {
      biasTrim = OSC24M_BIASTRIM_MAX;
    }
  }
  
  //Don't allow bias trim to dip below the offset regardless of LO.
  if(biasTrim<OSC24M_BIASTRIM_OFFSET) {
    biasTrim = OSC24M_BIASTRIM_OFFSET;
  }
  
  OSC24M_BIASTRIM = biasTrim;
}

static boolean setBiasCheckLow(void)
{
  OSC24M_BIASTRIM = biasTrim;
  halCommonDelayMicroseconds(1500);
  return ((OSC24M_COMP & OSC24M_LO) == OSC24M_LO);
}

void halInternalSearchForBiasTrim(void)
{
  uint8_t bit;
  
  //Enable the XTAL so we can search for the proper bias trim (NOTE: This
  //will also forcefully ensure we're on the OSCHF so that we don't
  //accidentally trip the NMI while searching.)
  OSC24M_CTRL = OSC24M_CTRL_OSC24M_EN;
  
  //Do a binary search of the 4-bit bias trim values to find
  //smallest bias trim value for which LO = 1.
  biasTrim = 0;
  bit = (OSC24M_BIASTRIM_MSB << 1);
  do {
    bit >>= 1;
    biasTrim += bit;
    //Set trim and wait for 1.5ms to allow the oscillator to stabilize.
    if(setBiasCheckLow()) {
      biasTrim -= bit;
    }
  } while(bit);
  
  //If the last bias value went too low, increment it.
  if((OSC24M_COMP & OSC24M_LO) != OSC24M_LO) {
    biasTrim++;
  }
  
  //Add an offset to the bias trim as a factor of safety.
  if(biasTrim < (OSC24M_BIASTRIM_MAX - OSC24M_BIASTRIM_OFFSET)) {
    biasTrim +=  OSC24M_BIASTRIM_OFFSET;
  } else {
    biasTrim = OSC24M_BIASTRIM_MAX;
  }
  
  //Using the shadow variable, the clock switch logic will take over from here,
  //enabling, verifying, and tweaking as needed.
}


//This function configures the flash access controller for optimal
//current consumption when FCLK is operating at 24MHz.  By providing
//this function the calling code does not have to be aware of the
//details of setting FLASH_ACCESS.
static void halInternalConfigXtal24MhzFlashAccess(void)
{
  ATOMIC(
    BYPASS_MPU( 
      #if defined(CORTEXM3_STM32W108)
        FLASH_ACCESS = (FLASH_ACCESS_PREFETCH_EN          |
                        (1<<FLASH_ACCESS_CODE_LATENCY_BIT));
      #endif
    )
  )
} 

//NOTE:  The global "shadow" variable biasTrim will be set by either:
// A) TOKEN_MFG_OSC24M_BIAS_TRIM when booting fresh
// B) searchForBiasTrim() when booting fresh and the token is not valid 
// C) halInternalSwitchToXtal() if halInternalSwitchToXtal() already ran
void halInternalSwitchToXtal(void)
{
  boolean loSet;
  boolean hiSet;
  boolean setTrimOneLastTime = FALSE;
  
  //If it hasn't yet been initialized, 
  //preload our biasTrim shadow variable from the token.  If the token is
  //not set, then run a search to find an initial value.  The bias trim
  //algorithm/clock switch logic will always use the biasTrim shadow
  //variable as the starting point for finding the bias, and then
  //save that new bias to the shadow variable.
  if(biasTrim == OSC24M_BIASTRIM_UNINIT) {
    halCommonGetMfgToken(&biasTrim, TOKEN_MFG_OSC24M_BIAS_TRIM);
    if(biasTrim == 0xFFFF) {
      halInternalSearchForBiasTrim();
    }
  }

  //Ensure the XTAL is enabled (with the side effect of ensuring we're
  //still on OSCHF).
  OSC24M_CTRL = OSC24M_CTRL_OSC24M_EN;
  
  do {
    //Set trim to our shadow variable and wait for 1.5ms to allow the
    //oscillator to stabilize.
    loSet = setBiasCheckLow();
    hiSet = (OSC24M_COMP & OSC24M_HI) == OSC24M_HI;
    
    //The bias is too low, so we need to increment the bias trim.
    if(!loSet) {
      biasTrim++;
    }
    
    //The bias is too high, so we need to decrement the bias trim.
    if(hiSet) {
      //but don't trim below our min value
      if(biasTrim>OSC24M_BIASTRIM_MIN) {
        biasTrim--;
        setTrimOneLastTime = TRUE;
      }
    }
    
    //Kickout when HI=0 and LO=1 or we've hit the MAX or the MIN
  } while( (hiSet || !loSet)              &&
           (biasTrim<OSC24M_BIASTRIM_MAX) &&
           (biasTrim>OSC24M_BIASTRIM_MIN) );
  
  //The LO bit being cleared means we've corrected up from the bottom and
  //therefore need to apply the offset.  Additionally, if our trim value
  //is below the offset, we still need to apply the offset.  And, when
  //applying the offset respect the max possible value of the trim.
  if(!loSet || (biasTrim<OSC24M_BIASTRIM_OFFSET)){  
    if(biasTrim < (OSC24M_BIASTRIM_MAX - OSC24M_BIASTRIM_OFFSET)) {
      biasTrim +=  OSC24M_BIASTRIM_OFFSET;
    } else {
      biasTrim = OSC24M_BIASTRIM_MAX;
    }
    setTrimOneLastTime = TRUE;
  }
  
  if(setTrimOneLastTime) {
    setBiasCheckLow();
  }
  
  //We've found a valid trim value and we've waited for the oscillator
  //to stabalize, it's now safe to select the XTAL
  OSC24M_CTRL |= OSC24M_CTRL_OSC24M_SEL;
  
  //If the XTAL switch failed, the NMI ISR will trigger, creeping the bias
  //trim up higher, and if max bias is reached the ISR will trigger a reset.
  
  //Our standard mode of operation is 24MHz (CPU/FCLK is sourced from SYSCLK)
  CPU_CLKSEL = CPU_CLKSEL_FIELD;
  //Configure flash access for optimal current consumption at 24MHz
  halInternalConfigXtal24MhzFlashAccess();
}
