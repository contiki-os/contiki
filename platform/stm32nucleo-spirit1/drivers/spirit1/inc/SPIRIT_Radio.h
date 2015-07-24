/**
  ******************************************************************************
 * @file    SPIRIT_Radio.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   This file provides all the low level API to manage Analog and Digital
  *          radio part of SPIRIT.
 * @details
 *
 * In order to configure the Radio main parameters, the user can
 * fit <i>SRadioInit</i> structure the and call the <i>SpiritRadioInit()</i>
 * function passing its pointer as an argument.
 *
 * <b>Example:</b>
 * @code
 *
 * SRadioInit radioInit = {
 *     0,                       // Xtal offset in ppm
 *     433.4e6,                 // base frequency
 *     20e3,                    // channel space
 *     0,                       // channel number
 *     FSK,                     // modulation select
 *     38400,                   // datarate
 *     20e3,                    // frequency deviation
 *     100.5e3                  // channel filter bandwidth
 * };
 *
 * ...
 *
 * SpiritRadioInit(&radioInit);
 * @endcode
 *
 * Another important parameter for the radio configuration is the
 * transmission power.
 * The user is allowed to configure it using the function <i>SpiritRadioSetPALeveldBm()</i>
 * which sets the PA LEVEL specified by the first argument to the
 * power expressed in dBm by the second parameter.
 *
 * <b>Example:</b>
 * @code
 *
 *  SpiritRadioSetPALeveldBm(0 , 10.0);
 *
 * @endcode
 *
 *
 * @note The effective power that is set can be a little different from the
 * passed argument in dBm because the function performs an approximation.
 *

  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIRIT_RADIO_H
#define __SPIRIT_RADIO_H


/* Includes ------------------------------------------------------------------*/

#include "SPIRIT_Regs.h"
#include "SPIRIT_Types.h"
#include "SPIRIT_Config.h"


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup SPIRIT_Libraries
 * @{
 */


/** @defgroup SPIRIT_Radio          Radio
 * @brief Configuration and management of SPIRIT RF Analog and Digital part.
 * @details See the file <i>@ref SPIRIT_Radio.h</i> for more details.
 * @{
 */



/** @defgroup Radio_Exported_Types      Radio Exported Types
 * @{
 */


/**
 * @brief  SPIRIT XTAL frequency enumeration
 */
typedef enum
{
  XTAL_FLAG_24_MHz     = 0x00, /*!< 24 MHz Xtal selected */
  XTAL_FLAG_26_MHz     = 0x01  /*!< 26 MHz Xtal selected */

}XtalFlag;


#define IS_XTAL_FLAG(FLAG) (((FLAG) == XTAL_FLAG_24_MHz) || \
                            ((FLAG) == XTAL_FLAG_26_MHz))

/**
 * @brief  SPIRIT Band enumeration
 */
typedef enum
{
  HIGH_BAND     = 0x00, /*!< High_Band selected: from 779 MHz to 915 MHz */
  MIDDLE_BAND   = 0x01, /*!< Middle Band selected: from 387 MHz to 470 MHz */
  LOW_BAND      = 0x02,  /*!< Low Band selected: from 300 MHz to 348 MHz */
  VERY_LOW_BAND = 0x03  /*!< Vary low Band selected: from 150 MHz to 174 MHz */
}BandSelect;


#define IS_BAND_SELECTED(BAND) ((BAND == HIGH_BAND) || \
                                (BAND == MIDDLE_BAND) || \
                                (BAND == LOW_BAND) || \
                                (BAND == VERY_LOW_BAND))

/**
 * @brief  SPIRIT Modulation enumeration
 */
typedef enum
{
  FSK         = 0x00, /*!< 2-FSK modulation selected */
  GFSK_BT05   = 0x50, /*!< GFSK modulation selected with BT=0.5 */
  GFSK_BT1    = 0x10, /*!< GFSK modulation selected with BT=1 */
  ASK_OOK     = 0x20, /*!< ASK or OOK modulation selected. ASK will use power ramping */
  MSK         = 0x30  /*!< MSK modulation selected */

}ModulationSelect;


