/**
  ******************************************************************************
  * @file    stm32l1xx_nucleo.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    5-September-2014
  * @brief   This file contains definitions for:
  *          - LEDs and push-button available on STM32L1XX-Nucleo Kit 
  *            from STMicroelectronics
  *          - LCD, joystick and microSD available on Adafruit 1.8" TFT LCD 
  *            shield (reference ID 802)
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

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32L1XX_NUCLEO
  * @{
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L1XX_NUCLEO_H
#define __STM32L1XX_NUCLEO_H

#ifdef __cplusplus
 extern "C" {
#endif
                                              
/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
   
   
/** @defgroup STM32L1XX_NUCLEO_Exported_Types Exported Types
  * @{
  */

typedef enum 
{  
  BUTTON_USER = 0,
} Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef; 

typedef enum 
{ 
  JOY_NONE = 0,
  JOY_SEL = 1,
  JOY_DOWN = 2,
  JOY_LEFT = 3,
  JOY_RIGHT = 4,
  JOY_UP = 5
} JOYState_TypeDef;

/**
  * @}
  */ 

/** @defgroup STM32L1XX_NUCLEO_Exported_Constants Exported Constants
  * @{
  */ 

/** 
  * @brief  Define for STM32L1xx_NUCLEO board  
  */ 
#if !defined (USE_STM32L1xx_NUCLEO)
 #define USE_STM32L1xx_NUCLEO
#endif
  
/** @defgroup STM32L1XX_NUCLEO_LED LED Constants
  * @{
  */

typedef enum
{
  LED1 = 0,
  LED2 = 1,
  LED3 = 2,
  LED4 = 3,
  LED5 = 4
} Led_TypeDef;



#define LEDn                             5

#define LED2_PIN                         GPIO_PIN_5
#define LED2_GPIO_PORT                   GPIOA
#define LED2_GPIO_CLK_ENABLE()           __GPIOA_CLK_ENABLE()  
#define LED2_GPIO_CLK_DISABLE()          __GPIOA_CLK_DISABLE()  


#define LED1_PIN                         GPIO_PIN_4
#define LED1_GPIO_PORT                   GPIOB
#define LED1_GPIO_CLK                    RCC_AHBPeriph_GPIOB

#define LED3_PIN                         GPIO_PIN_14
#define LED3_GPIO_PORT                   GPIOB
#define LED3_GPIO_CLK                    RCC_AHBPeriph_GPIOB

#define LED4_PIN                         GPIO_PIN_14
#define LED4_GPIO_PORT                   GPIOB
#define LED4_GPIO_CLK                    RCC_AHBPeriph_GPIOB

#define LED5_PIN                         GPIO_PIN_14
#define LED5_GPIO_PORT                   GPIOB
#define LED5_GPIO_CLK                    RCC_AHBPeriph_GPIOB

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)  (LED2_GPIO_CLK_ENABLE())

#define LEDx_GPIO_CLK_DISABLE(__INDEX__) (LED2_GPIO_CLK_DISABLE())

/**
  * @}
  */ 

/** @defgroup STM32L1XX_NUCLEO_BUTTON BUTTON Constants
  * @{
  */  
#define BUTTONn                          1  

/**
  * @brief Key push-button
 */
#define USER_BUTTON_PIN                  GPIO_PIN_13
#define USER_BUTTON_GPIO_PORT            GPIOC
#define USER_BUTTON_GPIO_CLK_ENABLE()    __GPIOC_CLK_ENABLE()
#define USER_BUTTON_GPIO_CLK_DISABLE()   __GPIOC_CLK_DISABLE()
#define USER_BUTTON_EXTI_IRQn            EXTI15_10_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__)    (USER_BUTTON_GPIO_CLK_ENABLE())

#define BUTTONx_GPIO_CLK_DISABLE(__INDEX__)    (USER_BUTTON_GPIO_CLK_DISABLE())
/**
  * @}
  */
    
/** @addtogroup STM32L1XX_NUCLEO_BUS BUS Constants
  * @{
  */
/*###################### SPI1 ###################################*/
#define NUCLEO_SPIx                                 SPI1
#define NUCLEO_SPIx_CLK_ENABLE()                    __SPI1_CLK_ENABLE()

#define NUCLEO_SPIx_SCK_AF                          GPIO_AF5_SPI1
#define NUCLEO_SPIx_SCK_GPIO_PORT                   GPIOA
#define NUCLEO_SPIx_SCK_PIN                         GPIO_PIN_5
#define NUCLEO_SPIx_SCK_GPIO_CLK_ENABLE()           __GPIOA_CLK_ENABLE()
#define NUCLEO_SPIx_SCK_GPIO_CLK_DISABLE()          __GPIOA_CLK_DISABLE()

