/**
 * @file    SPIRIT1_Util.h
 * @author  High End Analog & RF BU - AMS / ART Team IMS-Systems Lab
 * @version V3.0.1
 * @date    November 19, 2012
 * @brief   Identification functions for SPIRIT DK.
 * @details
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOURCE CODE IS PROTECTED BY A LICENSE.
 * FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
 * IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
 *
 * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIRIT1_UTIL_H
#define __SPIRIT1_UTIL_H


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Config.h"
#include "radio_gpio.h"

#ifdef __cplusplus
  "C" {
#endif


/**
 * @addtogroup ST_SPIRIT1
 * @{
 */
    
typedef struct 
{
  uint16_t nSpiritVersion;
  SpiritVersion xSpiritVersion;
}SpiritVersionMap;
   
#define CUT_MAX_NO 3
#define CUT_2_1v3 0x0103
#define CUT_2_1v4 0x0104
#define CUT_3_0   0x0130

/**
 * @brief  Range extender type
 */
typedef enum
{
  RANGE_EXT_NONE = 0x00,
  RANGE_EXT_SKYWORKS_169,
  RANGE_EXT_SKYWORKS_868
} RangeExtType;

/**
 * @addgroup SPIRIT1_Util_FUNCTIONS
 * @{
 */
void SpiritManagementIdentificationRFBoard(void);

RangeExtType SpiritManagementGetRangeExtender(void);
void SpiritManagementSetRangeExtender(RangeExtType xRangeType);
void SpiritManagementRangeExtInit(void);
void SpiritManagementSetBand(uint8_t value);
uint8_t SpiritManagementGetBand(void);

uint8_t SdkEvalGetHasEeprom(void);

void Spirit1InterfaceInit(void);
void Spirit1GpioIrqInit(SGpioInit *pGpioIRQ);
void Spirit1RadioInit(SRadioInit *pRadioInit);
void Spirit1SetPower(uint8_t cIndex, float fPowerdBm);
void Spirit1PacketConfig(void);
void Spirit1SetPayloadlength(uint8_t length);
void Spirit1SetDestinationAddress(uint8_t address);
void Spirit1EnableTxIrq(void);
void Spirit1EnableRxIrq(void);
void Spirit1DisableIrq(void);
void Spirit1SetRxTimeout(float cRxTimeOut);
void Spirit1EnableSQI(void);
void Spirit1SetRssiTH(int dbmValue);
float Spirit1GetRssiTH(void);
void Spirit1ClearIRQ(void);
void Spirit1StartRx(void);
void Spirit1GetRxPacket(uint8_t *buffer, uint8_t size );
void Spirit1StartTx(uint8_t *buffer, uint8_t size);

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


 /******************* (C) COPYRIGHT 2012 STMicroelectronics *****END OF FILE****/

