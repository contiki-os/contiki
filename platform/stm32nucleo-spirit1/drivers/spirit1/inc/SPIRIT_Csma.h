/**
  ******************************************************************************
 * @file    SPIRIT_Csma.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT CSMA.
 * @details
 *
 * The Spirit CSMA feature, when configured and enabled, is transparent
 * for the user. It means the user has only to call the <i>@ref SpiritCsmaInit()</i>
 * function on a filled structure and then enable the CSMA policy using the <i>@ref SpiritCsma()</i>
 * function.
 *
 * <b>Example:</b>
 * @code
 *
 * CsmaInit csmaInit={
 *   S_DISABLE,         // persistent mode
 *   TBIT_TIME_64,      // Tbit time
 *   TCCA_TIME_3,       // Tcca time
 *   5,                 // max number of backoffs
 *   0xFA21,            // BU counter seed
 *   32                 // CU prescaler
 * };
 *
 * ...
 *
 * SpiritCsmaInit(&csmaInit);
 * SpiritCsma(S_ENABLE);
 *
 *
 * @endcode
 *
 * @note The CS status depends of the RSSI threshold set. Please see the Spirit_Qi
 * module for details.
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
#ifndef __SPIRIT_CSMA_H
#define __SPIRIT_CSMA_H


/* Includes ------------------------------------------------------------------*/

#include "SPIRIT_Types.h"
#include "SPIRIT_Regs.h"


#ifdef __cplusplus
 extern "C" {
#endif


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @defgroup SPIRIT_Csma        CSMA
 * @brief Configuration and management of SPIRIT CSMA.
 * @details See the file <i>@ref SPIRIT_Csma.h</i> for more details.
 * @{
 */

/**
 * @defgroup Csma_Exported_Types        CSMA Exported Types
 * @{
 */


/**
 * @brief  Multiplier for Tcca time enumeration (Tcca = Multiplier*Tbit).
 */
typedef enum
{
  TBIT_TIME_64 = CSMA_CCA_PERIOD_64TBIT,      /*!< CSMA/CA: Sets CCA period to 64*TBIT */
  TBIT_TIME_128 = CSMA_CCA_PERIOD_128TBIT,    /*!< CSMA/CA: Sets CCA period to 128*TBIT */
  TBIT_TIME_256 = CSMA_CCA_PERIOD_256TBIT,    /*!< CSMA/CA: Sets CCA period to 256*TBIT */
  TBIT_TIME_512 = CSMA_CCA_PERIOD_512TBIT,    /*!< CSMA/CA: Sets CCA period to 512*TBIT */
}CcaPeriod;

#define IS_CCA_PERIOD(PERIOD)   (PERIOD == TBIT_TIME_64 || \
				 PERIOD == TBIT_TIME_128 || \
				 PERIOD == TBIT_TIME_256 || \
				 PERIOD == TBIT_TIME_512)


/**
 * @brief  Multiplier of Tcca time enumeration to obtain Tlisten (Tlisten = [1...15]*Tcca).
 */
typedef enum
{
  TCCA_TIME_0   = 0x00,     /*!< CSMA/CA: Sets CCA length to 0 */
  TCCA_TIME_1   = 0x10,     /*!< CSMA/CA: Sets CCA length to 1*TLISTEN */
  TCCA_TIME_2   = 0x20,     /*!< CSMA/CA: Sets CCA length to 2*TLISTEN */
  TCCA_TIME_3   = 0x30,     /*!< CSMA/CA: Sets CCA length to 3*TLISTEN */
  TCCA_TIME_4   = 0x40,     /*!< CSMA/CA: Sets CCA length to 4*TLISTEN */
  TCCA_TIME_5   = 0x50,     /*!< CSMA/CA: Sets CCA length to 5*TLISTEN */
  TCCA_TIME_6   = 0x60,     /*!< CSMA/CA: Sets CCA length to 6*TLISTEN */
  TCCA_TIME_7   = 0x70,     /*!< CSMA/CA: Sets CCA length to 7*TLISTEN */
  TCCA_TIME_8   = 0x80,     /*!< CSMA/CA: Sets CCA length to 8*TLISTEN */
  TCCA_TIME_9   = 0x90,     /*!< CSMA/CA: Sets CCA length to 9*TLISTEN */
  TCCA_TIME_10  = 0xA0,     /*!< CSMA/CA: Sets CCA length to 10*TLISTEN */
  TCCA_TIME_11  = 0xB0,     /*!< CSMA/CA: Sets CCA length to 11*TLISTEN */
  TCCA_TIME_12  = 0xC0,     /*!< CSMA/CA: Sets CCA length to 12*TLISTEN */
  TCCA_TIME_13  = 0xD0,     /*!< CSMA/CA: Sets CCA length to 13*TLISTEN */
  TCCA_TIME_14  = 0xE0,     /*!< CSMA/CA: Sets CCA length to 14*TLISTEN */
  TCCA_TIME_15  = 0xF0,     /*!< CSMA/CA: Sets CCA length to 15*TLISTEN */
}CsmaLength;

#define IS_CSMA_LENGTH(LENGTH)	(LENGTH == TCCA_TIME_0 || \
                                 LENGTH == TCCA_TIME_1 || \
				 LENGTH == TCCA_TIME_2 || \
				 LENGTH == TCCA_TIME_3 || \
				 LENGTH == TCCA_TIME_4 || \
				 LENGTH == TCCA_TIME_5 || \
				 LENGTH == TCCA_TIME_6 || \
				 LENGTH == TCCA_TIME_7 || \
				 LENGTH == TCCA_TIME_8 || \
				 LENGTH == TCCA_TIME_9 || \
				 LENGTH == TCCA_TIME_10 || \
				 LENGTH == TCCA_TIME_11 || \
				 LENGTH == TCCA_TIME_12 || \
				 LENGTH == TCCA_TIME_13 || \
				 LENGTH == TCCA_TIME_14 || \
				 LENGTH == TCCA_TIME_15)


