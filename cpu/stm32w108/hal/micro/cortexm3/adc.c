/** @file cpu/stm32w108/hal/micro/cortexm3/adc.c
 * @brief  ADC HAL functions
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#include PLATFORM_HEADER
#include "hal/error.h"
#include "hal/hal.h"
#include "hal/micro/adc.h"


#if (NUM_ADC_USERS > 8)
  #error NUM_ADC_USERS must not be greater than 8, or uint8_t variables in adc.c must be changed
#endif

static uint16_t adcData;             // conversion result written by DMA
static uint8_t adcPendingRequests;   // bitmap of pending requests
volatile static uint8_t adcPendingConversion; // id of pending conversion
static uint8_t adcReadingValid;      // bitmap of valid adcReadings
static uint16_t adcReadings[NUM_ADC_USERS];
static uint16_t adcConfig[NUM_ADC_USERS];
static boolean adcCalibrated;
static int16_t Nvss;
static int16_t Nvdd;
/* Modified the original ADC driver for enabling the ADC extended range mode required for
   supporting the STLM20 temperature sensor.
   NOTE:
   The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC
   (see STM32W108 errata). As consequence, it is not reccomended to use this ADC driver for getting
   the temperature values
*/
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
static int16_t Nvref;
static int16_t Nvref2;
#endif /* ENABLE_ADC_EXTENDED_RANGE_BROKEN */
static uint16_t adcStaticConfig;

void halAdcSetClock(boolean slow)
{
  if (slow) {
    adcStaticConfig |= ADC_1MHZCLK_MASK;
  } else {
    adcStaticConfig &= ~ADC_1MHZCLK_MASK;
  }
}

void halAdcSetRange(boolean high)
{
  if (high) {
    adcStaticConfig |= (ADC_HVSELP_MASK | ADC_HVSELN_MASK);
  } else {
    adcStaticConfig &= ~(ADC_HVSELP_MASK | ADC_HVSELN_MASK);
  }
}

boolean halAdcGetClock(void)
{
  /* Fix original function code */
  return (adcStaticConfig & ADC_1MHZCLK_MASK) ? TRUE : FALSE;
}

boolean halAdcGetRange(void)
{
  /* Fix original function code */
  return (adcStaticConfig & ((ADC_HVSELP_MASK | ADC_HVSELN_MASK))) ? TRUE : FALSE;
}



// Define a channel field that combines ADC_MUXP and ADC_MUXN
#define ADC_CHAN        (ADC_MUXP | ADC_MUXN)
#define ADC_CHAN_BIT    ADC_MUXN_BIT

void halAdcIsr(void)
{
  uint8_t i;
  uint8_t conversion = adcPendingConversion; //fix 'volatile' warning; costs no flash

  // make sure data is ready and the desired conversion is valid
  if ( (INT_ADCFLAG & INT_ADCULDFULL)
        && (conversion < NUM_ADC_USERS) ) {
    adcReadings[conversion] = adcData;
    adcReadingValid |= BIT(conversion); // mark the reading as valid
    // setup the next conversion if any
    if (adcPendingRequests) {
      for (i = 0; i < NUM_ADC_USERS; i++) {
        if (BIT(i) & adcPendingRequests) {
          adcPendingConversion = i;     // set pending conversion
          adcPendingRequests ^= BIT(i); //clear request: conversion is starting
          ADC_CFG = adcConfig[i];
          break; //conversion started, so we're done here (only one at a time)
        }
      }
    } else {                                // no conversion to do
      ADC_CFG = 0;                          // disable adc
      adcPendingConversion = NUM_ADC_USERS; //nothing pending, so go "idle"
    }
  }
  INT_ADCFLAG = 0xFFFF;
  asm("DMB");
}

// An internal support routine called from functions below.
// Returns the user number of the started conversion, or NUM_ADC_USERS
// otherwise.
ADCUser startNextConversion()
{
  uint8_t i;

  ATOMIC (
    // start the next requested conversion if any
    if (adcPendingRequests && !(ADC_CFG & ADC_ENABLE)) {
      for (i = 0; i < NUM_ADC_USERS; i++) {
        if ( BIT(i) & adcPendingRequests) {
          adcPendingConversion = i;     // set pending conversion
          adcPendingRequests ^= BIT(i); // clear request
          ADC_CFG = adcConfig[i];       // set the configuration to desired
          INT_ADCFLAG = 0xFFFF;
          INT_CFGSET = INT_ADC;
	  break;                       //see DDTS MBTst38936
        }
      }
    } else {
      i = NUM_ADC_USERS;
    }
  )
  return i;
}

