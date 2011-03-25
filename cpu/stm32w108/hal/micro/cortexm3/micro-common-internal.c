/*
 * File: micro-common-internal.c
 * Description: STM32W108 internal, micro specific HAL functions.
 * This file is provided for completeness and it should not be modified
 * by customers as it comtains code very tightly linked to undocumented
 * device features
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include "error.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"
#include "hal/micro/cortexm3/mfg-token.h"

#define HAL_STANDALONE
#ifdef HAL_STANDALONE

#define AUXADC_REG (0xC0u)
#define DUMMY                   0

#define ADC_6MHZ_CLOCK          0
#define ADC_1MHZ_CLOCK          1

#define ADC_SAMPLE_CLOCKS_32    0
#define ADC_SAMPLE_CLOCKS_64    1
#define ADC_SAMPLE_CLOCKS_128   2
#define ADC_SAMPLE_CLOCKS_256   3
#define ADC_SAMPLE_CLOCKS_512   4
#define ADC_SAMPLE_CLOCKS_1024  5
#define ADC_SAMPLE_CLOCKS_2048  6
#define ADC_SAMPLE_CLOCKS_4096  7

#define CAL_ADC_CHANNEL_VDD_4   0x00  //VDD_PADS/4
#define CAL_ADC_CHANNEL_VREG_2  0x01  //VREG_OUT/2
#define CAL_ADC_CHANNEL_TEMP    0x02
#define CAL_ADC_CHANNEL_GND     0x03
#define CAL_ADC_CHANNEL_VREF    0x04
#define CAL_ADC_CHANNEL_I       0x06
#define CAL_ADC_CHANNEL_Q       0x07
#define CAL_ADC_CHANNEL_ATEST_A 0x09

void stCalibrateVref(void)
{
  // Calibrate Vref by measuring a known voltage, Vdd/2.
  //
  // FIXME: add support for calibration if done in boost mode.
  extern int16u stmRadioTxPowerMode;
  tokTypeMfgAnalogueTrimBoth biasTrim;
  
  halCommonGetMfgToken(&biasTrim, TOKEN_MFG_ANALOG_TRIM_BOTH);
  
  if(biasTrim.auxadc == 0xFFFF) {
    assert(FALSE);
  } else {
    //The bias trim token is set, so use the trim directly
    int16u temp_value;
    int16u mask = 0xFFFF;

    // halClearLed(BOARDLED3);

    while (SCR_BUSY_REG) ;

    SCR_ADDR_REG = AUXADC_REG ;  // prepare the address to write to

    // initiate read (starts on falling edge of SCR_CTRL_SCR_READ)
    SCR_CTRL_REG = SCR_CTRL_SCR_READ_MASK;
    SCR_CTRL_REG = 0;

    // wait for read to complete
    while (SCR_BUSY_REG) ;

    temp_value = SCR_READ_REG & ~mask;
    temp_value |= biasTrim.auxadc & mask;
    
    SCR_WRITE_REG = temp_value;

    // initiate write (starts on falling edge of SCR_CTRL_SCR_WRITE_MASK)
    SCR_CTRL_REG = SCR_CTRL_SCR_WRITE_MASK;
    SCR_CTRL_REG = 0;

    while (SCR_BUSY_REG) ;
    
  }
}


void calDisableAdc(void) {
  // Disable the Calibration ADC to save current.
  CAL_ADC_CONFIG &= ~CAL_ADC_CONFIG_CAL_ADC_EN;
}



// These routines maintain the same signature as their hal- counterparts to
// facilitate simple support between phys.
// It is assumed (hoped?) that the compiler will optimize out unused arguments.
StStatus calStartAdcConversion(int8u dummy1, // Not used.
                                  int8u dummy2, // Not used.
                                  int8u channel,
                                  int8u rate,
                                  int8u clock) {
  // Disable the Calibration ADC interrupt so that we can poll it.
  INT_MGMTCFG &= ~INT_MGMTCALADC;

  ATOMIC(
    // Enable the Calibration ADC, choose source, set rate, and choose clock.
    CAL_ADC_CONFIG =((CAL_ADC_CONFIG_CAL_ADC_EN)                  |
                     (channel << CAL_ADC_CONFIG_CAL_ADC_MUX_BIT)  |
                     (rate << CAL_ADC_CONFIG_CAL_ADC_RATE_BIT)    |
                     (clock << CAL_ADC_CONFIG_CAL_ADC_CLKSEL_BIT) );
    // Clear any pending Calibration ADC interrupt.  Since we're atomic, the
    // one we're interested in hasn't happened yet (will take ~10us at minimum).
    // We're only clearing stale info.
    INT_MGMTFLAG = INT_MGMTCALADC;
  )
  return ST_SUCCESS;
}


StStatus calReadAdcBlocking(int8u  dummy,
                               int16u *value) {
  // Wait for conversion to complete.
  while ( ! (INT_MGMTFLAG & INT_MGMTCALADC) );
  // Clear the interrupt for this conversion.
  INT_MGMTFLAG = INT_MGMTCALADC;
  // Get the result.
  *value = (int16u)CAL_ADC_DATA;
  return ST_SUCCESS;
}




//Using 6MHz clock reduces resolution but greatly increases conversion speed.
//The sample clocks were chosen based upon empirical evidence and provided
//the fastest conversions with the greatest reasonable accuracy.  Variation
//across successive conversions appears to be +/-20mv of the average
//conversion.  Overall function time is <150us.
int16u stMeasureVddFast(void)
{
  int16u value;
  int32u Ngnd;
  int32u Nreg;
  int32u Nvdd;
  tokTypeMfgRegVoltage1V8 vregOutTok;
  halCommonGetMfgToken(&vregOutTok, TOKEN_MFG_1V8_REG_VOLTAGE);
  
  //Measure GND
  calStartAdcConversion(DUMMY,
                        DUMMY,
                        CAL_ADC_CHANNEL_GND,
                        ADC_SAMPLE_CLOCKS_128,
                        ADC_6MHZ_CLOCK);
  calReadAdcBlocking(DUMMY, &value);
  Ngnd = (int32u)value;
  
  //Measure VREG_OUT/2
  calStartAdcConversion(DUMMY,
                        DUMMY,
                        CAL_ADC_CHANNEL_VREG_2,
                        ADC_SAMPLE_CLOCKS_128,
                        ADC_6MHZ_CLOCK);
  calReadAdcBlocking(DUMMY, &value);
  Nreg = (int32u)value;
  
  //Measure VDD_PADS/4
  calStartAdcConversion(DUMMY,
                        DUMMY,
                        CAL_ADC_CHANNEL_VDD_4,
                        ADC_SAMPLE_CLOCKS_128,
                        ADC_6MHZ_CLOCK);
  calReadAdcBlocking(DUMMY, &value);
  Nvdd = (int32u)value;
  
  calDisableAdc();
  
  //Convert the value into mV.  VREG_OUT is ideally 1.8V, but it wont be
  //exactly 1.8V.  The actual value is stored in the manufacturing token
  //TOKEN_MFG_1V8_REG_VOLTAGE.  The token stores the value in 10^-4, but we
  //need 10^-3 so divide by 10.  If this token is not set (0xFFFF), then
  //assume 1800mV.
  if(vregOutTok == 0xFFFF) {
    vregOutTok = 1800;
  } else {
    vregOutTok /= 10;
  }
  return ((((((Nvdd-Ngnd)<<16)/(Nreg-Ngnd))*vregOutTok)*2)>>16);
}
#endif

void halCommonCalibratePads(void)
{
  if(stMeasureVddFast() < 2700) {
    GPIO_DBGCFG |= GPIO_DBGCFGRSVD;
  } else {
    GPIO_DBGCFG &= ~GPIO_DBGCFGRSVD;
  }
}


void halInternalSetRegTrim(boolean boostMode)
{
  tokTypeMfgRegTrim regTrim;
  int8u trim1V2;
  int8u trim1V8;
  
  halCommonGetMfgToken(&regTrim, TOKEN_MFG_REG_TRIM);
  // The compiler can optimize this function a bit more and keep the 
  // values in processor registers if we use separate local vars instead
  // of just accessing via the structure fields
  trim1V8 = regTrim.regTrim1V8;
  trim1V2 = regTrim.regTrim1V2;
  
  //If tokens are erased, default to reasonable values, otherwise use the
  //token values.
  if((trim1V2 == 0xFF) && (trim1V8 == 0xFF)) {
    trim1V8 = 4;
    trim1V2 = 0;
  }
  
  //When the radio is in boost mode, we have to increase the 1.8V trim.
  if(boostMode) {
    trim1V8 += 2;
  }
  
  //Clamp at 7 to ensure we don't exceed max values, accidentally set
  //other bits, or wrap values.
  if(trim1V8>7) {
    trim1V8 = 7;
  }
  if(trim1V2>7) {
    trim1V2 = 7;
  }
  
  VREG_REG = ( (trim1V8<<VREG_VREG_1V8_TRIM_BIT) |
               (trim1V2<<VREG_VREG_1V2_TRIM_BIT) );
}


// halCommonDelayMicroseconds
// -enables MAC Timer and leaves it enabled.
// -does not touch MAC Timer Compare registers.
// -max delay is 65535 usec.
// NOTE: This function primarily designed for when the chip is running off of
//       the XTAL, which is the most common situation.  When running from
//       OSCHF, though, the clock speed is cut in half, so the input parameter
//       is divided by two.  With respect to accuracy, we're now limited by
//       the accuracy of OSCHF (much lower than XTAL).
void halCommonDelayMicroseconds(int16u us)
{
  int32u beginTime = ReadRegister(MAC_TIMER);
  
  //If we're not using the XTAL, the MAC Timer is running off OSCHF,
  //that means the clock is half speed, 6MHz.  We need to halve our delay
  //time.
  if((OSC24M_CTRL&OSC24M_CTRL_OSC24M_SEL)!=OSC24M_CTRL_OSC24M_SEL) {
    us >>= 1;
  }
    
  //we have about 2us of overhead in the calculations
  if(us<=2) {
    return;
  }
  
  // MAC Timer is enabled in stmRadioInit, which may not have been called yet.
  // This algorithm needs the MAC Timer so we enable it here.
  MAC_TIMER_CTRL |= MAC_TIMER_CTRL_MAC_TIMER_EN;

  // since our max delay (65535<<1) is less than half the size of the 
  //  20 bit mac timer, we can easily just handle the potential for
  //  mac timer wrapping by subtracting the time delta and masking out
  //  the extra bits
  while( ((MAC_TIMER-beginTime)&MAC_TIMER_MAC_TIMER_MASK) < us ) {
    ; // spin
  }
}


//Burning cycles for milliseconds is generally a bad idea, but it is
//necessary in some situations.  If you have to burn more than 65ms of time,
//the halCommonDelayMicroseconds function becomes cumbersome, so this
//function gives you millisecond granularity.
void halCommonDelayMilliseconds(int16u ms)
{
  if(ms==0) {
    return;
  }
  
  while(ms-->0) {
    halCommonDelayMicroseconds(1000);
  }
}