#define IS_MODULATION_SELECTED(MOD) (((MOD) == FSK) || \
                                     ((MOD) == GFSK_BT05) || \
                                     ((MOD) == GFSK_BT1) || \
                                     ((MOD) == ASK_OOK) || \
                                     ((MOD) == MSK))


/**
 * @brief  SPIRIT PA additional load capacitors bank enumeration
 */
typedef enum
{
  LOAD_0_PF    = PA_POWER0_CWC_0,    /*!< No additional PA load capacitor */
  LOAD_1_2_PF  = PA_POWER0_CWC_1_2P, /*!< 1.2pF additional PA load capacitor */
  LOAD_2_4_PF  = PA_POWER0_CWC_2_4P, /*!< 2.4pF additional PA load capacitor */
  LOAD_3_6_PF  = PA_POWER0_CWC_3_6P  /*!< 3.6pF additional PA load capacitor */

}PALoadCapacitor;

#define IS_PA_LOAD_CAP(CWC) (((CWC) == LOAD_0_PF) || \
                             ((CWC) == LOAD_1_2_PF) || \
                             ((CWC) == LOAD_2_4_PF) || \
                             ((CWC) == LOAD_3_6_PF))


/**
 * @brief  SPIRIT AFC Mode selection
 */
typedef enum
{
  AFC_SLICER_CORRECTION  = AFC2_AFC_MODE_SLICER,    /*!< AFC loop closed on slicer */
  AFC_2ND_IF_CORRECTION  = AFC2_AFC_MODE_MIXER      /*!< AFC loop closed on 2nd conversion stage */

}AFCMode;

#define IS_AFC_MODE(MODE)   ((MODE) == AFC_SLICER_CORRECTION || (MODE) == AFC_2ND_IF_CORRECTION)


/**
 * @brief  SPIRIT AGC Mode selection
 */
typedef enum
{
  AGC_LINEAR_MODE  = AGCCTRL0_AGC_MODE_LINEAR,    /*!< AGC works in linear mode */
  AGC_BINARY_MODE  = AGCCTRL0_AGC_MODE_BINARY     /*!< AGC works in binary mode */

}AGCMode;

#define IS_AGC_MODE(MODE)   ((MODE) == AGC_LINEAR_MODE || (MODE) == AGC_BINARY_MODE)


/**
 * @brief  SPIRIT Clock Recovery Mode selection
 */
typedef enum
{
  CLK_REC_PLL  = FDEV0_CLOCK_REG_ALGO_SEL_PLL,    /*!< PLL alogrithm for clock recovery */
  CLK_REC_DLL  = FDEV0_CLOCK_REG_ALGO_SEL_DLL     /*!< DLL alogrithm for clock recovery */

}ClkRecMode;

#define IS_CLK_REC_MODE(MODE)   ((MODE) == CLK_REC_PLL || (MODE) == CLK_REC_DLL)


/**
 * @brief  SPIRIT Postfilter length
 */
typedef enum
{
  PSTFLT_LENGTH_8   = 0x00,    /*!< Postfilter length is 8 symbols */
  PSTFLT_LENGTH_16  = 0x10     /*!< Postfilter length is 16 symbols */

}PstFltLength;

#define IS_PST_FLT_LENGTH(LENGTH)   ((LENGTH) == PSTFLT_LENGTH_8 || (LENGTH) == PSTFLT_LENGTH_16)


/**
 * @brief  SPIRIT OOK Peak Decay
 */
typedef enum
{
  FAST_DECAY   = 0x00,        /*!< Peak decay control for OOK: fast decay */
  MEDIUM_FAST_DECAY  = 0x01,  /*!< Peak decay control for OOK: medium_fast decay */
  MEDIUM_SLOW_DECAY = 0x02,   /*!< Peak decay control for OOK: medium_fast decay */
  SLOW_DECAY = 0x03           /*!< Peak decay control for OOK: slow decay */

}OokPeakDecay;