#define NUCLEO_SPIx_MISO_MOSI_AF                    GPIO_AF5_SPI1
#define NUCLEO_SPIx_MISO_MOSI_GPIO_PORT             GPIOA
#define NUCLEO_SPIx_MISO_MOSI_GPIO_CLK_ENABLE()     __GPIOA_CLK_ENABLE()
#define NUCLEO_SPIx_MISO_MOSI_GPIO_CLK_DISABLE()    __GPIOA_CLK_DISABLE()
#define NUCLEO_SPIx_MISO_PIN                        GPIO_PIN_6
#define NUCLEO_SPIx_MOSI_PIN                        GPIO_PIN_7
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define NUCLEO_SPIx_TIMEOUT_MAX                   1000

#define NUCLEO_I2C_SHIELDS_EV_IRQn                    I2C1_EV_IRQn
/**
  * @brief  SD Control Lines management
  */  
#define SD_CS_LOW()       HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_CS_HIGH()      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_SET)
    
/**
  * @brief  LCD Control Lines management
  */
#define LCD_CS_LOW()      HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define LCD_CS_HIGH()     HAL_GPIO_WritePin(LCD_CS_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define LCD_DC_LOW()      HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define LCD_DC_HIGH()     HAL_GPIO_WritePin(LCD_DC_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET)

/**
  * @brief  SD Control Interface pins
  */
#define SD_CS_PIN                                 GPIO_PIN_5
#define SD_CS_GPIO_PORT                           GPIOB
#define SD_CS_GPIO_CLK_ENABLE()                   __GPIOB_CLK_ENABLE()
#define SD_CS_GPIO_CLK_DISABLE()                  __GPIOB_CLK_DISABLE()

/**
  * @brief  LCD Control Interface pins
  */
#define LCD_CS_PIN                                 GPIO_PIN_6
#define LCD_CS_GPIO_PORT                           GPIOB
#define LCD_CS_GPIO_CLK_ENABLE()                   __GPIOB_CLK_ENABLE()
#define LCD_CS_GPIO_CLK_DISABLE()                  __GPIOB_CLK_DISABLE()

/**
  * @brief  LCD Data/Command Interface pins
  */
#define LCD_DC_PIN                                 GPIO_PIN_9
#define LCD_DC_GPIO_PORT                           GPIOA
#define LCD_DC_GPIO_CLK_ENABLE()                   __GPIOA_CLK_ENABLE()
#define LCD_DC_GPIO_CLK_DISABLE()                  __GPIOA_CLK_DISABLE()
     
/*##################### ADC1 ###################################*/
/**
  * @brief  ADC Interface pins
  *         used to detect motion of Joystick available on Adafruit 1.8" TFT shield
  */
#define NUCLEO_ADCx                                 ADC1
#define NUCLEO_ADCx_CLK_ENABLE()                    __ADC1_CLK_ENABLE()
    
#define NUCLEO_ADCx_GPIO_PORT                       GPIOB
#define NUCLEO_ADCx_GPIO_PIN                        GPIO_PIN_0
#define NUCLEO_ADCx_GPIO_CLK_ENABLE()               __GPIOB_CLK_ENABLE()
#define NUCLEO_ADCx_GPIO_CLK_DISABLE()              __GPIOB_CLK_DISABLE()
    

/** @defgroup STM32L1XX_NUCLEO_Exported_Functions
  * @{
  */
uint32_t        BSP_GetVersion(void);
/** @addtogroup STM32L1XX_NUCLEO_LED_Functions
  * @{
  */ 

void            BSP_LED_Init(Led_TypeDef Led);
void            BSP_LED_On(Led_TypeDef Led);
void            BSP_LED_Off(Led_TypeDef Led);
void            BSP_LED_Toggle(Led_TypeDef Led);

/**
  * @}
  */

/** @addtogroup STM32L1XX_NUCLEO_BUTTON_Functions
  * @{
  */

void            BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
uint32_t        BSP_PB_GetState(Button_TypeDef Button);

#ifdef HAL_ADC_MODULE_ENABLED
uint8_t          BSP_JOY_Init(void);
JOYState_TypeDef BSP_JOY_GetState(void);
#endif /* HAL_ADC_MODULE_ENABLED */


/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L1XX_NUCLEO_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
