/**
******************************************************************************
* @file    radio_gpio.h
* @author  System Lab - NOIDA
* @version V1.0.0
* @date    15-May-2014
* @brief   This file contains all the functions prototypes for the gpio  
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
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported Variables ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIO_GPIO_H
#define __RADIO_GPIO_H
#ifdef __cplusplus
extern "C" {
#endif
  
/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "SPIRIT_Types.h"
  
/**
 * @addtogroup BSP
 * @{
 */


/* Exported types ------------------------------------------------------------*/
  /* MCU GPIO pin working mode for GPIO */
typedef enum                                                                                          
{
    RADIO_MODE_GPIO_IN  = 0x00,   /*!< Work as GPIO input */
    RADIO_MODE_EXTI_IN,           /*!< Work as EXTI */
    RADIO_MODE_GPIO_OUT,          /*!< Work as GPIO output */
}RadioGpioMode;  

 /* MCU GPIO pin enumeration for GPIO */
typedef enum 
{
  RADIO_GPIO_0     = 0x00, /*!< GPIO_0 selected */
  RADIO_GPIO_1     = 0x01, /*!< GPIO_1 selected */
  RADIO_GPIO_2     = 0x02, /*!< GPIO_2 selected */
  RADIO_GPIO_3     = 0x03, /*!< GPIO_3 selected */
  RADIO_GPIO_SDN   = 0x04, /*!< GPIO_SDN selected */
} 
RadioGpioPin;   

  
/* Exported constants --------------------------------------------------------*/
  
  
/* Exported macro ------------------------------------------------------------*/
 /* MCU GPIO pin working mode for GPIO */
#define IS_RADIO_GPIO_MODE(MODE) (((MODE) == RADIO_MODE_GPIO_IN) || \
                               ((MODE) == RADIO_MODE_EXTI_IN) || \
                               ((MODE) == RADIO_MODE_GPIO_OUT))

/* Number of Arduino pins used for RADIO GPIO interface */
#define RADIO_GPIO_NUMBER    ((uint8_t)5)

/* MCU GPIO pin enumeration for GPIO */
#define IS_RADIO_GPIO_PIN(PIN)   (((PIN) == RADIO_GPIO_0) || \
                               ((PIN) == RADIO_GPIO_1) || \
                               ((PIN) == RADIO_GPIO_2) || \
                               ((PIN) == RADIO_GPIO_3) || \
                               ((PIN) == RADIO_GPIO_SDN))

/* Define for RADIO board */
#if !defined (USE_SPIRIT1_DEFAULT)
 #define USE_SPIRIT1_DEFAULT
#endif  

/* @defgroup Radio_Gpio_config_Define */
/*NOTE: GPIO0, GPIO1, GPIO2 of SPIRIT1 is not used in the shield*/

#define RADIO_GPIO_0_PORT                          GPIOC
#define RADIO_GPIO_0_PIN                           GPIO_PIN_1
#define RADIO_GPIO_0_CLOCK_ENABLE()                __GPIOC_CLK_ENABLE()
#define RADIO_GPIO_0_CLOCK_DISABLE()               __GPIOC_CLK_ENABLE()   
#define RADIO_GPIO_0_SPEED                         GPIO_SPEED_HIGH
#define RADIO_GPIO_0_PUPD                          GPIO_NOPULL
#define RADIO_GPIO_0_EXTI_LINE                     GPIO_PIN_1
#define RADIO_GPIO_0_EXTI_MODE                     GPIO_MODE_IT_FALLING
#define RADIO_GPIO_0_EXTI_IRQN                     EXTI1_IRQn 
#define RADIO_GPIO_0_EXTI_PREEMPTION_PRIORITY      2
#define RADIO_GPIO_0_EXTI_SUB_PRIORITY             2
#define RADIO_GPIO_0_EXTI_IRQ_HANDLER              EXTI1_IRQHandler

#define RADIO_GPIO_1_PORT                          GPIOB
#define RADIO_GPIO_1_PIN                           GPIO_PIN_0
#define RADIO_GPIO_1_CLOCK_ENABLE()                __GPIOB_CLK_ENABLE()
#define RADIO_GPIO_1_CLOCK_DISABLE()               __GPIOB_CLK_ENABLE()   
#define RADIO_GPIO_1_SPEED                         GPIO_SPEED_HIGH
#define RADIO_GPIO_1_PUPD                          GPIO_NOPULL
#define RADIO_GPIO_1_EXTI_LINE                     GPIO_PIN_0
#define RADIO_GPIO_1_EXTI_MODE                     GPIO_MODE_IT_FALLING
#define RADIO_GPIO_1_EXTI_IRQN                     EXTI0_IRQn 
#define RADIO_GPIO_1_EXTI_PREEMPTION_PRIORITY      2
#define RADIO_GPIO_1_EXTI_SUB_PRIORITY             2
#define RADIO_GPIO_1_EXTI_IRQ_HANDLER              EXTI0_IRQHandler