#define IS_OOK_PEAK_DECAY(DECAY)   (((DECAY) == FAST_DECAY) ||\
                                    ((DECAY) == MEDIUM_FAST_DECAY) ||\
                                    ((DECAY) == MEDIUM_SLOW_DECAY) ||\
                                    ((DECAY) == SLOW_DECAY))


/**
 * @brief  SPIRIT Radio Init structure definition
 */
typedef struct
{
  int16_t           nXtalOffsetPpm;     /*!< Specifies the offset frequency (in ppm)
                                             to compensate crystal inaccuracy expressed
                                             as signed value.*/

  uint32_t          lFrequencyBase;     /*!< Specifies the base carrier frequency (in Hz),
                                             i.e. the carrier frequency of channel #0.
                                             This parameter can be in one of the following ranges:
                                             High_Band: from 779 MHz to 915 MHz
                                             Middle Band: from 387 MHz to 470 MHz
                                             Low Band: from 300 MHz to 348 MHz */
  uint32_t          nChannelSpace;      /*!< Specifies the channel spacing expressed in Hz.
                                             The channel spacing is expressed as:
                                             NxFREQUENCY_STEPS, where FREQUENCY STEPS
                                             is F_Xo/2^15.
                                             This parameter can be in the range: [0, F_Xo/2^15*255] Hz */
  uint8_t           cChannelNumber;      /*!< Specifies the channel number. This value
                                             is multiplied by the channel spacing and
                                             added to synthesizer base frequency to
                                             generate the actual RF carrier frequency */
  ModulationSelect  xModulationSelect;   /*!< Specifies the modulation. This
                                             parameter can be any value of
                                             @ref ModulationSelect */
  uint32_t          lDatarate;          /*!< Specifies the datarate expressed in bps.
                                             This parameter can be in the range between
                                             100 bps and 500 kbps */
  uint32_t          lFreqDev;           /*!< Specifies the frequency deviation expressed in Hz.
                                             This parameter can be in the range: [F_Xo*8/2^18, F_Xo*7680/2^18] Hz */
  uint32_t          lBandwidth;          /*!< Specifies the channel filter bandwidth
                                             expressed in Hz. This parameter can be
                                             in the range between 1100 and 800100 Hz */

}SRadioInit;

/**
 * @}
 */



/** @defgroup Radio_Exported_Constants       Radio Exported Constants
 * @{
 */

/** @defgroup Radio_Band
 * @{
 */

#define FBASE_DIVIDER           262144           /*!< 2^18 factor dividing fxo in fbase formula */

#define HIGH_BAND_FACTOR      6       /*!< Band select factor for high band. Factor B in the equation 2 */
#define MIDDLE_BAND_FACTOR    12      /*!< Band select factor for middle band. Factor B in the equation 2 */
#define LOW_BAND_FACTOR       16      /*!< Band select factor for low band. Factor B in the equation 2 */
#define VERY_LOW_BAND_FACTOR  32      /*!< Band select factor for very low band. Factor B in the equation 2 */

#define HIGH_BAND_LOWER_LIMIT         778000000   /*!< Lower limit of the high band: 779 MHz */
#define HIGH_BAND_UPPER_LIMIT         957100000   /*!< Upper limit of the high band: 956 MHz */
#define MIDDLE_BAND_LOWER_LIMIT       386000000   /*!< Lower limit of the middle band: 387 MHz */
#define MIDDLE_BAND_UPPER_LIMIT       471100000   /*!< Upper limit of the middle band: 470 MHz */
#define LOW_BAND_LOWER_LIMIT          299000000   /*!< Lower limit of the low band: 300 MHz */
#define LOW_BAND_UPPER_LIMIT          349100000   /*!< Upper limit of the low band: 348 MHz */
#define VERY_LOW_BAND_LOWER_LIMIT     149000000   /*!< Lower limit of the very low band: 150 MHz */
#define VERY_LOW_BAND_UPPER_LIMIT     175100000   /*!< Upper limit of the very low band: 174 MHz */

