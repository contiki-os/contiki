/**
******************************************************************************
* @file    radio_shield_config.c
* @author  System Lab - NOIDA
* @version V1.0.0
* @date    15-May-2014
* @brief   This file provides set of firmware functions to manage:
*          - LEDs and push-button available on radio Shield
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

/* Includes ------------------------------------------------------------------*/
#include "radio_shield_config.h"

/** @addtogroup BSP
* @{
*/ 

/** @addtogroup RADIO_SHILED
* @{
*/   

  
/** @addtogroup RADIO_SHILED_LOW_LEVEL 
* @brief This file provides set of firmware functions to manage
* manage Leds and push-button available on Radio shield.     
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_TypeDef*  aLED_GPIO_PORT[RADIO_SHIELD_LEDn] = {RADIO_SHIELD_LED_GPIO_PORT};
const uint16_t aLED_GPIO_PIN[RADIO_SHIELD_LEDn] = {RADIO_SHIELD_LED_GPIO_PIN};

/* Private function prototypes -----------------------------------------------*/
void RadioShieldLedOn(Led_t Led);
void Spirit1_LED_Toggle(Led_t Led);
void Spirit1_LED_Off(Led_t Led);
void RadioShieldLedInit(Led_t Led);
void RadioShieldLedOff(Led_t Led);
void RadioShieldLedToggle(Led_t Led);

/* Private functions ---------------------------------------------------------*/


/**
* @brief  Configures LED GPIO.
* @param  Led: LED to be configured. 
*   This parameter can be one of the following values:
* @arg    Led_t Led
* @retval None
*/
void RadioShieldLedInit(Led_t Led)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* Enable the GPIO_LED Clock */
  __GPIOB_CLK_ENABLE();

  /* Configure the GPIO_LED pin */
  GPIO_InitStruct.Pin = aLED_GPIO_PIN[Led];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
  
  HAL_GPIO_Init(aLED_GPIO_PORT[Led], &GPIO_InitStruct);
}


/**
* @brief  Turns selected LED On.
* @param  Led: Specifies the Led to be set on. 
*   This parameter can be one of following parameters:
* @arg  Led_t Led
* @retval None
*/
void RadioShieldLedOn(Led_t Led)
{
    HAL_GPIO_WritePin(aLED_GPIO_PORT[Led], aLED_GPIO_PIN[Led], GPIO_PIN_SET); 
}


/**
* @brief  Turns selected LED Off. 
* @param  Led: Specifies the Led to be set off. 
*   This parameter can be one of following parameters:
* @arg Led_t Led
* @retval None
*/
void RadioShieldLedOff(Led_t Led)
{
    HAL_GPIO_WritePin(aLED_GPIO_PORT[Led], aLED_GPIO_PIN[Led], GPIO_PIN_RESET); 
}


/**
* @brief  Toggles the selected LED.
* @param  Led: Specifies the Led to be toggled. 
*   This parameter can be one of following parameters:
* @arg Led_t Led
* @retval None
*/
void RadioShieldLedToggle(Led_t Led)
{
    HAL_GPIO_TogglePin(aLED_GPIO_PORT[Led], aLED_GPIO_PIN[Led]);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
