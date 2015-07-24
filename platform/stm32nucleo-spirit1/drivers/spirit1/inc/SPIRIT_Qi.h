/**
  ******************************************************************************
 * @file    SPIRIT_Qi.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT QI.
 * @details
 *
 * This module can be used to configure and read some quality indicators
 * used by Spirit.
 * API to set thresholds and to read values in raw mode or in dBm are
 * provided.
 *
 * <b>Example:</b>
 * @code
 *
 *   float rssiValuedBm;
 *   uint8_t pqiValue, sqiValue;
 *
 *   SpiritQiPqiCheck(S_ENABLE);
 *   SpiritQiSqiCheck(S_ENABLE);
 *
 *   ...
 *
  *   rssiValueDbm = SpiritQiGetRssidBm();
 *   pqiValue = SpiritQiGetPqi();
 *   sqiValue = SpiritQiGetSqi();
 *
 *   ...
 *
 * @endcode
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
#ifndef __SPIRIT_QI_H
#define __SPIRIT_QI_H


/* Includes ------------------------------------------------------------------*/

#include "SPIRIT_Regs.h"
#include "SPIRIT_Types.h"


#ifdef __cplusplus
 extern "C" {
#endif


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @defgroup SPIRIT_Qi          QI
 * @brief Configuration and management of SPIRIT QI.
 * @details See the file <i>@ref SPIRIT_Qi.h</i> for more details.
 * @{
 */

/**
 * @defgroup Qi_Exported_Types  QI Exported Types
 * @{
 */


/**
 * @brief  PQI threshold value enumeration.
 */
typedef enum
{
   PQI_TH_0=0x00,
   PQI_TH_1=0x04,
   PQI_TH_2=0x08,
   PQI_TH_3=0x0C,
   PQI_TH_4=0x10,
   PQI_TH_5=0x14,
   PQI_TH_6=0x18,
   PQI_TH_7=0x1C,
   PQI_TH_8=0x20,
   PQI_TH_9=0x24,
   PQI_TH_10=0x28,
   PQI_TH_11=0x2C,
   PQI_TH_12=0x30,
   PQI_TH_13=0x34,
   PQI_TH_14=0x38,
   PQI_TH_15=0x3C

} PqiThreshold;

#define IS_PQI_THR(VALUE)   (VALUE==PQI_TH_0 ||\
                             VALUE==PQI_TH_1 ||\
                             VALUE==PQI_TH_2 ||\
                             VALUE==PQI_TH_3 ||\
                             VALUE==PQI_TH_4 ||\
                             VALUE==PQI_TH_5 ||\
                             VALUE==PQI_TH_6 ||\
                             VALUE==PQI_TH_7 ||\
                             VALUE==PQI_TH_8 ||\
                             VALUE==PQI_TH_9 ||\
                             VALUE==PQI_TH_10 ||\
                             VALUE==PQI_TH_11 ||\
                             VALUE==PQI_TH_12 ||\
                             VALUE==PQI_TH_13 ||\
                             VALUE==PQI_TH_14 ||\
                             VALUE==PQI_TH_15)

/**
 * @brief  SQI threshold value enumeration.
 */
typedef enum
{
   SQI_TH_0=0x00,
   SQI_TH_1=0x40,
   SQI_TH_2=0x80,
   SQI_TH_3=0xC0

} SqiThreshold;

#define IS_SQI_THR(VALUE)   (VALUE==SQI_TH_0 ||\
                             VALUE==SQI_TH_1 ||\
                             VALUE==SQI_TH_2 ||\
                             VALUE==SQI_TH_3)


/**
 * @brief  RSSI filter gain value enumeration.
 */
typedef enum
{
   RSSI_FG_0=0x00,
   RSSI_FG_1=0x10,
   RSSI_FG_2=0x20,
   RSSI_FG_3=0x30,
   RSSI_FG_4=0x40,
   RSSI_FG_5=0x50,
   RSSI_FG_6=0x60,
   RSSI_FG_7=0x70,
   RSSI_FG_8=0x80,
   RSSI_FG_9=0x90,
   RSSI_FG_10=0xA0,
   RSSI_FG_11=0xB0,
   RSSI_FG_12=0xC0,
   RSSI_FG_13=0xD0,
   RSSI_FG_14=0xE0,     /*<! recommended value */
   RSSI_FG_15=0xF0

} RssiFilterGain;