#define IS_FREQUENCY_BAND_HIGH(FREQUENCY) ((FREQUENCY)>=HIGH_BAND_LOWER_LIMIT && \
                                           (FREQUENCY)<=HIGH_BAND_UPPER_LIMIT)

#define IS_FREQUENCY_BAND_MIDDLE(FREQUENCY) ((FREQUENCY)>=MIDDLE_BAND_LOWER_LIMIT && \
                                             (FREQUENCY)<=MIDDLE_BAND_UPPER_LIMIT)

#define IS_FREQUENCY_BAND_LOW(FREQUENCY) ((FREQUENCY)>=LOW_BAND_LOWER_LIMIT && \
                                          (FREQUENCY)<=LOW_BAND_UPPER_LIMIT)

#define IS_FREQUENCY_BAND_VERY_LOW(FREQUENCY) ((FREQUENCY)>=VERY_LOW_BAND_LOWER_LIMIT && \
                                          (FREQUENCY)<=VERY_LOW_BAND_UPPER_LIMIT)

#define IS_FREQUENCY_BAND(FREQUENCY) (IS_FREQUENCY_BAND_HIGH(FREQUENCY)|| \
                                      IS_FREQUENCY_BAND_MIDDLE(FREQUENCY)|| \
                                      IS_FREQUENCY_BAND_LOW(FREQUENCY)|| \
                                      IS_FREQUENCY_BAND_VERY_LOW(FREQUENCY))

/**
 * @}
 */


/** @defgroup Radio_IF_Offset           Radio IF Offset
 * @{
 */
#define IF_OFFSET_ANA(F_Xo) (lroundf(480140.0/(F_Xo)*12288-64.0))      /*!< It represents the IF_OFFSET_ANA in order
                                                                               to have an intermediate frequency of 480 kHz */
/**
 * @}
 */


/** @defgroup Radio_FC_Offset                   Radio FC Offset
 * @{
 */
#define F_OFFSET_DIVIDER           262144             /*!< 2^18 factor dividing fxo in foffset formula */
#define PPM_FACTOR                 1000000            /*!< 10^6 factor to use with Xtal_offset_ppm */


#define F_OFFSET_LOWER_LIMIT(F_Xo)			((-(int32_t)F_Xo)/F_OFFSET_DIVIDER*2048)
#define F_OFFSET_UPPER_LIMIT(F_Xo)			((int32_t)(F_Xo/F_OFFSET_DIVIDER*2047))

#define IS_FREQUENCY_OFFSET(OFFSET, F_Xo) (OFFSET>=F_OFFSET_LOWER_LIMIT(F_Xo) && OFFSET<=F_OFFSET_UPPER_LIMIT(F_Xo))


/**
 * @}
 */


/** @defgroup Radio_Channel_Space          Radio Channel Space
 * @{
 */


#define CHSPACE_DIVIDER         32768              /*!< 2^15 factor dividing fxo in channel space formula */

#define IS_CHANNEL_SPACE(CHANNELSPACE, F_Xo)    (CHANNELSPACE<=(F_Xo/32768*255))





/**
 * @}
 */


/** @defgroup Radio_Datarate                    Radio Datarate
 * @{
 */
#define MINIMUM_DATARATE                 100  /*!< Minimum datarate supported by SPIRIT1 100 bps */
#define MAXIMUM_DATARATE                 510000  /*!< Maximum datarate supported by SPIRIT1 500 kbps */

#define IS_DATARATE(DATARATE)           (DATARATE>=MINIMUM_DATARATE && DATARATE<=MAXIMUM_DATARATE)

/**
 * @}
 */