void halInternalInitAdc(void)
{
  // reset the state variables
  adcPendingRequests = 0;
  adcPendingConversion = NUM_ADC_USERS;
  adcCalibrated = FALSE;
  adcStaticConfig = ADC_1MHZCLK | ADC_ENABLE; // init config: 1MHz, low voltage

  // set all adcReadings as invalid
  adcReadingValid = 0;

  // turn off the ADC
  ADC_CFG = 0;                   // disable ADC, turn off HV buffers
  ADC_OFFSET = ADC_OFFSET_RESET;
  ADC_GAIN = ADC_GAIN_RESET;
  ADC_DMACFG = ADC_DMARST;
  ADC_DMABEG = (uint32_t)&adcData;
  ADC_DMASIZE = 1;
  ADC_DMACFG = (ADC_DMAAUTOWRAP | ADC_DMALOAD);

  // clear the ADC interrupts and enable
  INT_ADCCFG = INT_ADCULDFULL;
  INT_ADCFLAG = 0xFFFF;
  INT_CFGSET = INT_ADC;

  stCalibrateVref();
}

StStatus halStartAdcConversion(ADCUser id,
                               ADCReferenceType reference,
                               ADCChannelType channel,
                               ADCRateType rate)
{

   if(reference != ADC_REF_INT)
    return ST_ERR_FATAL;

  // save the chosen configuration for this user
  adcConfig[id] = ( ((rate << ADC_PERIOD_BIT) & ADC_PERIOD)
                  | ((channel << ADC_CHAN_BIT) & ADC_CHAN)
                  | adcStaticConfig);

  // if the user already has a pending request, overwrite params
  if (adcPendingRequests & BIT(id)) {
    return ST_ADC_CONVERSION_DEFERRED;
  }

  ATOMIC (
    // otherwise, queue the transaction
    adcPendingRequests |= BIT(id);
    // try and start the conversion if there is not one happening
    adcReadingValid &= ~BIT(id);
  )
  if (startNextConversion() == id)
    return ST_ADC_CONVERSION_BUSY;
  else
    return ST_ADC_CONVERSION_DEFERRED;
}

StStatus halRequestAdcData(ADCUser id, uint16_t *value)
{
  //Both the ADC interrupt and the global interrupt need to be enabled,
  //otherwise the ADC ISR cannot be serviced.
  boolean intsAreOff = ( INTERRUPTS_ARE_OFF()
                        || !(INT_CFGSET & INT_ADC)
                        || !(INT_ADCCFG & INT_ADCULDFULL) );
  StStatus stat;

  ATOMIC (
    // If interupts are disabled but the flag is set,
    // manually run the isr...
    //FIXME -= is this valid???
    if( intsAreOff
      && ( (INT_CFGSET & INT_ADC) && (INT_ADCCFG & INT_ADCULDFULL) )) {
      halAdcIsr();
    }

    // check if we are done
    if (BIT(id) & adcReadingValid) {
      *value = adcReadings[id];
      adcReadingValid ^= BIT(id);
      stat = ST_ADC_CONVERSION_DONE;
    } else if (adcPendingRequests & BIT(id)) {
      stat = ST_ADC_CONVERSION_DEFERRED;
    } else if (adcPendingConversion == id) {
      stat = ST_ADC_CONVERSION_BUSY;
    } else {
      stat = ST_ADC_NO_CONVERSION_PENDING;
    }
  )
  return stat;
}

StStatus halReadAdcBlocking(ADCUser id, uint16_t *value)
{
  StStatus stat;

  do {
    stat = halRequestAdcData(id, value);
    if (stat == ST_ADC_NO_CONVERSION_PENDING)
      break;
  } while(stat != ST_ADC_CONVERSION_DONE);
  return stat;
}