#define IS_RSSI_FILTER_GAIN(VALUE)  (VALUE==RSSI_FG_0 ||\
                                     VALUE==RSSI_FG_1 ||\
                                     VALUE==RSSI_FG_2 ||\
                                     VALUE==RSSI_FG_3 ||\
                                     VALUE==RSSI_FG_4 ||\
                                     VALUE==RSSI_FG_5 ||\
                                     VALUE==RSSI_FG_6 ||\
                                     VALUE==RSSI_FG_7 ||\
                                     VALUE==RSSI_FG_8 ||\
                                     VALUE==RSSI_FG_9 ||\
                                     VALUE==RSSI_FG_10 ||\
                                     VALUE==RSSI_FG_11 ||\
                                     VALUE==RSSI_FG_12 ||\
                                     VALUE==RSSI_FG_13 ||\
                                     VALUE==RSSI_FG_14 ||\
                                     VALUE==RSSI_FG_15)

/**
 * @brief  CS mode enumeration.
 */
typedef enum
{
   CS_MODE_STATIC_3DB=0x00,
   CS_MODE_DYNAMIC_6DB=0x04,
   CS_MODE_DYNAMIC_12DB=0x08,
   CS_MODE_DYNAMIC_18DB=0x0C

} CSMode;

#define IS_CS_MODE(MODE)    (MODE==CS_MODE_STATIC_3DB ||\
                             MODE==CS_MODE_DYNAMIC_6DB ||\
                             MODE==CS_MODE_DYNAMIC_12DB ||\
                             MODE==CS_MODE_DYNAMIC_18DB)

/**
  *@}
  */


/**
 * @defgroup Qi_Exported_Constants      QI Exported Constants
 * @{
 */

/*  range for the RSSI Threshold in dBm  */
#define IS_RSSI_THR_DBM(VALUE)  (VALUE>=-130 && VALUE<=-2)

/**
  *@}
  */


/**
 * @defgroup Qi_Exported_Macros         QI Exported Macros
 * @{
 */

/**
 * @brief  Macro to obtain the RSSI value in dBm
 * @param  None.
 * @retval RSSI in dBm.
 *         This parameter is a float.
 */
#define SpiritQiGetRssidBm()            (-120.0+((float)(SpiritQiGetRssi()-20))/2)

/**
 *@}
 */


/**
 * @defgroup Qi_Exported_Functions       QI Exported Functions
 * @{
 */

void SpiritQiPqiCheck(SpiritFunctionalState xNewState);
void SpiritQiSqiCheck(SpiritFunctionalState xNewState);
void SpiritQiSetPqiThreshold(PqiThreshold xPqiThr);
PqiThreshold SpiritQiGetPqiThreshold(void);
void SpiritQiSetSqiThreshold(SqiThreshold xSqiThr);
SqiThreshold SpiritQiGetSqiThreshold(void);
void SpiritQiSetRssiThreshold(uint8_t cRssiThr);
uint8_t SpiritQiGetRssiThreshold(void);
uint8_t SpiritQiComputeRssiThreshold(int cDbmValue);
void SpiritQiSetRssiThresholddBm(int nDbmValue);
uint8_t SpiritQiGetPqi(void);
uint8_t SpiritQiGetSqi(void);
uint8_t SpiritQiGetLqi(void);
SpiritFlagStatus SpiritQiGetCs(void);
uint8_t SpiritQiGetRssi(void);
void SpiritQiSetRssiFilterGain(RssiFilterGain xRssiFg);
RssiFilterGain SpiritQiGetRssiFilterGain(void);
void SpiritQiSetCsMode(CSMode xCsMode);
CSMode SpiritQiGetCsMode(void);
void SpiritQiCsTimeoutMask(SpiritFunctionalState xNewState);
void SpiritQiPqiTimeoutMask(SpiritFunctionalState xNewState);
void SpiritQiSqiTimeoutMask(SpiritFunctionalState xNewState);


/**
 *@}
 */

/**
 *@}
 */


/**
 *@}
 */


#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