/** @defgroup Radio_Frequency_Deviation         Radio Frequency Deviation
 * @{
 */
#define F_DEV_MANTISSA_UPPER_LIMIT      7  /*!< Maximum value for the mantissa in frequency deviation formula */
#define F_DEV_EXPONENT_UPPER_LIMIT      9  /*!< Maximum value for the exponent in frequency deviation formula */

#define F_DEV_LOWER_LIMIT(F_Xo)		(F_Xo>>16)
#define F_DEV_UPPER_LIMIT(F_Xo)		((F_Xo*15)>>10)

#define IS_F_DEV(FDEV,F_Xo)             (FDEV>=F_DEV_LOWER_LIMIT(F_Xo) && FDEV<=F_DEV_UPPER_LIMIT(F_Xo))


/**
 * @}
 */


/** @defgroup Radio_Channel_Bandwidth           Radio Channel Bandwidth
 * @{
 */
#define CH_BW_LOWER_LIMIT(F_Xo)      1100*(F_Xo/1000000)/26  /*!< Minimum value of the channel filter bandwidth */
#define CH_BW_UPPER_LIMIT(F_Xo)    800100*(F_Xo/1000000)/26  /*!< Maximum value of the channel filter bandwidth */

#define IS_CH_BW(BW,F_Xo)         ((BW)>=CH_BW_LOWER_LIMIT(F_Xo) && (BW)<=CH_BW_UPPER_LIMIT(F_Xo))

/**
 * @}
 */


/** @defgroup Radio_Power_Amplifier                     Radio Power Amplifier
 * @{
 */

#define IS_PA_MAX_INDEX(INDEX)       ((INDEX)<=7)
#define IS_PAPOWER_DBM(PATABLE)      ((PATABLE)>= (-31) && (PATABLE)<=(12))
#define IS_PAPOWER(PATABLE)          ((PATABLE)<=90)
#define IS_PA_STEP_WIDTH(WIDTH)      ((WIDTH)>=1 && (WIDTH)<=4)

/**
 * @}
 */


/** @defgroup Radio_Automatic_Frequency_Correction              Radio Automatic Frequency Correction
 * @{
 */

#define IS_AFC_FAST_GAIN(GAIN)      	((GAIN)<=15)
#define IS_AFC_SLOW_GAIN(GAIN)      	((GAIN)<=15)
#define IS_AFC_PD_LEAKAGE(LEAKAGE)      ((LEAKAGE)<=31)

/**
 * @}
 */

/** @defgroup Radio_Automatic_Gain_Control                      Radio Automatic Gain Control
 * @{
 */

#define AGC_MEASURE_TIME_UPPER_LIMIT_US(F_Xo)		(393216.0/F_Xo)

#define IS_AGC_MEASURE_TIME_US(TIME, F_Xo)              (TIME<=AGC_MEASURE_TIME_UPPER_LIMIT_US(F_Xo))

#define IS_AGC_MEASURE_TIME(TIME)                       (TIME<=15)

#define AGC_HOLD_TIME_UPPER_LIMIT_US(F_Xo)		(756.0/F_Xo)

#define IS_AGC_HOLD_TIME_US(TIME,F_Xo)                  (TIME<=AGC_HOLD_TIME_UPPER_LIMIT_US(F_Xo))


#define IS_AGC_HOLD_TIME(TIME)                          (TIME<=63)

#define IS_AGC_THRESHOLD(THRESHOLD)                     (THRESHOLD<=15)

/**
 * @}
 */


/** @defgroup Radio_Clock_Recovery                      Radio Clock Recovery
 * @{
 */

#define IS_CLK_REC_P_GAIN(GAIN)       ((GAIN)<=7)
#define IS_CLK_REC_I_GAIN(GAIN)       ((GAIN)<=15)

/**
 * @}
 */

/**
 * @}
 */



/** @defgroup Radio_Exported_Macros                             Radio Exported Macros
 * @{
 */


