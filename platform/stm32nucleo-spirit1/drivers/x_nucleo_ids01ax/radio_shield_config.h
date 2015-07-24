/**
******************************************************************************
* @file    radio_shield_config.h
* @author  System Lab - NOIDA
* @version V1.0.0
* @date    15-May-2014
* @brief   This file contains definitions for:
*          - LEDs and push-button available on RF shields
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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
*       without specific prior written permission.
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
#ifndef __RADIO_SHIELD_CONFIG_H
#define __RADIO_SHIELD_CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup X-NUCLEO-IDS02Ax
  * @{
  */

/** @addtogroup RADIO_SHILED_LOW_LEVEL
  * @{
  */   
   
/* Exported types ------------------------------------------------------------*/
typedef enum 
{
  RADIO_SHIELD_LED = 0
} Led_t;
 


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
 /** @addtogroup RF_SHIELD_CONFIG_LOW_LEVEL_LED
 * @{
 */
#define RADIO_SHIELD_LEDn                               ((uint8_t)1)

#define RADIO_SHIELD_LED_GPIO_PIN                      GPIO_PIN_4                         /*Rx Indicator LED*/
#define RADIO_SHIELD_LED_GPIO_PORT                     GPIOB
#define RADIO_SHIELD_LED_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()  
#define RADIO_SHIELD_LED_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()


/* Exported Variables ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void RadioShieldLedInit(Led_t Led);
void RadioShieldLedOn(Led_t Led);
void RadioShieldLedOff(Led_t Led);
void RadioShieldLedToggle(Led_t Led);                 
                


#ifdef __cplusplus
}
#endif

#endif /* __RADIO_SHIELD_CONFIG_H */
/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */ 
    
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

