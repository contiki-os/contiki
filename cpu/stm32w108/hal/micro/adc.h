/** @file /hal/micro/adc.h
 * @brief Header for A/D converter.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
/** @addtogroup adc
 * Sample A/D converter driver.
 *
 * See adc.h for source code.
 *
 * @note Stm32w108xx ADC driver support is preliminary and essentailly untested -
 * please do not attempt to use this ADC driver on this platform.
 *
 * @note Except for the Stm32w108xx, the StZNet stack does use these functions.
 *
 * To use the ADC system, include this file and ensure that
 * ::halInternalInitAdc() is called whenever the microcontroller is
 * started.  
 *
 * A "user" is a separate thread of execution and usage.  That is,
 * internal St code is one user and clients are a different user.
 * But a client that is calling the ADC in two different functions
 * constitutes only one user, as long as the ADC access is not
 * interleaved.
 *
 * @note This code does not allow access to the continuous reading mode of
 * the ADC, which some clients may require.
 *
 * Many functions in this file return an ::StStatus value.  See
 * error-def.h for definitions of all ::StStatus return values.
 *
 *@{
 */
#ifndef __ADC_H__
#define __ADC_H__

#ifdef CORTEXM3_STM32W108

// A type for the ADC User enumeration.
typedef int8u ADCUser;
enum
{
  /** LQI User ID. */
  ADC_USER_LQI = 0,
  /** Application User ID */
  ADC_USER_APP = 1,
  /** Application User ID */
  ADC_USER_APP2 = 2
};

/** @brief Be sure to update ::NUM_ADC_USERS if additional users are added
 * to the ::ADCUser list.
 */
#define NUM_ADC_USERS 3 // make sure to update if the above is adjusted


// A type for the reference enumeration.
typedef int8u ADCReferenceType;
enum
{
  /** AREF pin reference. */
  ADC_REF_AREF = 0x00,
  /** AVCC pin reference. */
  ADC_REF_AVCC = 0x40,
  /** Internal reference. */
  ADC_REF_INT  = 0xC0
};

// A type for the rate enumeration.
typedef int8u ADCRateType;
enum
{
  /** Rate 32 us, 5 effective bits in ADC_DATA[15:11] */
  ADC_CONVERSION_TIME_US_32   = 0x0,
  /** Rate 64 us, 6 effective bits in ADC_DATA[15:10] */
  ADC_CONVERSION_TIME_US_64   = 0x1,
  /** Rate 128 us, 7 effective bits in ADC_DATA[15:9] */
  ADC_CONVERSION_TIME_US_128  = 0x2,
  /** Rate 256 us, 8 effective bits in ADC_DATA[15:8] */
  ADC_CONVERSION_TIME_US_256  = 0x3,
  /** Rate 512 us, 9 effective bits in ADC_DATA[15:7] */
  ADC_CONVERSION_TIME_US_512  = 0x4,
  /** Rate 1024 us, 10 effective bits in ADC_DATA[15:6] */
  ADC_CONVERSION_TIME_US_1024 = 0x5,
  /** Rate 2048 us, 11 effective bits in ADC_DATA[15:5] */
  ADC_CONVERSION_TIME_US_2048 = 0x6,
  /** Rate 4096 us, 12 effective bits in ADC_DATA[15:4] */
  ADC_CONVERSION_TIME_US_4096 = 0x7,
};


#if defined (CORTEXM3)
  /** Channel 0 : ADC0 on PB5 */
#define ADC_MUX_ADC0    0x0
  /** Channel 1 : ADC1 on PB6 */
#define ADC_MUX_ADC1    0x1
  /** Channel 2 : ADC2 on PB7 */
#define ADC_MUX_ADC2    0x2
  /** Channel 3 : ADC3 on PC1 */
#define ADC_MUX_ADC3    0x3
  /** Channel 4 : ADC4 on PA4 */
#define ADC_MUX_ADC4    0x4
  /** Channel 5 : ADC5 on PA5 */
#define ADC_MUX_ADC5    0x5
  /** Channel 8 : VSS (0V) - not for high voltage range */
#define ADC_MUX_GND     0x8
  /** Channel 9 : VREF/2 (0.6V) */
#define ADC_MUX_VREF2   0x9
  /** Channel A : VREF (1.2V)*/
#define ADC_MUX_VREF    0xA
  /** Channel B : Regulator/2 (0.9V) - not for high voltage range */