/**
  * @brief  SPIRIT CSMA Init structure definition
  */
typedef struct
{
  SpiritFunctionalState     xCsmaPersistentMode;          /*!< Specifies if the CSMA persistent mode has to be on or off.
                                                               This parameter can be S_ENABLE or S_DISABLE */
  CcaPeriod                 xMultiplierTbit;              /*!< Specifies the Tbit multiplier to obtain the Tcca.
                                                               This parameter can be a value of @ref CcaPeriod */
  CsmaLength                xCcaLength;                   /*!< Specifies the Tcca multiplier to determinate the Tlisten.
                                                               This parameter can be a value of @ref CsmaLength. */
  uint8_t                   cMaxNb;                       /*!< Specifies the max number of backoff cycles. Not used in persistent mode.
                                                               This parameter is an uint8_t. */
  uint16_t                  nBuCounterSeed;               /*!< Specifies the BU counter seed. Not used in persistent mode.
                                                               This parameter can be a value of 16 bits. */
  uint8_t                   cBuPrescaler;                 /*!< Specifies the BU prescaler. Not used in persistent mode.
                                                               This parameter can be a value of 6 bits. */
}CsmaInit;


/**
 *@}
 */


/**
 * @defgroup Csma_Exported_Constants    CSMA Exported Constants
 * @{
 */

/**
 * @defgroup Csma_Parameters            CSMA Parameters
 * @{
 */

#define IS_BU_COUNTER_SEED(SEED)	(SEED!=0)
#define IS_BU_PRESCALER(PRESCALER)	(PRESCALER<64)
#define IS_CMAX_NB(NB)			(NB<8)

/**
 *@}
 */

/**
 *@}
 */


/**
 * @defgroup Csma_Exported_Macros       CSMA Exported Macros
 * @{
 */


/**
 *@}
 */


/**
 * @defgroup Csma_Exported_Functions    CSMA Exported Functions
 * @{
 */

void SpiritCsmaInit(CsmaInit* pxCsmaInit);
void SpiritCsmaGetInfo(CsmaInit* pxCsmaInit);
void SpiritCsma(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritCsmaGetCsma(void);
void SpiritCsmaPersistentMode(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritCsmaGetPersistentMode(void);
void SpiritCsmaSeedReloadMode(SpiritFunctionalState xNewState);
SpiritFunctionalState SpiritCsmaGetSeedReloadMode(void);
void SpiritCsmaSetBuCounterSeed(uint16_t nBuCounterSeed);
uint16_t SpiritCsmaGetBuCounterSeed(void);
void SpiritCsmaSetBuPrescaler(uint8_t cBuPrescaler);
uint8_t SpiritCsmaGetBuPrescaler(void);
void SpiritCsmaSetCcaPeriod(CcaPeriod xMultiplierTbit);
CcaPeriod SpiritCsmaGetCcaPeriod(void);
void SpiritCsmaSetCcaLength(CsmaLength xCcaLength);
uint8_t SpiritCsmaGetCcaLength(void);
void SpiritCsmaSetMaxNumberBackoff(uint8_t cMaxNb);
uint8_t SpiritCsmaGetMaxNumberBackoff(void);


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