#define RADIO_GPIO_2_PORT                          GPIOA
#define RADIO_GPIO_2_PIN                           GPIO_PIN_4
#define RADIO_GPIO_2_CLOCK_ENABLE()                __GPIOA_CLK_ENABLE()
#define RADIO_GPIO_2_CLOCK_DISABLE()               __GPIOA_CLK_ENABLE()   
#define RADIO_GPIO_2_SPEED                         GPIO_SPEED_HIGH
#define RADIO_GPIO_2_PUPD                          GPIO_NOPULL
#define RADIO_GPIO_2_EXTI_LINE                     GPIO_PIN_4
#define RADIO_GPIO_2_EXTI_MODE                     GPIO_MODE_IT_FALLING
#define RADIO_GPIO_2_EXTI_IRQN                     EXTI4_IRQn 
#define RADIO_GPIO_2_EXTI_PREEMPTION_PRIORITY      2
#define RADIO_GPIO_2_EXTI_SUB_PRIORITY             2
#define RADIO_GPIO_2_EXTI_IRQ_HANDLER              EXTI4_IRQHandler


#if defined (USE_SPIRIT1_DEFAULT)


#define RADIO_GPIO_3_PORT                          GPIOC
#define RADIO_GPIO_3_PIN                           GPIO_PIN_7
#define RADIO_GPIO_3_CLOCK_ENABLE()              __GPIOC_CLK_ENABLE()
#define RADIO_GPIO_3_CLOCK_DISABLE()             __GPIOC_CLK_DISABLE()   
#define RADIO_GPIO_3_SPEED                         GPIO_SPEED_HIGH
#define RADIO_GPIO_3_PUPD                          GPIO_NOPULL
#define RADIO_GPIO_3_EXTI_LINE                     GPIO_PIN_7
#define RADIO_GPIO_3_EXTI_MODE                     GPIO_MODE_IT_FALLING
#define RADIO_GPIO_3_EXTI_IRQN                     EXTI9_5_IRQn 
#define RADIO_GPIO_3_EXTI_PREEMPTION_PRIORITY      2
#define RADIO_GPIO_3_EXTI_SUB_PRIORITY             2
#define RADIO_GPIO_3_EXTI_IRQ_HANDLER              EXTI9_5_IRQHandler

#else

#define RADIO_GPIO_3_PORT                        GPIOA
#define RADIO_GPIO_3_PIN                         GPIO_PIN_0
#define RADIO_GPIO_3_CLOCK_ENABLE()                __GPIOA_CLK_ENABLE()
#define RADIO_GPIO_3_CLOCK_DISABLE()               __GPIOA_CLK_DISABLE() 
#define RADIO_GPIO_3_SPEED                       GPIO_SPEED_HIGH
#define RADIO_GPIO_3_PUPD                        GPIO_NOPULL
#define RADIO_GPIO_3_EXTI_LINE                   GPIO_PIN_0
#define RADIO_GPIO_3_EXTI_MODE                   GPIO_MODE_IT_FALLING
#define RADIO_GPIO_3_EXTI_IRQN                   EXTI0_IRQn
#define RADIO_GPIO_3_EXTI_PREEMPTION_PRIORITY    2
#define RADIO_GPIO_3_EXTI_SUB_PRIORITY           2
#define RADIO_GPIO_3_EXTI_IRQ_HANDLER            EXTI0_IRQHandler

#endif

#define RADIO_GPIO_SDN_PORT                        GPIOA
#define RADIO_GPIO_SDN_PIN                         GPIO_PIN_10
#define RADIO_GPIO_SDN_CLOCK_ENABLE()            __GPIOA_CLK_ENABLE()
#define RADIO_GPIO_SDN_CLOCK_DISABLE()           __GPIOA_CLK_DISABLE()
#define RADIO_GPIO_SDN_SPEED                       GPIO_SPEED_HIGH
#define RADIO_GPIO_SDN_PUPD                        GPIO_PULLUP


#define RADIO_GPIO_IRQ		RADIO_GPIO_3
#define SPIRIT_GPIO_IRQ         SPIRIT_GPIO_3

/* Exported Variables ------------------------------------------------------------*/
  
  
/* Exported functions ------------------------------------------------------- */
FlagStatus RadioGpioGetLevel(RadioGpioPin xGpio);
void RadioGpioSetLevel(RadioGpioPin xGpio, GPIO_PinState xState);
void SdkEvalEnterShutdown(void);
void SdkEvalExitShutdown(void);
SpiritFlagStatus SdkEvalCheckShutdown(void);
void RadioGpioInit(RadioGpioPin xGpio, RadioGpioMode xGpioMode);
void RadioGpioInterruptCmd(RadioGpioPin xGpio, uint8_t nPreemption, uint8_t nSubpriority, FunctionalState xNewState);


#ifdef __cplusplus
}
#endif
#endif /*__RADIO_GPIO_H */

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