#define ADC_MUX_VREG2   0xB

// ADC_SOURCE_<pos>_<neg> selects <pos> as the positive input and <neg> as  
// the negative input.
enum
{
  ADC_SOURCE_ADC0_VREF2  = ((ADC_MUX_ADC0  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC0_GND    = ((ADC_MUX_ADC0  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC1_VREF2  = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC1_GND    = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC2_VREF2  = ((ADC_MUX_ADC2  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC2_GND    = ((ADC_MUX_ADC2  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC3_VREF2  = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC3_GND    = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC4_VREF2  = ((ADC_MUX_ADC4  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC4_GND    = ((ADC_MUX_ADC4  <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_ADC5_VREF2  = ((ADC_MUX_ADC5  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_ADC5_GND    = ((ADC_MUX_ADC5  <<ADC_MUXN_BITS) + ADC_MUX_GND),  

  ADC_SOURCE_ADC1_ADC0   = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_ADC0),
  ADC_SOURCE_ADC0_ADC1   = ((ADC_MUX_ADC1  <<ADC_MUXN_BITS) + ADC_MUX_ADC0),

  ADC_SOURCE_ADC3_ADC2   = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_ADC2),
  ADC_SOURCE_ADC2_ADC3   = ((ADC_MUX_ADC3  <<ADC_MUXN_BITS) + ADC_MUX_ADC2),

  ADC_SOURCE_ADC5_ADC4   = ((ADC_MUX_ADC5  <<ADC_MUXN_BITS) + ADC_MUX_ADC4),

  ADC_SOURCE_GND_VREF2   = ((ADC_MUX_GND   <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VGND        = ((ADC_MUX_GND   <<ADC_MUXN_BITS) + ADC_MUX_GND),

  ADC_SOURCE_VREF_VREF2  = ((ADC_MUX_VREF  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VREF        = ((ADC_MUX_VREF  <<ADC_MUXN_BITS) + ADC_MUX_GND),
/* Modified the original ADC driver for enabling the ADC extended range mode required for 
   supporting the STLM20 temperature sensor.
   NOTE: 
   The ADC extended range is inaccurate due to the high voltage mode bug of the general purpose ADC 
   (see STM32W108 errata). As consequence, it is not reccomended to use this ADC driver for getting
   the temperature values 
*/
#ifdef ENABLE_ADC_EXTENDED_RANGE_BROKEN
  ADC_SOURCE_VREF2_VREF2  = ((ADC_MUX_VREF2  <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VREF2        = ((ADC_MUX_VREF2  <<ADC_MUXN_BITS) + ADC_MUX_GND),
#endif /* ENABLE_ADC_EXTENDED_RANGE_BROKEN */

  ADC_SOURCE_VREG2_VREF2 = ((ADC_MUX_VREG2 <<ADC_MUXN_BITS) + ADC_MUX_VREF2),
  ADC_SOURCE_VDD_GND     = ((ADC_MUX_VREG2 <<ADC_MUXN_BITS) + ADC_MUX_GND)
};

/** @brief Macro that returns the ADCChannelType, from a given couple of sources
 * (positive and negative). To be used with halStartAdcConversion().
 */
#define ADC_SOURCE(P,N)   (( P << ADC_MUXN_BITS ) + N)

#endif // defined (CORTEXM3)


/** @brief A type for the channel enumeration 
 * (such as ::ADC_SOURCE_ADC0_GND)
 */
typedef int8u ADCChannelType;

/** @brief Returns the ADC channel from a given GPIO. Its value can can be used
 * inside the ADC_SOURCE(P,N) macro to retrieve the input pair for
 * halStartAdcConversion().
 *
 * @param io  The GPIO pin (it can be specified with the convenience macros
 *            PORTA_PIN(), PORTB_PIN(), PORTC_PIN() )
 * 
 * @return The ADC_MUX value connected to the given GPIO.
 */
int8u halGetADCChannelFromGPIO(int32u io);


/** @brief Initializes and powers-up the ADC. 
 */
void halInternalInitAdc(void);

/** @brief Starts an ADC conversion for the user specified by \c id.
 *
 * @appusage The application must set \c reference to the voltage
 * reference desired (see the ADC references enum), 
 * set \c channel to the channel number
 * required (see the ADC channel enum), and set \c rate to reflect the
 * number of bits of accuracy desired (see the ADC rates enum)
 *
 * @param id        An ADC user.
 * 
 * @param reference Voltage reference to use, chosen from enum
 * 
 * @param channel   Microprocessor channel number.  
 * 
 * @param rate      rate number (see the ADC rate enum).
 * 
 * @return One of the following: 
 * - ADC_CONVERSION_DEFERRED   if the conversion is still waiting 
 * to start.
 * - ADC_CONVERSION_BUSY       if the conversion is currently taking 
 * place.
 * - ST_ERR_FATAL                 if a passed parameter is invalid.
 */
StStatus halStartAdcConversion(ADCUser id,
                                  ADCReferenceType reference,
                                  ADCChannelType channel,
                                  ADCRateType rate);

/** @brief Returns the status of a pending conversion
 * previously started by ::halStartAdcConversion().  If the conversion
 * is complete, writes the raw register value of the conversion (the unaltered
 * value taken directly from the ADC's data register) into \c value.
 *
 * @param id     An ADC user.
 *
 * @param value  Pointer to an int16u to be loaded with the new value. 
 * 
 * @return One of the following: 
 * - ::ST_ADC_CONVERSION_DONE       if the conversion is complete.
 * - ::ST_ADC_CONVERSION_DEFERRED   if the conversion is still waiting 
 * to start.
 * - ::ST_ADC_CONVERSION_BUSY       if the conversion is currently taking 
 * place.
 * - ::ST_ADC_NO_CONVERSION_PENDING if \c id does not have a pending 
 * conversion.
 */
StStatus halRequestAdcData(ADCUser id, int16u *value);


/** @brief Waits for the user's request to complete and then,
 * if a conversion was done, writes the raw register value of the conversion
 * (the unaltered value taken directly from the ADC's data register) into
 * \c value and returns ::ADC_CONVERSION_DONE, or immediately
 * returns ::ADC_NO_CONVERSION_PENDING.
 *
 * @param id     An ADC user.
 *
 * @param value  Pointer to an int16u to be loaded with the new value. 
 * 
 * @return One of the following: 
 * - ::ST_ADC_CONVERSION_DONE        if the conversion is complete.
 * - ::ST_ADC_NO_CONVERSION_PENDING  if \c id does not have a pending 
 * conversion.
 */
StStatus halReadAdcBlocking(ADCUser id, int16u *value);


/** @brief Calibrates or recalibrates the ADC system. 
 *
 * @appusage Use this function to (re)calibrate as needed. This function is
 * intended for the microcontroller, which requires proper calibration to calculate
 * a human readible value (a value in volts).  If the app does not call this
 * function, the first time (and only the first time) the function 
 * ::halConvertValueToVolts() is called, this function is invoked.  To
 * maintain accurate volt calculations, the application should call this
 * whenever it expects the temperature of the micro to change.
 *
 * @param id  An ADC user.
 *
 * @return One of the following: 
 * - ::ST_ADC_CONVERSION_DONE        if the calibration is complete.
 * - ::ST_ERR_FATAL                  if the calibration failed.
 */
StStatus halAdcCalibrate(ADCUser id);


/** @brief Convert the raw register value (the unaltered value taken
 * directly from the ADC's data register) into a signed fixed point value with
 * units 10^-4 Volts.  The returned value will be in the range -12000 to
 * +12000 (-1.2000 volts to +1.2000 volts).
 *
 * @appusage Use this function to get a human useful value.
 *
 * @param value  An int16u to be converted.
 * 
 * @return Volts as signed fixed point with units 10^-4 Volts. 
 */
int16s halConvertValueToVolts(int16u value);


/** @brief Calibrates Vref to be 1.2V +/-10mV.
 *
 *  @appusage This function must be called from halInternalInitAdc() before
 *  making ADC readings.  This function is not intended to be called from any
 *  function other than halInternalInitAdc().  This function ensures that the
 *  master cell voltage and current bias values are calibrated before
 *  calibrating Vref.
 */
void stCalibrateVref(void);

#ifdef CORTEXM3
void halAdcSetClock(boolean fast);
void halAdcSetRange(boolean high);
boolean halAdcGetClock(void);
boolean halAdcGetRange(void);
#endif

#endif /* CORTEXM3_STM32W108 */

#ifdef CORTEXM3_STM32F103
#include "micro/cortexm3/stm32f103ret/adc.h"
#endif /* CORTEXM3_STM32F103 */

#endif // __ADC_H__

/** @} // END addtogroup
 */