/**
 * @}
 */

/** @defgroup Radio_Exported_Functions                          Radio Exported Functions
 * @{
 */

uint8_t SpiritRadioInit(SRadioInit* pxSRadioInitStruct);
void SpiritRadioGetInfo(SRadioInit* pxSRadioInitStruct);
void SpiritRadioSetXtalFlag(XtalFlag xXtal);
XtalFlag SpiritRadioGetXtalFlag(void);
uint8_t SpiritRadioSearchWCP(uint32_t lFc);
void SpiritRadioSetSynthWord(uint32_t lSynthWord);
uint32_t SpiritRadioGetSynthWord(void);
void SpiritRadioSetBand(BandSelect xBand);
BandSelect SpiritRadioGetBand(void);
void SpiritRadioSetChannel(uint8_t cChannel);
uint8_t SpiritRadioGetChannel(void);
void SpiritRadioSetChannelSpace(uint32_t lChannelSpace);
uint32_t SpiritRadioGetChannelSpace(void);
void SpiritRadioSetFrequencyOffsetPpm(int16_t nXtalPpm);
void SpiritRadioSetFrequencyOffset(int32_t lFOffset);
int32_t SpiritRadioGetFrequencyOffset(void);
void SpiritRadioVcoCalibrationWAFB(SpiritFunctionalState xNewstate);
uint8_t SpiritRadioSetFrequencyBase(uint32_t lFBase);
uint32_t SpiritRadioGetFrequencyBase(void);
uint32_t SpiritRadioGetCenterFrequency(void);
void SpiritRadioSearchDatarateME(uint32_t lDatarate, uint8_t* pcM, uint8_t* pcE);
void SpiritRadioSearchFreqDevME(uint32_t lFDev, uint8_t* pcM, uint8_t* pcE);
void SpiritRadioSearchChannelBwME(uint32_t lBandwidth, uint8_t* pcM, uint8_t* pcE);
void SpiritRadioSetDatarate(uint32_t lDatarate);
uint32_t SpiritRadioGetDatarate(void);
void SpiritRadioSetFrequencyDev(uint32_t lFDev);
uint32_t SpiritRadioGetFrequencyDev(void);
void SpiritRadioSetChannelBW(uint32_t lBandwidth);
uint32_t SpiritRadioGetChannelBW(void);
void SpiritRadioSetModulation(ModulationSelect xModulation);
ModulationSelect SpiritRadioGetModulation(void);
void SpiritRadioCWTransmitMode(SpiritFunctionalState xNewState);
void SpiritRadioSetOokPeakDecay(OokPeakDecay xOokDecay);
OokPeakDecay SpiritRadioGetOokPeakDecay(void);
uint8_t SpiritRadioGetdBm2Reg(uint32_t lFBase, float fPowerdBm);
float SpiritRadioGetReg2dBm(uint32_t lFBase, uint8_t cPowerReg);
void SpiritRadioSetPATabledBm(uint8_t cPALevelMaxIndex, uint8_t cWidth, PALoadCapacitor xCLoad, float* pfPAtabledBm);
void SpiritRadioGetPATabledBm(uint8_t* pcPALevelMaxIndex, float* pfPAtabledBm);
void SpiritRadioSetPATable(uint8_t cPALevelMaxIndex, uint8_t cWidth, PALoadCapacitor xCLoad, uint8_t* pcPAtable);
void SpiritRadioGetPATable(uint8_t* pcPALevelMaxIndex, uint8_t* pcPAtable);
void SpiritRadioSetPALeveldBm(uint8_t cIndex, float fPowerdBm);
float SpiritRadioGetPALeveldBm(uint8_t cIndex);
void SpiritRadioSetPALevel(uint8_t cIndex, uint8_t cPower);
uint8_t SpiritRadioGetPALevel(uint8_t cIndex);
void SpiritRadioSetPACwc(PALoadCapacitor xCLoad);
PALoadCapacitor SpiritRadioGetPACwc(void);
void SpiritRadioSetPALevelMaxIndex(uint8_t cIndex);
uint8_t SpiritRadioGetPALevelMaxIndex(void);
void SpiritRadioSetPAStepWidth(uint8_t cWidth);
uint8_t SpiritRadioGetPAStepWidth(void);
void SpiritRadioPARamping(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritRadioGetPARamping(void);
void SpiritRadioAFC(SpiritFunctionalState xNewState);
void SpiritRadioAFCFreezeOnSync(SpiritFunctionalState xNewState);
void SpiritRadioSetAFCMode(AFCMode xMode);
AFCMode SpiritRadioGetAFCMode(void);
void SpiritRadioSetAFCPDLeakage(uint8_t cLeakage);
uint8_t SpiritRadioGetAFCPDLeakage(void);
void SpiritRadioSetAFCFastPeriod(uint8_t cLength);
uint8_t SpiritRadioGetAFCFastPeriod(void);
void SpiritRadioSetAFCFastGain(uint8_t cGain);
uint8_t SpiritRadioGetAFCFastGain(void);
void SpiritRadioSetAFCSlowGain(uint8_t cGain);
uint8_t SpiritRadioGetAFCSlowGain(void);
int8_t SpiritRadioGetAFCCorrectionReg(void);
int32_t SpiritRadioGetAFCCorrectionHz(void);
void SpiritRadioAGC(SpiritFunctionalState xNewState);
void SpiritRadioSetAGCMode(AGCMode xMode);
AGCMode SpiritRadioGetAGCMode(void);
void SpiritRadioAGCFreezeOnSteady(SpiritFunctionalState xNewState);
void SpiritRadioAGCFreezeOnSync(SpiritFunctionalState xNewState);
void SpiritRadioAGCStartMaxAttenuation(SpiritFunctionalState xNewState);
void SpiritRadioSetAGCMeasureTimeUs(uint16_t nTime);
uint16_t SpiritRadioGetAGCMeasureTimeUs(void);
void SpiritRadioSetAGCMeasureTime(uint8_t cTime);
uint8_t SpiritRadioGetAGCMeasureTime(void);
void SpiritRadioSetAGCHoldTimeUs(uint8_t cTime);
uint8_t SpiritRadioGetAGCHoldTimeUs(void);
void SpiritRadioSetAGCHoldTime(uint8_t cTime);
uint8_t SpiritRadioGetAGCHoldTime(void);
void SpiritRadioSetAGCHighThreshold(uint8_t cHighThreshold);
uint8_t SpiritRadioGetAGCHighThreshold(void);
void SpiritRadioSetAGCLowThreshold(uint8_t cLowThreshold);
uint8_t SpiritRadioGetAGCLowThreshold(void);
void SpiritRadioSetClkRecMode(ClkRecMode xMode);
ClkRecMode SpiritRadioGetClkRecMode(void);
void SpiritRadioSetClkRecPGain(uint8_t cPGain);
uint8_t SpiritRadioGetClkRecPGain(void);
void SpiritRadioSetClkRecIGain(uint8_t cIGain);
uint8_t SpiritRadioGetClkRecIGain(void);
void SpiritRadioSetClkRecPstFltLength(PstFltLength xLength);
PstFltLength SpiritRadioGetClkRecPstFltLength(void);
void SpiritRadioCsBlanking(SpiritFunctionalState xNewState);
void SpiritRadioPersistenRx(SpiritFunctionalState xNewState);
uint32_t SpiritRadioGetXtalFrequency(void);
void SpiritRadioSetXtalFrequency(uint32_t lXtalFrequency);
void SpiritRadioSetRefDiv(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritRadioGetRefDiv(void);
void SpiritRadioSetDigDiv(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritRadioGetDigDiv(void);
/**
 * @}
 */

/**
 * @}
 */


/**
 * @}
 */



#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
