/**
  ******************************************************************************
 * @file    SPIRIT_General.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT General functionalities.
 * @details
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
#ifndef __SPIRIT_GENERAL_H
#define __SPIRIT_GENERAL_H


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
 * @defgroup SPIRIT_General     General
 * @brief Configuration and management of SPIRIT General functionalities.
 * @details See the file <i>@ref SPIRIT_General.h</i> for more details.
 * @{
 */

/**
 * @defgroup General_Exported_Types     General Exported Types
 * @{
 */


/**
 * @brief  SPIRIT ModeExtRef enumeration
 */

typedef enum
{
  MODE_EXT_XO = 0,
  MODE_EXT_XIN = !MODE_EXT_XO
} ModeExtRef;

#define IS_MODE_EXT(MODE)   (MODE == MODE_EXT_XO || \
                             MODE == MODE_EXT_XIN)


/**
 * @brief  SPIRIT BatteryLevel enumeration
 */

typedef enum
{
  BLD_LVL_2_7_V = 0,
  BLD_LVL_2_5_V = 1,
  BLD_LVL_2_3_V = 2,
  BLD_LVL_2_1_V = 3
} BatteryLevel;

#define IS_BLD_LVL(MODE)  (MODE == BLD_LVL_2_7_V || \
                           MODE == BLD_LVL_2_5_V || \
                           MODE == BLD_LVL_2_3_V || \
                           MODE == BLD_LVL_2_1_V)


/**
 * @brief  SPIRIT GmConf enumeration
 */

typedef enum
{
  GM_SU_13_2 = 0,
  GM_SU_18_2,
  GM_SU_21_5,
  GM_SU_25_6,
  GM_SU_28_8,
  GM_SU_33_9,
  GM_SU_38_5,
  GM_SU_43_0
} GmConf;

#define IS_GM_CONF(MODE)    (MODE == GM_SU_13_2 || \
                             MODE == GM_SU_18_2 || \
                             MODE == GM_SU_21_5 || \
                             MODE == GM_SU_25_6 || \
                             MODE == GM_SU_28_8 || \
                             MODE == GM_SU_33_9 || \
                             MODE == GM_SU_38_5 || \
                             MODE == GM_SU_43_0)


/**
 * @brief  SPIRIT packet type enumeration
 */

typedef enum
{
  PKT_BASIC = 0x00,
  PKT_MBUS = 0x02,
  PKT_STACK

} PacketType;

#define IS_PKT_TYPE(TYPE)    (TYPE == PKT_BASIC || \
                             TYPE == PKT_MBUS || \
                             TYPE == PKT_STACK || \
                             )


/**
 * @brief  SPIRIT version type enumeration
 */

typedef enum
{
  SPIRIT_VERSION_2_1 = 0x01,    /* Deprecated */
  SPIRIT_VERSION_3_0,           /* The only version of SPIRIT1 */
} SpiritVersion;


/**
 * @}
 */


/**
 * @defgroup General_Exported_Constants         General Exported Constants
 * @{
 */


/**
 * @}
 */


/**
 * @defgroup General_Exported_Macros            General Exported Macros
 * @{
 */
#define SpiritGeneralLibraryVersion() "Spirit1_Libraries_v.3.2.0"


/**
 * @}
 */


/**
 * @defgroup General_Exported_Functions         General Exported Functions
 * @{
 */


void SpiritGeneralBatteryLevel(SpiritFunctionalState xNewState);
void SpiritGeneralSetBatteryLevel(BatteryLevel xBatteryLevel);
BatteryLevel SpiritGeneralGetBatteryLevel(void);
void SpiritGeneralBrownOut(SpiritFunctionalState xNewState);
void SpiritGeneralHighPwr(SpiritFunctionalState xNewState);
void SpiritGeneralSetExtRef(ModeExtRef xExtMode);
ModeExtRef SpiritGeneralGetExtRef(void);
void SpiritGeneralSetXoGm(GmConf xGm);
GmConf SpiritGeneralGetXoGm(void);
PacketType SpiritGeneralGetPktType(void);
uint16_t SpiritGeneralGetDevicePartNumber(void);
uint8_t SpiritGeneralGetSpiritVersion(void);

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