StStatus halAdcCalibrate(ADCUser id)
{
  StStatus stat;
/* Modified the original ADC driver for enabling the ADC extended range mode required for
     supporting the STLM20 temperature sensor.
     NOTE:
     The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC
     (see STM32W108 errata). As consequence, it is not reccomended to use this ADC driver for getting
     the temperature values
   */
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
  if(halAdcGetRange()){

    halStartAdcConversion(id,
                          ADC_REF_INT,
                          ADC_SOURCE_VREF_VREF2,
                          ADC_CONVERSION_TIME_US_4096);

    stat = halReadAdcBlocking(id, (uint16_t *)(&Nvref));
    if (stat == ST_ADC_CONVERSION_DONE) {
      halStartAdcConversion(id,
                            ADC_REF_INT,
                            ADC_SOURCE_VREF2_VREF2,
                            ADC_CONVERSION_TIME_US_4096);
      stat = halReadAdcBlocking(id, (uint16_t *)(&Nvref2));
    }
    if (stat == ST_ADC_CONVERSION_DONE) {
      adcCalibrated = TRUE;
    } else {
      adcCalibrated = FALSE;
      stat = ST_ERR_FATAL;
    }
    return stat;

  }
#endif /* ENABLE_ADC_EXTENDED_RANGE_BROKEN */
  halStartAdcConversion(id,
                        ADC_REF_INT,
                        ADC_SOURCE_GND_VREF2,
                        ADC_CONVERSION_TIME_US_4096);
  stat = halReadAdcBlocking(id, (uint16_t *)(&Nvss));
  if (stat == ST_ADC_CONVERSION_DONE) {
    halStartAdcConversion(id,
                          ADC_REF_INT,
                          ADC_SOURCE_VREG2_VREF2,
                          ADC_CONVERSION_TIME_US_4096);
    stat = halReadAdcBlocking(id, (uint16_t *)(&Nvdd));
  }
  if (stat == ST_ADC_CONVERSION_DONE) {
    Nvdd -= Nvss;
    adcCalibrated = TRUE;
  } else {
    adcCalibrated = FALSE;
    stat = ST_ERR_FATAL;
  }
  return stat;
}

// Use the ratio of the sample reading to the of VDD_PADSA/2, known to be 900mV,
// to convert to 100uV units.
// FIXME: support external Vref
//        use #define of Vref, ignore VDD_PADSA
// FIXME: support  high voltage range
//        use Vref-Vref/2 to calibrate
// FIXME: check for mfg token specifying measured VDD_PADSA
int16_t halConvertValueToVolts(uint16_t value)
{
  int32_t N;
  int16_t V;
  int32_t nvalue;

  if (!adcCalibrated) {
    halAdcCalibrate(ADC_USER_LQI);
  }
  if (adcCalibrated) {
 /* Modified the original ADC driver for enabling the ADC extended range mode required for
     supporting the STLM20 temperature sensor.
     NOTE:
     The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC
     (see STM32W108 errata). As consequence, it is not reccomended to use this ADC driver for getting
     the temperature values
   */
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
    if(halAdcGetRange()){  // High range.

      N = (((int32_t)value + Nvref - 2*Nvref2) << 16)/(2*(Nvref-Nvref2));
      // Calculate voltage with: V = (N * VREF) / (2^16) where VDD = 1.2 volts
      // Mutiplying by 1.2*10000 makes the result of this equation 100 uVolts
      V = (int16_t)((N*12000L) >> 16);
      if (V > 21000) {  // VDD_PADS ?
        V = 21000;
      }

    }
    else {
 #endif /* ENABLE_ADC_EXTENDED_RANGE_BROKEN */
      assert(Nvdd);
      nvalue = value - Nvss;
      // Convert input value (minus ground) to a fraction of VDD/2.
      N = ((nvalue << 16) + Nvdd/2) / Nvdd;
      // Calculate voltage with: V = (N * VDD/2) / (2^16) where VDD/2 = 0.9 volts
      // Mutiplying by0.9*10000 makes the result of this equation 100 uVolts
      // (in fixed point E-4 which allows for 13.5 bits vs millivolts
      // which is only 10.2 bits).
      V = (int16_t)((N*9000L) >> 16);
      if (V > 12000) {
        V = 12000;
      }
 #ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
    }
 #endif /* ENABLE_ADC_EXTENDED_RANGE_BROKEN */
  } else {
    V = -32768;
  }
  return V;
}

uint8_t halGetADCChannelFromGPIO(uint32_t io)
{
	switch(io)
	{
	case PORTB_PIN(5):
		return ADC_MUX_ADC0;

	case PORTB_PIN(6):
		return ADC_MUX_ADC1;

	case PORTB_PIN(7):
		return ADC_MUX_ADC2;

	case PORTC_PIN(1):
		return ADC_MUX_ADC3;

	case PORTA_PIN(4):
		return ADC_MUX_ADC4;

	case PORTA_PIN(5):
		return ADC_MUX_ADC5;

	case PORTB_PIN(0):
		return ADC_MUX_VREF;

	default :
		return 0x0F; // Invalid analogue source

	}
}
